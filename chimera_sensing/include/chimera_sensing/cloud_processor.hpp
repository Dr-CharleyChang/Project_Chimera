#ifndef CHIMERA_SENSING_CLOUD_PROCESSOR_HPP_
#define CHIMERA_SENSING_CLOUD_PROCESSOR_HPP_

#include <span>
#include <algorithm>
#include "point_types.hpp"
#include <Eigen/Geometry>

namespace chimera::sensing
{
    inline void transformPointCloud(std::span<Point3D> cloud, const Eigen::Affine3f& transform)
    {
        const Eigen::Matrix3f rotation = transform.rotation().cast<float>();
        const Eigen::Vector3f translation = transform.translation().cast<float>();

        // Apply transformation to each point in the cloud
        for (auto& point : cloud)
        {
            Eigen::Vector3f pt(point.x, point.y, point.z);
            Eigen::Vector3f transformed_pt = rotation * pt + translation;
            point.x = transformed_pt.x();
            point.y = transformed_pt.y();
            point.z = transformed_pt.z();
        }
    }

}  // namespace chimera::sensing

#endif  // CHIMERA_SENSING_CLOUD_PROCESSOR_HPP_