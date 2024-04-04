//
// Created by Xuan Zhai on 2024/3/23.
//

#ifndef XUANJAMESZHAI_A1_VKSHADOWMAPS_H
#define XUANJAMESZHAI_A1_VKSHADOWMAPS_H


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <string>

#include <memory>
#include <map>
#include "S72Helper.h"

#include "XZMath.h"

class VulkanHelper;

/**
 * @brief A light VP matrices that will be used when creating the shadow map.
 */
struct UniformShadowObject{
    alignas(64) XZM::mat4 view;
    alignas(64) XZM::mat4 proj;
};

class VkShadowMaps {

    public:
        /* Number of lights/shadow maps. */
        uint32_t shadowCount = 0;

        /* Shader for computing the shadow map. */
        const std::string shadowVertexFileName = "Shaders/shadowMap.vert.spv";
        /* Render pass and pipeline for creating the shadow maps. */
        VkRenderPass renderPass = VK_NULL_HANDLE;
        VkPipeline shadowPipeline = VK_NULL_HANDLE;
        VkPipelineLayout shadowPipelineLayout = VK_NULL_HANDLE;
        /* Format of the shadow map. */
        VkFormat format;
        /* A list of VP matrices and target to render to. */
        std::vector<uint32_t> shadowMapSize;
        std::vector<VkPushConstantRange> pushConstantRange;
        std::vector<VkFramebuffer> shadowMapFrameBuffer;
        std::vector<UniformShadowObject> USOMatrices;
        std::vector<VkImage> shadowMapImage;
        std::vector<VkDeviceMemory> shadowMapImageMemory;
        std::vector<VkImageView> shadowMapImageView;

        VkImage defaultShadowMapImage;
        VkDeviceMemory defaultShadowMapImageMemory;
        VkImageView defaultShadowMapImageView;

        /* Create a 1x1 Shadow map as a placeholder for the descriptor set. */
        void CreateDefaultShadowMap(VulkanHelper* vulkanHelper);
        /* Create the shadow pass. */
        void CreateRenderPass(VulkanHelper* vulkanHelper);
        /* Create the pipeline for the shadow pass. */
        void CreatePipeline(VulkanHelper* vulkanHelper, const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
                            const std::vector<VkPushConstantRange>& pushConstants);
        /* Create the image views for the shadow maps. */
        void CreateShadowMapImageAndView(VulkanHelper* vulkanHelper, uint32_t size);
        /* Create the frame buffers for the image views. */
        void CreateFrameBuffer(const VkDevice& device);
        /* Set the VP matrices. */
        void SetViewAndProjectionMatrix(const S72Object::Light& light);
        /* Create the push constant for the VP matrices. */
        void CreatePushConstant();
        /* Dealloc the resources.*/
        void CleanUp(const VkDevice& device);
};


#endif //XUANJAMESZHAI_A1_VKSHADOWMAPS_H
