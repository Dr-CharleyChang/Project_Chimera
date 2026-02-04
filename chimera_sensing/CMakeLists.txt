cmake_minimum_required(VERSION 3.8)
project(chimera_sensing)

# =========== [新增] C++20 标准强制开启 ===========
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# ===============================================
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


ament_package()