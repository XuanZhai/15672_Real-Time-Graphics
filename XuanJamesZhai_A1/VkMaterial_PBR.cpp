//
// Created by Xuan Zhai on 2024/3/5.
//

#include "VkMaterial_PBR.h"
#include "VulkanHelper.h"

/**
 * @brief Create the descriptor set layout for the material.
 */
void VkMaterial_PBR::CreateDescriptorSetLayout() {

    std::array<VkDescriptorSetLayoutBinding,8> bindings{};

    /* Set the binding info for ubo */
    bindings[0].binding = 0;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;    // The type of descriptor is a uniform buffer object
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    bindings[0].pImmutableSamplers = nullptr;

    /* Set the normal map sampler */
    bindings[1].binding = 1;
    bindings[1].descriptorCount = 1;
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[1].pImmutableSamplers = nullptr;
    bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;     // Use the image sampler in the fragment shader stage.

    /* Set the height map sampler */
    bindings[2].binding = 2;
    bindings[2].descriptorCount = 1;
    bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[2].pImmutableSamplers = nullptr;
    bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;     // Use the image sampler in the fragment shader stage.

    /* Set the albedo map sampler */
    bindings[3].binding = 3;
    bindings[3].descriptorCount = 1;
    bindings[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[3].pImmutableSamplers = nullptr;
    bindings[3].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;     // Use the image sampler in the fragment shader stage.

    /* Set the roughness map sample. */
    bindings[4].binding = 4;
    bindings[4].descriptorCount = 1;
    bindings[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[4].pImmutableSamplers = nullptr;
    bindings[4].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;     // Use the image sampler in the fragment shader stage.

    /* Set the metallic map sample. */
    bindings[5].binding = 5;
    bindings[5].descriptorCount = 1;
    bindings[5].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[5].pImmutableSamplers = nullptr;
    bindings[5].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;     // Use the image sampler in the fragment shader stage.

    /* Set the brdf map sampler */
    bindings[6].binding = 6;
    bindings[6].descriptorCount = 1;
    bindings[6].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[6].pImmutableSamplers = nullptr;
    bindings[6].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;     // Use the image sampler in the fragment shader stage.

    /* Set the cube map sampler */
    bindings[7].binding = 7;
    bindings[7].descriptorCount = 10;
    bindings[7].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[7].pImmutableSamplers = nullptr;
    bindings[7].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;     // Use the image sampler in the fragment shader stage.

    /* Combine all the bindings into a single object */
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}


/**
 * @brief Create the descriptor pool for the material.
 */
void VkMaterial_PBR::CreateDescriptorPool() {
    /* Describe which descriptor types our descriptor sets are going to contain */
    /* The first is used for the uniform buffer. The second is used for the image sampler */
    std::array<VkDescriptorPoolSize,8> poolSizes{};

    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[2].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    poolSizes[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[3].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    poolSizes[4].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[4].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    poolSizes[5].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[5].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    poolSizes[6].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[6].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    poolSizes[7].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[7].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * 10;

    /* Create the pool info for allocation */
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}


/**
 * @brief Create the descriptor set for the environment/mirror material.
 * @param uniformBuffers The UBO buffer.
 * @param textureSampler The sampler for the texture.
 * @param cubeMap A list of GGX cube map, seperated by the roughness.
 */
void VkMaterial_PBR::CreateDescriptorSets(const std::vector<VkBuffer> &uniformBuffers, VkSampler const &textureSampler, VkImageView const &normalMap, const VkImageView& heightMap, const std::vector<VkImageView>& cubeMaps, const VkImageView& brdfLUT) {
    /* Create one descriptor set for each frame in flight */
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    /* Configure each descriptor set */
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        std::array<VkDescriptorImageInfo,1> normalMapInfo{};
        normalMapInfo[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        normalMapInfo[0].imageView = normalMap;
        normalMapInfo[0].sampler = textureSampler;

        std::array<VkDescriptorImageInfo,1> heightMapInfo{};
        heightMapInfo[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        heightMapInfo[0].imageView = heightMap;
        heightMapInfo[0].sampler = textureSampler;

        std::vector<VkDescriptorImageInfo> cubeMapInfo;
        for(const auto& cubeMap : cubeMaps){
            VkDescriptorImageInfo newInfo;
            cubeMapInfo.emplace_back(newInfo);
            cubeMapInfo.back().imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            cubeMapInfo.back().imageView = cubeMap;
            cubeMapInfo.back().sampler = textureSampler;
        }

        std::array<VkDescriptorImageInfo,1> brdfInfo{};
        brdfInfo[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        brdfInfo[0].imageView = brdfLUT;
        brdfInfo[0].sampler = textureSampler;

        std::array<VkDescriptorImageInfo,1> albedoInfo{};
        albedoInfo[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        albedoInfo[0].imageView = albedoImageView;
        albedoInfo[0].sampler = textureSampler;

        std::array<VkDescriptorImageInfo,1> roughnessInfo{};
        roughnessInfo[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        roughnessInfo[0].imageView = roughnessImageView;
        roughnessInfo[0].sampler = textureSampler;

        std::array<VkDescriptorImageInfo,1> metallicInfo{};
        metallicInfo[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        metallicInfo[0].imageView = metallicImageView;
        metallicInfo[0].sampler = textureSampler;

        std::array<VkWriteDescriptorSet, 8> descriptorWrites{};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = normalMapInfo.size();
        descriptorWrites[1].pImageInfo = normalMapInfo.data();

        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[2].dstSet = descriptorSets[i];
        descriptorWrites[2].dstBinding = 2;
        descriptorWrites[2].dstArrayElement = 0;
        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[2].descriptorCount = heightMapInfo.size();
        descriptorWrites[2].pImageInfo = heightMapInfo.data();

        descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[3].dstSet = descriptorSets[i];
        descriptorWrites[3].dstBinding = 3;
        descriptorWrites[3].dstArrayElement = 0;
        descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[3].descriptorCount = albedoInfo.size();
        descriptorWrites[3].pImageInfo = albedoInfo.data();

        descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[4].dstSet = descriptorSets[i];
        descriptorWrites[4].dstBinding = 4;
        descriptorWrites[4].dstArrayElement = 0;
        descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[4].descriptorCount = roughnessInfo.size();
        descriptorWrites[4].pImageInfo = roughnessInfo.data();

        descriptorWrites[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[5].dstSet = descriptorSets[i];
        descriptorWrites[5].dstBinding = 5;
        descriptorWrites[5].dstArrayElement = 0;
        descriptorWrites[5].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[5].descriptorCount = metallicInfo.size();
        descriptorWrites[5].pImageInfo = metallicInfo.data();

        descriptorWrites[6].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[6].dstSet = descriptorSets[i];
        descriptorWrites[6].dstBinding = 6;
        descriptorWrites[6].dstArrayElement = 0;
        descriptorWrites[6].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[6].descriptorCount = brdfInfo.size();
        descriptorWrites[6].pImageInfo = brdfInfo.data();

        descriptorWrites[7].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[7].dstSet = descriptorSets[i];
        descriptorWrites[7].dstBinding = 7;
        descriptorWrites[7].dstArrayElement = 0;
        descriptorWrites[7].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[7].descriptorCount = cubeMapInfo.size();
        descriptorWrites[7].pImageInfo = cubeMapInfo.data();

        vkUpdateDescriptorSets(device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
    }
}

void VkMaterial_PBR::CleanUp() {
    VkMaterial::CleanUp();

    vkDestroyImageView(device, albedoImageView, nullptr);
    vkDestroyImage(device, albedoImage, nullptr);
    vkFreeMemory(device, albedoImageMemory, nullptr);

    vkDestroyImageView(device, roughnessImageView, nullptr);
    vkDestroyImage(device, roughnessImage, nullptr);
    vkFreeMemory(device, roughnessImageMemory, nullptr);

    vkDestroyImageView(device, metallicImageView, nullptr);
    vkDestroyImage(device, metallicImage, nullptr);
    vkFreeMemory(device, metallicImageMemory, nullptr);
}
