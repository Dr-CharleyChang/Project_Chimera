cmake_minimum_required(VERSION 3.8)
project(chimera_sensing)

# 1. 依赖查找
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(Eigen3 REQUIRED) # [关键] 必须有这一行

# 2. 包含目录
include_directories(include)
include_directories(${EIGEN3_INCLUDE_DIR})

# 3. 编译主库 (Day 1 内容)
add_library(static_tf_broadcaster SHARED src/static_tf_broadcaster.cpp)
ament_target_dependencies(static_tf_broadcaster
  rclcpp rclcpp_components tf2 tf2_ros geometry_msgs
)
rclcpp_components_register_nodes(static_tf_broadcaster "chimera::sensing::StaticTFBroadcaster")

# 4. 安装规则
install(TARGETS static_tf_broadcaster
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)
install(DIRECTORY launch DESTINATION share/${PROJECT_NAME})
install(DIRECTORY include/ DESTINATION include)

# ===============================================
# 5. [缺失的部分] 单元测试配置
# ===============================================
if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)
  
  # 告诉 CMake：我要编译 test_spatial_math.cpp 这个文件
  ament_add_gtest(test_spatial_math test/test_spatial_math.cpp)
  
  # 链接依赖
  target_include_directories(test_spatial_math PUBLIC ${EIGEN3_INCLUDE_DIR})
  ament_target_dependencies(test_spatial_math geometry_msgs)
endif()

ament_package()