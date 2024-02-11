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
#include "FrustumCulling.h"



/* A macro to select if we want to create a window using WSI. If false it will be created with GLFW. */
#define USE_WSI false

const std::string MODEL_PATH = "models/viking_room.obj";
const std::string TEXTURE_PATH = "Textures/viking_room.png";

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
#endif // NDEBUG

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
    alignas(16) XZM::mat4 transposeModel;
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

    /* Used to specify the 'uniform' values in the pipeline; the uniform and push values referenced by the shader that can be updated at draw time */
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

    /* An instance of the render pipeline */
    VkPipeline graphicsPipeline = VK_NULL_HANDLE;

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

    /* The vertex buffers used to store the vertex data */
    std::vector<VkBuffer> vertexBuffers;

    /* Handle to the memory of the vertex buffers */
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

    /* Handles to the descriptor sets */
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

    /* Contain the mesh data and the scene graph */
    std::shared_ptr<S72Helper> s72Instance = nullptr;

    /* Refers to the camera instance that is using currently */
    std::shared_ptr<S72Object::Camera> currCamera = nullptr;

    /* The culling mode used for rendering. Can be none or frustum. */
    std::string cullingMode;


    bool useHeadlessRendering = false;


    std::vector<VkDeviceMemory> headlessImageMemory;

    std::vector<void*> headlessImageMapped;

    uint32_t headlessImageIndex = 0;


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

    /* Initialize the GLFW window used for the Vulkan Display. */
    void InitWindow();

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


    void CreateHeadlessSwapChain();

    /* Create an image view instance based on the image and its format. */
    VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);

    /* Create the Image Views that will handle the images in the swap chain. */
    void CreateImageViews();

    /* Create a module to wrap the info in the binary shader vector. */
    VkShaderModule CreateShaderModule(const std::vector<char>& code);

    /* Provide details about every descriptor binding and create the descriptor layout. */
    void CreateDescriptorSetLayout();

    /* Read the shaders and create the graphics pipeline. */
    void CreateGraphicsPipeline();

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

    /* Create a list of vertex buffers. One for each mesh instance in the s72. */
    void CreateVertexBuffers();

    /* Create the vertex buffer to store the vertex data. */
    void CreateVertexBuffer(const S72Object::Mesh& newMesh, size_t index);

    /* For a binding description struct based on the info of a mesh instance. */
    static VkVertexInputBindingDescription2EXT CreateBindingDescription(const S72Object::Mesh& newMesh);

    /* Form an attribute description struct based on the info of a mesh instance. */
    static std::array<VkVertexInputAttributeDescription2EXT, 3> CreateAttributeDescription(const S72Object::Mesh& newMesh);

    /* Create the index buffer to store the index relations. */
    void CreateIndexBuffer();

    /* Create the uniform buffer to store the uniform data. */
    void CreateUniformBuffers();

    /* Update the uniform buffer on the current image with given ubo data. */
    void UpdateUniformBuffer(uint32_t currentImage, const S72Object::Mesh& mesh, size_t instanceIndex, size_t totalIndex);

    /* Create a pool to allocate descriptor sets. */
    void CreateDescriptorPool();

    /* Create descriptor sets, one for each frame. */
    void CreateDescriptorSets();

    /* Create the command pool which will be used to allocate the memory for the command buffer. */
    void CreateCommandPool();

    /* Create the command buffer which can submit the drawing command. */
    void CreateCommandBuffers();

    /* Writes the commands we want to execute into a command buffer. */
    void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    /* Create the image instance. */
    void CreateImage(uint32_t width, uint32_t height, uint32_t newMipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

    /* Copy a VkBuffer to a VkImage. */
    void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

    /* Transit the image's layout with a new layout using a pipeline barrier. */
    void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t newMipLevels);

    /* Create the texture image with a given texture. */
    void CreateTextureImage();

    /* Create the image view to access and present the texture image. */
    void CreateTextureImageView();

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

    /* Load an obj model from the given path. */
    void LoadModel();

    void CopyImageToData(const VkImage& image, const VkDeviceMemory& imageMemory, void*& data);

    void SaveImageToPPM(const VkImage& image, const VkDeviceMemory& imageMemory, const std::string&);

    /* Clean up the swap chain and all the related resources. */
    void CleanUpSwapChain();

    /* Recreate the swap chain if the window is changed and the previous one become invalid. */
    void RecreateSwapChain();

    /* Generate the mipmaps through the command buffer. */
    void GenerateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t newMipLevels);

    /* Initialize the Vulkan application and setup. */
    void InitVulkan();

    /* Create a main loop to let the window keep opening. */
    void MainLoop();

    /* Create a main loop to let the window keep opening (Looping using the MSG mechanism). */
    static void MainLoopWIN();

public:

    /* Initialization the vulkan with s72 and the data passed from the command line. */
    void InitializeData(const std::shared_ptr<S72Helper>& news72Instance, uint32_t width, uint32_t height, const std::string& newDeviceName, const std::string& cameraName, const std::string& newCullingMode);

    /* Run the vulkan api with the s72 helper instance. */
    void Run();

    /* Run the Vulkan application using the WSI. */
    void RunWIN(HINSTANCE new_Instance, HWND new_hwnd);

    /* Handle the GLFW input based on the key the user typed. */
    void ProcessGLFWInputCallBack(char key);

    /* Draw the frame and submit the command buffer. */
    void DrawFrame();

    /* CleanUp used for destroy the instance and related destruction. */
    void CleanUp();

};


#endif //XUANJAMESZHAI_A1_VULKANHELPER_H
