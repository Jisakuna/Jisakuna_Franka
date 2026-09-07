// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <array>

namespace panda_rt {

// Default FCI robot address (Panda).
inline constexpr const char* kDefaultRobotIP = "192.168.1.2";

// Standard Panda home pose: {0, -pi/4, 0, -3pi/4, 0, pi/2, pi/4}
inline const std::array<double, 7> kHomePose{
    {0.0, -0.7853981633974483, 0.0, -2.356194490192345, 0.0, 1.5707963267948966,
     0.7853981633974483}};

// Set SCHED_FIFO realtime priority and lock all memory for the current thread.
// Returns true on success.
bool setRealtime(int priority = 90);

}  // namespace panda_rt
