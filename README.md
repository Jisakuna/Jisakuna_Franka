# Jisakuna_Franka

**English** | [简体中文](README.zh-CN.md)

A ROS Noetic workspace, real-time control utilities, and installation archives for the **Franka Emika Panda**.

This repository preserves the C++ source and launch files for `panda_rt_tools`, together with the original control host's environment notes. `franka_ros` is pinned as a Git submodule, and dependency archives are available as release assets for reproducing the Panda / libfranka 0.9.2 development environment.

## Contents and versions

The first project release is **v1.0.0**. Individual ROS packages and dependencies retain their own versions.

| Component | Version / description |
| --- | --- |
| `panda_rt_tools` | 0.1.0; custom state readout, joint motion, home positioning, and real-time diagnostics |
| `franka_ros` | Package version 0.10.1; pinned to `30e598aa6fb703cc80a203481e6427f397337b4c` |
| `libfranka` | 0.9.2; original source ZIP archive |
| `panda-python` | 0.8.1 + libfranka 0.9.2; Linux x86_64 Python wheel archive |

```text
Jisakuna_Franka/
├── README.md                     # English (default)
├── README.zh-CN.md               # Simplified Chinese
├── LICENSE                       # Apache License 2.0
├── NOTICE
├── THIRD_PARTY_NOTICES.md
├── SHA256SUMS                    # Checksums for release archives
└── catkin_ws/
    ├── .catkin_workspace
    └── src/
        ├── franka_ros/            # Pinned upstream Git submodule
        └── panda_rt_tools/
            ├── include/
            ├── src/
            ├── launch/
            ├── CMakeLists.txt
            └── package.xml
```

Machine-specific `build/`, `devel/`, and `install/` directories are excluded from Git. Rebuild the workspace after cloning. GitHub-generated source ZIPs do not include submodule contents; use the recursive clone command below.

## Original development environment

These details come from the existing [environment notes](catkin_ws/src/panda_rt_tools/README.md). They do not imply that hardware validation was repeated for this release.

| Component | Recorded configuration |
| --- | --- |
| Operating system | Ubuntu 20.04.6 LTS |
| ROS | ROS 1 Noetic |
| Real-time kernel | Linux 5.15.92-rt57 / PREEMPT_RT |
| Build tools | GCC / G++ 9.4, CMake ≥ 3.16, C++17 |
| Dependencies | libfranka 0.9.2, Eigen3, Poco, Threads, roscpp |
| Control host | Intel Core i9-10900X, 64 GB RAM |
| FCI interface | Intel I210 on a dedicated wired link |
| Example network | Host `192.168.1.1/24`; robot `192.168.1.2` |

This project uses **ROS 1 catkin** and cannot be built directly as a ROS 2 Jazzy colcon workspace. Confirm robot firmware compatibility with libfranka for your actual hardware.

## Get the project and installation archives

This is a private repository. Authenticate with a GitHub account that has access.

```bash
git clone --recurse-submodules https://github.com/Jisakuna/Jisakuna_Franka.git
cd Jisakuna_Franka

# If you already cloned without submodules:
git submodule update --init --recursive
```

Download the following files from the [v1.0.0 release](https://github.com/Jisakuna/Jisakuna_Franka/releases/tag/v1.0.0) and place them in the repository root:

| File | Purpose |
| --- | --- |
| `libfranka-0.9.2.zip` | Original libfranka source archive |
| `panda_py_0.8.1_libfranka_0.9.2.zip` | Linux x86_64 wheels for CPython 3.7–3.12 |

```bash
sha256sum -c SHA256SUMS
```

Both archives are preserved without renaming or repacking. **The libfranka ZIP contains an empty `common/` submodule directory and cannot be built from the ZIP alone.** Use the recursive clone instructions below to build the library.

## Build the C++ tools

These commands target an Ubuntu 20.04 control host with ROS Noetic already installed.

### 1. Install build dependencies

```bash
sudo apt update
sudo apt install build-essential cmake git libeigen3-dev libpoco-dev \
  ros-noetic-roscpp
```

### 2. Build libfranka 0.9.2

Skip this step if libfranka 0.9.2 is already installed and discoverable by CMake. Avoid mixing different installed versions of the library.

```bash
# Run from the Jisakuna_Franka repository root.
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

This builds the custom tools and their required workspace dependencies. To build the full `franka_ros` control, description, visualization, or simulation packages, install all dependencies listed in its [upstream README](catkin_ws/src/franka_ros/README.md), then run `catkin_make -DCATKIN_WHITELIST_PACKAGES=""` from `catkin_ws`.

Source ROS and this workspace again in each new terminal. If CMake cannot find Franka, pass `-DFranka_DIR=/actual/install/path/lib/cmake/Franka`, pointing to the directory containing `FrankaConfig.cmake`.

## Python wheels (optional)

The Python packages are independent of the C++ catkin tools. The archive contains wheels for CPython 3.7, 3.8, 3.9, 3.10, 3.11, and 3.12 on manylinux x86_64. Select the wheel that matches your Python virtual environment, for example:

```bash
unzip panda_py_0.8.1_libfranka_0.9.2.zip -d dependencies/panda_py
# CPython 3.8 / Linux x86_64 only; select another filename for other versions.
python -m pip install dependencies/panda_py/panda_python-0.8.1+libfranka.0.9.2-cp38-cp38-manylinux_2_17_x86_64.manylinux2014_x86_64.whl
```

The archive does not contain all Python dependencies. pip may still need network access to install them.

## Tool usage

Before running a tool, enable the robot's FCI and check the robot IP, dedicated network interface, real-time permissions, and emergency stop. Replace the example IP with your actual robot address.

| Executable | Purpose | Arguments |
| --- | --- | --- |
| `read_state` | Read joint positions, robot mode, and communication success rate | `[robot_ip]` |
| `slow_move` | Move one joint by a relative angle | `[robot_ip] [joint] [angle_deg] [speed_deg_s]` |
| `reset_home` | Move all joints to the home pose defined in the source | `[robot_ip] [speed_deg_s]` |
| `recover` | Invoke automatic error recovery; changes robot state | `[robot_ip]` |
| `rt_loop_test` | Run a zero additional torque control loop and collect timing statistics | `[robot_ip]`; review the limitations below first |

Start by reading the state:

```bash
rosrun panda_rt_tools read_state 192.168.1.2
# Equivalent launch entry point:
roslaunch panda_rt_tools read_state.launch robot_ip:=192.168.1.2
```

Only attempt a small motion after confirming a clear workspace and a working real-time configuration. **Joint indices are 0–6; index 1 refers to the second joint.**

```bash
# Move the second joint by +5 degrees with a speed parameter of 2 degrees/s.
rosrun panda_rt_tools slow_move 192.168.1.2 1 5 2

# Move to the home pose defined in the source.
rosrun panda_rt_tools reset_home 192.168.1.2 2

# Recover only after addressing the cause of the fault.
rosrun panda_rt_tools recover 192.168.1.2
```

The corresponding launch files are `slow_move.launch`, `reset_home.launch`, and `recover.launch`. They pass launch arguments to the executables through `args`.

## Real-time configuration and current limitations

See the [package README](catkin_ws/src/panda_rt_tools/README.md) for the original host configuration. The tools attempt to use `SCHED_FIFO` priority 90 and call `mlockall`. The control host should have a PREEMPT_RT kernel, appropriate `rtprio` / `memlock` permissions, and a stable wired FCI connection.

This repository preserves existing development tools. The motion logic was not changed during release preparation. The following limitations are present in the source:

- `slow_move` and `reset_home` advance trajectory time by a fixed 1 ms instead of using the actual callback period. Validation of speed, numeric arguments, and target joint limits is incomplete.
- Motion tools do not immediately exit when real-time scheduling setup fails. Some caught control exceptions may still result in a successful process exit code. Do not use the exit code alone to determine whether a motion succeeded.
- After 10000 samples, `rt_loop_test` only changes a local flag; it does not return `franka::MotionFinished`. It therefore does not stop automatically as intended. Fix and validate its termination behavior before use.
- Zero additional torque does not lock the robot's pose. Do not treat this diagnostic as a motion-free state reader. The motion tools also do not perform obstacle-aware path planning.

Do not run these tools unattended without validating their behavior. Keep the emergency stop available and people and obstacles clear of the robot's motion range during motion or torque control.

## Release validation

Release preparation included file inventory checks, XML parsing, license consistency checks, archive integrity checks, and SHA-256 verification. ROS Noetic compilation and robot hardware tests were not performed on the current host. Real-time performance figures in the environment notes apply only to the original control host.

## License

Original project source, configuration, and documentation are licensed under **Apache License 2.0**; see [LICENSE](LICENSE) and [NOTICE](NOTICE). The declaration in `panda_rt_tools/package.xml` is consistent with this license.

Third-party projects and archives retain their respective copyrights and licenses. See [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).
