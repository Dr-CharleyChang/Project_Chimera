#include <tf2/LinearMath/Quaternion.h>
#include <tf2_ros/static_transform_broadcaster.h>

#include <geometry_msgs/msg/transform_stamped.hpp>
#include <rclcpp/rclcpp.hpp>

namespace chimera::sensing {

class StaticTFBroadcaster : public rclcpp::Node {
 public:
  explicit StaticTFBroadcaster(const rclcpp::NodeOptions& options)
      : Node("static_tf_broadcaster", options) {
    // 【架构点】使用 Shared Pointer 管理生命周期，防止内存泄漏
    broadcaster_ = std::make_shared<tf2_ros::StaticTransformBroadcaster>(this);

    // 【参数化设计】严禁硬编码。
    // 这些参数对应 MATLAB 中的 config struct，必须能在运行时通过 YAML 修改
    this->declare_parameter("parent_frame", "base_link");  // 车辆几何中心
    this->declare_parameter("child_frame", "laser_link");  // 雷达光心
    this->declare_parameter("x", 0.0);
    this->declare_parameter("y", 0.0);
    this->declare_parameter("z", 0.0);
    this->declare_parameter("roll", 0.0);
    this->declare_parameter("pitch", 0.0);
    this->declare_parameter("yaw", 0.0);

    publishTransform();
  }

 private:
  void publishTransform() {
    geometry_msgs::msg::TransformStamped t;

    // 【时序控制】虽然是静态变换，但打上当前时间戳是 ROS 规范
    t.header.stamp = this->get_clock()->now();
    t.header.frame_id = this->get_parameter("parent_frame").as_string();
    t.child_frame_id = this->get_parameter("child_frame").as_string();

    // 1. 平移向量 (Translation)
    t.transform.translation.x = this->get_parameter("x").as_double();
    t.transform.translation.y = this->get_parameter("y").as_double();
    t.transform.translation.z = this->get_parameter("z").as_double();

    // 2. 旋转变换 (Rotation) - 重点！
    // 信号处理类比：这是从“极坐标”(r/p/y) 到 “笛卡尔坐标”(quaternion)
    // 的编译过程
    double r = this->get_parameter("roll").as_double();
    double p = this->get_parameter("pitch").as_double();
    double y = this->get_parameter("yaw").as_double();

    tf2::Quaternion q;
    q.setRPY(r, p, y);  // 输入：弧度 (Radians)

    // 四元数 (x,y,z,w) 类似于复数 (i,j,k,Real)
    // 它是计算机图形学和机器人学中描述旋转的“机器语言”
    t.transform.rotation.x = q.x();
    t.transform.rotation.y = q.y();
    t.transform.rotation.z = q.z();
    t.transform.rotation.w = q.w();

    // 【通信层】发送锁存消息 (Latched Message)
    // 就像把通知钉在公告板上，后来的人也能看到
    broadcaster_->sendTransform(t);

    RCLCPP_INFO(this->get_logger(), "Static TF Ready: %s -> %s",
                t.header.frame_id.c_str(), t.child_frame_id.c_str());
  }

  std::shared_ptr<tf2_ros::StaticTransformBroadcaster> broadcaster_;
};

}  // namespace chimera::sensing

// 【组件注册】允许该节点被动态加载，无需写 main 函数
#include <rclcpp_components/register_node_macro.hpp>
RCLCPP_COMPONENTS_REGISTER_NODE(chimera::sensing::StaticTFBroadcaster)