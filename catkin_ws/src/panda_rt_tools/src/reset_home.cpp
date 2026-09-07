// SPDX-License-Identifier: Apache-2.0
#include <franka/robot.h>
#include <franka/duration.h>
#include <franka/exception.h>
#include <franka/rate_limiting.h>

#include "panda_rt/realtime_util.h"

#include <array>
#include <cmath>
#include <iostream>
#include <thread>

int main(int argc, char** argv) {
  std::string ip = "192.168.1.2";
  if (argc > 1) ip = argv[1];

  double max_speed_deg_s = 8.0;  // max joint speed
  if (argc > 2) max_speed_deg_s = std::stod(argv[2]);

  // Standard Panda home pose: {0, -pi/4, 0, -3pi/4, 0, pi/2, pi/4}
  const std::array<double, 7> kHome{
      {0.0, -M_PI / 4, 0.0, -3 * M_PI / 4, 0.0, M_PI / 2, M_PI / 4}};

  try {
    franka::Robot robot(ip);
    robot.setCollisionBehavior(
        {{20.0, 20.0, 18.0, 18.0, 16.0, 14.0, 12.0}},
        {{20.0, 20.0, 18.0, 18.0, 16.0, 14.0, 12.0}},
        {{20.0, 20.0, 18.0, 18.0, 16.0, 14.0}},
        {{20.0, 20.0, 18.0, 18.0, 16.0, 14.0}});

    franka::RobotState initial_state = robot.readOnce();
    std::array<double, 7> q0 = initial_state.q;

    std::cout << "Current joint positions (rad): ";
    for (double q : q0) std::cout << q << " ";
    std::cout << std::endl;
    std::cout << "Target home (rad): ";
    for (double q : kHome) std::cout << q << " ";
    std::cout << std::endl;

    double max_speed = max_speed_deg_s * M_PI / 180.0;

    // Compute the longest joint travel; all joints finish at the same time.
    double max_travel = 0.0;
    for (size_t i = 0; i < 7; ++i) {
      max_travel = std::max(max_travel, std::abs(kHome[i] - q0[i]));
    }
    // Smoothstep duration scaled so max joint speed stays below max_speed.
    // d(s)/dt max is 1.5; joint speed ~ max_travel * 1.5 / duration.
    double duration = std::max(1.0, 1.5 * max_travel / max_speed);

    panda_rt::setRealtime();

    double t = 0.0;

    auto motion = [&](const franka::RobotState& robot_state, franka::Duration) {
      t += 0.001;
      double s = std::min(1.0, t / duration);
      // smoothstep: zero velocity and acceleration at both ends
      double ss = s * s * (3.0 - 2.0 * s);

      std::array<double, 7> q;
      for (size_t i = 0; i < 7; ++i) {
        q[i] = q0[i] + (kHome[i] - q0[i]) * ss;
      }

      if (t > 0.1 && s >= 1.0) {
        // Trajectory done: wait until the robot actually settles before finishing.
        bool settled = true;
        for (size_t i = 0; i < 7; ++i) {
          if (std::abs(robot_state.dq[i]) > 0.02) settled = false;
          if (std::abs(kHome[i] - robot_state.q[i]) > 1.0e-3) settled = false;
        }
        if (settled) {
          std::cout << "Reached home pose and settled, finishing." << std::endl;
          return franka::MotionFinished(franka::JointPositions(robot_state.q_d));
        }
        return franka::JointPositions(q);
      }
      return franka::JointPositions(q);
    };

    try {
      robot.control(motion, franka::ControllerMode::kJointImpedance);
      std::cout << "Motion finished normally." << std::endl;
    } catch (const franka::Exception& e) {
      std::cout << "Control ended: " << e.what() << std::endl;
    }

    franka::RobotState final_state = robot.readOnce();
    std::cout << "\n=== RESULT ===" << std::endl;
    std::cout << "final joint positions (rad): ";
    for (double q : final_state.q) std::cout << q << " ";
    std::cout << std::endl;

    double max_err = 0.0;
    for (size_t i = 0; i < 7; ++i) {
      max_err = std::max(max_err, std::abs(final_state.q[i] - kHome[i]));
    }
    std::cout << "max error vs home: " << max_err * 180.0 / M_PI << " deg" << std::endl;
    if (max_err < 1.0 * M_PI / 180.0) {
      std::cout << "Robot reset to home OK." << std::endl;
    } else {
      std::cout << "Warning: robot not exactly at home." << std::endl;
    }

  } catch (const franka::Exception& e) {
    std::cerr << "franka exception: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}
