// include/chimera_perception/chimera_types.hpp
#pragma once
#include <cmath>
#include <cstdint>

// [FPGA 契约] 强制16字节对齐
struct alignas(16) LidarPoint {
  float x;
  float y;
  float z;
  float intensity;

  // 辅助函数，计算距离
  [[nodiscard]] float range() const noexcept {
    return std::sqrt(x * x + y * y + z * z);
  }
};

// 静态编译检查：如果这个断言失败，编译会直接报错停止。
// 这是架构师防止实习生改坏代码的锁。
static_assert(sizeof(LidarPoint) == 16, "lidarPoint size mismatch!");