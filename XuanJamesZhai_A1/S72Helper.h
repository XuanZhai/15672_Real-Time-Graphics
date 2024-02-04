//
// Created by Xuan Zhai on 2024/2/2.
//

#ifndef XUANJAMESZHAI_A1_S72HELPER_H
#define XUANJAMESZHAI_A1_S72HELPER_H


#include <string>
#include <vector>
#include "XZJParser.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vulkan/vulkan.h>
#include <unordered_map>


class Camera{

public:
    std::shared_ptr<ParserNode> data = nullptr;

    glm::mat4 viewMatrix{};
    glm::mat4 projMatrix{};

    std::string name = "hello";

    explicit Camera(std::shared_ptr<ParserNode>& node);

    void SetName();

    void ComputeViewMatrix();

    void ComputeProjectionMatrix();
};


class Mesh{
    std::shared_ptr<ParserNode> data = nullptr;

public:
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
    void SetSrc(const std::string& srcPath);
    void SetFormat(size_t channel, const std::string& new_Format);
    void SetTopology(const std::string& new_topology);
};



class S72Helper {

private:
    /* The scene object as the root of the data structure */
    std::shared_ptr<ParserNode> root;

public:

    /* The cameras in the scene */
    std::vector<std::shared_ptr<Camera>> cameras;

    //std::vector<std::shared_ptr<ParserNode>> nodes;
    std::vector<std::shared_ptr<Mesh>> meshes;


    void ReadS72(const std::string &filename);

    /* Reconstruct all the nodes to form a tree structure and let the scene object to be the root */
    void ReconstructRoot();

    /* Reconstruct a node and reset all its children */
    void ReconstructNode(std::shared_ptr<ParserNode>&, glm::vec3 translation, glm::quat rotation, glm::vec3 scale);

    void UpdateNodes(std::shared_ptr<ParserNode>&, glm::vec3 translation, glm::quat rotation, glm::vec3 scale);

    static glm::vec3 FindTranslation(const ParserNode&);

    static glm::quat FindRotation(const ParserNode&);

    static glm::vec3 FindScale(const ParserNode&);
};


#endif //XUANJAMESZHAI_A1_S72HELPER_H
