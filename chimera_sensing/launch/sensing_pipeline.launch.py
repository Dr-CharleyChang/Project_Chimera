from launch import LaunchDescription
from launch_ros.actions import ComposableNodeContainer
from launch_ros.descriptions import ComposableNode

def generate_launch_description():
    return LaunchDescription([
        # 创建一个组件容器 (Container)
        ComposableNodeContainer(
            name='sensing_container',
            namespace='',
            package='rclcpp_components',
            executable='component_container',
            composable_node_descriptions=[
                # 节点 1: 静态 TF (Driver Layer)
                ComposableNode(
                    package='chimera_sensing',
                    plugin='chimera::sensing::StaticTFBroadcaster',
                    name='static_tf',
                    parameters=[{'child_frame': 'laser_link', 'x': 0.5, 'z': 0.2}]
                ),
                # 节点 2: 雷达处理器 (Processor Layer)
                ComposableNode(
                    package='chimera_sensing',
                    plugin='chimera::sensing::LidarProcessor',
                    name='lidar_processor',
                    # Remap: 将默认的 "lidar_points_raw" 映射到仿真器的 Topic
                    remappings=[('lidar_points_raw', '/sim/lidar/points')]
                )
            ],
            output='screen',
        )
    ])