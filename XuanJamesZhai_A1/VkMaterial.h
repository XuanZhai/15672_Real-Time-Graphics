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

struct UniformBufferObject;


class VkMaterial {

public:
    std::string name;

    VkDevice device;

    VkPipeline pipeline = VK_NULL_HANDLE;

    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;

    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;

    std::vector<VkDescriptorSet> descriptorSets;

    void SetDevice(VkDevice newDevice);

    void CreateDescriptorSetLayout();

    void CreateGraphicsPipeline();

    void CreateDescriptorPool();

    void CreateDescriptorSets(const VkSampler& textureSampler, const std::vector<VkBuffer>& uniformBuffers, const VkImageView& cubeMap);

    void CleanUp();
};


#endif //XUANJAMESZHAI_A1_VKMATERIAL_H
