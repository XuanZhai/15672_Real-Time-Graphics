//
// Created by Xuan Zhai on 2024/2/8.
//

#include "FrustumCulling.h"
#include "S72Helper.h"


/**
 * @brief Default constructor for the frustum.
 */
Frustum::Frustum(){
    near_right = 0;
    near_top = 0;
    near_plane = 0;
    far_plane = 0;
}


/**
 * @brief Default constructor for the AABB bounding box. It starts with the Float_Min and Float_Max.
 */
AABB::AABB() {
    b_max = XZM::vec3(-std::numeric_limits<float>::max(),-std::numeric_limits<float>::max(),-std::numeric_limits<float>::max());
    b_min = XZM::vec3(std::numeric_limits<float>::max(),std::numeric_limits<float>::max(),std::numeric_limits<float>::max());
}


/**
 * @brief Check if a mesh with a transform matrix will be culled by the camera. The function is static so that it won't
 * need to be called by an instance.
 * @param camera The view camera.
 * @param mesh The target mesh.
 * @param modelMatrix The model transform matrix for the mesh.
 * @return
 */
bool FrustumCulling::IsCulled(const std::shared_ptr<S72Object::Camera>& camera, const std::shared_ptr<S72Object::Mesh>& mesh, XZM::mat4 modelMatrix){

    float z_near = camera->frustum.near_plane;
    float z_far = camera->frustum.far_plane;
    float x_near = camera->frustum.near_right;
    float y_near = camera->frustum.near_top;

    /* Corner of the AABB bounding box */
    XZM::vec3 corners[] = {
            {mesh->boundingBox.b_min.data[0], mesh->boundingBox.b_min.data[1], mesh->boundingBox.b_min.data[2]},
            {mesh->boundingBox.b_max.data[0], mesh->boundingBox.b_min.data[1], mesh->boundingBox.b_min.data[2]},
            {mesh->boundingBox.b_min.data[0], mesh->boundingBox.b_max.data[1], mesh->boundingBox.b_min.data[2]},
            {mesh->boundingBox.b_min.data[0], mesh->boundingBox.b_min.data[1], mesh->boundingBox.b_max.data[2]},
    };

    /* Need to combine the model matrix and the view matrix to let the AABB go to the view space. */
    XZM::mat4 mvMatrix = modelMatrix * camera->viewMatrix;

    /* Transform the corner of the bounding box. */
    for (auto & corner : corners) {
        corner = mvMatrix * corner;
    }

    /* Create an oriented bounding box for the transformed AABB box. */
    OBB obb;
    obb.axes = {corners[1] - corners[0],corners[2] - corners[0],corners[3] - corners[0]},
    obb.center = corners[0] + (obb.axes[0] + obb.axes[1] + obb.axes[2]) * 0.5f ;
    obb.extents = XZM::vec3{obb.axes[0].Length(), obb.axes[1].Length(), obb.axes[2].Length() };
    obb.axes[0] = obb.axes[0] / obb.extents.data[0];
    obb.axes[1] = obb.axes[1] / obb.extents.data[1];
    obb.axes[2] = obb.axes[2] / obb.extents.data[2];
    obb.extents =  obb.extents * 0.5f;


    /* Project the OBB to the axis. */
    XZM::vec3 M;
    float MoX;
    float MoY;
    float MoZ;

    /* Projected the center of the OBB. */
    float MoC = obb.center.data[2];
    /* Apply the extent to the axis based in the center and the extent. */
    float radius = 0.0f;
    for (size_t i = 0; i < 3; i++) {
        radius += fabsf(obb.axes[i].data[2]) * obb.extents.data[i];
    }

    float obb_min = MoC - radius;
    float obb_max = MoC + radius;
    // We can skip calculating the projection here, it's known
    float m0 = z_far; // Since the frustum's direction is negative z, far is smaller than near
    float m1 = z_near;

    /* If there's no overlap, it is culled. */
    if (obb_min > m1 || obb_max < m0) {
        return true;
    }

    /* OBB normal vector cases. */
    {
        // Frustum normals
        const std::array<XZM::vec3, 4> M_normal = {
                XZM::vec3({0.0, -z_near, y_near}), // Top plane
                XZM::vec3({0.0, z_near, y_near}), // Bottom plane
                XZM::vec3({-z_near, 0.0f, x_near}), // Right plane
                XZM::vec3({z_near, 0.0f, x_near}), // Left Plane
        };

        for (size_t m = 0; m < 4; m++) {
            MoX = fabsf(M_normal[m].data[0]);
            MoY = fabsf(M_normal[m].data[1]);
            MoZ = M_normal[m].data[2];
            MoC = XZM::DotProduct(M_normal[m], obb.center);

            /* Its radius/extent is derived by the normal and axis. */
            float obb_radius = 0.0f;
            for (size_t i = 0; i < 3; i++) {
                obb_radius += fabsf(XZM::DotProduct(M_normal[m], obb.axes[i])) * obb.extents.data[i];
            }
            obb_min = MoC - obb_radius;
            obb_max = MoC + obb_radius;

            /* Apply the project to the axis and check if there's an overlap. */
            float p = x_near * MoX + y_near * MoY;
            float tau_0 = z_near * MoZ - p;
            float tau_1 = z_near * MoZ + p;

            if (tau_0 < 0.0f) {
                tau_0 *= z_far / z_near;
            }
            if (tau_1 > 0.0f) {
                tau_1 *= z_far / z_near;
            }

            if (obb_min > tau_1 || obb_max < tau_0) {
                return true;
            }
        }
    }

    /* OBB parallel to axis cases. */
    {
        for (size_t m = 0; m < 3; m++) {
            M = obb.axes[m];
            MoX = fabsf(M.data[0]);
            MoY = fabsf(M.data[1]);
            MoZ = M.data[2];
            MoC = XZM::DotProduct(M, obb.center);

            /* Since it's parallel, the radius is equal to the extent. */
            float obb_radius = obb.extents.data[m];
            obb_min = MoC - obb_radius;
            obb_max = MoC + obb_radius;

            /* Similar to the calculation above. */
            float p = x_near * MoX + y_near * MoY;
            float tau_0 = z_near * MoZ - p;
            float tau_1 = z_near * MoZ + p;
            if (tau_0 < 0.0f) {
                tau_0 *= z_far / z_near;
            }
            if (tau_1 > 0.0f) {
                tau_1 *= z_far / z_near;
            }

            if (obb_min > tau_1 || obb_max < tau_0) {
                return true;
            }
        }
    }

    /* OBB A-axis and the right axis projection case. */
    {
        for (size_t m = 0; m < 3; m++) {
            M = { 0.0f, -obb.axes[m].data[2], obb.axes[m].data[1] };
            /* Right axis to 0 so that it's parallel. */
            MoX = 0.0f;
            MoY = fabsf(M.data[1]);
            MoZ = M.data[2];
            MoC = M.data[1] * obb.center.data[1] + M.data[2] * obb.center.data[2];

            float obb_radius = 0.0f;
            for (size_t i = 0; i < 3; i++) {
                obb_radius += fabsf(XZM::DotProduct(M, obb.axes[i])) * obb.extents.data[i];
            }

            obb_min = MoC - obb_radius;
            obb_max = MoC + obb_radius;

            /* Similar to the calculation above. */
            float p = x_near * MoX + y_near * MoY;
            float tau_0 = z_near * MoZ - p;
            float tau_1 = z_near * MoZ + p;
            if (tau_0 < 0.0f) {
                tau_0 *= z_far / z_near;
            }
            if (tau_1 > 0.0f) {
                tau_1 *= z_far / z_near;
            }

            if (obb_min > tau_1 || obb_max < tau_0) {
                return true;
            }
        }
    }

    /* OBB A-axis and the Up axis projection case. */
    {
        for (size_t m = 0; m < 3; m++) {
            M = { obb.axes[m].data[2], 0.0f, -obb.axes[m].data[0] };
            MoX = fabsf(M.data[0]);
            /* Up axis to 0 so that it's parallel. */
            MoY = 0.0f;
            MoZ = M.data[2];
            MoC = M.data[0] * obb.center.data[0] + M.data[2] * obb.center.data[2];

            float obb_radius = 0.0f;
            for (size_t i = 0; i < 3; i++) {
                obb_radius += fabsf(XZM::DotProduct(M, obb.axes[i])) * obb.extents.data[i];
            }

            obb_min = MoC - obb_radius;
            obb_max = MoC + obb_radius;

            /* Similar to the calculation above. */
            float p = x_near * MoX + y_near * MoY;
            float tau_0 = z_near * MoZ - p;
            float tau_1 = z_near * MoZ + p;
            if (tau_0 < 0.0f) {
                tau_0 *= z_far / z_near;
            }
            if (tau_1 > 0.0f) {
                tau_1 *= z_far / z_near;
            }

            if (obb_min > tau_1 || obb_max < tau_0) {
                return true;
            }
        }
    }

    /* OBB A-axis and the frustum 12 edges projection case. */
    {
        for (size_t obb_edge_idx = 0; obb_edge_idx < 3; obb_edge_idx++) {
            const std::array<XZM::vec3,4> M_Edge = {
                    /* Left plane */
                    XZM::CrossProduct({-x_near, 0.0f, z_near}, obb.axes[obb_edge_idx]),
                    /* Right plane */
                    XZM::CrossProduct({ x_near, 0.0f, z_near }, obb.axes[obb_edge_idx]),
                    /* Top plane */
                    XZM::CrossProduct({ 0.0f, y_near, z_near }, obb.axes[obb_edge_idx]),
                    /* Bottom plane */
                    XZM::CrossProduct({ 0.0, -y_near, z_near }, obb.axes[obb_edge_idx])
            };

            for (size_t m = 0; m < 4; m++) {
                MoX = fabsf(M_Edge[m].data[0]);
                MoY = fabsf(M_Edge[m].data[1]);
                MoZ = M_Edge[m].data[2];

                constexpr float epsilon = 1e-4;
                if (MoX < epsilon && MoY < epsilon && fabsf(MoZ) < epsilon) continue;

                MoC = XZM::DotProduct(M_Edge[m], obb.center);

                float obb_radius = 0.0f;
                for (size_t i = 0; i < 3; i++) {
                    obb_radius += fabsf(XZM::DotProduct(M_Edge[m], obb.axes[i])) * obb.extents.data[i];
                }

                obb_min = MoC - obb_radius;
                obb_max = MoC + obb_radius;

                float p = x_near * MoX + y_near * MoY;
                float tau_0 = z_near * MoZ - p;
                float tau_1 = z_near * MoZ + p;
                if (tau_0 < 0.0f) {
                    tau_0 *= z_far / z_near;
                }
                if (tau_1 > 0.0f) {
                    tau_1 *= z_far / z_near;
                }

                if (obb_min > tau_1 || obb_max < tau_0) {
                    return true;
                }
            }
        }
    }
    return false;
}