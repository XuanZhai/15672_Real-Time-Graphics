//
// Created by Xuan Zhai on 2024/2/8.
//

#ifndef XUANJAMESZHAI_A1_FRUSTUMCULLING_H
#define XUANJAMESZHAI_A1_FRUSTUMCULLING_H

#include "XZMath.h"
#include <limits>
#include <memory>

namespace S72Object{
    class Camera;
    class Mesh;
}

/* Reference: https://bruop.github.io/improved_frustum_culling/ */

/**
 * @brief The camera's frustum, it has data to do the projection for the SAT.
 */
class Frustum{
public:
    float near_right;
    float near_top;
    float near_plane;
    float far_plane;

    Frustum();
};


/**
 * @brief A bounding box for the mesh. It has the min/max value at each axis.
 */
class AABB{
public:
    XZM::vec3 b_min = {};
    XZM::vec3 b_max = {};

    AABB();
};


/**
 * @brief Oriented Bounding Box. The volume to the oriented mesh to apply the Separating Axis Theorem
 */
class OBB{
public:
    XZM::vec3 center = {};
    XZM::vec3 extents = {};
    std::array<XZM::vec3,3> axes = {};
};


class FrustumCulling {
public:
    /* Check if a mesh with a transform matrix will be culled by the camera. */
    static bool IsCulled(const std::shared_ptr<S72Object::Camera>& camera, const std::shared_ptr<S72Object::Mesh>& mesh, XZM::mat4 modelMatrix);
};


#endif //XUANJAMESZHAI_A1_FRUSTUMCULLING_H
