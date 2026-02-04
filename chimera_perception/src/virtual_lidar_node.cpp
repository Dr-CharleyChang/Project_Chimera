#include <algorithm>  // std::ranges::for_each
#include <chrono>
#include <cmath>
#include <ranges>  // C++20 Ranges & Views
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>
#include <span>  // C++20 内存视图

#include "chimera_types.hpp"

using namespace std::chrono_literals;

class VirtualLidarNode : public rclcpp::Node {
 public:
  VirtualLidarNode() : Node("virtual_lidar") {
    pub_ = this->create_publisher<sensor_msgs::msg::PointCloud2>(
        "/rslidar_points", rclcpp::SensorDataQoS());

    timer_ =
        this->create_wall_timer(100ms, [this]() { this->publish_fake_scan(); });

    RCLCPP_INFO(this->get_logger(),
                "Virtual Lidar Node (C++20 Ranges Optimized) Started.");
  }

 private:
  void publish_fake_scan() {
    const int num_points = 2000;
    double time_sec = this->now().seconds();

    // 1. 直接创建消息对象 (在堆上)
    auto msg = std::make_unique<sensor_msgs::msg::PointCloud2>();

    // 2. 预先分配内存 (这一步是唯一的内存分配)
    msg->data.resize(num_points * sizeof(LidarPoint));

    // 3. 【核心：原位映射】将字节数组强转为 LidarPoint 的视图
    // 就像 MATLAB 里的 reshape：把一维字节流变成对象数组
    std::span<LidarPoint> points_span(
        reinterpret_cast<LidarPoint*>(msg->data.data()), num_points);

    // 4. 【核心：C++20 Ranges Pipeline】
    // std::views::iota(0, n): 生成一个 0 到 n-1 的索引流 [0, 1, 2...]
    // 这不是一个真正的数组，而是一个 Lazy Generator (延迟生成器)
    auto index_stream = std::views::iota(0, num_points);

    // 5. 使用 for_each 直接把数学计算结果填入 points_span 的内存地址
    std::ranges::for_each(index_stream, [&points_span, time_sec](int i) {
      float x = static_cast<float>(i) * 0.05f - 50.0f;
      // 数学模型：z = A \cdot \sin(\omega \cdot x + t)
      float z = 2.5f * std::sin(0.2f * x + 2.0f * time_sec);

      // 直接操作 msg->data 指向的内存，无需中间 vector，无需 memcpy
      points_span[i] = {x, 0.0f, z, 100.0f};
    });

    // 6. 填充消息元数据 (Schema 定义)
    setup_message_metadata(msg, num_points);

    pub_->publish(std::move(msg));
  }

  void setup_message_metadata(
      std::unique_ptr<sensor_msgs::msg::PointCloud2>& msg, int n) {
    msg->header.stamp = this->now();
    msg->header.frame_id = "map";
    msg->height = 1;
    msg->width = n;
    msg->point_step = sizeof(LidarPoint);
    msg->row_step = msg->width * msg->point_step;
    msg->is_dense = true;

    // 定义 PointField：告诉接收端如何解析内存中的 XYZI
    auto add_field = [&](const char* name, uint32_t offset) {
      sensor_msgs::msg::PointField pf;
      pf.name = name;
      pf.offset = offset;
      pf.count = 1;
      pf.datatype = sensor_msgs::msg::PointField::FLOAT32;
      msg->fields.push_back(pf);
    };
    add_field("x", 0);
    add_field("y", 4);
    add_field("z", 8);
    add_field("intensity", 12);
  }

  rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr pub_;
  rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char** argv) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<VirtualLidarNode>());
  rclcpp::shutdown();
  return 0;
}