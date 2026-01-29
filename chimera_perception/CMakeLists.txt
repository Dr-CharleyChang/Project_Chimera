cmake_minimum_required(VERSION 3.8)
project(chimera_perception)

# [关键] 开启 C++20 以支持 std::span
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
# 注意：我们移除了 PCL 和 pcl_conversions 的 find_package

include_directories(include)

add_executable(lidar_processor_node src/lidar_processor.cpp)
ament_target_dependencies(lidar_processor_node
  rclcpp
  sensor_msgs
)

# 定义虚拟雷达节点
add_executable(virtual_lidar_node src/virtual_lidar_node.cpp)
ament_target_dependencies(virtual_lidar_node rclcpp sensor_msgs)
target_include_directories(virtual_lidar_node PUBLIC include)

if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)
  
  # 定义测试可执行文件
  ament_add_gtest(chimera_test tests/test_chimera_types.cpp)
  
  # 链接你的库或包含目录
  target_include_directories(chimera_test PUBLIC include)
  
  # 如果你的 LidarPoint 定义在库里，需要 target_link_libraries
  # 这里假设它是纯头文件，所以只需要 include
endif()

# 安装规则保持不变
install(TARGETS lidar_processor_node
  virtual_lidar_node
  DESTINATION lib/${PROJECT_NAME})
  
install(DIRECTORY include/
  DESTINATION include)

ament_package()