// SPDX-License-Identifier: Apache-2.0
#include <franka/exception.h>
#include <franka/errors.h>
#include <franka/robot.h>

#include <iostream>
#include <string>

int main(int argc, char** argv) {
  std::string ip = "192.168.1.2";
  if (argc > 1) ip = argv[1];

  try {
    franka::Robot robot(ip);
    franka::RobotState st = robot.readOnce();

    std::cout << "robot_mode: " << static_cast<int>(st.robot_mode) << std::endl;
    bool has_errors = false;
    if (st.last_motion_errors) {
      std::cout << "Last motion errors: " << std::string(st.last_motion_errors) << std::endl;
      has_errors = true;
    } else {
      std::cout << "No motion errors." << std::endl;
    }

    if (st.robot_mode == franka::RobotMode::kReflex) {
      std::cout << "Robot in Reflex mode, running automatic error recovery..." << std::endl;
      robot.automaticErrorRecovery();
      franka::RobotState after = robot.readOnce();
      std::cout << "After recovery, robot_mode: " << static_cast<int>(after.robot_mode) << std::endl;
      if (after.robot_mode == franka::RobotMode::kReflex) {
        std::cerr << "Recovery did not clear the reflex. Check the robot on Desk." << std::endl;
        return 1;
      }
      std::cout << "Recovery OK." << std::endl;
    } else if (has_errors) {
      std::cout << "Errors present but not in Reflex; recovery may still be needed." << std::endl;
      robot.automaticErrorRecovery();
      std::cout << "Recovery done." << std::endl;
    } else {
      std::cout << "Robot is ready." << std::endl;
    }

  } catch (const franka::Exception& e) {
    std::cerr << "franka exception: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}
