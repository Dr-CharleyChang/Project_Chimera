#ifndef CHIMERA_SENSING__SPATIAL_UTILS_HPP_
#define CHIMERA_SENSING__SPATIAL_UTILS_HPP_

#include <geometry_msgs/msg/transform.hpp>
#include <Eigen/Geometry> 
#include <Eigen/Dense>

namespace chimera::sensing {

/**
 * @brief 核心数学转换算子
 * * 信号处理视角：
 * 这是一个 "Basis Change Operator" (基变换算子)。
 * 输入：ROS 定义的位姿 (Translation + Quaternion)
 * 输出：线性代数中的 4x4 仿射变换矩阵 (Affine Matrix)
 * * 数学模型： T = [ R  t ]
 * [ 0  1 ]
 */
inline Eigen::Affine3d toEigen(const geometry_msgs::msg::Transform& msg) {
    // 1. 提取平移 (Translation) -> 3x1 向量
    Eigen::Vector3d translation(
        msg.translation.x,
        msg.translation.y,
        msg.translation.z
    );

    // 2. 提取旋转 (Rotation) -> 四元数
    // 注意：Eigen 的四元数构造顺序是 (w, x, y, z)，这与某些库不同，需小心！
    Eigen::Quaterniond rotation(
        msg.rotation.w,
        msg.rotation.x,
        msg.rotation.y,
        msg.rotation.z
    );

    // 3. 构建仿射变换矩阵 (4x4 Homogeneous Matrix)
    // Identity() 相当于 MATLAB 的 eye(4)
    Eigen::Affine3d affine = Eigen::Affine3d::Identity();
    
    // 链式法则：先旋转，再平移 (线性代数标准顺序)
    affine.translate(translation);
    affine.rotate(rotation);

    return affine;
}

} // namespace chimera::sensing

#endif // CHIMERA_SENSING__SPATIAL_UTILS_HPP_