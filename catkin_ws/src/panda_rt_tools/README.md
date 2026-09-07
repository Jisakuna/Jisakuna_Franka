# panda_rt_tools

**English** | [简体中文](README.zh-CN.md)

Real-time control utilities for the **Franka Emika Panda** using `libfranka 0.9.2`.

> These hardware, network, user, and system settings describe the original development host. They are environment records, not an installation script or results revalidated for this release. For a fresh setup, see the [repository README](../../../README.md). Joint indices are 0–6.
>
> `rt_loop_test` does not currently terminate automatically as intended, and zero additional torque does not guarantee a fixed pose. Read [Real-time configuration and current limitations](../../../README.md#real-time-configuration-and-current-limitations) before using motion, recovery, or diagnostic tools.

## 1. Hardware

The original environment notes record development and validation on this control host:

| Component | Model |
| --- | --- |
| CPU | Intel Core i9-10900X @ 3.70 GHz (10 cores / 20 threads) |
| Motherboard | Gigabyte X299-WU8-CF |
| Memory | 64 GB DDR4 |
| System drive | WD Blue SN550 1 TB (NVMe) |
| GPU | 2 × NVIDIA GeForce RTX 2080 Ti (TU102, 11 GB) |
| FCI NIC | Intel I210 Gigabit (PCIe, `enp4s0`) |
| Onboard NIC | Intel I219-LM (not used for FCI) |
| Wireless | USB adapter `wlx90de806bb0f6` (general network access, separate from FCI) |

FCI traffic uses the dedicated Intel I210 interface `enp4s0`, physically separate from the wireless and onboard interfaces to reduce interference with real-time communication.

## 2. Software environment

| Component | Recorded version / configuration |
| --- | --- |
| Operating system | Ubuntu 20.04.6 LTS (Focal Fossa) |
| Kernel | `5.15.92-rt57` (PREEMPT_RT; `/sys/kernel/realtime` = 1) |
| ROS | Noetic (`/opt/ros/noetic`) |
| libfranka | **0.9.2**; confirm compatibility with the actual Panda firmware |
| franka_ros | 0.10.1 |
| NVIDIA driver | 525.85.05, built for the RT kernel |
| Compiler | GCC / G++ 9.4.0; CMake ≥ 3.16 |
| Dependencies | Eigen3, Poco, Threads |

### 2.1 Real-time kernel

```bash
uname -a                 # Recorded kernel: 5.15.92-rt57, PREEMPT_RT
cat /sys/kernel/realtime # Expected: 1
```

- Built from `linux-5.15.92` with `patch-5.15.92-rt57` and `CONFIG_PREEMPT_RT=y`.
- Installed through the `linux-image-5.15.92-rt57` and `linux-headers-5.15.92-rt57` Debian packages.

### 2.2 Real-time permissions

On the original host, user `jia` belonged to the `realtime` group and `/etc/security/limits.conf` contained:

```text
@realtime soft rtprio 99
@realtime soft priority 99
@realtime soft memlock 102400
@realtime hard rtprio 99
@realtime hard priority 99
@realtime hard memlock 102400
```

`ulimit -r` should report `99` in that configuration. If group or limit changes are not active in a session, log out and sign in again.

### 2.3 FCI network (NetworkManager connection `robot-link`)

| Setting | Recorded value |
| --- | --- |
| Interface | `enp4s0` |
| Host IP | `192.168.1.1/24` (static) |
| Robot IP | `192.168.1.2` |
| MTU | 1500 |
| IPv6 | Disabled / ignore |
| TX queue length | 10000 (udev `70-fci-net.rules`) |

### 2.4 Persistent real-time tuning on the original host

| Setting | Configuration / recorded result |
| --- | --- |
| CPU governor: `performance` | systemd `cpu-performance.service` |
| irqbalance disabled | `systemctl disable irqbalance` |
| NIC interrupt coalescing disabled (`rx/tx-usecs=0`) | udev `70-fci-net.rules` |
| FCI interrupts assigned to CPUs 5–8 | udev `71-fci-irqaffinity.rules` |
| Scheduling measurement | cyclictest at priority 99: recorded maximum 8 µs |

These host service and udev file names are references to the original setup; this package does not install them.

## 3. Prerequisites

- Ubuntu 20.04 with ROS Noetic.
- PREEMPT_RT kernel; the recorded host used `5.15.92-rt57`.
- libfranka 0.9.2, discoverable through CMake `find_package(Franka)`.
- FCI enabled through Desk, with the actual robot IP configured.
- Active real-time scheduling and memory-lock permissions for the control user.

## 4. Build

Follow the [root build instructions](../../../README.md#build-the-c-tools) to initialize a fresh workspace and install libfranka. From the repository root:

```bash
source /opt/ros/noetic/setup.bash
cd catkin_ws
catkin_make -DCMAKE_BUILD_TYPE=Release --only-pkg-with-deps panda_rt_tools
source devel/setup.bash
```

The original host sourced its workspace through `.bashrc`. On another machine, source ROS and the workspace explicitly in each new terminal unless you have configured that behavior yourself.

## 5. Tools

| Tool | Purpose |
| --- | --- |
| `read_state` | Read joint positions, robot mode, and communication success rate |
| `slow_move` | Move one joint by a given relative angle |
| `reset_home` | Move all joints to the home pose defined in the source |
| `recover` | Invoke automatic error recovery; changes robot state |
| `rt_loop_test` | Run a zero additional torque loop to collect timing statistics; review the known limitations first |

## 6. Usage

```bash
# Read state without commanding motion.
rosrun panda_rt_tools read_state 192.168.1.2

# Joint index 2 (third joint): +10 degrees, speed parameter 5 degrees/s.
rosrun panda_rt_tools slow_move 192.168.1.2 2 10 5

# Move to the home pose with a speed parameter of 8 degrees/s.
rosrun panda_rt_tools reset_home 192.168.1.2 8

# Automatic recovery: address the fault cause first.
rosrun panda_rt_tools recover 192.168.1.2

# Fix and validate termination behavior before using this diagnostic.
# Zero additional torque does not guarantee a fixed pose.
# rosrun panda_rt_tools rt_loop_test 192.168.1.2
```

Launch files pass their arguments to the executables:

```bash
roslaunch panda_rt_tools slow_move.launch robot_ip:=192.168.1.2 joint:=0 angle_deg:=5 speed_deg_s:=4
roslaunch panda_rt_tools reset_home.launch robot_ip:=192.168.1.2 speed_deg_s:=8
roslaunch panda_rt_tools recover.launch robot_ip:=192.168.1.2
roslaunch panda_rt_tools read_state.launch robot_ip:=192.168.1.2
```

## 7. Operational notes

- Keep the emergency stop within reach and start with small angles and low speeds.
- Read the [known limitations](../../../README.md#real-time-configuration-and-current-limitations) before commanding the robot.
- If a shared library cannot be found at runtime, check the linker configuration for the actual installation path. The original host used `/opt/ros/noetic/lib/x86_64-linux-gnu` in `LD_LIBRARY_PATH`.
- The original environment notes advise against running `modprobe nvidia` during an active graphical session because it may freeze the display; reboot to load the driver instead.

## 8. Real-time behavior

The tools attempt to set `SCHED_FIFO` priority 90 and lock memory with `mlockall`. The configured `rtprio` and `memlock` limits must be active for these calls to succeed. The current motion tools do not immediately abort when this setup fails; validate the configuration before operating the robot.

## License

Apache License 2.0. See the repository [LICENSE](../../../LICENSE) and [NOTICE](../../../NOTICE).
