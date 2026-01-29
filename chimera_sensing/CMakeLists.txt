cmake_minimum_required(VERSION 3.8)
project(chimera_sensing)

# 1. 寻找依赖库
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(geometry_msgs REQUIRED)

# 2. 编译组件库 (生成 libstatic_tf_broadcaster.so)
add_library(static_tf_broadcaster SHARED src/static_tf_broadcaster.cpp)
ament_target_dependencies(static_tf_broadcaster
  rclcpp rclcpp_components tf2 tf2_ros geometry_msgs
)

# 3. 注册组件 (关键步骤：让 ROS 找到这个插件)
rclcpp_components_register_nodes(static_tf_broadcaster "chimera::sensing::StaticTFBroadcaster")

# 4. 安装规则
install(TARGETS static_tf_broadcaster
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

ament_package()