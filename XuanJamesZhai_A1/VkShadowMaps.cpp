//
// Created by Xuan Zhai on 2024/3/23.
//

#include "VkShadowMaps.h"
#include "VulkanHelper.h"


/**
* @brief Read a file and store its data into a vector.
* Used for reading the spv binary file for vertex shaders and fragment shaders.
* @param[in] filename The file path and name of the input.
* @return The generated char array.
*/
static std::vector<char> ReadFile(const std::string& filename) {
    /* Read the file starting at the end of the file and as a binary file */
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    /* Use end of the file to determine the file size */
    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    /* Go back to the start pos and read the whole file */
    file.seekg(0);
    file.read(buffer.data(), (std::streamsize)fileSize);
    file.close();

    return buffer;
}


void VkShadowMaps::CreateRenderPass(VulkanHelper* vulkanHelper){

    format = vulkanHelper->FindDepthFormat();

    std::array<VkAttachmentDescription,1> attachments{};

    // Depth attachment (shadow map)
    attachments[0].format = format;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    attachments[0].flags = 0;

    // Attachment references from sub pass
    VkAttachmentReference depthAttachmentRef;
    depthAttachmentRef.attachment = 0;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // Subpass 0: shadow map rendering
    VkSubpassDescription subPass = {};
    subPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subPass.flags = 0;
    subPass.inputAttachmentCount = 0;
    subPass.pInputAttachments = VK_NULL_HANDLE;
    subPass.colorAttachmentCount = 0;
    subPass.pColorAttachments = VK_NULL_HANDLE;
    subPass.pResolveAttachments = VK_NULL_HANDLE;
    subPass.pDepthStencilAttachment = &depthAttachmentRef;
    subPass.preserveAttachmentCount = 0;
    subPass.pPreserveAttachments = VK_NULL_HANDLE;

    std::array<VkSubpassDependency, 2> dependencies{};

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    // Create render pass
    VkRenderPassCreateInfo renderPassInfo;
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.pNext = VK_NULL_HANDLE;
    renderPassInfo.attachmentCount = attachments.size();
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subPass;
    //renderPassInfo.dependencyCount = 0;
    //renderPassInfo.pDependencies =VK_NULL_HANDLE;
    renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
    renderPassInfo.pDependencies = dependencies.data();
    renderPassInfo.flags = 0;

    if (vkCreateRenderPass(vulkanHelper->device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}


void VkShadowMaps::CreatePipeline(VulkanHelper* vulkanHelper, const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
                                  const std::vector<VkPushConstantRange>& pushConstants){
    /* Read the file into a char array */
    auto vertShaderCode = ReadFile(shadowVertexFileName);

    /* Create modules for shaders to wrap the file*/
    VkShaderModule vertShaderModule = vulkanHelper->CreateShaderModule(vertShaderCode);

    /* Assign the shaders to a specific pipeline stage. with module and start function name */
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo};

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    /* Describe the rasterizer in the pipeline */
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;     // False = discard and True = Clamp
    rasterizer.rasterizerDiscardEnable = VK_FALSE;  // True = disables any output to the framebuffer.
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;  // Fill the area of the polygon with fragments.
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_NONE;   // Culling mode.
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;         // We update this since we change the Y-flip
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

    /* Describe the multisampling features in the pipeline */
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    /* Enable the depth test */
    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;        // If the new depth that passed should be written to the depth buffer
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 0;

    /* Specify the layout, the 'uniform' item in the pipeline */
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    if(descriptorSetLayouts.empty()){
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
    }
    else{
        pipelineLayoutInfo.setLayoutCount = descriptorSetLayouts.size();
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    }

    if(pushConstants.empty()){
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;
    }
    else{
        pipelineLayoutInfo.pushConstantRangeCount = pushConstants.size();
        pipelineLayoutInfo.pPushConstantRanges = pushConstants.data();
    }

    if (vkCreatePipelineLayout(vulkanHelper->device, &pipelineLayoutInfo, nullptr, &shadowPipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    /* Specify the dynamic states in the pipeline */
    std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_VERTEX_INPUT_EXT,VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY};

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    /* Describe the final pipeline */
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 1;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = VK_NULL_HANDLE;    // Vertex Input State is nullptr cuz we do it dynamically.
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = shadowPipelineLayout;       // Layout
    pipelineInfo.renderPass = renderPass;       // Render pass
    pipelineInfo.subpass = 0;

    /* Option to create derived pipeline */
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional

    if (vkCreateGraphicsPipelines(vulkanHelper->device, VK_NULL_HANDLE, 1, &pipelineInfo, VK_NULL_HANDLE, &shadowPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(vulkanHelper->device, vertShaderModule, nullptr);
}


void VkShadowMaps::CreateSyncObject(const VkDevice& device){

    /* Create the info for the semaphore and the fence */
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;     // Make it signaled to avoid the first-frame dilemma

    if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &signalSemaphore) != VK_SUCCESS||
        vkCreateFence(device, &fenceInfo, nullptr, &fence) != VK_SUCCESS)  {
        throw std::runtime_error("failed to create synchronization objects for the shadow map!");
    }
}


void VkShadowMaps::CreateShadowMapImageAndView(VulkanHelper* vulkanHelper, uint32_t size){
    shadowMapSize.emplace_back(size);

    shadowMapImage.emplace_back();
    shadowMapImageMemory.emplace_back();

    format = vulkanHelper->FindDepthFormat();

    vulkanHelper->CreateImage(size, size, 1, 1, format,
                              VK_IMAGE_TILING_OPTIMAL,
                              VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |  VK_IMAGE_USAGE_SAMPLED_BIT,
                              0, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, shadowMapImage.back(), shadowMapImageMemory.back());


    shadowMapImageView.emplace_back(vulkanHelper->CreateImageView(shadowMapImage.back(),
                                                                  format,VK_IMAGE_VIEW_TYPE_2D,
                                                                  VK_IMAGE_ASPECT_DEPTH_BIT,1,1));
    shadowCount++;
}


void VkShadowMaps::ComputeViewAndProjectionMatrix(float fov, float light_near, float light_far, const XZM::vec3& pos, const XZM::vec3& dir){

    XZM::mat4 viewMatrix;
    viewMatrix = XZM::RotateMat4(viewMatrix, XZM::vec3(1,0,0) , -dir.data[0]);
    viewMatrix = XZM::RotateMat4(viewMatrix, XZM::vec3(0,1,0) , -dir.data[1]);
    viewMatrix = XZM::RotateMat4(viewMatrix, XZM::vec3(0,0,1) , -dir.data[2]);
    viewMatrix = viewMatrix * XZM::Translation(pos);

    XZM::mat4 clip;
    clip.data[1][1] = -1.0f;
    clip.data[2][2] = -1.0f;
    clip.data[2][3] = -1.0f;

    XZM::mat4 light_projection = clip * XZM::Perspective(fov,1,light_near,light_far);
    light_projection.data[1][1] *= -1;

    USOMatrices.emplace_back();
    USOMatrices.back().view = viewMatrix;
    USOMatrices.back().proj = light_projection;
}


void VkShadowMaps::CreateFrameBuffer(const VkDevice& device){
    VkFramebufferCreateInfo framebufferInfo;
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.pNext = VK_NULL_HANDLE;
    framebufferInfo.renderPass = renderPass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = &shadowMapImageView.back();
    framebufferInfo.width = shadowMapSize.back();
    framebufferInfo.height = shadowMapSize.back();
    framebufferInfo.layers = 1;
    framebufferInfo.flags = 0;

    shadowMapFrameBuffer.emplace_back();
    if (vkCreateFramebuffer(device, &framebufferInfo, VK_NULL_HANDLE, &shadowMapFrameBuffer.back()) != VK_SUCCESS) {
        throw std::runtime_error("failed to create framebuffer!");
    }
}


void VkShadowMaps::CreatePushConstant(){
    pushConstantRange.emplace_back();
    pushConstantRange.back().stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // Shader stages that can access push constants
    pushConstantRange.back().offset = 0; // Offset into push constant memory
    pushConstantRange.back().size = sizeof(UniformShadowObject); // Size of push constant data
}


void VkShadowMaps::CleanUp(const VkDevice& device){

    vkDestroySemaphore(device, signalSemaphore, nullptr);
    vkDestroyFence(device,fence, nullptr);

    for(uint32_t i = 0; i < shadowCount; i++){
        vkDestroyFramebuffer(device, shadowMapFrameBuffer[i], nullptr);
        vkDestroyImageView(device, shadowMapImageView[i], nullptr);
        vkDestroyImage(device, shadowMapImage[i], nullptr);
        vkFreeMemory(device, shadowMapImageMemory[i], nullptr);
    }

    vkDestroyPipeline(device, shadowPipeline, nullptr);
    vkDestroyPipelineLayout(device, shadowPipelineLayout, nullptr);
    vkDestroyRenderPass(device, renderPass, nullptr);
    vkDestroyCommandPool(device, commandPool, nullptr);
}