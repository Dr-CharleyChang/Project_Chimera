from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        Node(
            package='chimera_sensing',
            executable='static_tf_broadcaster',
            name='lidar_static_tf',
            output='screen',
            parameters=[{
                'parent_frame': 'base_link',
                'child_frame': 'laser_link',
                'x': 0.2,      # 前向 0.2米
                'y': 0.0,
                'z': 0.15,     # 高度 0.15米
                'roll': 0.0,
                'pitch': 0.0,
                'yaw': 0.0
            }]
        )
    ])