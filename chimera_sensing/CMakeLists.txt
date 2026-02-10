cmake_minimum_required(VERSION 3.8)
project(chimera_sensing)

# ===============================================
# [架构修复点 1] 强制开启 C++20 标准
# 原因：我们需要使用 std::span (C++20 特性)
# ===============================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. 查找依赖
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(geometry_msgs REQUIRED)
# [新增修复] Day 4 必须依赖 sensor_msgs (PointCloud2)
find_package(sensor_msgs REQUIRED) 
find_package(Eigen3 REQUIRED)

# 2. 包含目录
include_directories(include)
include_directories(${EIGEN3_INCLUDE_DIR})

# ===============================================
# 3. 编译库组件 (Components)
# ===============================================

# 3.1 组件一：静态 TF 广播器 (Day 1-2)
add_library(static_tf_broadcaster SHARED src/static_tf_broadcaster.cpp)
ament_target_dependencies(static_tf_broadcaster
  rclcpp rclcpp_components tf2 tf2_ros geometry_msgs
)
# 注册组件 (让它能被 Launch 文件动态加载)
rclcpp_components_register_nodes(static_tf_broadcaster "chimera::sensing::StaticTFBroadcaster")

# 3.2 [新增修复] 组件二：雷达处理器 (Day 4)
add_library(lidar_processor SHARED src/lidar_processor.cpp)
ament_target_dependencies(lidar_processor
  # 注意这里加了 sensor_msgs 和 Eigen3
  rclcpp rclcpp_components tf2 tf2_ros geometry_msgs sensor_msgs Eigen3
)
# 注册组件
rclcpp_components_register_nodes(lidar_processor "chimera::sensing::LidarProcessor")

# ===============================================
# 4. 安装规则 (Install Rules)
# ===============================================

# [优化修复] 合并安装两个库
install(TARGETS 
  static_tf_broadcaster
  lidar_processor
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

# 安装 Launch 脚本 (关键！否则 ros2 launch 找不到文件)
install(DIRECTORY launch DESTINATION share/${PROJECT_NAME})
# 安装头文件
install(DIRECTORY include/ DESTINATION include)

# ===============================================
# 5. 单元测试配置 (GTest)
# ===============================================
if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)
  
  # 编译测试可执行文件
  ament_add_gtest(test_spatial_math test/test_spatial_math.cpp)
  
  # 链接依赖 (如果测试代码引用了库代码，这里可能需要 link)
  # 目前 test_spatial_math.cpp 是 header-only 测试，所以这就够了
  target_include_directories(test_spatial_math PUBLIC ${EIGEN3_INCLUDE_DIR})
  ament_target_dependencies(test_spatial_math geometry_msgs)
endif()

ament_package()