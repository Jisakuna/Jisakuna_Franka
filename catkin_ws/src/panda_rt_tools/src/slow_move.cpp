// SPDX-License-Identifier: Apache-2.0
#include <franka/robot.h>
#include <franka/duration.h>
#include <franka/exception.h>
#include <franka/rate_limiting.h>

#include "panda_rt/realtime_util.h"

#include <array>
#include <cmath>
#include <iostream>
#include <string>

int main(int argc, char** argv) {
  std::string ip = "192.168.1.2";
  if (argc > 1) ip = argv[1];

  size_t joint = 0;              // joint index (0..6)
  if (argc > 2) joint = static_cast<size_t>(std::stoul(argv[2]));
  if (joint > 6) {
    std::cerr << "joint index must be 0..6" << std::endl;
    return 1;
  }

  double deg = 5.0;              // target movement in degrees
  if (argc > 3) deg = std::stod(argv[3]);
  double max_speed_deg_s = 8.0;  // max joint speed in deg/s
  if (argc > 4) max_speed_deg_s = std::stod(argv[4]);

  double target_delta = deg * M_PI / 180.0;
  double max_speed = max_speed_deg_s * M_PI / 180.0;

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

    std::array<double, 7> q_target = q0;
    q_target[joint] += target_delta;

    std::cout << "Moving joint[" << joint << "] by " << deg << " deg"
              << " (target " << q_target[joint] << " rad), max speed = "
              << max_speed_deg_s << " deg/s" << std::endl;

    panda_rt::setRealtime();

    // Trapezoidal velocity profile on the selected joint.
    double accel = max_speed / 0.5;  // reach max speed in 0.5 s
    double t = 0.0;
    double dist = 0.0;               // travelled distance (rad)

    auto motion = [&](const franka::RobotState& robot_state, franka::Duration) {
      t += 0.001;
      double remaining = std::abs(target_delta) - dist;
      double v_brake = std::sqrt(2.0 * accel * remaining);  // decel-limited speed
      double v_ramp = accel * t;
      double v = std::min({max_speed, v_ramp, v_brake});
      if (remaining <= 0.0) {
        // Target reached: hold the target and wait until the robot settles
        // (near target position, speed near zero) before finishing.
        bool near_target = std::abs(robot_state.q[joint] - q_target[joint]) < 1.0e-3;
        bool settled = std::abs(robot_state.dq[joint]) < 0.02;
        if (t > 0.1 && near_target && settled) {
          return franka::MotionFinished(franka::JointPositions(robot_state.q_d));
        }
        return franka::JointPositions(q_target);
      }
      dist += v * 0.001;
      std::array<double, 7> q = q0;
      q[joint] = q0[joint] + std::copysign(dist, target_delta);
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
    std::cout << "joint[" << joint << "] start: " << q0[joint] << " rad" << std::endl;
    std::cout << "joint[" << joint << "] final: " << final_state.q[joint] << " rad" << std::endl;
    std::cout << "moved: " << (final_state.q[joint] - q0[joint]) * 180.0 / M_PI << " deg" << std::endl;

  } catch (const franka::Exception& e) {
    std::cerr << "franka exception: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}
