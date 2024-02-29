//
// Created by Xuan Zhai on 2024/2/24.
//

#include "VkMaterial.h"


/**
 * @brief Set the physical device
 * @param newDevice The new device.
 */
void VkMaterial::SetDevice(VkDevice newDevice){
    device = newDevice;
}


/**
 * @brief Destruct the property and free the memory.
 */
void VkMaterial::CleanUp() {
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
    vkDestroyPipeline(device, pipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);

    vkDestroyImageView(device, normalImageView, nullptr);
    vkDestroyImage(device, normalImage, nullptr);
    vkFreeMemory(device, normalImageMemory, nullptr);
}