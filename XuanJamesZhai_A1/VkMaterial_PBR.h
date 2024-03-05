//
// Created by Xuan Zhai on 2024/3/5.
//

#ifndef XUANJAMESZHAI_A1_VKMATERIAL_PBR_H
#define XUANJAMESZHAI_A1_VKMATERIAL_PBR_H

#include "VkMaterial.h"


class VkMaterial_PBR : public VkMaterial {

public:
    VkImage albedoImage = VK_NULL_HANDLE;
    VkDeviceMemory albedoImageMemory = VK_NULL_HANDLE;
    VkImageView albedoImageView = VK_NULL_HANDLE;

    VkImage roughnessImage = VK_NULL_HANDLE;
    VkDeviceMemory roughnessImageMemory = VK_NULL_HANDLE;
    VkImageView roughnessImageView = VK_NULL_HANDLE;

    VkImage metallicImage = VK_NULL_HANDLE;
    VkDeviceMemory metallicImageMemory = VK_NULL_HANDLE;
    VkImageView metallicImageView = VK_NULL_HANDLE;

    void CreateDescriptorSetLayout() override;

    void CreateDescriptorPool() override;

    void CreateDescriptorSets(const std::vector<VkBuffer>& uniformBuffers, const VkSampler& textureSampler, const VkImageView& normalMap, const std::vector<VkImageView>& cubeMap);

    void CleanUp() override;
};


#endif //XUANJAMESZHAI_A1_VKMATERIAL_PBR_H
