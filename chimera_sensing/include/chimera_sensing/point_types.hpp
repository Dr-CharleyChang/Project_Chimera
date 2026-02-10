#ifndef CHIMERA_SENSING_POINT_TYPES_HPP_
#define CHIMERA_SENSING_POINT_TYPES_HPP_

namespace chimera::sensing {

    // RAM alignment
    // alignas(16) to make sure the structure is 16-byte aligned
    struct alignas(16) Point3D{
        float x;
        float y;
        float z;
        float intensity;
    };

}  // namespace chimera::sensing

#endif  // CHIMERA_SENSING_POINT_TYPES_HPP_