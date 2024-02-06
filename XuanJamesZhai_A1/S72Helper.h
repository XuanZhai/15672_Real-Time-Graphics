//
// Created by Xuan Zhai on 2024/2/2.
//

#ifndef XUANJAMESZHAI_A1_S72HELPER_H
#define XUANJAMESZHAI_A1_S72HELPER_H


#include <string>
#include <vector>
#include "XZJParser.h"

#include <vulkan/vulkan.h>
#include <unordered_map>

#include "XZMath.h"

class Camera{

public:
    std::shared_ptr<ParserNode> data = nullptr;

    XZM::mat4 viewMatrix;
    XZM::mat4 projMatrix;

    XZM::vec3 cameraPos;
    XZM::vec3 targetPos;

    std::string name = "Camera";

    explicit Camera(std::shared_ptr<ParserNode>& node);

    void SetName();

    void ComputeViewMatrix();

    void ComputeProjectionMatrix();

    void ProcessCamera(const XZM::mat4& transMatrix);
};


class Mesh{

private:
    void SetSrc(const std::string& srcPath);
    void SetFormat(size_t channel, const std::string& new_Format);
    void SetTopology(const std::string& new_topology);

public:
    std::shared_ptr<ParserNode> data = nullptr;

    std::string name;
    std::string src;
    uint32_t stride;
    uint32_t count;
    VkPrimitiveTopology topology;

    VkFormat pFormat;
    VkFormat nFormat;
    VkFormat cFormat;

    uint32_t pOffset;
    uint32_t nOffset;
    uint32_t cOffset;

    explicit Mesh(std::shared_ptr<ParserNode>& node);

    void ProcessMesh();
};


class MeshInstance{

public:
    std::shared_ptr<Mesh> mesh = nullptr;
    XZM::mat4 modelMatrix = XZM::mat4();

    MeshInstance(const std::shared_ptr<Mesh>& newMesh, const XZM::mat4& newModelMatrix){
        mesh = newMesh;
        modelMatrix = newModelMatrix;
    }
};



class S72Helper {

private:
    /* The scene object as the root of the data structure */
    std::shared_ptr<ParserNode> root;

public:

    /* The cameras in the scene */
    std::vector<std::shared_ptr<Camera>> cameras;

    //std::vector<std::shared_ptr<ParserNode>> nodes;
    std::vector<MeshInstance> meshes;

   // std::vector<std::shared_ptr<ParserNode>> tracingPath;

    //std::vector<std::pair<std::shared_ptr<Mesh>,XZM::mat4>> meshInstances;


    void ReadS72(const std::string &filename);

    /* Reconstruct all the nodes to form a tree structure and let the scene object to be the root */
    void ReconstructRoot();

    /* Reconstruct a node and reset all its children */
    void ReconstructNode(std::shared_ptr<ParserNode>, XZM::mat4 newMat);

    void UpdateNodes(std::shared_ptr<ParserNode>&, XZM::vec3 translation, XZM::quat rotation, XZM::vec3 scale);

    XZM::mat4 GetModelMatrix();

    XZM::mat4 GetModelMatrix(std::shared_ptr<ParserNode> targetNode);

    static XZM::vec3 FindTranslation(const ParserNode&);

    static XZM::quat FindRotation(const ParserNode&);

    static XZM::vec3 FindScale(const ParserNode&);
};


#endif //XUANJAMESZHAI_A1_S72HELPER_H
