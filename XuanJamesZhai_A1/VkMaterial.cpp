//
// Created by Xuan Zhai on 2024/2/24.
//

#include "VkMaterial.h"
#include "VulkanHelper.h"



void VkMaterial::SetDevice(VkDevice newDevice){
    device = newDevice;
    Max_In_Flight = MAX_FRAMES_IN_FLIGHT;
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


void VkMaterial::CleanUp() {
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
    vkDestroyPipeline(device, pipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
}