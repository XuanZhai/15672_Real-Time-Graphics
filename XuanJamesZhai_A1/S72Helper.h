//
// Created by Xuan Zhai on 2024/2/2.
//

#ifndef XUANJAMESZHAI_A1_S72HELPER_H
#define XUANJAMESZHAI_A1_S72HELPER_H


#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#include <unordered_map>

#include "XZJParser.h"
#include "XZMath.h"

class Camera{

private:
    /* Node to the camera object in the s72 file. */
    std::shared_ptr<ParserNode> data = nullptr;

    XZM::vec3 cameraPos;
    XZM::vec3 targetPos;

public:
    XZM::mat4 viewMatrix;
    XZM::mat4 projMatrix;

    std::string name = "Camera";

    /* Construct a camera based on the node and its name. */
    explicit Camera(const std::shared_ptr<ParserNode>& node, const std::string& newName);
    /* Compute and set the view matrix. */
    void ComputeViewMatrix();
    /* Compute and set the project matrix. */
    void ComputeProjectionMatrix();
    /* Given the transform matrix, find its view matrix and projection matrix. */
    void ProcessCamera(const XZM::mat4& transMatrix);
};


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

    /* Construct a mesh based on the node. */
    explicit Mesh(std::shared_ptr<ParserNode>& node);
    /* Set all the mesh's variable based on the data in the s72 file. */
    void ProcessMesh();
};


class S72Helper {

private:
    /* The scene object as the root of the data structure */
    std::shared_ptr<ParserNode> root;

public:

    /* The cameras in the scene */
    std::vector<std::shared_ptr<Camera>> cameras;
    /* The mesh object, they are identified by its unique name. */
    std::map<std::string, std::shared_ptr<Mesh>> meshes;
    /* The total number of mesh instance. */
    size_t instanceCount = 0;

    /* Read and parse a s72 file from a given path. */
    void ReadS72(const std::string &filename);

    /* Reconstruct all the nodes to form a tree structure and let the scene object to be the root */
    void ReconstructRoot();

    /* Reconstruct a node and reset all its children */
    void ReconstructNode(std::shared_ptr<ParserNode>, XZM::mat4 newMat);

    void UpdateNodes(std::shared_ptr<ParserNode>&, XZM::vec3 translation, XZM::quat rotation, XZM::vec3 scale);

    /* Extract the translation data as a vec3 from a ParserNode. */
    static XZM::vec3 FindTranslation(const ParserNode&);

    /* Extract the rotation data as a quaternion from a ParserNode. */
    static XZM::quat FindRotation(const ParserNode&);

    /* Extract the scale data as a vec3 from a ParserNode. */
    static XZM::vec3 FindScale(const ParserNode&);
};


#endif //XUANJAMESZHAI_A1_S72HELPER_H
