// SPDX-License-Identifier: Apache-2.0
#include <franka/exception.h>
#include <franka/robot.h>

#include <iostream>
#include <string>

int main(int argc, char** argv) {
  std::string ip = "192.168.1.2";
  if (argc > 1) ip = argv[1];

  try {
    franka::Robot robot(ip);
    franka::RobotState st = robot.readOnce();
    std::cout << "q: ";
    for (double q : st.q) std::cout << q << " ";
    std::cout << std::endl;
    std::cout << "control_command_success_rate: " << st.control_command_success_rate << std::endl;
    std::cout << "robot_mode: " << static_cast<int>(st.robot_mode) << std::endl;
    std::cout << "running_time: " << st.time.toSec() << " s" << std::endl;
  } catch (const franka::Exception& e) {
    std::cerr << "franka exception: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}
