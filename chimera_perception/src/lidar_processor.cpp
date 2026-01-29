#include <algorithm>  // C++20 算法
#include <cstring>    // for std::memcpy
#include <ranges>     // C++20 范围视图
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>
#include <span>  // C++20 核心武器
#include <vector>

#include "chimera_types.hpp"  // 包含 LidarPoint 结构体定义

class LidarProcessorNode : public rclcpp::Node {
 public:
  LidarProcessorNode() : Node("lidar_processor") {
    // 1. 设置 QoS：BEST_EFFORT 保证最低延迟
    auto qos = rclcpp::SensorDataQoS();

    // 2. 订阅
    sub_ = this->create_subscription<sensor_msgs::msg::PointCloud2>(
        "/rslidar_points", qos,
        [this](const sensor_msgs::msg::PointCloud2::ConstSharedPtr msg) {
          this->topic_callback(msg);
        });

    // 3. 发布
    pub_ = this->create_publisher<sensor_msgs::msg::PointCloud2>(
        "/chimera/filtered_points", qos);

    RCLCPP_INFO(this->get_logger(),
                "LidarProcessor (Zero-Copy + Ranges) Started.");
  }

 private:
  // [架构重构] 纯算法函数：输入 Span，输出 Vector
  // 这个函数完全解耦，不涉及 ROS 消息机制，非常适合做 GTest 单元测试
  // 甚至可以直接移植到非 ROS 环境（如 pure C++ SDK）
  void filter_cloud(std::span<const LidarPoint> input,
                    std::vector<LidarPoint>& output) {
    // 清空输出容器
    output.clear();
    // 启发式预留空间，避免动态扩容带来的性能损耗
    output.reserve(input.size());

    // 1. 定义过滤规则 (Lambda) —— 这里的逻辑就是未来 FPGA 的 IP 核逻辑
    auto filter_logic = [](const LidarPoint& p) {
      // 过滤掉过高点、过低点和超远点
      return p.z > -1.5f && p.z < 2.0f && p.range() < 50.0f;
    };

    // 2. C++20 Ranges Pipeline (声明式编程)
    // 数据像水流一样通过管道：input -> 过滤器 -> 收集器
    auto filtered_view = input | std::views::filter(filter_logic);

    // 3. 执行拷贝 (Sink)
    std::ranges::copy(filtered_view, std::back_inserter(output));
  }

  void topic_callback(const sensor_msgs::msg::PointCloud2::ConstSharedPtr msg) {
    // [Safety Check] 确保数据对齐
    if (msg->point_step != sizeof(LidarPoint)) {
      RCLCPP_ERROR_THROTTLE(this->get_logger(), *this->get_clock(), 1000,
                            "Point step mismatch! Expected %lu, got %u.",
                            sizeof(LidarPoint), msg->point_step);
      return;
    }

    // 1. Zero-Copy 视图构建
    auto raw_ptr = reinterpret_cast<const LidarPoint*>(msg->data.data());
    size_t point_count = msg->width * msg->height;
    std::span<const LidarPoint> cloud_view(raw_ptr, point_count);

    // 2. 调用核心算法 (重用成员变量 filtered_cache_
    // 以减少内存分配也是一种优化手段，
    //    但在多线程 Executor 下需加锁。这里暂用局部变量以保安全)
    std::vector<LidarPoint> filtered_points;
    filter_cloud(cloud_view, filtered_points);

    // 3. 构造输出消息
    auto out_msg = std::make_unique<sensor_msgs::msg::PointCloud2>();
    out_msg->header = msg->header;
    out_msg->height = 1;
    out_msg->width = filtered_points.size();
    out_msg->fields = msg->fields;
    out_msg->point_step = msg->point_step;
    out_msg->is_bigendian = msg->is_bigendian;
    // 计算新的 row_step
    out_msg->row_step = out_msg->width * out_msg->point_step;

    // 4. 批量序列化 (极速内存拷贝)
    // 相比原来的 for 循环 insert，memcpy 是 CPU 指令级优化，极快
    out_msg->data.resize(out_msg->row_step);
    if (!filtered_points.empty()) {
      std::memcpy(out_msg->data.data(), filtered_points.data(),
                  out_msg->data.size());
    }

    pub_->publish(std::move(out_msg));
  }

  rclcpp::Subscription<sensor_msgs::msg::PointCloud2>::SharedPtr sub_;
  rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr pub_;
};

int main(int argc, char* argv[]) {
  rclcpp::init(argc, argv);
  rclcpp::executors::MultiThreadedExecutor executor;
  auto node = std::make_shared<LidarProcessorNode>();
  executor.add_node(node);
  executor.spin();
  rclcpp::shutdown();
  return 0;
}