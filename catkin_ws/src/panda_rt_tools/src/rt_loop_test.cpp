// SPDX-License-Identifier: Apache-2.0
#include <franka/robot.h>
#include <franka/duration.h>
#include <franka/exception.h>
#include <franka/rate_limiting.h>

#include "panda_rt/realtime_util.h"

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <sstream>
#include <vector>

int main(int argc, char** argv) {
  std::string ip = "192.168.1.2";
  if (argc > 1) {
    ip = argv[1];
  }

  try {
    franka::Robot robot(ip);
    robot.setCollisionBehavior(
        {{20.0, 20.0, 18.0, 18.0, 16.0, 14.0, 12.0}},
        {{20.0, 20.0, 18.0, 18.0, 16.0, 14.0, 12.0}},
        {{20.0, 20.0, 18.0, 18.0, 16.0, 14.0}},
        {{20.0, 20.0, 18.0, 18.0, 16.0, 14.0}});

    std::atomic<bool> running{true};
    if (!panda_rt::setRealtime()) {
      std::fprintf(stderr, "WARNING: not running with realtime priority!\n");
    }

    auto start = std::chrono::steady_clock::now();
    long total_samples = 0;
    long max_latency_us = 0;
    long late_samples = 0;  // samples exceeding the 1ms control period

    auto callback = [&](const franka::RobotState&, franka::Duration period) {
      auto now = std::chrono::steady_clock::now();
      auto elapsed_us =
          std::chrono::duration_cast<std::chrono::microseconds>(now - start).count();
      long cycle_us = static_cast<long>(period.toSec() * 1e6);
      total_samples++;

      // The callback should be invoked every ~1ms (1000Hz). A period that
      // differs from 1000us by more than 50% indicates a missed deadline.
      if (cycle_us > 1500) {
        late_samples++;
      }
      if (cycle_us > max_latency_us) {
        max_latency_us = cycle_us;
      }

      if (total_samples % 1000 == 0) {
        std::printf("[%5lds] samples=%6ld late=%6ld max_cycle=%5ldus\n",
                    total_samples / 1000, total_samples, late_samples, max_latency_us);
        fflush(stdout);
      }
      if (total_samples >= 10000) {
        running.store(false);
      }

      // zero torque: hold current pose via gravity compensation
      return franka::Torques{{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}};
    };

    robot.control(callback, true);

    std::printf("\n=== RESULTS ===\n");
    std::printf("total_samples: %ld\n", total_samples);
    std::printf("late_samples (>1.5ms): %ld\n", late_samples);
    std::printf("late rate: %.2f%%\n", 100.0 * late_samples / total_samples);
    std::printf("max_cycle: %ld us\n", max_latency_us);

  } catch (const franka::Exception& e) {
    std::cerr << "franka exception: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}
