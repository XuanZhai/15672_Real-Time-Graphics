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



/* A macro to select if we want to create a window using WSI. If false it will be created with GLFW. */
#define ISWINWINDOW false

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;


const std::string MODEL_PATH = "models/viking_room.obj";
const std::string TEXTURE_PATH = "textures/viking_room.png";

/* How many frames should be processed concurrently */
const int MAX_FRAMES_IN_FLIGHT = 2;

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
#endif // DEBUG

/* Structure of a Vertex which is a position and a color */
struct Vertex {
    XZM::vec3 pos;
    XZM::vec3 color;
    XZM::vec3 normal;

    /* Override the == operator for comparison */
    bool operator==(const Vertex& other) const {
        return pos == other.pos && color == other.color && normal == other.normal;
    }
};


/* MVP data for the vertices */
struct UniformBufferObject {
    alignas(16) XZM::mat4 model;
    alignas(16) XZM::mat4 view;
    alignas(16) XZM::mat4 proj;
};


/* ===================================================================================== */


class VulkanHelper {
private:

    /* The GLFW Window instance */
    GLFWwindow* window = nullptr;

    /* The WIN Window instance */
    HWND hwnd = nullptr;

    /* Thw WIN Window handle instance */
    HINSTANCE hInstance = nullptr;

    /* Vulkan instance */
    VkInstance instance = VK_NULL_HANDLE;

    /* Vulkan Debug unil */
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

    /* Used to specify the 'uniform' values in the pipline; the uniform and push values referenced by the shader that can be updated at draw time */
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

    /* An instance of the render pipline */
    VkPipeline graphicsPipeline = VK_NULL_HANDLE;

    /* The frame buffer instances. Each for an image in the swap chain */
    std::vector<VkFramebuffer> swapChainFramebuffers;

    /* Manage the command buffer */
    VkCommandPool commandPool = VK_NULL_HANDLE;

    /* Store the commands like the drawing operation */
    std::vector<VkCommandBuffer> commandBuffers;

    /* Semaphore and fence to synchronize the Swapchain operations and waiting for the previous frame to finish */
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;

    /* keep track of the current frame in flight */
    uint32_t currentFrame = 0;

    /* Used to handle the explicit window resize event */
    bool framebufferResized = false;

    /* The vertex buffer used to store the vertex data */
    //VkBuffer vertexBuffer = VK_NULL_HANDLE;
    std::vector<VkBuffer> vertexBuffers;

    /* Handle to the memory of the vertex buffer */
    //VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;
    std::vector<VkDeviceMemory> vertexBufferMemories;

    /* The index buffer used to store the indices */
    //VkBuffer indexBuffer = VK_NULL_HANDLE;

    /* Handle to the memory of the index buffer */
    //VkDeviceMemory indexBufferMemory = VK_NULL_HANDLE;

    /* Specifies the types of resources that are going to be accessed by the pipeline like the MVP matrix */
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;

    /* Buffer that contains UBO data */
    std::vector<VkBuffer> uniformBuffers;

    /* Memory that is allocated for the uniform buffer */
    std::vector<VkDeviceMemory> uniformBuffersMemory;

    /* A reference map to the uniform buffer which can put data into them */
    std::vector<void*> uniformBuffersMapped;

    /* Handle to the descriptor pool */
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;

    /* Handles to the desciptor sets */
    std::vector<VkDescriptorSet> descriptorSets;

    /* The image of the texture */
    VkImage textureImage = VK_NULL_HANDLE;

    /* The mipmap level of the texture */
    uint32_t mipLevels = 0;

    /* The handle to the memory of the texture */
    VkDeviceMemory textureImageMemory = VK_NULL_HANDLE;

    /* The image view to access the texture image */
    VkImageView textureImageView = VK_NULL_HANDLE;

    /* The texture sampler instance to sample the texture image */
    VkSampler textureSampler = VK_NULL_HANDLE;

    /* An image of the depth */
    VkImage depthImage = VK_NULL_HANDLE;

    /* The memory who holds the depth image */
    VkDeviceMemory depthImageMemory = VK_NULL_HANDLE;

    /* The image view of the depth image */
    VkImageView depthImageView = VK_NULL_HANDLE;

    /* A list of vertex which are vertices to draw */
    //std::vector<Vertex> vertices;

    /* indices to arrange the order of vertices to draw */
    //std::vector<uint32_t> indices;

    std::shared_ptr<S72Helper> s72Instance = nullptr;

    std::shared_ptr<Camera> currCamera = nullptr;

    /* A struct of queue that will be submitted to Vulkan */
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;        // Queue family index. Use optional to see if it has a value
        std::optional<uint32_t> presentFamily;      // Queue family that's used to present on the window

        bool isComplete() const {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    /* A struct of details of Swap Chain */
    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;      // Basic surface capabilities (min/max number of images in swap chain, min/max width and height of images)
        std::vector<VkSurfaceFormatKHR> formats;    // Surface formats (pixel format, color space)
        std::vector<VkPresentModeKHR> presentModes; // Available presentation modes
    };


    static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);


    void InitWindow();


    void InitWindowWIN(HINSTANCE new_hInstance, HWND new_hwnd);


    SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice newDevice);


    static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);


    static VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);


    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);


    static std::vector<const char*> GetRequiredExtensions();


    static bool CheckValidationLayerSupport();


    static void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);


    void SetupDebugMessenger();


    void CreateInstance();


    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice newDevice);


    bool IsDeviceSuitable(VkPhysicalDevice newDevice);


    static bool CheckDeviceExtensionSupport(VkPhysicalDevice newDevice);


    void PickPhysicalDevice();


    void CreateLogicalDevice();


    void CreateSurface();


    void CreateSwapChain();


    VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);


    void CreateImageViews();


    VkShaderModule CreateShaderModule(const std::vector<char>& code);


    void CreateDescriptorSetLayout();


    void CreateGraphicsPipeline();


    void CreateRenderPass();


    void CreateFrameBuffers();


    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);


    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);


    VkCommandBuffer BeginSingleTimeCommands();


    void EndSingleTimeCommands(VkCommandBuffer commandBuffer);


    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);


    void CreateVertexBuffer(const Mesh& newMesh, size_t index);


    void CreateVertexBuffers();

    static VkVertexInputBindingDescription2EXT CreateBindingDescription(const Mesh& newMesh);

    static std::array<VkVertexInputAttributeDescription2EXT, 3> CreateAttributeDescription(const Mesh& newMesh);


    void CreateIndexBuffer();


    void CreateUniformBuffers();


    void UpdateUniformBuffer(uint32_t currentImage,size_t index);


    void CreateDescriptorPool();


    void CreateDescriptorSets();


    void CreateCommandPool();


    void CreateCommandBuffers();


    void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);


    void CreateImage(uint32_t width, uint32_t height, uint32_t newMipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);


    void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);


    void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t newMipLevels);


    void CreateTextureImage();


    void CreateTextureImageView();


    void CreateTextureSampler();


    void CreateSyncObjects();


    VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);


    VkFormat FindDepthFormat();


    static bool HasStencilComponent(VkFormat format);


    void CreateDepthResources();


    void LoadModel();


    void CleanUpSwapChain();


    void RecreateSwapChain();


    void GenerateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t newMipLevels);


    void InitVulkan();


    void MainLoop();


    static void MainLoopWIN();


public:


    void Run();


    void RunWIN(HINSTANCE new_Instance, HWND new_hwnd);

    void Run(const std::shared_ptr<S72Helper>& news72Instance);

    void UpdateFrame();


    void DrawFrame();


    void CleanUp();
};


#endif //XUANJAMESZHAI_A1_VULKANHELPER_H
