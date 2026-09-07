# Jisakuna_Franka

[English](README.md) | **简体中文**

面向 **Franka Emika Panda** 的 ROS Noetic 工作空间、实时控制工具与安装依赖归档。

本仓库保存 `panda_rt_tools` 的 C++ 源码、launch 文件和原控制主机的环境记录；`franka_ros` 以固定提交的 Git 子模块管理，依赖压缩包通过仓库 Release 提供。适用于复现原 Panda / libfranka 0.9.2 开发环境。

## 内容与版本

项目首个发布版本为 **v1.0.0**；各 ROS 包与依赖保留其自身版本号。

| 内容 | 版本 / 说明 |
| --- | --- |
| `panda_rt_tools` | 0.1.0，自定义状态读取、关节运动、回零和实时诊断工具 |
| `franka_ros` | 包版本 0.10.1，固定提交 `30e598aa6fb703cc80a203481e6427f397337b4c` |
| `libfranka` | 0.9.2，原始源码 ZIP 归档 |
| `panda-python` | 0.8.1 + libfranka 0.9.2，Linux x86_64 Python wheel 归档 |

```text
Jisakuna_Franka/
├── README.md                     # English (default)
├── README.zh-CN.md               # 简体中文
├── LICENSE                       # Apache License 2.0
├── NOTICE
├── THIRD_PARTY_NOTICES.md
├── SHA256SUMS                    # Release 压缩包校验值
└── catkin_ws/
    ├── .catkin_workspace
    └── src/
        ├── franka_ros/            # 固定版本的上游 Git 子模块
        └── panda_rt_tools/
            ├── include/
            ├── src/
            ├── launch/
            ├── CMakeLists.txt
            └── package.xml
```

`build/`、`devel/`、`install/` 等本机产物不纳入 Git，克隆后需要重新构建。GitHub 自动生成的源码 ZIP 不包含子模块源码，请使用下面的递归克隆命令。

## 原开发环境

以下信息来自项目已有的[环境记录](catkin_ws/src/panda_rt_tools/README.zh-CN.md)，不表示本次发布已重新完成硬件验证。

| 组件 | 原记录 |
| --- | --- |
| 系统 | Ubuntu 20.04.6 LTS |
| ROS | ROS 1 Noetic |
| 实时内核 | Linux 5.15.92-rt57 / PREEMPT_RT |
| 编译工具 | GCC / G++ 9.4、CMake ≥ 3.16、C++17 |
| 依赖 | libfranka 0.9.2、Eigen3、Poco、Threads、roscpp |
| 控制主机 | Intel Core i9-10900X、64 GB RAM |
| FCI 网卡 | Intel I210，独立有线链路 |
| 示例网络 | 主机 `192.168.1.1/24`，机器人 `192.168.1.2` |

本项目使用 **ROS 1 catkin**，不能直接在 ROS 2 Jazzy 的 colcon 工作空间中构建。机器人固件与 libfranka 的版本兼容性应按实际设备确认。

## 获取项目和安装归档

此仓库为公开仓库，无需 GitHub 账号认证即可克隆代码和下载 Release 附件。

```bash
git clone --recurse-submodules https://github.com/Jisakuna/Jisakuna_Franka.git
cd Jisakuna_Franka

# 已经普通克隆过时，补全子模块
git submodule update --init --recursive
```

在 [v1.0.0 Release](https://github.com/Jisakuna/Jisakuna_Franka/releases/tag/v1.0.0) 下载以下文件，放在仓库根目录：

| 文件 | 用途 |
| --- | --- |
| `libfranka-0.9.2.zip` | 保存原始 libfranka 源码归档 |
| `panda_py_0.8.1_libfranka_0.9.2.zip` | 保存 CPython 3.7–3.12 的 Linux x86_64 wheels |

```bash
sha256sum -c SHA256SUMS
```

两个压缩包保持原样，不改名、不重新打包。**libfranka ZIP 中的 `common/` 子模块为空，单独解压不能直接构建**；需要构建时使用下方的递归克隆方式。

## 构建 C++ 工具

以下命令用于已安装 ROS Noetic 的 Ubuntu 20.04 控制主机。

### 1. 安装构建依赖

```bash
sudo apt update
sudo apt install build-essential cmake git libeigen3-dev libpoco-dev \
  ros-noetic-roscpp
```

### 2. 构建 libfranka 0.9.2

若已安装可被 CMake 找到的 libfranka 0.9.2，可跳过这一步。避免同时混用不同版本的系统库。

```bash
# 在 Jisakuna_Franka 根目录执行
mkdir -p dependencies
git clone --branch 0.9.2 --recurse-submodules \
  https://github.com/frankarobotics/libfranka.git dependencies/libfranka
cmake -S dependencies/libfranka -B dependencies/libfranka/build \
  -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF -DBUILD_EXAMPLES=OFF
cmake --build dependencies/libfranka/build --parallel 4
sudo cmake --install dependencies/libfranka/build
sudo ldconfig
```

### 3. 构建 panda_rt_tools

```bash
source /opt/ros/noetic/setup.bash
cd catkin_ws/src
catkin_init_workspace
cd ..
catkin_make -DCMAKE_BUILD_TYPE=Release --only-pkg-with-deps panda_rt_tools
source devel/setup.bash
```

上述步骤只构建自定义工具及工作空间内所需依赖。若还需要完整的 `franka_ros` 控制、描述、可视化或仿真包，请先按其[上游 README](catkin_ws/src/franka_ros/README.md)安装全部依赖，然后在 `catkin_ws` 执行 `catkin_make -DCATKIN_WHITELIST_PACKAGES=""`。

新终端需要重新 source ROS 和本工作空间。若 CMake 找不到 Franka，可通过 `-DFranka_DIR=/实际安装路径/lib/cmake/Franka` 指定包含 `FrankaConfig.cmake` 的目录。

## Python wheel（可选）

Python 安装包与 C++ catkin 工具独立。压缩包中的 wheel 分别匹配 CPython 3.7、3.8、3.9、3.10、3.11、3.12，平台为 manylinux x86_64。请在匹配的 Python 虚拟环境中选择一个 wheel，例如：

```bash
unzip panda_py_0.8.1_libfranka_0.9.2.zip -d dependencies/panda_py
# 仅适用于 CPython 3.8 / Linux x86_64；其他版本请替换 wheel 文件名
python -m pip install dependencies/panda_py/panda_python-0.8.1+libfranka.0.9.2-cp38-cp38-manylinux_2_17_x86_64.manylinux2014_x86_64.whl
```

归档不包含全部 Python 依赖，pip 可能仍需联网获取依赖。

## 工具用法

运行前启用机器人 FCI，确认 IP、独立网卡、实时权限与急停状态。将示例 IP 替换成实际地址。

| 可执行程序 | 功能 | 参数 |
| --- | --- | --- |
| `read_state` | 读取关节角、机器人模式与通信成功率 | `[robot_ip]` |
| `slow_move` | 单关节角度增量运动 | `[robot_ip] [joint] [angle_deg] [speed_deg_s]` |
| `reset_home` | 全关节移动到代码定义的 home 姿态 | `[robot_ip] [speed_deg_s]` |
| `recover` | 调用自动错误恢复，会改变机器人状态 | `[robot_ip]` |
| `rt_loop_test` | 零附加力矩控制循环与周期统计 | `[robot_ip]`，使用前见下文限制 |

先读取状态：

```bash
rosrun panda_rt_tools read_state 192.168.1.2
# 等价 launch 入口
roslaunch panda_rt_tools read_state.launch robot_ip:=192.168.1.2
```

确认工作空间安全、实时配置有效后，再进行小幅运动。**关节索引为 0–6，因此索引 1 对应第 2 个关节。**

```bash
# 第 2 个关节移动 +5°，速度参数为 2°/s
rosrun panda_rt_tools slow_move 192.168.1.2 1 5 2

# 回到源码中定义的 home 姿态
rosrun panda_rt_tools reset_home 192.168.1.2 2

# 确认故障原因已排除后执行自动恢复
rosrun panda_rt_tools recover 192.168.1.2
```

对应 launch 文件为 `slow_move.launch`、`reset_home.launch`、`recover.launch`，参数通过 launch 的 `args` 传入可执行程序。

## 实时配置与当前限制

原环境配置详情见[包内 README](catkin_ws/src/panda_rt_tools/README.zh-CN.md)。程序尝试设置 `SCHED_FIFO` 优先级 90，并调用 `mlockall`。控制主机应具备 PREEMPT_RT 内核、适当的 `rtprio` / `memlock` 权限和稳定的 FCI 有线链路。

本仓库保存的是现有开发工具，运动逻辑没有在此次整理中修改。使用前应注意源码中已存在的限制：

- `slow_move` / `reset_home` 按固定 1 ms 累加轨迹时间，没有使用回调的实际周期；速度、数值参数和目标关节限位的校验也不完整。
- 运动工具没有在实时调度设置失败时立即退出，部分控制异常被捕获后仍可能返回成功退出码。不能仅用进程返回码判定动作成功。
- `rt_loop_test` 达到 10000 个样本后只修改局部标志，没有返回 `franka::MotionFinished`，因此不会按预期自动结束；使用前需修复退出逻辑并验证。
- 零附加力矩控制不等于锁定姿态，不应把该诊断程序当作无运动风险的状态读取工具。运动工具也没有路径避障规划。

不要在未验证上述行为的情况下无人值守运行。运行运动或力矩控制时保持急停可用，确保人员和障碍物远离机器人运动范围。

## 发布检查

此次整理执行了文件清单、XML 语法、许可证一致性、压缩包完整性与 SHA-256 校验；没有在当前主机执行 ROS Noetic 编译或机器人硬件测试。原记录中的实时性能数据仅代表原控制主机。

## 许可证

本项目自有源码、配置和文档采用 **Apache License 2.0**，见 [LICENSE](LICENSE) 和 [NOTICE](NOTICE)。`panda_rt_tools/package.xml` 的许可证声明已同步。

第三方项目与归档保留各自版权和许可证，详见 [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md)。
