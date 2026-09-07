cmake_minimum_required(VERSION 3.10)
project(panda_rt_tools)

find_package(catkin REQUIRED COMPONENTS roscpp)
find_package(Franka REQUIRED)
find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

catkin_package()

# --- realtime library (shared code) ---
add_library(panda_rt_common
  src/realtime_util.cpp
)
target_include_directories(panda_rt_common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(panda_rt_common ${catkin_LIBRARIES})

# --- executables ---
function(add_rt_tool name src)
  add_executable(${name} ${src})
  target_link_libraries(${name}
    panda_rt_common
    Franka::Franka
    ${catkin_LIBRARIES}
    Threads::Threads
  )
  install(TARGETS ${name}
    RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
  )
endfunction()

add_rt_tool(read_state    src/read_state.cpp)
add_rt_tool(slow_move     src/slow_move.cpp)
add_rt_tool(reset_home    src/reset_home.cpp)
add_rt_tool(recover       src/recover.cpp)
add_rt_tool(rt_loop_test  src/rt_loop_test.cpp)

install(DIRECTORY launch
  DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}
)
