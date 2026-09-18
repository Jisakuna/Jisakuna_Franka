# panda_rt_tools

[English](README.md) | **简体中文**

Real-time control utilities for the **Franka Emika Panda** using `libfranka 0.9.2`.

## 1. 硬件平台

本包已经在以下控制主机上验证：

| 部件 | 型号 |
|------|------|
| CPU | Intel Core i9-10900X @ 3.70 GHz（10 核 20 线程） |
| 主板 | Gigabyte X299-WU8-CF |
| 内存 | 64 GB DDR4 |
| 系统盘 | WD Blue SN550 1TB (NVMe) |
| GPU | 2 × NVIDIA GeForce RTX 2080 Ti (TU102, 11 GB) |
| FCI 网卡 | Intel I210 Gigabit (PCIe, `enp4s0`) |

> FCI 控制流量走独立网卡 `enp4s0`（Intel I210），与无线/板载网卡物理隔离，避免干扰实时通信。

## 2. 软件运行环境

| 组件 | 版本/配置 |
|------|-----------|
| 操作系统 | Ubuntu 20.04.6 LTS (Focal Fossa) |
| 内核 | `5.15.92-rt57`（PREEMPT_RT，`/sys/kernel/realtime` = 1） |
| ROS | Noetic（`/opt/ros/noetic`） |
| libfranka | **0.9.2**（适配 Panda 固件 ≥ 4.2.1） |
| franka_ros | 0.10.1 |
| NVIDIA 驱动 | 525.85.05（已针对 RT 内核编译） |
| 编译器 | gcc 9.4.0 / g++ 9.4.0, cmake ≥ 3.16 |
| 依赖 | Eigen3, Poco, Threads |

### 2.1 实时内核

```bash
uname -a        # 5.15.92-rt57 #1 SMP PREEMPT_RT
cat /sys/kernel/realtime  # 1
```

- 从源码构建：`linux-5.15.92` + `patch-5.15.92-rt57`，`CONFIG_PREEMPT_RT=y`。
- 构建产物：`linux-image-5.15.92-rt57` / `linux-headers-5.15.92-rt57`

### 2.2 实时权限

`/etc/security/limits.conf` 配置：

```text
@realtime soft rtprio 99
@realtime soft priority 99
@realtime soft memlock 102400
@realtime hard rtprio 99
@realtime hard priority 99
@realtime hard memlock 102400
```

验证：`ulimit -r` 应输出 `99`（若为 0，注销重新登录）。

### 2.3 FCI 网络配置

| 项 | 值 |
|----|-----|
| 接口 | `enp4s0` |
| IP | `192.168.1.1/24`（静态） |
| 机器人 IP | `192.168.1.2`（Desk 默认） |
| MTU | 1500 |
| IPv6 | 关闭 |
| TX 队列 | 10000（udev `70-fci-net.rules`） |

### 2.4 实时优化

| 优化 | 配置位置 |
|------|----------|
| CPU governor = `performance` | systemd `cpu-performance.service` |
| irqbalance 禁用 | `systemctl disable irqbalance` |
| 网卡中断合并禁用（rx/tx-usecs=0） | udev `70-fci-net.rules` |
| FCI 中断固定 CPU 5-8 | udev `71-fci-irqaffinity.rules` |
| 实时调度验证 | cyclictest P:99 → max 8 µs |

## 3. 构建依赖

- Ubuntu 20.04 + ROS Noetic
- PREEMPT_RT 内核（5.15.92-rt57）
- `libfranka`（0.9.x for Panda），可通过 CMake `find_package(Franka)` 找到
- FCI 已启用（Desk → Activate FCI），机器人 IP `192.168.1.2`
- 用户属于 `realtime` 组，rtprio/memlock 限制生效

## 4. 构建

```bash
source /opt/ros/noetic/setup.bash
cd ~/catkin_ws
catkin_make --pkg panda_rt_tools
source devel/setup.bash
```

> `.bashrc` 已自动 source catkin 工作区，新终端无需手动 source。

## 5. 工具

| Tool | Purpose |
|------|---------|
| `read_state` | Read current joint positions, mode, success rate (no motion) |
| `slow_move` | Slowly move one joint by a given angle (trapezoidal velocity) |
| `reset_home` | Move all joints back to the standard Panda home pose |
| `recover` | Clear motion errors / Reflex state (automatic error recovery) |
| `rt_loop_test` | Run a zero-torque RT loop to diagnose scheduling/latency |

## 6. Usage (用法)

```bash
# Read state (no motion)
rosrun panda_rt_tools read_state 192.168.1.2

# Slow move: joint index 2 (third joint) by +10 deg at 5 deg/s
rosrun panda_rt_tools slow_move 192.168.1.2 2 10 5

# Reset to home (max 8 deg/s)
rosrun panda_rt_tools reset_home 192.168.1.2 8

# Automatic recovery: changes robot state; check the fault cause first
rosrun panda_rt_tools recover 192.168.1.2

# RT loop diagnostic: fix termination behavior and validate before use.
# Zero additional torque does not guarantee a fixed pose.
# rosrun panda_rt_tools rt_loop_test 192.168.1.2
```

或通过文件操作:

```bash
roslaunch panda_rt_tools slow_move.launch robot_ip:=192.168.1.2 joint:=0 angle_deg:=5 speed_deg_s:=4
roslaunch panda_rt_tools reset_home.launch robot_ip:=192.168.1.2 speed_deg_s:=8
roslaunch panda_rt_tools recover.launch robot_ip:=192.168.1.2
roslaunch panda_rt_tools read_state.launch robot_ip:=192.168.1.2
```

## 7. 安全

- 运动流程保持急停按钮可随时被按下。
- 推荐以低速度启动。

## 8. 实时说明

这些工具会采用 SCHED_FIFO（先进先出的实时调度策略），实时优先级设置为 90，并通过 mlockall 锁定内存。
要成功执行这些操作，必须让 realtime 用户组的资源限制配置生效（运行 ulimit -r 时，应该显示 99）。
