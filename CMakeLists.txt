cmake_minimum_required(VERSION 3.5)
project(map_server)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(yaml-cpp REQUIRED)

include_directories(include)

add_executable(map_server src/map_server.cpp)

target_include_directories(map_server PRIVATE
  ${rclcpp_INCLUDE_DIRS}
  ${nav_msgs_INCLUDE_DIRS}
)

target_link_libraries(map_server PRIVATE
  ${rclcpp_LIBRARIES}
  ${nav_msgs_LIBRARIES}
  yaml-cpp
)

install(TARGETS
  map_server
  DESTINATION lib/${PROJECT_NAME})

ament_package()
