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
 * @brief A Vulkan-side material base object. Contains the pipeline and the descriptor info.
 */
class VkMaterial {

protected:
    /* The device it is working one. */
    VkDevice device = VK_NULL_HANDLE;

    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;

public:
    std::string name;

    VkPipeline pipeline = VK_NULL_HANDLE;

    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;

    std::vector<VkDescriptorSet> descriptorSets;

    VkImage normalImage = VK_NULL_HANDLE;

    VkDeviceMemory normalImageMemory = VK_NULL_HANDLE;

    VkImageView normalImageView = VK_NULL_HANDLE;

    VkImage heightImage = VK_NULL_HANDLE;

    VkDeviceMemory heightImageMemory = VK_NULL_HANDLE;

    VkImageView heightImageView = VK_NULL_HANDLE;

    /* Set the physical device. */
    void SetDevice(VkDevice newDevice);

    /* Create the descriptor set layout. */
    virtual void CreateDescriptorSetLayout() = 0;

    /* Create the descriptor pool. */
    virtual void CreateDescriptorPool() = 0;

    /* Destructor. */
    virtual void CleanUp();
};


#endif //XUANJAMESZHAI_A1_VKMATERIAL_H
