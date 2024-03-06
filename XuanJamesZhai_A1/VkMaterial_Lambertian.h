//
// Created by Xuan Zhai on 2024/2/25.
//

#ifndef XUANJAMESZHAI_A1_VKMATERIAL_LAMBERTIAN_H
#define XUANJAMESZHAI_A1_VKMATERIAL_LAMBERTIAN_H

#include "VkMaterial.h"

/**
 * @brief A subclass of VkMaterial, used for the environment and mirror material.
 */
class VkMaterial_Lambertian : public VkMaterial {

public:
    VkImage albedoImage = VK_NULL_HANDLE;
    VkDeviceMemory albedoImageMemory = VK_NULL_HANDLE;
    VkImageView albedoImageView = VK_NULL_HANDLE;


    void CreateDescriptorSetLayout() override;

    void CreateDescriptorPool() override;

    void CreateDescriptorSets(const std::vector<VkBuffer>& uniformBuffers, const VkSampler& textureSampler, const VkImageView& normalMap, const VkImageView& heightMap, const VkImageView& cubeMap);

    void CleanUp() override;
};


#endif //XUANJAMESZHAI_A1_VKMATERIAL_LAMBERTIAN_H
