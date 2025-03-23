cmake_minimum_required(VERSION 3.5)
project(map_server)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(yaml-cpp REQUIRED)

include_directories(include)

add_executable(map_server src/map_server.cpp)
ament_target_dependencies(map_server rclcpp nav_msgs)
target_link_libraries(map_server yaml-cpp)

install(TARGETS
  map_server
  DESTINATION lib/${PROJECT_NAME})

ament_package()