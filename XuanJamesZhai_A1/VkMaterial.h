//
// Created by Xuan Zhai on 2024/2/24.
//

#ifndef XUANJAMESZHAI_A1_VKMATERIAL_H
#define XUANJAMESZHAI_A1_VKMATERIAL_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include <vector>
#include <stdexcept>
#include <unordered_map>
#include <array>
#include "S72Helper.h"

/**
 * @brief A Vulkan-side material base object. Contains the pipeline and the descriptor layout.
 */
class VkMaterial {

protected:

    bool operator < (const VkMaterial& newVkMat) const;

public:
    std::string name;

    VkPipeline pipeline = VK_NULL_HANDLE;

    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

    VkDescriptorSetLayout VKMDescriptorSetLayout = VK_NULL_HANDLE;

    VkDescriptorPool VKMDescriptorPool = VK_NULL_HANDLE;

    std::vector<VkDescriptorSet> VKMDescriptorSets;

    /* Create the descriptor set layout. */
    virtual void CreateDescriptorSetLayout(const VkDevice& device) = 0;
    /* Create the descriptor pool. */
    virtual void CreateDescriptorPool(const VkDevice& device) = 0;

    /* Destructor. */
    virtual void CleanUp(const VkDevice& device);
};


/**
 * @brief A subclass of VkMaterial, used for the simple color material.
 */
class VkMaterial_Simple : public VkMaterial{

public:
    void CreateDescriptorSetLayout(const VkDevice& device) override;
    void CreateDescriptorPool(const VkDevice& device) override;
};


/**
 * @brief A subclass of VkMaterial, used for the environment and mirror material.
 */
class VkMaterial_EnvironmentMirror : public VkMaterial {

public:
    void CreateDescriptorSetLayout(const VkDevice& device) override;
    void CreateDescriptorPool(const VkDevice& device) override;
    void CreateDescriptorSets(const VkDevice& device, VkSampler const &textureSampler, VkImageView const &cubeMap);
};


/**
 * @brief A subclass of VkMaterial, used for the Lambertian material.
 */
class VkMaterial_Lambertian : public VkMaterial {

public:
    void CreateDescriptorSetLayout(const VkDevice& device) override;
    void CreateDescriptorPool(const VkDevice& device) override;
    void CreateDescriptorSets(const VkDevice& device, VkSampler const &textureSampler, VkImageView const &cubeMap);
};


/**
 * @brief A subclass of VkMaterial, used for the PBR material.
 */
class VkMaterial_PBR : public VkMaterial {
public:
    void CreateDescriptorSetLayout(const VkDevice& device) override;
    void CreateDescriptorPool(const VkDevice& device) override;
    void CreateDescriptorSets(const VkDevice& device, VkSampler const &textureSampler, VkImageView const &LamCubeMap, const std::vector<VkImageView>& cubeMaps, const VkImageView& brdfLUT);
};



#endif //XUANJAMESZHAI_A1_VKMATERIAL_H
