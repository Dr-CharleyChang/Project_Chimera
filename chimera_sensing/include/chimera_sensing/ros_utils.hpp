#ifndef CHIMERA_SENSING__ROS_UTILS_HPP_
#define CHIMERA_SENSING__ROS_UTILS_HPP_

#include <sensor_msgs/msg/point_cloud2.hpp>
#include <span>
#include <stdexcept>  // 运行时抛出异常
#include <vector>

#include "point_types.hpp"

namespace chimera::sensing {

/**
 * @brief 从ROS消息中零拷贝解包数据（Unsafe Mode）
 * 警告：此函数设输入点云是紧密排列的XYZI格式（float32×4）
 */
inline std::span<Point3D> unpackPointCloud(
    std::vector<Point3D>& buffer, const sensor_msgs::msg::PointCloud2& msg) {
  // 1. Sanity Check
  // Point3D is 16Bytes (4 floats)
  // if the point_step of ROS messages is not 16 Bytes, it means unmatch between
  // Point3D and msgs
  if (msg.point_step != sizeof(Point3D)) {
    throw std::runtime_error(
        "PointCloud2 layout mismatch! Expected 16 bytes/point.");
  }

  // 2 Pre-allocation
  size_t num_points = msg.width * msg.height;
  if (buffer.capacity() < num_points) {
    buffer.reserve(num_points);  // reserve capacity at the first call
  }
  buffer.resize(num_points);

  // 3. Data copy
  std::memcpy(buffer.data(), msg.data.data(), msg.data.size());

  // 4. Return a span view
  return std::span<Point3D>(buffer.data(), buffer.size());
}

/**
 * @brief pack Point3D points into ROS messages
 * warning: this function assumes the input points are in XYZ format, and will
 * set intensity to 0.0f
 */
inline void packPointCloud(sensor_msgs::msg::PointCloud2& msg,
                           std::span<const Point3D> points,
                           const std::string& frame_id) {
  // 1. set header
  msg.header.stamp = rclcpp::Clock().now();
  msg.header.frame_id = frame_id;

  // 2. set meta data
  msg.height = 1;  // unorganized point cloud
  msg.width = points.size();
  msg.is_dense = true;                        // no invalid points
  msg.is_bigendian = false;                   // little-endian
  msg.point_step = sizeof(Point3D);           // 16 bytes per point
  msg.row_step = msg.point_step * msg.width;  // total bytes per row

  // 3. define fields (x, y, z, intensity)
  sensor_msgs::msg::PointField f;
  f.datatype = sensor_msgs::msg::PointField::FLOAT32;
  f.count = 1;

  msg.fields.clear();
  f.name = "x";
  f.offset = 0;
  msg.fields.push_back(f);
  f.name = "y";
  f.offset = 4;
  msg.fields.push_back(f);
  f.name = "z";
  f.offset = 8;
  msg.fields.push_back(f);
  f.name = "intensity";
  f.offset = 12;
  msg.fields.push_back(f);

  // 4. allocate data buffer
  msg.data.resize(points.size_bytes());
  std::memcpy(msg.data.data(), points.data(), points.size_bytes());
}
}  // namespace chimera::sensing

#endif  // CHIMERA_SENSING__ROS_UTILS_HPP_