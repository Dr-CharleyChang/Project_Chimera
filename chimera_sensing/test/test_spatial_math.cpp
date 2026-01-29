#include <gtest/gtest.h>
#include "chimera_sensing/spatial_utils.hpp"

// 测试套件：SpatialMathTest
TEST(SpatialMathTest, TranslationAndRotation) {
    // 1. 构造一个模拟的 ROS 消息 (Input)
    // 场景：雷达向前 x=1.0，向左旋转 90度 (Yaw=90 deg)
    geometry_msgs::msg::Transform t_msg;
    t_msg.translation.x = 1.0;
    t_msg.translation.y = 0.0;
    t_msg.translation.z = 0.0;
    
    // Yaw = 90度 -> 四元数 (0, 0, 0.707, 0.707)
    // MATLAB: angle2quat(pi/2, 0, 0, 'ZYX')
    double angle = M_PI / 2.0; 
    t_msg.rotation.z = sin(angle / 2.0); // 0.707
    t_msg.rotation.w = cos(angle / 2.0); // 0.707

    // 2. 执行转换 (Process)
    Eigen::Affine3d T = chimera::sensing::toEigen(t_msg);

    // 3. 验证 (Assert)
    // 验证平移部分
    EXPECT_NEAR(T.translation().x(), 1.0, 1e-6);
    
    // 验证矩阵变换逻辑：
    // 如果有一个点 P_laser 在雷达坐标系下是 [1, 0, 0] (雷达正前方1米)
    // 转换到车体坐标系 P_base = T * P_laser
    // 结果应该是：车体前方1米(雷达位置) + 左转后的前方(即车的左边) -> x=1, y=1
    Eigen::Vector3d p_laser(1.0, 0.0, 0.0);
    Eigen::Vector3d p_base = T * p_laser;

    // 可以在纸上画一下：
    // 雷达在 (1,0)，且转了90度朝左。
    // 雷达前方1米的点，相对于车体，应该是 (1, 1, 0)。
    EXPECT_NEAR(p_base.x(), 1.0, 1e-6); 
    EXPECT_NEAR(p_base.y(), 1.0, 1e-6);
    EXPECT_NEAR(p_base.z(), 0.0, 1e-6);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}