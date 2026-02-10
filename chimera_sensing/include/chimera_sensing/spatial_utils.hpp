#ifndef CHIMERA_SENSING_SPATIAL_UTILS_HPP_
#define CHIMERA_SENSING_SPATIAL_UTILS_HPP_

#include <Eigen/Dense>
#include <Eigen/Geometry>
#include <geometry_msgs/msg/transform.hpp>

namespace chimera::sensing {

/**
 * @brief Kernel function to transform
 * This is a basic change operator to transform
 * input: position and orientation in ROS (Translation + Quaternion)
 * output: 4x4 transformation matrix in Eigen
 */
inline Eigen::Affine3d toEigen(const geometry_msgs::msg::Transform& msg) {
  // 1. Translation
  Eigen::Vector3d translation(msg.translation.x, msg.translation.y,
                              msg.translation.z);

  // 2. Rotation (Quaternion to Rotation Matrix)
  Eigen::Quaterniond rotation(msg.rotation.w, msg.rotation.x, msg.rotation.y,
                              msg.rotation.z);

  // 3. Combine into Affine3d
  Eigen::Affine3d affine = Eigen::Affine3d::Identity();
  // rules: rotation first, then translation
  // attention! the order is important
  affine.translate(translation);
  affine.rotate(rotation);

  return affine;
}
}  // namespace chimera::sensing

#endif  // CHIMERA_SENSING_SPATIAL_UTILS_HPP_