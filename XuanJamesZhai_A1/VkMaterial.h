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

class VkMaterial {

protected:
    VkDevice device = VK_NULL_HANDLE;

    uint32_t Max_In_Flight = 1;

    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;

public:
    std::string name;

    VkPipeline pipeline = VK_NULL_HANDLE;

    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;

    std::vector<VkDescriptorSet> descriptorSets;

    void SetDevice(VkDevice newDevice);

    virtual void CreateDescriptorSetLayout();

    virtual void CreateDescriptorPool();

    //virtual void CreateDescriptorSets(const VkSampler& textureSampler, const std::vector<VkBuffer>& uniformBuffers, const VkImageView& cubeMap);

    virtual void CleanUp();
};


#endif //XUANJAMESZHAI_A1_VKMATERIAL_H
