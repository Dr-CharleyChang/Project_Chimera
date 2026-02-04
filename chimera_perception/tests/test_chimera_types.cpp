#include <gtest/gtest.h>

#include <ranges>

#include "chimera_types.hpp"

// Test Case 1: 验证内存对齐 (FPGA Contract)
TEST(ChimeraTypesTest, MemoryAlignment) {
  // 必须是 16 字节 (128-bit)
  EXPECT_EQ(sizeof(LidarPoint), 16);
  // 内存地址必须是 16 字节对齐的
  EXPECT_EQ(alignof(LidarPoint), 16);
}

// Test Case 2: 验证 C++20 Ranges 逻辑
TEST(ChimeraLogicTest, ZFilterLogic) {
  std::vector<LidarPoint> input_data = {
      {0, 0, 10.0f, 0},  // Noise
      {0, 0, 1.0f, 0},   // Valid
      {0, 0, 3.0f, 0}    // Noise
  };

  std::vector<LidarPoint> output_data;

  // 复制我们在 Node 里写的逻辑 (Unit Test 应该测试逻辑单元)
  auto z_filter = [](const auto& p) { return p.z < 2.0f; };

  auto view = input_data | std::views::filter(z_filter);
  std::ranges::copy(view, std::back_inserter(output_data));

  // 断言：应该只剩 1 个点
  EXPECT_EQ(output_data.size(), 1);
  // 断言：剩下的点 z 应该是 1.0
  EXPECT_FLOAT_EQ(output_data[0].z, 1.0f);
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}