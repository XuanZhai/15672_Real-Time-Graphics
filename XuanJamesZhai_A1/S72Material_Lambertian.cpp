//
// Created by Xuan Zhai on 2024/3/8.
//
#include "S72Materials.h"
#include "VulkanHelper.h"


/**
 * @brief Read a node and load all the info. Overrode for the lambertian material.
 * @param node The node we want to load.
 */
void S72Object::Material_Lambertian::ProcessMaterial(const std::shared_ptr<ParserNode> &node) {

    Material::ProcessMaterial(node);

    if (node->GetObjectValue("lambertian") == nullptr) {
        throw std::runtime_error("It is not a lambertian material!");
    }

    auto lambertian = node->GetObjectValue("lambertian");
    auto newAlbedo = lambertian->GetObjectValue("albedo");

    if (std::get_if<ParserNode::PNVector>(&newAlbedo->data) != nullptr) {
        ParserNode::PNVector color = std::get<ParserNode::PNVector>(newAlbedo->data);

        float r = std::get<float>(color[0]->data);
        float g = std::get<float>(color[1]->data);
        float b = std::get<float>(color[2]->data);

        albedo = std::string() + (char) (r * 256) + (char) (g * 256) + (char) (b * 256) + (char)(255u);
        albedoHeight = 1;
        albedoWidth = 1;
        albedoChannel = 4;
        albedoMipLevels = static_cast<uint32_t>(std::floor(std::log2(max(albedoWidth, albedoHeight)))) + 1;
    }
    else {
        std::string src = S72Helper::s72fileName + "/../" + std::get<std::string>(newAlbedo->GetObjectValue("src")->data);
        ReadPNG(src,albedo,albedoWidth,albedoHeight,albedoChannel,albedoMipLevels);
    }
}


/**
 * @brief Create the descriptor pool for the material.
 * @param device The physical device.
 */
void S72Object::Material_Lambertian::CreateDescriptorPool(const VkDevice& device){
    /* Describe which descriptor types our descriptor sets are going to contain */
    std::array<VkDescriptorPoolSize,5> poolSizes{};

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
 * @brief Create the descriptor set for the lambertian material.
 * @param device The physical device.
 * @param descriptorSetLayout The descriptor set layout.
 * @param uniformBuffers The UBO buffer.
 * @param textureSampler The sampler for the texture.
 * @param cubeMap The lambertian cube map.
 */
void S72Object::Material_Lambertian::CreateDescriptorSets(const VkDevice& device, const VkDescriptorSetLayout& descriptorSetLayout,
                          const std::vector<VkBuffer> &uniformBuffers,VkSampler const &textureSampler,
                          const VkImageView& cubeMap){

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
        normalMapInfo[0].imageView = normalImageView;
        normalMapInfo[0].sampler = textureSampler;

        std::array<VkDescriptorImageInfo,1> heightMapInfo{};
        heightMapInfo[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        heightMapInfo[0].imageView = heightImageView;
        heightMapInfo[0].sampler = textureSampler;

        std::array<VkDescriptorImageInfo,1> cubeMapInfo{};
        cubeMapInfo[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        cubeMapInfo[0].imageView = cubeMap;
        cubeMapInfo[0].sampler = textureSampler;

        std::array<VkDescriptorImageInfo,1> albedoInfo{};
        albedoInfo[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        albedoInfo[0].imageView = albedoImageView;
        albedoInfo[0].sampler = textureSampler;

        std::array<VkWriteDescriptorSet, 5> descriptorWrites{};
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
        descriptorWrites[1].descriptorCount = static_cast<uint32_t>(normalMapInfo.size());
        descriptorWrites[1].pImageInfo = normalMapInfo.data();

        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[2].dstSet = descriptorSets[i];
        descriptorWrites[2].dstBinding = 2;
        descriptorWrites[2].dstArrayElement = 0;
        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[2].descriptorCount = static_cast<uint32_t>(heightMapInfo.size());
        descriptorWrites[2].pImageInfo = heightMapInfo.data();

        descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[3].dstSet = descriptorSets[i];
        descriptorWrites[3].dstBinding = 3;
        descriptorWrites[3].dstArrayElement = 0;
        descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[3].descriptorCount = static_cast<uint32_t>(albedoInfo.size());
        descriptorWrites[3].pImageInfo = albedoInfo.data();

        descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[4].dstSet = descriptorSets[i];
        descriptorWrites[4].dstBinding = 4;
        descriptorWrites[4].dstArrayElement = 0;
        descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[4].descriptorCount = static_cast<uint32_t>(cubeMapInfo.size());
        descriptorWrites[4].pImageInfo = cubeMapInfo.data();

        vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}


/**
 * @brief Deallocate and free the memory of the albedo data.
 * @param device The physical device.
 */
void S72Object::Material_Lambertian::CleanUp(const VkDevice& device){
    S72Object::Material::CleanUp(device);

    vkDestroyImageView(device, albedoImageView, nullptr);
    vkDestroyImage(device, albedoImage, nullptr);
    vkFreeMemory(device, albedoImageMemory, nullptr);
}
