#include <gtest/gtest.h>
#include <vector>
#include <Eigen/Dense>

// 引入我们的核心库
#include "chimera_sensing/spatial_utils.hpp"
#include "chimera_sensing/point_types.hpp"
#include "chimera_sensing/cloud_processor.hpp"

// --- Case 1: 单个矩阵逻辑测试 ---
TEST(SpatialMathTest, TranslationAndRotation) {
    geometry_msgs::msg::Transform t_msg;
    t_msg.translation.x = 1.0;
    
    // 绕 Z 轴转 90 度 (四元数)
    double angle = M_PI / 2.0; 
    t_msg.rotation.z = sin(angle / 2.0); 
    t_msg.rotation.w = cos(angle / 2.0); 

    Eigen::Affine3d T = chimera::sensing::toEigen(t_msg);

    // 验证逻辑：(1,0,0) -> 旋转90度 -> (0,1,0) -> 平移x+1 -> (1,1,0)
    Eigen::Vector3d p_laser(1.0, 0.0, 0.0);
    Eigen::Vector3d p_base = T * p_laser;

    EXPECT_NEAR(p_base.x(), 1.0, 1e-6); 
    EXPECT_NEAR(p_base.y(), 1.0, 1e-6);
}

// --- Case 2: 批量处理引擎测试 ---
TEST(SpatialMathTest, BatchProcessing) {
    // 1. 造数据 (Mock Data)
    size_t num_points = 1000;
    std::vector<chimera::sensing::Point3D> cloud(num_points);
    
    // 初始化点云：排成一条直线 x=0,1,2...
    for (size_t i = 0; i < num_points; ++i) {
        cloud[i] = {static_cast<float>(i), 0.0f, 0.0f, 1.0f}; 
    }

    // 2. 造变换
    // 简单的平移：x + 10.0
    Eigen::Affine3f T = Eigen::Affine3f::Identity();
    T.translate(Eigen::Vector3f(10.0f, 0.0f, 0.0f));

    // 3. 执行批量变换
    // [重点] std::vector 自动隐式转换为 std::span 传入函数
    chimera::sensing::transformPointCloud(cloud, T);

    // 4. 验证
    // 第0个点: 0 -> 10
    EXPECT_NEAR(cloud[0].x, 10.0f, 1e-4);
    // 第1个点: 1 -> 11
    EXPECT_NEAR(cloud[1].x, 11.0f, 1e-4);
    // 尾部点: 999 -> 1009
    EXPECT_NEAR(cloud[999].x, 1009.0f, 1e-4);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}