//
// Created by Xuan Zhai on 2024/3/8.
//
#include "S72Materials.h"
#include "VulkanHelper.h"


/**
 * @brief Given a PBR S72 Material node, Read its data into the instance.
 * @param node The PBR S72 Material node.
 */
void S72Object::Material_PBR::ProcessMaterial(const std::shared_ptr<ParserNode>& node){

    Material::ProcessMaterial(node);

    if (node->GetObjectValue("pbr") == nullptr) {
        throw std::runtime_error("It is not a pbr material!");
    }

    auto pbr = node->GetObjectValue("pbr");

    /* Read the albedo value. */
    auto albedoNode = pbr->GetObjectValue("albedo");
    if (std::get_if<ParserNode::PNVector>(&albedoNode->data) != nullptr) {
        ParserNode::PNVector color = std::get<ParserNode::PNVector>(albedoNode->data);

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
        std::string src = S72Helper::s72fileName + "/../" + std::get<std::string>(albedoNode->GetObjectValue("src")->data);
        ReadPNG(src,albedo,albedoWidth,albedoHeight,albedoChannel,albedoMipLevels);
    }

    /* Read the roughness value. */
    auto roughnessNode = pbr->GetObjectValue("roughness");
    if(std::get_if<float>(&roughnessNode->data) != nullptr){
        float r = std::get<float>(roughnessNode->data);
        roughness = std::string() + (char) (r * 256);
        roughnessWidth = 1;
        roughnessHeight = 1;
        roughnessMipLevels = static_cast<uint32_t>(std::floor(std::log2(max(roughnessWidth, roughnessHeight)))) + 1;
    }
    else{
        std::string src = S72Helper::s72fileName + "/../" + std::get<std::string>(roughnessNode->GetObjectValue("src")->data);
        int tempChannel = 0;
        ReadPNG(src,roughness,roughnessWidth,roughnessHeight,tempChannel,roughnessMipLevels);
    }

    /* Read the metallic value. */
    auto metallicNode = pbr->GetObjectValue("metalness");
    if(std::get_if<float>(&metallicNode->data) != nullptr){
        float r = std::get<float>(metallicNode->data);
        metallic = std::string() + (char) (r * 256);
        metallicWidth = 1;
        metallicHeight = 1;
        metallicMipLevels = static_cast<uint32_t>(std::floor(std::log2(max(metallicWidth, metallicHeight)))) + 1;
    }
    else{
        std::string src = S72Helper::s72fileName + "/../" + std::get<std::string>(metallicNode->GetObjectValue("src")->data);
        int tempChannel = 0;
        ReadPNG(src,metallic,metallicWidth,metallicHeight,tempChannel,metallicMipLevels);
    }
}


/**
 * @brief Create the descriptor pool for the material.
 * @param The physical device.
 */
void S72Object::Material_PBR::CreateDescriptorPool(const VkDevice& device){
    /* Describe which descriptor types our descriptor sets are going to contain */
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
 * @param device The physical device.
 * @param descriptorSetLayout The descriptor set layout.
 * @param uniformBuffers The UBO buffer.
 * @param textureSampler The sampler for the texture.
 * @param cubeMap A list of GGX cube map, seperated by the roughness.
 * @param brdfLUT The integrated BRDF LUT.
 */
void S72Object::Material_PBR::CreateDescriptorSets(const VkDevice& device, const VkDescriptorSetLayout& descriptorSetLayout,
                          const std::vector<VkBuffer>& uniformBuffers, const VkSampler& textureSampler,
                          const std::vector<VkImageView>& cubeMaps, const VkImageView& brdfLUT){

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

        std::vector<VkDescriptorImageInfo> cubeMapInfo;
        for (const auto &cubeMap: cubeMaps) {
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
        descriptorWrites[4].descriptorCount = static_cast<uint32_t>(roughnessInfo.size());
        descriptorWrites[4].pImageInfo = roughnessInfo.data();

        descriptorWrites[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[5].dstSet = descriptorSets[i];
        descriptorWrites[5].dstBinding = 5;
        descriptorWrites[5].dstArrayElement = 0;
        descriptorWrites[5].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[5].descriptorCount = static_cast<uint32_t>(metallicInfo.size());
        descriptorWrites[5].pImageInfo = metallicInfo.data();

        descriptorWrites[6].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[6].dstSet = descriptorSets[i];
        descriptorWrites[6].dstBinding = 6;
        descriptorWrites[6].dstArrayElement = 0;
        descriptorWrites[6].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[6].descriptorCount = static_cast<uint32_t>(brdfInfo.size());
        descriptorWrites[6].pImageInfo = brdfInfo.data();

        descriptorWrites[7].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[7].dstSet = descriptorSets[i];
        descriptorWrites[7].dstBinding = 7;
        descriptorWrites[7].dstArrayElement = 0;
        descriptorWrites[7].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[7].descriptorCount = static_cast<uint32_t>(cubeMapInfo.size());
        descriptorWrites[7].pImageInfo = cubeMapInfo.data();

        vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}


/**
 * @brief Deallocate and free the memory of the albedo/roughness/metallic data.
 * @param device The physical device.
 */
void S72Object::Material_PBR::CleanUp(const VkDevice& device){
    S72Object::Material::CleanUp(device);

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