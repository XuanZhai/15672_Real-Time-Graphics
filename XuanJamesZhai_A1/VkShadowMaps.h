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

struct UniformShadowObject{
    alignas(64) XZM::mat4 view;
    alignas(64) XZM::mat4 proj;
};

class VkShadowMaps {

    public:
        uint32_t shadowCount = 0;
        VkRenderPass renderPass = VK_NULL_HANDLE;
        const std::string shadowVertexFileName = "Shaders/shadowMap.vert.spv";
        const std::string shadowFragmentFileName = "Shaders/shadowMap.frag.spv";

        VkPipeline shadowPipeline = VK_NULL_HANDLE;
        VkPipelineLayout shadowPipelineLayout = VK_NULL_HANDLE;
        std::vector<VkPushConstantRange> pushConstantRange;
        std::vector<VkFramebuffer> shadowMapFrameBuffer;
        std::vector<UniformShadowObject> USOMatrices;
        VkSemaphore signalSemaphore;
        VkFence fence;
        VkFormat format;

        std::vector<VkImage> shadowMapImage;
        std::vector<VkDeviceMemory> shadowMapImageMemory;
        std::vector<VkImageView> shadowMapImageView;

        VkCommandPool commandPool = VK_NULL_HANDLE;
        std::vector<VkCommandBuffer> commandBuffer;

        std::vector<uint32_t> shadowMapSize;


        void CreateRenderPass(VulkanHelper* vulkanHelper);

        void CreatePipeline(VulkanHelper* vulkanHelper, const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
                            const std::vector<VkPushConstantRange>& pushConstants);

        void CreateShadowMapImageAndView(VulkanHelper* vulkanHelper, uint32_t size);
        void SetViewAndProjectionMatrix(const S72Object::Light& light);
        void CreateFrameBuffer(const VkDevice& device);

        void CreatePushConstant();

        void CreateSyncObject(const VkDevice& device);

        void CleanUp(const VkDevice& device);
};


#endif //XUANJAMESZHAI_A1_VKSHADOWMAPS_H
