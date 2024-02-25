//
// Created by Xuan Zhai on 2024/2/24.
//

#include "VkMaterial.h"
#include "VulkanHelper.h"



void VkMaterial::SetDevice(VkDevice newDevice){
    device = newDevice;
}


void VkMaterial::CreateDescriptorSetLayout(){

    /* Set the binding info for ubo */
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;    // The type of descriptor is a uniform buffer object
    uboLayoutBinding.descriptorCount = 1;

    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr;

    std::vector<VkDescriptorSetLayoutBinding> bindings;

    if(name == "simple"){
        bindings.emplace_back(uboLayoutBinding);
    }
    else if(name == "environment" || name == "mirror"){
        /* Set the environment cube map sampler */
        VkDescriptorSetLayoutBinding envLayoutBinding{};
        envLayoutBinding.binding = 1;
        envLayoutBinding.descriptorCount = 1;
        envLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        envLayoutBinding.pImmutableSamplers = nullptr;
        envLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;     // Use the image sampler in the fragment shader stage.

        // TODO: May need the normal map.
        bindings.emplace_back(uboLayoutBinding);
        bindings.emplace_back(envLayoutBinding);
    }
    else if(name == "lambertian"){
        /* Set the environment cube map sampler */
        //VkDescriptorSetLayoutBinding envLayoutBinding{};
        //envLayoutBinding.binding = 1;
        //envLayoutBinding.descriptorCount = 1;
        //envLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        //envLayoutBinding.pImmutableSamplers = nullptr;
        //envLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;     // Use the image sampler in the fragment shader stage.
    }

    /* Combine all the bindings into a single object */
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}


void VkMaterial::CreateDescriptorPool(){

    /* Describe which descriptor types our descriptor sets are going to contain */
    /* The first is used for the uniform buffer. The second is used for the image sampler */
    std::vector<VkDescriptorPoolSize> poolSizes;

    VkDescriptorPoolSize uboPool;
    uboPool.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    uboPool.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    if(name == "simple"){
        poolSizes.emplace_back(uboPool);
    }
    else if(name == "environment" || name == "mirror"){
        VkDescriptorPoolSize cubeMapPool;
        cubeMapPool.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        cubeMapPool.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

        poolSizes.emplace_back(uboPool);
        poolSizes.emplace_back(cubeMapPool);
    }

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


void VkMaterial::CreateDescriptorSets(const VkSampler& textureSampler, const std::vector<VkBuffer>& uniformBuffers, const VkImageView& cubeMap){
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


        std::vector<VkDescriptorImageInfo> imageInfo;

        if(name == "simple"){

        }
        else if(name == "environment" || name == "mirror"){
            VkDescriptorImageInfo cubeMapInfo;
            cubeMapInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            cubeMapInfo.imageView = cubeMap;
            cubeMapInfo.sampler = textureSampler;
            imageInfo.emplace_back(cubeMapInfo);
        }


        /* =========================================================================== */

        std::array<VkWriteDescriptorSet,5> descriptorWrites{};
        uint32_t sizeUsed = 0;

        if(name == "simple"){
            //VkWriteDescriptorSet uboWrite;
            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = descriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;

            sizeUsed = 1;
            //descriptorWrites.emplace_back(uboWrite);
        }
        else if(name == "environment" || name == "mirror"){

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
            descriptorWrites[1].descriptorCount = imageInfo.size();
            descriptorWrites[1].pImageInfo = imageInfo.data();

            sizeUsed = 2;
        }

        vkUpdateDescriptorSets(device, sizeUsed, descriptorWrites.data(), 0, nullptr);
    }
}


void VkMaterial::CleanUp() {
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
    vkDestroyPipeline(device, pipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
}