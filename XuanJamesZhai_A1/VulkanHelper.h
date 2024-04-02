//
// Created by Xuan Zhai on 2024/2/1.
//

#ifndef XUANJAMESZHAI_A1_VULKANHELPER_H
#define XUANJAMESZHAI_A1_VULKANHELPER_H

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <windows.h>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <cstring>
#include <optional>
#include <set>
#include <cstdint> // Necessary for uint32_t
#include <limits> // Necessary for std::numeric_limits
#include <algorithm> // Necessary for std::clamp
#include <fstream>
#include <array>
#include <chrono>
#include <unordered_map>
#include <cmath>

#include "S72Helper.h"
#include "EventHelper.h"
#include "VkMaterial.h"
#include "VkMesh.h"
#include "VkShadowMaps.h"



/* A macro to select if we want to create a window using WSI. If false it will be created with GLFW. */
#define USE_WSI false


/* A map of materials and its shader files. */
const std::unordered_map<S72Object::EMaterial, std::array<std::string,2>> shaderMap = {
        {S72Object::EMaterial::simple, {"Shaders/simple.vert.spv","Shaders/simple.frag.spv"}},
        {S72Object::EMaterial::environment, {"Shaders/environment.vert.spv","Shaders/environment.frag.spv"}},
        {S72Object::EMaterial::mirror, {"Shaders/mirror.vert.spv","Shaders/mirror.frag.spv"}},
        {S72Object::EMaterial::lambertian, {"Shaders/lambertian.vert.spv","Shaders/lambertian.frag.spv"}},
        {S72Object::EMaterial::pbr, {"Shaders/pbr.vert.spv","Shaders/pbr.frag.spv"}}
};

/* The number of PBR environment maps. */
const int GGX_LEVELS = 10;

/* How many frames should be processed concurrently */
const int MAX_FRAMES_IN_FLIGHT = 3;

/* Type of the validation layer we used */
const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };

/* A list of required device extensions for swap chain */
const std::vector<const char*> deviceExtensions = {
        VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME,
        VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME,
        VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME,
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif // NDEBUG


const size_t MAX_NUM_LIGHTS = 10;


/* Camera ubo data */
struct UniformBufferObject {
    alignas(64) XZM::mat4 view;
    alignas(64) XZM::mat4 proj;
    alignas(64) XZM::vec3 viewPos;
};


/* Light ubo data. */
struct UniformLight {
    /* 0 = sun, 1 = sphere, 2 = spot */
    alignas(4) uint32_t type = 0;
    alignas(4) float angle = 0;
    alignas(4) float strength = 0;
    alignas(4) float radius = 0;
    alignas(4) float power = 0;
    alignas(4) float limit = 0;
    alignas(4) float fov = 0;
    alignas(4) float blend = 0;
    alignas(16) XZM::vec3 pos = XZM::vec3();
    alignas(16) XZM::vec3 dir = XZM::vec3();
    alignas(16) XZM::vec3 tint = XZM::vec3(1.0f,1.0f,1.0f);
    alignas(16) XZM::mat4 view;
    alignas(16) XZM::mat4 proj;
};

/* A container of light data. */
struct UniformLights {
    alignas(4) uint32_t lightSize = 0;
    alignas(16) std::array<UniformLight,MAX_NUM_LIGHTS> lights;
};


/* ===================================================================================== */


class VulkanHelper {
private:

    /* The display window width */
    uint32_t windowWidth = 1920;

    /* The display window height */
    uint32_t windowHeight = 1080;

    /* The name of the physical device that will be used */
    std::string deviceName;

    /* The GLFW Window instance */
    GLFWwindow* window = nullptr;

    /* The WIN Window instance */
    HWND hwnd = nullptr;

    /* Thw WIN Window handle instance */
    HINSTANCE hInstance = nullptr;

    /* Vulkan instance */
    VkInstance instance = VK_NULL_HANDLE;

    /* Vulkan Debug util */
    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;

    /* Vulkan physical device handle, will be implicitly destroyed */
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    /* Vulkan logical device handle */
    VkDevice device = VK_NULL_HANDLE;

    /* Handles for graphics queue */
    VkQueue graphicsQueue = VK_NULL_HANDLE;

    /* Window Surface instance */
    VkSurfaceKHR surface = VK_NULL_HANDLE;

    /* The presentation queue and retrieve the VkQueue handle */
    VkQueue presentQueue = VK_NULL_HANDLE;

    /* The Swap Chain object for presentation */
    VkSwapchainKHR swapChain = VK_NULL_HANDLE;

    /* Images retrieved from the swap chain */
    std::vector<VkImage> swapChainImages;

    /* Store the swap chain format */
    VkFormat swapChainImageFormat;

    /* Store the swap chain extent*/
    VkExtent2D swapChainExtent;

    /* Use to access the swap chain images in the VKImage */
    std::vector<VkImageView> swapChainImageViews;

    /* Vulkan render pass; the attachments referenced by the pipeline stages and their usage */
    VkRenderPass renderPass = VK_NULL_HANDLE;

    /* The frame buffer instances. Each for an image in the swap chain */
    std::vector<VkFramebuffer> swapChainFramebuffers;

    /* Manage the command buffer */
    VkCommandPool commandPool = VK_NULL_HANDLE;

    /* Store the commands like the drawing operation */
    std::vector<VkCommandBuffer> commandBuffers;

    /* Semaphore and fence to synchronize the swap chain operations and waiting for the previous frame to finish */
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;

    /* keep track of the current frame in flight */
    uint32_t currentFrame = 0;

    /* Used to handle the explicit window resize event */
    bool framebufferResized = false;

    /* A map of VkMeshes hold all the vertex info in the GPU. */
    std::unordered_map<std::string,std::shared_ptr<VkMesh>> VkMeshes;

    /* Global descriptor set to store all the ubo data. */
    VkDescriptorSetLayout globalDescriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool globalDescriptorPool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> globalDescriptorSets;

    /* Buffer that contains camera UBO data */
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;

    /* Buffer that contains light UBO data */
    std::vector<VkBuffer> uniformLightBuffers;
    std::vector<VkDeviceMemory> uniformLightBuffersMemory;
    std::vector<void*> uniformLightBuffersMapped;

    /* A map of VkMaterials hold all the material info in the GPU. */
    std::map<std::shared_ptr<VkMaterial>,std::vector<std::shared_ptr<S72Object::Material>>> VkMaterials;

    /* The filename of the environment cube map. */
    std::string envFileName;

    /* Data for the environment map. */
    VkImage envTextureImage = VK_NULL_HANDLE;
    VkDeviceMemory envTextureImageMemory = VK_NULL_HANDLE;
    VkImageView envTextureImageView = VK_NULL_HANDLE;

    /* Data for the lambertian environment map. */
    VkImage lamTextureImage = VK_NULL_HANDLE;
    VkDeviceMemory lamTextureImageMemory = VK_NULL_HANDLE;
    VkImageView lamTextureImageView = VK_NULL_HANDLE;

    /* Data for the pre-compute PBR environment map. */
    std::vector<VkImage> pbrTextureImage; 
    std::vector<VkDeviceMemory> pbrTextureImageMemory;
    std::vector<VkImageView> pbrTextureImageView;

    /* Data for pre-compute BRDF. */
    VkImage pbrBRDFImage;
    VkDeviceMemory pbrBRDFImageMemory;
    VkImageView pbrBRDFImageView;

    /* The texture sampler instance to sample the texture image */
    VkSampler textureSampler = VK_NULL_HANDLE;

    /* An image of the depth */
    VkImage depthImage = VK_NULL_HANDLE;

    /* The memory who holds the depth image */
    VkDeviceMemory depthImageMemory = VK_NULL_HANDLE;

    /* The image view of the depth image */
    VkImageView depthImageView = VK_NULL_HANDLE;

    /* Contain the mesh data and the scene graph */
    std::shared_ptr<S72Helper> s72Instance = nullptr;

    /* Refers to the camera instance that is using currently */
    std::shared_ptr<S72Object::Camera> currCamera = nullptr;

    /* The culling mode used for rendering. Can be none or frustum. */
    std::string cullingMode;

    /* Set if we are doing the off-screen rendering. */
    bool useHeadlessRendering = false;

    /* Memory allocated for the off-screen image. */
    std::vector<VkDeviceMemory> headlessImageMemory;

    /* data mapped to the headless memory. */
    std::vector<void*> headlessImageMapped;

    /* The current rendered image into headless list. */
    uint32_t headlessImageIndex = 0;
    /* Refers to the instance of VkShadowMaps */

    std::shared_ptr<VkShadowMaps> shadowMaps = nullptr;


    /* A struct of queue that will be submitted to Vulkan */
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;        // Queue family index. Use optional to see if it has a value
        std::optional<uint32_t> presentFamily;      // Queue family that's used to present on the window

        bool isComplete(bool isHeadless) const {
            if(isHeadless) return graphicsFamily.has_value();
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    /* A struct of details of the Swap Chain */
    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;      // Basic surface capabilities (min/max number of images in swap chain, min/max width and height of images)
        std::vector<VkSurfaceFormatKHR> formats;    // Surface formats (pixel format, color space)
        std::vector<VkPresentModeKHR> presentModes; // Available presentation modes
    };

    /* A callback function use to detect if a window is resized. */
    static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);

    /* Initialize the window without GLFW. Assign the HWND instance and the HINSTANCE to the member variable. */
    void InitWindowWIN(HINSTANCE new_hInstance, HWND new_hwnd);

    /* Populate the detail of swap chain. */
    SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice newDevice);

    /* Select the Swap Chain surface format. */
    static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

    /* Select the Swap Chain present mode. */
    static VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

    /* The swap extent is the resolution of the swap chain images. */
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    /* Get the list of extensions based on whether validation layers are enabled or not. */
    static std::vector<const char*> GetRequiredExtensions();

    /* Check if there is a valid validation layer. */
    static bool CheckValidationLayerSupport();

    /* Construct the VkDebugUtilsMessengerCreateInfoEXT with flags. */
    static void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

    /* Set up the debugger tools used for the Vulkan application. */
    void SetupDebugMessenger();

    /* Create a Vulkan instance. */
    void CreateInstance();

    /* Get the queue family indices that support the given device. */
    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice newDevice);

    /* Check if the physical device is suitable for the vulkan rendering. */
    bool IsDeviceSuitable(VkPhysicalDevice newDevice);

    /* Verify that your graphics card is indeed capable of creating a swap chain. */
    static bool CheckDeviceExtensionSupport(VkPhysicalDevice newDevice);

    /* Pick the graphics card that fits for the need for the Vulkan rendering. */
    void PickPhysicalDevice();

    /* Create the logical device for the Vulkan rendering. */
    void CreateLogicalDevice();

    /* Create the window surface that will be used for display the output. */
    void CreateSurface();

    /* Create the swap chain that will be used for display the output. */
    void CreateSwapChain();

    /* Create the swap chain as a list of images for the headless mode. */
    void CreateHeadlessSwapChain();

    /* Create the image instance. */
    void CreateImage(uint32_t width, uint32_t height, uint32_t newMipLevels, uint32_t newArrayLayers, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

    /* Create an image view instance based on the image and its format. */
    VkImageView CreateImageView(VkImage image, VkFormat format, VkImageViewType viewType, VkImageAspectFlags aspectFlags, uint32_t newMipLevels, uint32_t layerCount);

    /* Create the Image Views that will handle the images in the swap chain. */
    void CreateImageViews();

    /* Create a module to wrap the info in the binary shader vector. */
    VkShaderModule CreateShaderModule(const std::vector<char>& code);

    /* Read the shaders and create the graphics pipeline. */
    void CreateGraphicsPipeline(const VkRenderPass& newRenderPass, const std::string& vertexFileName,
                                const std::string& fragmentFileName, const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
                                const std::vector<VkPushConstantRange>& pushConstants, VkPipeline& graphicsPipeline, VkPipelineLayout& pipelineLayout);

    /* Create the render pass to attach the framebuffer. */
    void CreateRenderPass();

    /* Create the frame buffer which has the data to present. */
    void CreateFrameBuffers();

    /* Combine the requirements of the buffer and our own application requirements to find the right type of memory to use. */
    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    /* Create a buffer, can be used to create the vertex buffer and the index buffer. */
    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

    /* Start a command buffer. Will be ended in a separate function. */
    VkCommandBuffer BeginSingleTimeCommands();

    /* End a given command buffer. */
    void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

    /* Copy a vulkan buffer from source to destination. */
    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    /* Fill the VkMesh map based on the data in the s72Instance. */
    void CreateMeshes();

    /* Create the vertex buffer to store the vertex data. */
    void CreateVertexBuffer(const S72Object::Mesh& newMesh, VkMesh& vkMesh);

    /* For a binding description struct based on the info of a mesh instance. */
    static std::array<VkVertexInputBindingDescription2EXT,2> CreateBindingDescription(const S72Object::Mesh& newMesh);

    /* Form an attribute description struct based on the info of a mesh instance. */
    static std::array<VkVertexInputAttributeDescription2EXT, 9> CreateAttributeDescription(const S72Object::Mesh& newMesh);

    /* Create the index buffer to store the index relations. */
    void CreateIndexBuffer(const S72Object::Mesh& newMesh, VkMesh& vkMesh);

    /* Create a single dynamic instance buffer for a mesh. */
    void CreateInstanceBuffer(VkMesh& vkMesh);

    /* Update an instance buffer with the new instance data. */
    void UpdateInstanceBuffer(const S72Object::Mesh& newMesh);

    /* Create the uniform buffer to store the general uniform data. */
    void CreateUniformBuffers();

    /* Update the uniform buffer on the current image. */
    void UpdateUniformBuffer(uint32_t currentImage);

    /* Create the uniform buffer to store the light uniform data. */
    void CreateUniformLightBuffers();

    /* Update the light uniform buffer on the current image. */
    void UpdateUniformLightBuffers(uint32_t currentImage);

    /* Create the descriptor set layout, pool, and sets for the global descriptor set. */
    void CreateGlobalDescriptorSets();

    /* Create the command pool which will be used to allocate the memory for the command buffer. */
    void CreateCommandPool(VkCommandPool& newCommandPool);

    /* Create the command buffer which can submit the drawing command. */
    void CreateCommandBuffers(VkCommandPool& newCommandPool, std::vector<VkCommandBuffer>& newCommandBuffers);

    /* Render the shadow passes. */
    void RenderShadowPass(VkCommandBuffer commandBuffer);

    /* Writes the commands we want to execute into a command buffer. */
    void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    /* Copy a VkBuffer to a VkImage. */
    void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

    /* Copy a VkBuffer which contains a cube map to a VkImage. */
    void CopyBufferToImageCube(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height,uint32_t nChannel);

    /* Transit the image's layout with a new layout using a pipeline barrier. */
    void TransitionImageLayout(VkImage image,uint32_t layerCount, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t newMipLevels);

    /* Convert a RGBE Image to a float RGB Image. */
    static void ProcessRGBEImage(const unsigned char* src, float*& dst, int texWidth, int texHeight);

    /* Create the VkImage and the VkImageView for a cube map. */
    void CreateCubeTextureImageAndView(const std::string& filename, VkImage& image, VkDeviceMemory& imageMemory, VkImageView& imageView);

    /* Create the VkImage and the VkImageView for pre-compute BRDF LUT. */
    void CreateBRDFImageAndView(const std::string& filename);

    /* Create the three environment cube maps. */
    void CreateEnvironments();

    /* Fill the VkMaterial list based on the material data in the s72Instance. */
    void CreateMaterials();

    /* Create the texture image with a given texture. */
    void CreateTextureImage(const std::string& src, int texWidth, int texHeight, int nChannels, uint32_t mipLevels, VkImage& textureImage, VkDeviceMemory& textureImageMemory);

    /* Create the image view to access and present the texture image. */
    void CreateTextureImageView(const VkImage& textureImage, VkImageView& textureImageView, int nChannels, uint32_t mipLevels);

    /* Given A S72 Material, Create its VkImage and VkImageView. */
    void CreateMaterialImageView(const std::shared_ptr<S72Object::Material>& newMat);

    /* Create the texture sampler to access the texture. */
    void CreateTextureSampler();

    /* Create all the semaphores and fences used for sync updating the command buffers. */
    void CreateSyncObjects();

    /* Find the supported vkFormat that supports the tiling mode and the features. */
    VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

    /* Select a format with a depth component that supports usage as depth attachment. */
    VkFormat FindDepthFormat();

    /* Check if a format contains a stencil component. (Can be used for the depth test) */
    static bool HasStencilComponent(VkFormat format);

    /* Create the depth image and the image view. */
    void CreateDepthResources();

    /* Copy a VkImage to a mapped array through a staging buffer. */
    void CopyImageToData(const VkImage& image, const VkDeviceMemory& imageMemory, void*& data, uint32_t imageWidth, uint32_t imageHeight);

    /* Same a VKImage to a PPM file with a given name. */
    void SaveImageToPPM(const VkImage& image, const VkDeviceMemory& imageMemory, const std::string&, uint32_t imageWidth, uint32_t imageHeight);

    /* Clean up the swap chain and all the related resources. */
    void CleanUpSwapChain();

    /* Recreate the swap chain if the window is changed and the previous one become invalid. */
    void RecreateSwapChain();

    /* Generate the mipmaps through the command buffer. */
    void GenerateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t newMipLevels, uint32_t newLayerCount);

    /* Create a main loop to let the window keep opening (Looping using the MSG mechanism). */
    static void MainLoopWIN();

    /* Initialize the GLFW window used for the Vulkan Display. */
    void InitWindow();

    /* Initialize the Vulkan application and setup. */
    void InitVulkan();

    /* Initialize the shadow map data. */
    void InitShadowMaps();

    /* Update the VP matrices to produce new shadow maps. */
    void UpdateShadowMaps();

    /* Draw the frame and submit the command buffer. */
    void DrawFrame();

    /* CleanUp used for destroy the instance and related destruction. */
    void CleanUp();

public:

    /* Make the render helper can access the vulkan helper's private properties. */
    friend class RenderHelper;

    friend class VkShadowMaps;

    /* Set the s72helper with a new instance. */
    void SetS72Instance(const std::shared_ptr<S72Helper>& s72Instance);

    /* Set the vulkan's window size. */
    void SetWindowSize(uint32_t width, uint32_t height);

    /* Set a specific physical device name. */
    void SetDeviceName(const std::string& deviceName);

    /* Set a specific camera name. */
    void SetCameraName(const std::string& cameraName);

    /* Set a specific culling mode. */
    void SetCullingMode(const std::string& cullingMode);

    /* Set if we use the off-screen rendering. */
    void SetHeadlessMode(bool);

    /* Save the rendered image to a ppm file. Only work if it's the off-screen rendering. */
    void SaveRenderResult(const std::string& filename);

    /* Run the Vulkan application using the WSI. */
    void RunWIN(HINSTANCE new_Instance, HWND new_hwnd);

    /* Handle the GLFW input based on the key the user typed. */
    void ProcessGLFWInputCallBack(char key);
};


#endif //XUANJAMESZHAI_A1_VULKANHELPER_H
