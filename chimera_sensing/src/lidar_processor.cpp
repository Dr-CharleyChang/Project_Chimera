#include <tf2_ros/buffer.h>
#include <tf2_ros/transform_listener.h>

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>

#include "chimera_sensing/cloud_processor.hpp"
#include "chimera_sensing/point_types.hpp"
#include "chimera_sensing/ros_utils.hpp"
#include "chimera_sensing/spatial_utils.hpp"

namespace chimera::sensing {

class LidarProcessor : public rclcpp::Node {
 public:
  explicit LidarProcessor(const rclcpp::NodeOptions& options)
      : Node("lidar_processor", options) {
    // 1. Initialize TF2 buffer and listener
    // Buffer need Node's clock for time synchronization
    tf_buffer_ = std::make_unique<tf2_ros::Buffer>(this->get_clock());
    tf_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_);

    // 2. Create Publisher
    pub_cloud_ = this->create_publisher<sensor_msgs::msg::PointCloud2>(
        "lidar_points_base", 10);

    // 3. Create Subscriber
    sub_cloud_ = this->create_subscription<sensor_msgs::msg::PointCloud2>(
        "lidar_points_raw", rclcpp::SensorDataQoS(),
        [this](const sensor_msgs::msg::PointCloud2::SharedPtr msgs) {
          topic_callback(msgs);
        });

    // 4. Reserve buffer for point cloud unpacking
    cloud_buffer_.reserve(
        50000);  // reserve space for 50k points, avoid frequent reallocations

    RCLCPP_INFO(this->get_logger(), "LidarProcessor node initialized.");
  }

 private:
  void topic_callback(const sensor_msgs::msg::PointCloud2::SharedPtr msg) {
    // record start time
    auto start_time = std::chrono::steady_clock::now();

    // Check TF (Lidar Coordination->Vehicle Coordination)
    // target_frame: "base_link"
    // source_frame: msg->header.frame_id
    geometry_msgs::msg::TransformStamped t_stamped;
    try {
      // lookup Transform(target, source, time)
      // tf2::TimePointZero represents the latest available transform
      t_stamped = tf_buffer_->lookupTransform("base_link", msg->header.frame_id,
                                              tf2::TimePointZero);
    } catch (const tf2::TransformException& ex) {
      RCLCPP_WARN(this->get_logger(), "Could not transform: %s", ex.what());
      return;
    }

    // 2. ROS TF->Eigen AffinMatrix
    Eigen::Affine3d T_d = toEigen(t_stamped.transform);
    Eigen::Affine3f T = T_d.cast<float>();

    // Unpack
    std::span<Point3D> points_view;
    try {
      points_view = unpackPointCloud(cloud_buffer_, *msg);
    } catch (const std::exception& e) {
      RCLCPP_ERROR(this->get_logger(), "Data unpacking failed: %s", e.what());
      return;
    }

    // Transform
    transformPointCloud(points_view, T);

    // Pack
    sensor_msgs::msg::PointCloud2 out_msg;
    out_msg.header.stamp = msg->header.stamp;
    packPointCloud(out_msg, points_view, "base_link");

    // Publish
    pub_cloud_->publish(out_msg);

    // system performance
    auto end_time = std::chrono::steady_clock::now();
    auto latency =
        std::chrono::duration<double, std::milli>(end_time - start_time)
            .count();
    if (latency > 5.0) {
      RCLCPP_WARN(this->get_logger(), "High latency: %.2f ms", latency);
    }
  }
  // member variables
  std::unique_ptr<tf2_ros::Buffer> tf_buffer_;
  std::shared_ptr<tf2_ros::TransformListener> tf_listener_;
  rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr pub_cloud_;
  rclcpp::Subscription<sensor_msgs::msg::PointCloud2>::SharedPtr sub_cloud_;

  // Memory reserve
  std::vector<Point3D> cloud_buffer_;
};
}  // namespace chimera::sensing

// 【组件注册】允许该节点被动态加载，无需写 main 函数

#include <rclcpp_components/register_node_macro.hpp>
RCLCPP_COMPONENTS_REGISTER_NODE(chimera::sensing::LidarProcessor)