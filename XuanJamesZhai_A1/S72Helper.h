//
// Created by Xuan Zhai on 2024/2/2.
//

#ifndef XUANJAMESZHAI_A1_S72HELPER_H
#define XUANJAMESZHAI_A1_S72HELPER_H


#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#include <unordered_map>
#include <chrono>

#include "XZJParser.h"
#include "XZMath.h"
#include "FrustumCulling.h"


/**
 * @brief A camera object listed in the s72 file.
 */
class Camera{

private:
    /* Node to the camera object in the s72 file. */
    std::shared_ptr<ParserNode> data = nullptr;

    XZM::vec3 cameraPos;
    /* The normalized vector the camera is facing. */
    XZM::vec3 cameraDir;

    /* Fundamental data for a camera, use to build the projection matrix. */
    float aspect;
    float v_fov;
    float near_z;
    float far_z;

public:
    XZM::mat4 viewMatrix;
    XZM::mat4 projMatrix;

    std::string name = "Camera";

    /* If this camera can be controlled by the keyboard input. */
    bool isMovable = false;

    /* The frustum based on the camera's view angle and distance. */
    Frustum frustum;

    Camera();
    /* Construct a camera based on the node and its name. */
    explicit Camera(std::shared_ptr<ParserNode> node, const std::string& newName);
    /* Set the data for the camera. */
    void SetCameraData(float newAspect, float new_V_fov, float newNear, float newFar);
    /* Compute and set the view matrix. */
    void ComputeViewMatrix();
    /* Compute and set the project matrix. */
    void ComputeProjectionMatrix();
    /* Given the transform matrix, find its view matrix and projection matrix. */
    void ProcessCamera(const XZM::mat4& transMatrix);
    /* Move the camera position based along its facing direction. */
    void MoveCameraForwardBackward(bool isForward);
    /* Rotate the camera around the world right direction. */
    void MoveCameraUpDown(bool isUp);
    /* Rotate the camera around the world up direction. */
    void MoveCameraLeftRight(bool isRight);
    /* Let the camera loop toward the world center. */
    void ReFocusToCenter();
};


/**
 * @brief A mesh object listed in the s72 file.
 */
class Mesh{

private:
    /* Node to the mesh object in the s72 file. */
    std::shared_ptr<ParserNode> data = nullptr;

    /* Load the mesh data from a b72 file given its path. */
    void SetSrc(const std::string& srcPath);
    /* Given the position,normal,color channel, set its corresponding format. */
    void SetFormat(size_t channel, const std::string& new_Format);
    /* Find its corresponding topology and set it in the instance variable. */
    void SetTopology(const std::string& new_topology);

public:
    /* Name will be used as the identifier of the mesh object. */
    std::string name;
    std::string src;
    uint32_t stride;
    uint32_t count;
    VkPrimitiveTopology topology;
    /* Position, Normal, and Color Vulkan formats */
    VkFormat pFormat;
    VkFormat nFormat;
    VkFormat cFormat;
    /* Position, Normal, and Color Vulkan offsets */
    uint32_t pOffset;
    uint32_t nOffset;
    uint32_t cOffset;
    /* A list of mesh instance, they are represented by its unique model matrix. */
    std::vector<XZM::mat4> instances;

    /* An AABB bounding box for the mesh. */
    AABB boundingBox;

    /* Construct a mesh based on the node. */
    explicit Mesh(std::shared_ptr<ParserNode>& node);
    /* Set all the mesh's variable based on the data in the s72 file. */
    void ProcessMesh();
    /* Given a mesh's b72 data, read and set the mesh's bounding box. */
    void ReadBoundingBox(std::stringstream& buffer);
};


class Driver{
public:
    int nodeIndex = 0;
    std::string channel;
    std::vector<float> timers;
    std::vector<std::variant<XZM::vec3,XZM::quat>> values;

    void Initialization(const std::shared_ptr<ParserNode>& node);
    std::variant<XZM::vec3,XZM::quat> GetCurrentData(float currTime);
    std::string HasMatchNodeAndChannel(const std::shared_ptr<ParserNode>& node);
};



class S72Helper {

private:
    /* The scene object as the root of the data structure */
    std::shared_ptr<ParserNode> root;

public:

    /* The cameras in the scene */
    std::map<std::string, std::shared_ptr<Camera>> cameras;
    /* The mesh object, they are identified by its unique name. */
    std::map<std::string, std::shared_ptr<Mesh>> meshes;
    /* The total number of mesh instance. */
    size_t instanceCount = 0;


    std::chrono::steady_clock::time_point startTimePoint;
    float currTime = 0;

    std::vector<std::shared_ptr<Driver>> movingNodes;

    S72Helper();
    /* Read and parse a s72 file from a given path. */
    void ReadS72(const std::string &filename);
    /* Reconstruct all the nodes to form a tree structure and let the scene object to be the root */
    void ReconstructRoot();
    /* Reconstruct a node and reset all its children */
    void ReconstructNode(std::shared_ptr<ParserNode>, XZM::mat4 newMat);
    /* Recursively update all object's transform data. */
    void UpdateObjects();
    /* Update an object's transform data and visit its children. */
    void UpdateObject(const std::shared_ptr<ParserNode>&, XZM::mat4 newMat);
    /* Extract the translation data as a vec3 from a ParserNode. */
    static XZM::vec3 FindTranslation(const ParserNode&);
    /* Extract the rotation data as a quaternion from a ParserNode. */
    static XZM::quat FindRotation(const ParserNode&);
    /* Extract the scale data as a vec3 from a ParserNode. */
    static XZM::vec3 FindScale(const ParserNode&);
};


#endif //XUANJAMESZHAI_A1_S72HELPER_H
