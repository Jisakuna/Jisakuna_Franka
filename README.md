# Jisakuna_Franka

**English** | [简体中文](README.zh-CN.md)

This repository recreates the original Panda / libfranka 0.9.2 development environment for the **Franka Emika Panda** robot using ROS Noetic. It provides a build workspace, real-time control tools, and support for real-time communication via FCI.

## Contents

| Component | Version / Description |
| --- | --- |
| `panda_rt_tools` | Version 0.1.0; custom tools for state reading, joint motion, returning to the home pose, and real-time diagnostics |
| `franka_ros` | Package version 0.10.1, pinned to commit `30e598aa6fb703cc80a203481e6427f397337b4c` |
| `libfranka` | Version 0.9.2; Franka runtime library |
| `panda-python` | Version 0.8.1 + libfranka 0.9.2 |

```text
Jisakuna_Franka/
├── README.md
├── README.zh-CN.md
├── LICENSE                       # Apache License 2.0
├── NOTICE
├── THIRD_PARTY_NOTICES.md
├── SHA256SUMS                    # Release archive checksums
└── catkin_ws/
    ├── .catkin_workspace
    └── src/
        ├── franka_ros/            # Upstream Git submodule pinned to a specific revision
        └── panda_rt_tools/
            ├── include/
            ├── src/
            ├── launch/
            ├── CMakeLists.txt
            └── package.xml
```

## Development Environment

| Component | Originally Recorded Configuration |
| --- | --- |
| Operating system | Ubuntu 20.04.6 LTS |
| ROS | ROS 1 Noetic |
| Real-time kernel | Linux 5.15.92-rt57 / PREEMPT_RT |
| Build tools | GCC / G++ 9.4, CMake ≥ 3.16, C++17 |
| Dependencies | libfranka 0.9.2, Eigen3, Poco, Threads, roscpp |
| Control host | Intel Core i9-10900X, 64 GB RAM |
| FCI network adapter | Intel I210, dedicated wired link |

This project uses **ROS 1 catkin** and does not support building in a ROS 2 Jazzy colcon workspace. Verify that the robot's firmware version is compatible with the version of libfranka in use.

## Usage Instructions

Download the following files from the [v1.0.0 Release](https://github.com/Jisakuna/Jisakuna_Franka/releases/tag/v1.0.0) and place them in the repository root:

| File | Purpose |
| --- | --- |
| `libfranka-0.9.2.zip` | Archive of the original libfranka source code |
| `panda_py_0.8.1_libfranka_0.9.2.zip` | Archive containing Linux x86_64 wheels for CPython 3.7–3.12 |

```bash
sha256sum -c SHA256SUMS
```

Keep both archives unchanged: do not rename or repackage them. **The `common/` submodule is empty in the libfranka ZIP archive, so the extracted archive cannot be built directly.** To build libfranka, use the recursive clone procedure below.

## Building the C++ Tools

The following commands are intended for an Ubuntu 20.04 control host with ROS Noetic already installed.

### 1. Install Build Dependencies

```bash
sudo apt update
sudo apt install build-essential cmake git libeigen3-dev libpoco-dev \
  ros-noetic-roscpp
```

### 2. Build libfranka 0.9.2

Skip this step if libfranka 0.9.2 is already installed and can be found by CMake. Avoid mixing different versions of the library on the same system.

```bash
# Run from the Jisakuna_Franka repository root
mkdir -p dependencies
git clone --branch 0.9.2 --recurse-submodules \
  https://github.com/frankarobotics/libfranka.git dependencies/libfranka
cmake -S dependencies/libfranka -B dependencies/libfranka/build \
  -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF -DBUILD_EXAMPLES=OFF
cmake --build dependencies/libfranka/build --parallel 4
sudo cmake --install dependencies/libfranka/build
sudo ldconfig
```

### 3. Build panda_rt_tools

```bash
source /opt/ros/noetic/setup.bash
cd catkin_ws/src
catkin_init_workspace
cd ..
catkin_make -DCMAKE_BUILD_TYPE=Release --only-pkg-with-deps panda_rt_tools
source devel/setup.bash
```

These steps build the custom tools and their required dependencies within the workspace.

In each new terminal, source both the ROS environment and this workspace again. If CMake cannot find Franka, use `-DFranka_DIR=/path/to/installation/lib/cmake/Franka` to specify the directory containing `FrankaConfig.cmake`.

## Python Wheels (Optional)

The Python package is independent of the C++ catkin tools. The archive contains separate wheels for CPython 3.7, 3.8, 3.9, 3.10, 3.11, and 3.12 on manylinux x86_64. Select the wheel that matches your Python virtual environment. For example:

```bash
unzip panda_py_0.8.1_libfranka_0.9.2.zip -d dependencies/panda_py
# For CPython 3.8 / Linux x86_64 only; replace the wheel filename for other versions
python -m pip install dependencies/panda_py/panda_python-0.8.1+libfranka.0.9.2-cp38-cp38-manylinux_2_17_x86_64.manylinux2014_x86_64.whl
```

The archive does not include all Python dependencies, so pip may still need an internet connection to download them.

## Tool Usage

Before running the tools, enable FCI on the robot and verify the IP address, dedicated network interface, real-time permissions, and emergency-stop status. Replace the example IP address with the actual robot IP address.

| Executable | Function | Arguments |
| --- | --- | --- |
| `read_state` | Read joint angles, robot mode, and communication success rate | `[robot_ip]` |
| `slow_move` | Move a single joint by an angular increment | `[robot_ip] [joint] [angle_deg] [speed_deg_s]` |
| `reset_home` | Move all joints to the home pose defined in the code | `[robot_ip] [speed_deg_s]` |
| `recover` | Perform automatic error recovery; changes the robot's state | `[robot_ip]` |
| `rt_loop_test` | Run a zero-additional-torque control loop and collect cycle timing statistics | `[robot_ip]`; see the limitations below before use |

Read the robot state:

```bash
rosrun panda_rt_tools read_state 192.168.1.2
# Equivalent launch command
roslaunch panda_rt_tools read_state.launch robot_ip:=192.168.1.2
```

Small-increment motion. **Joint indices range from 0 to 6, so index 1 refers to the second joint.**

```bash
# Move the second joint by +5° with a speed parameter of 2°/s
rosrun panda_rt_tools slow_move 192.168.1.2 1 5 2

# Return to the home pose defined in the source code
rosrun panda_rt_tools reset_home 192.168.1.2 2

# Run automatic recovery only after confirming that the cause of the fault has been resolved
rosrun panda_rt_tools recover 192.168.1.2
```

The corresponding launch files are `slow_move.launch`, `reset_home.launch`, and `recover.launch`. Parameters are passed to the executables via `args` in the launch files.

## Real-Time Configuration and Current Limitations

The programs set `SCHED_FIFO` priority to 90 and call `mlockall`. The control host must have a PREEMPT_RT kernel, appropriate `rtprio` / `memlock` permissions, and a stable wired FCI connection.

## Release Checks

The preparation of this release included checks of the file inventory, XML syntax, license consistency, archive integrity, and SHA-256 checksums. No ROS Noetic build or robot hardware tests were performed on the current host. Real-time performance data in the original records applies only to the original control host.

## License

The source code, configuration files, and documentation authored for this project are licensed under the **Apache License 2.0**. See [LICENSE](LICENSE) and [NOTICE](NOTICE).

Third-party projects and archives retain their respective copyrights and licenses. See [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md) for details.
