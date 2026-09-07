# Third-party components

The root Apache-2.0 license applies to this project's original material.
Third-party components retain their own copyright and license notices,
including notices for any bundled libraries.

| Component | Source / version | License location |
| --- | --- | --- |
| franka_ros | https://github.com/frankarobotics/franka_ros, package version 0.10.1, commit `30e598aa6fb703cc80a203481e6427f397337b4c` | Apache 2.0; submodule `LICENSE` and `NOTICE` |
| libfranka | `libfranka-0.9.2.zip`, original source archive | Apache 2.0; archive `libfranka-0.9.2/LICENSE` and accompanying notices |
| panda-python | `panda_py_0.8.1_libfranka_0.9.2.zip`, wheels for 0.8.1 + libfranka 0.9.2 | Apache 2.0 in each wheel's `.dist-info/licenses/LICENSE`; preserve other bundled notices |

The franka_ros NOTICE identifies **Copyright 2017 Franka Emika GmbH**.
The panda-python wheel metadata identifies **Jean Elsner** as the author.
These attributions do not transfer ownership of third-party material.

The original dependency archives are distributed unchanged through the
`v1.0.0` GitHub Release. SHA-256 values are recorded in
`SHA256SUMS`. The libfranka ZIP has an empty `common/` submodule directory;
use the recursive clone instructions in the root README when building it.
