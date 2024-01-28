#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

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
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif // DEBUG

/* Structure of a Vertex which is a position and a color */
struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    /* Describes at which rate to load data from memory throughout the vertices */
    static VkVertexInputBindingDescription getBindingDescription() 
    {
        VkVertexInputBindingDescription bindingDescription{};

        bindingDescription.binding = 0;         // Specifies the index of the binding in the array of bindings.
        bindingDescription.stride = sizeof(Vertex);     // Specifies the number of bytes from one entry to the next.
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;  // Move to the next data entry after each vertex.

        return bindingDescription;
    }

    /* Describes how to handle vertex input */
    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() 
    {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

        /* Attribute for the vertex data */
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;  // Describes the type of data for the attribute
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        /* Attribute for the color data */
        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

        return attributeDescriptions;
    }

    /* Override the == operator for comparison */
    bool operator==(const Vertex& other) const {
        return pos == other.pos && color == other.color && texCoord == other.texCoord;
    }
};


/* MVP data for the vertices */
struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};


/* Hash the vertex data */
namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.pos) ^
                (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
                (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}



/**
* @brief A callback function after the debug is triggered.
* @param[in] messageSeverity: It specifies the severity of the message, which is many flags
* @param[in] messageType: It refers to the flag types of the message
* @param[in] pCallbackData: It refers to a VkDebugUtilsMessengerCallbackDataEXT struct containing the details of the message itself
* @param[in] pUserData: It contains a pointer that was specified during the setup of the callback and allows you to pass your own data to it
*/
static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}


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
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}


/**
* @brief It creates a VkDebugUtilsMessengerEXT object that's used for the validation extension
*/
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}


/**
* @brief It destroys the VkDebugUtilsMessengerEXT object that's used for the validation extension
*/
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}




class HelloTriangle {
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
    VkBuffer vertexBuffer = VK_NULL_HANDLE;

    /* Handle to the memory of the vertex buffer */
    VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;

    /* The index buffer used to store the indices */
    VkBuffer indexBuffer = VK_NULL_HANDLE;

    /* Handle to the memory of the index buffer */
    VkDeviceMemory indexBufferMemory = VK_NULL_HANDLE;

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
    std::vector<Vertex> vertices;

    /* indices to arrange the order of vertices to draw */
    std::vector<uint32_t> indices;


    /* A struct of queue that will be submitted to Vulkan */
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;        // Queue family index. Use optional to see if it has a value 
        std::optional<uint32_t> presentFamily;      // Queue family that's used to present on the window

        bool isComplete() {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    /* A struct of details of Swap Chain */
    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;      // Basic surface capabilities (min/max number of images in swap chain, min/max width and height of images)
        std::vector<VkSurfaceFormatKHR> formats;    // Surface formats (pixel format, color space)
        std::vector<VkPresentModeKHR> presentModes; // Available presentation modes
    };


    /**
    * @brief A callback function use to detect if a window is resized.
    * @param[in] window: The window we are detecting
    * @param[in] width: The new width
    * @param[in] height: The new height
    */
    static void FramebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<HelloTriangle*>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
    }


    /**
    * @brief Initialize the GLFW window used for the Vulkan Display
    */
    void InitWindow() 
    {
        glfwInit();      // Initialize GLFW 

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);       // Tell it to not create an OpenGL context
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);     // Disable resize window

        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);    // Create a GLFW window

        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, FramebufferResizeCallback);      // Callback for window resize
    }


    /**
    * @brief Assign the HWND instance and the HINSTANCE to the member variable.
    */
    void InitWindowWIN(HINSTANCE new_hInstance, HWND new_hwnd)
    {
        hInstance = new_hInstance;
        hwnd = new_hwnd;
    }


    /**
    * @brief Populate the detail of swap chain
    * @param[in] device: The physical device that is using
    * @return The Swap Chain Detail struct
    */
    SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device) 
    {
        SwapChainSupportDetails details;

        /* Determine the surface capability */
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

        /* Determine the surface format */
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        }

        /* Query the supporting presenting mode */
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }


    /**
    * @brief Select the Swap Chain surface format.
    * @param[in] availableFormats: The given available formats.
    * @return The format we want to choose from the list.
    */
    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) 
    {

        /* Set the format (the color channels and types) and the color space (if the SRGB color space is supported or not) */
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }


    /**
    * @brief Select the Swap Chain present mode
    * VK_PRESENT_MODE_IMMEDIATE_KHR: Submit right away. may have tearing.
    * VK_PRESENT_MODE_FIFO_KHR swap chain with a queue.
    * VK_PRESENT_MODE_FIFO_RELAXED_KHR Instead of waiting for the next vertical blank, the image is transferred right away when it finally arrives.
    * VK_PRESENT_MODE_MAILBOX_KHR When the queue is full. Instead of waiting, just replace the last one.
    * @param[in] availablePresentModes: The given available modes
    * @return The mode we want from the list.
    */
    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) 
    {

        /* MAILBOX mode has higher priority */
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }


    /**
    * @brief The swap extent is the resolution of the swap chain images 
    * and it¡¯s almost always exactly equal to the resolution of the window that we¡¯re drawing to in pixels (more on that in a moment). 
    * @param[in] capabilities: The surface capabilities for the swap chain image.
    * @return The 2D extent we choose.
    */
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) 
    {
        if (capabilities.currentExtent.width != (std::numeric_limits<uint32_t>::max)() ) {
            return capabilities.currentExtent;
        }
        else {
            int width, height;

            if (ISWINWINDOW) {
                RECT rect;
                if (GetWindowRect(hwnd, &rect))
                {
                    width = rect.right - rect.left;
                    height = rect.bottom - rect.top;
                }
                else {
                    throw std::runtime_error("failed to get window rect!");
                }
            }
            else {
                glfwGetFramebufferSize(window, &width, &height);
            }

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }


    /**
    * @brief Get the list of extensions based on whether validation layers are enabled or not.
    * @return The required list of extensions.
    */
    std::vector<const char*> GetRequiredExtensions() 
    {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }


    /**
    * @brief Check if there is a valid validation layer.
    * @return True if it is valid.
    */
    bool CheckValidationLayerSupport() 
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        /* Loop through layers, check if all of the layers in validationLayers exist in the availableLayers list */
        for (const char* layerName : validationLayers) {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }
        return true;
    }


    /**
    * @brief Construct the VkDebugUtilsMessengerCreateInfoEXT with flags.
    * @param[out] createInfo: The debug create info we will work on.
    */
    void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) 
    {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = DebugCallback;
    }


    /**
    * @brief Set up the debugger tools used for the Vulkan application.
    */
    void SetupDebugMessenger() 
    {
        if (!enableValidationLayers) return;

        /* Add the debug info */
        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        PopulateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }


    /**
    * @brief Create a Vulkan instance.
    */
    void CreateInstance() 
    {
        /* Check if we need the validation layers */
        if (enableValidationLayers && !CheckValidationLayerSupport()) {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        /* Vulkan Application Info. Some of them are optional */
        VkApplicationInfo appInfo{};

        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;
           
        /* Specify The global extension and the validation layers */
        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        /* Gather the extensions used for rendering */
        std::vector<const char*> extensions;
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> extensionProps(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensionProps.data());

        if (ISWINWINDOW) {
            extensions.resize(extensionCount);

            for (uint32_t i = 0; i < extensionCount; i++) {
                extensions[i] = extensionProps[i].extensionName;
            }
        }
        else {
            extensions = GetRequiredExtensions();
            extensionCount = static_cast<uint32_t>(extensions.size());
        }

        createInfo.enabledExtensionCount = extensionCount;
        createInfo.ppEnabledExtensionNames = extensions.data();

        /* Add the validation layers into the info */
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();

            /* Set the debug message to pNext */
            PopulateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
        }
        else {
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
        }

        /* Create the Vulkan instance */
        VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);

        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!");
        }

    }


    /**
    * @brief Get the queue family indices that support the given device.
    * @param[in] device: The device we have.
    * @return The first queue family we find.
    */
    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device) 
    {
        QueueFamilyIndices indices;
        
        /* Retrieving the list of queue families */
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        /* Find at least one queue family that supports VK_QUEUE_GRAPHICS_BIT */
        int i = 0;
        for (const auto& queueFamily : queueFamilies) {

            if (indices.isComplete()) {
                break;
            }

            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }

            /* Check if the device support surface */
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
            if (presentSupport) {
                indices.presentFamily = i;
            }

            i++;
        }

        return indices;
    }


    /**
    * @brief Check if the physical device is suitable for the vulkan rendering.
    * @param[in] device: The physical device we are assessing.
    * @return True if this device is suitable for the rendering.
    */
    bool IsDeviceSuitable(VkPhysicalDevice device) 
    {

        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);

        /* The support for optional features like texture compression, 64 bit floats and multi viewport rendering */
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        QueueFamilyIndices indices = FindQueueFamilies(device);

        /* Check support for swap chain*/
        bool extensionsSupported = CheckDeviceExtensionSupport(device);

        bool swapChainAdequate = false;
        if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
            deviceFeatures.geometryShader && indices.isComplete() && extensionsSupported && swapChainAdequate && &deviceFeatures.samplerAnisotropy;
    }


    /**
    * @brief Verify that your graphics card is indeed capable of creating a swap chain.
    * @param[in] device: The physical device that's using.
    * @return Return true if it is capable.
    */
    bool CheckDeviceExtensionSupport(VkPhysicalDevice device) 
    {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }


    /**
    * @brief Pick the graphics card that fits for the need for the Vulkan rendering.
    */
    void PickPhysicalDevice() 
    {

        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        if (deviceCount == 0) {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        /* A list of physical devices handles */
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        /* Loop through those devices and check if find suitable */
        for (const auto& device : devices) {
            if (IsDeviceSuitable(device)) {
                physicalDevice = device;
                break;
            }
        }

        if (physicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
    }


    /**
    * @brief Create the logical device for the Vulkan rendering.
    */
    void CreateLogicalDevice() 
    {

        QueueFamilyIndices indices = FindQueueFamilies(physicalDevice);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        /* We need to have multiple VkDeviceQueueCreateInfo structs to create a queue from both queue families. */
        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE;     // Enable the anisotropy feature

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());

        createInfo.pEnabledFeatures = &deviceFeatures;

        /* Enable extensions for swap chain */
        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();

        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else {
            createInfo.enabledLayerCount = 0;
        }

        VkResult result = vkCreateDevice(physicalDevice, &createInfo, nullptr, &device);

        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!");
        }

        /* Retrieve queue handles for each queue family */
        vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
        /* Retrieve queue handles for each present family*/
        vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
    }


    /**
    * @brief Create the window surface that will be used for display the output.
    */
    void CreateSurface()
    {
        VkWin32SurfaceCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;

        if (ISWINWINDOW) {
            createInfo.hwnd = hwnd;
            createInfo.hinstance = hInstance;
        }
        else {
            createInfo.hwnd = glfwGetWin32Window(window);
            createInfo.hinstance = GetModuleHandle(nullptr);
        }

        /* Create the surface with WSI extension from the Windows native API */
        if (vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
    }


    /**
    * @brief Create the swap chain that will be used for display the output.
    */
    void CreateSwapChain() 
    {
        SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(physicalDevice);

        VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
        swapChainImageFormat = surfaceFormat.format;
        VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
        swapChainExtent = ChooseSwapExtent(swapChainSupport.capabilities);

        /* How many images we would like to have in the swap chain. Add one to add more room and avoid delay */
        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

        /* Avoid exceed the max count */
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        /* Need to set the info for Swap Chain */
        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = swapChainExtent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = FindQueueFamilies(physicalDevice);
        uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        /* If the queue families differ, then we¡¯ll be using the concurrent mode in this tutorial to avoid having to do the ownership chapters */
        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0; // Optional
            createInfo.pQueueFamilyIndices = nullptr; // Optional
        }

        /* Specifies if the alpha channel should be used for blending with other windows in the window system. */
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        /* Create the swap chain instance with info needed */
        if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
            throw std::runtime_error("failed to create swap chain!");
        }

        /* Retrieve the swap chain image to the swapChainImages */
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());
    }


    /**
    * @brief Create a image view instance based on the image and its format
    * @param[in] image: The image we are using.
    * @param[in] format: The format of the image view.
    * @param[in] aspectFlags: The aspect mask. Can be color or depth.
    */
    VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;

        /* The subresourceRange field describes what the image¡¯s purpose is and which part of the image should be accessed. */
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        /* Create the image view*/
        VkImageView imageView;
        if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image view!");
        }

        return imageView;
    }


    /**
    * @brief Create the Image Views that will handle the images in the swap chain.
    */
    void CreateImageViews() 
    {
        swapChainImageViews.resize(swapChainImages.size());

        for (uint32_t i = 0; i < swapChainImages.size(); i++) {
            swapChainImageViews[i] = CreateImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
        }
    }


    /**
    * @brief Create a module to wrap the info in the binary shader vector.
    * @param[in] code The binary char array of the shaders
    * @return A shader module with file's data.
    */
    VkShaderModule CreateShaderModule(const std::vector<char>& code) 
    {

        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
        }

        return shaderModule;
    }


    /**
    * @brief Provide details about every descriptor binding and create the descriptor layout.
    */
    void CreateDescriptorSetLayout()
    {
        /* Set the binding info */
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;    // The type of descriptor is a uniform buffer object
        uboLayoutBinding.descriptorCount = 1;

        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        uboLayoutBinding.pImmutableSamplers = nullptr;

        /* Set the combined image sampler */
        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = 1;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;     // Use the image sampler in the fragment shader stage.

        std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };

        /* Combine all the bindings into a single object */
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }


    /**
    * @brief Read the shaders and create the graphics pipline.
    */
    void CreateGraphicsPipeline() 
    {
        /* Read the file into a cahr array */
        auto vertShaderCode = ReadFile("Shaders/vert.spv");
        auto fragShaderCode = ReadFile("Shaders/frag.spv");

        /* Create modules for shaders to wrap the file*/
        VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);

        /* Assign the shaders to a specific pipeline stage. with module and start function name */
        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

        /* Describes the format of the vertex data that will be passed to the vertex shader */
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        /* Make the pipline accept Vertex data as the vertex input */
        auto bindingDescription = Vertex::getBindingDescription();
        auto attributeDescriptions = Vertex::getAttributeDescriptions();

        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        /* Describes the primitive we are drawing */
        /* VK_PRIMITIVE_TOPOLOGY_POINT_LIST: points from vertices */
        /* VK_PRIMITIVE_TOPOLOGY_LINE_LIST: line from every 2 vertices without reuse */
        /* VK_PRIMITIVE_TOPOLOGY_LINE_STRIP: the end vertex of every line is used as start vertex for the next line */
        /* VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST: triangle from every 3 vertices without reuse */
        /* VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP: the second and third vertex of every triangle are used as first two vertices of the next triangle */
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        /* Describe the viewport we are rendering to */
        /* Describe the scissor rectangles about in which regions pixels will actually be stored. */
        /* Note: minDepth and maxDepth must be within 0 and 1 */
        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        /* Describe the rasterizer in the pipline */
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;     // False = discard and True = Clamp
        rasterizer.rasterizerDiscardEnable = VK_FALSE;  // True = disables any output to the framebuffer.
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;  // Fill the area of the polygon with fragments.
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;   // Face culling mode.
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;         // We update this since we change the Y-filp
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.0f; // Optional
        rasterizer.depthBiasClamp = 0.0f; // Optional
        rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

        /* Describe the multisampling features in the pipline */
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

        /* Describe the color blending in the pipline */
        /* Blending the new color from the shader with the old one in the frame buffer */
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f; // Optional
        colorBlending.blendConstants[1] = 0.0f; // Optional
        colorBlending.blendConstants[2] = 0.0f; // Optional
        colorBlending.blendConstants[3] = 0.0f; // Optional

        /* Specify the layout, the 'uniform' item in the pipline */
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1; // Optional
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout; // Optional
        pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
        pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

        if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        /* Specify the dynamic states in the pipline */
        std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        /* Describe the final pipline */
        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = pipelineLayout;       // Layout
        pipelineInfo.renderPass = renderPass;       // Render pass
        pipelineInfo.subpass = 0;

        /* Option to create derived pipline */
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        pipelineInfo.basePipelineIndex = -1; // Optional

        if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline!");
        }

        vkDestroyShaderModule(device, fragShaderModule, nullptr);
        vkDestroyShaderModule(device, vertShaderModule, nullptr);
    }


    /**
    * @brief Create the render pass to attach the framebuffer
    */
    void CreateRenderPass() 
    {
        /* Single color buffer by the Image from the swap chain. */
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = swapChainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;        // No multisampling here

        /* Determine what to do with the data */
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;   // Clear the framebuffer to black before drawing a new frame
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;     // Store the framebuffer content into the memory 

        /* Stencil buffer has special load and store command */
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        /* VKImage needs to be transitioned to specific layouts */
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;      // We don't care about the previous layout
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;  // We want the image to be ready for presentation using the swap chain after rendering

        /* Create the attachment for the depth image */
        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = FindDepthFormat();
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

        /* We don't need to worry about store data because it uses once */
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        /* Describe the attachment layout */
        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        /* Describe the depth attachment layout */
        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        /* Describe the subpass. Used for example like post processing */
        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;        // Connect it with the color attachment.
        subpass.pDepthStencilAttachment = &depthAttachmentRef;  // Connect it with the depth attachment. Its count is always 1.

        /* Fill the render pass instance with info */
        std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;

        /* Add the subpass dependency to the render pass */
        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        /* The depth image is first accessed in the early fragment test pipeline stage */
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }

    }


    /**
    * @brief Create the frame buffer which has the data to present.
    */
    void CreateFrameBuffers()
    {
        swapChainFramebuffers.resize(swapChainImageViews.size());

        /* Iterate through each image and create the buffer for it */
        for (size_t i = 0; i < swapChainImageViews.size(); i++) {

            /* Attach the color image and the depth image into the frame buffer */
            /* The same depth image can be used by all of them because only a single subpass is running at the same time due to our semaphores.*/
            std::array<VkImageView, 2> attachments = {
                swapChainImageViews[i],
                depthImageView
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass;        // Specify the render pass
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = swapChainExtent.width;
            framebufferInfo.height = swapChainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
    }


    /**
    * @brief Combine the requirements of the buffer and our own application requirements to find the right type of memory to use.
    * @param[in] typeFilter: Specify the bit field of memory types
    * @param[in] properties: Define special features of the memory
    */
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) 
    {

        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

        /* Find the memory that is suitable for the buffer to use */
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            /* typeFilter specify the bit field of memory types */
            /* The properties define special features of the memory, like being able to map it so we can write to it from the CPU. */
            if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }


    /**
    * @brief Create a buffer, can be used to create the vertex buffer and the index buffer.
    * @param[in] size: The buffer size.
    * @param[in] usage: The buffer usage.
    * @param[in] properties: The buffer memory's flag.
    * @param[out] buffer: The buffer created.
    * @param[out] bufferMemory: The memory handle for the buffer
    */
    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
    {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;        // Size of the buffer
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create vertex buffer!");
        }

        /* Allocate memory for the vertex buffer, first get its requirement */
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

        /* Determine the right memory type and allocate the memory */
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate vertex buffer memory!");
        }

        /* Bind the memory with the vertex buffer */
        vkBindBufferMemory(device, buffer, bufferMemory, 0);
    }



    VkCommandBuffer BeginSingleTimeCommands() {

        /* Allocate a small command buffer used to apply this transfer command */
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        /* Record the command buffer */
        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }


    void EndSingleTimeCommands(VkCommandBuffer commandBuffer) {
        vkEndCommandBuffer(commandBuffer);

        /* Stop recording right after it's copied */
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(graphicsQueue);

        /* Free the command buffer */
        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
    }


    /**
    * @brief Copy a vulkan buffer from sourse to destination.
    * @param[in] srcBuffer: The source buffer.
    * @param[in] dstBuffer: The destination buffer.
    * @param[in] size: The copied buffer size.
    */
    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) 
    {
        VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        EndSingleTimeCommands(commandBuffer);
    }


    /**
    * @brief Create the vertex buffer to store the vertex data.
    */
    void CreateVertexBuffer()
    {
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
        
        /* Create a staging buffer as temporary buffer and use a device local one as actual vertex buffer. */
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        /* Copy the vertex data to the buffer using memcpy */
        void* data;
        vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, vertices.data(), (size_t)bufferSize);
        vkUnmapMemory(device, stagingBufferMemory);

        /* The vertex buffer is now device local */
        CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

        /* Copy the buffer from the staging buffer to the device local vertex buffer */
        CopyBuffer(stagingBuffer, vertexBuffer, bufferSize);

        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
    }


    /**
    * @brief Create the index buffer to store the index relations.
    */
    void CreateIndexBuffer()
    {
        VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, indices.data(), (size_t)bufferSize);
        vkUnmapMemory(device, stagingBufferMemory);

        /* One difference is set its usage to INDEX_BUFFER */
        CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

        CopyBuffer(stagingBuffer, indexBuffer, bufferSize);

        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
    }


    /**
    * @brief Create the uniform buffer to store the uniform data.
    */
    void CreateUniformBuffers()
    {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
        uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);

            vkMapMemory(device, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
        }
    }


    /**
    * @brief Update the uniform buffer on the current image with given ubo data.
    */
    void UpdateUniformBuffer(uint32_t currentImage)
    {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();


        /* Define the model, view and projection transformations in the uniform buffer object.*/
        UniformBufferObject ubo{};

        /* Rotation 90 degrees per second */
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(15.0f), glm::vec3(0.0f, 0.0f, 1.0f));

        /* Look at the geometry from above at a 45 degree angle */
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

        /* Use a perspective projection with a 45 degree vertical field-of-view. */
        if (swapChainExtent.width == 0 || (float)swapChainExtent.height == 0) {
            ubo.proj = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 10.0f);
        }
        else {
            ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f);
        }

        /* Since GLM was used for OpenGL which has different Y-Dir than Vulkan, we need to flip it */
        ubo.proj[1][1] *= -1;

        /* Copy the data to the current uniform buffer */
        memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
    }


    /** 
    * @brief Create a pool to allocate descriptor sets 
    */
    void CreateDescriptorPool()
    {
        /* Describe which descriptor types our descriptor sets are going to contain */
        /* The first is used for the uniform buffer. The second is used for the image sampler */
        std::array<VkDescriptorPoolSize, 2> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

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
    * @brief Create descriptor sets, one for each frame.
    */
    void CreateDescriptorSets()
    {
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

            /* Bind the actual image and sampler resources to the descriptors. */
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = textureImageView;
            imageInfo.sampler = textureSampler;

            std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = descriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = descriptorSets[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &imageInfo;

            vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
    }


    /**
    * @brief Create the command pool which will be used to allocate the memory for the command buffer.
    */
    void CreateCommandPool()
    {
        QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(physicalDevice);

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

        if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create command pool!");
        }
    }


    /**
    * @brief Create the command buffer which can submit the drawing command.
    */
    void CreateCommandBuffers()
    {
        commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;                // Allocate the buffer on the command pool
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

        if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }


    /**
    * @brief Writes the commands we want to execute into a command buffer
    * @param[in] commandBuffer: The buffer we are writing to
    * @param[in] imageIndex: The index of the current swapchain image we want to write to
    */
    void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0; // Optional
        beginInfo.pInheritanceInfo = nullptr; // Optional

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        /* Start the render pass and start drawing */
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];     // Bind the frame buffer with the swap chain image

        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = swapChainExtent;

        /* Describe the clear color and the depth when we clear the view */
        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
        /* The depth is between 0 to 1 */
        clearValues[1].depthStencil = { 1.0f, 0 };

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        /* Begin the render pass*/
        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        /* Bind the command buffer with the pipline*/
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

        /* Bind the vertex buffer during rendering operations */
        VkBuffer vertexBuffers[] = { vertexBuffer };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

        /* Bind the index buffer */
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

        /* Since We set the viewport and scissor state for this pipeline to be dynamic. */
        /* Here we need to set it now */
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(swapChainExtent.width);
        viewport.height = static_cast<float>(swapChainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = swapChainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        /* Bind the descriptor sets */
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[currentFrame], 0, nullptr);

        /* Issue the drawing command */
        /* Second param: vertexCount */
        /* Third param: instanceCount (Used for instanced rendering) */
        /* Fourth param: firstVertex (An offset into the vertex buffer) */
        /* Fifth param: firstInstance (An offset for instanced rendering) */
        // Draw Command without index buffer: vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

        /* End the render pass */
        vkCmdEndRenderPass(commandBuffer);

        /* Finish recording the command buffer */
        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }


    /**
    * @brief Create the image instance
    * @param[in] width: The image's width.
    * @param[in] height: The image's height.
    * @param[in] format: Image's format
    * @param[in] tiling: Image's tiling mode
    * @param[in] usage: The image's usage type.
    * @param[in] properties: The image's property.
    * @param[out] image: The image instance.
    * @param[out] imageMemory: The memory used to allocate the image.
    */
    void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
    {
        /* Set the info for the texture */
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;                 // Texture is 2D
        imageInfo.extent.width = static_cast<uint32_t>(width);
        imageInfo.extent.height = static_cast<uint32_t>(height);
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;                                // Mipmap level is 1
        imageInfo.arrayLayers = 1;

        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;              // Used for multi-sampling
        imageInfo.flags = 0; // Optional

        if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image!");
        }

        /* Allocate the memory for the image */
        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(device, image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate image memory!");
        }

        /* Bind the image with the memory */
        vkBindImageMemory(device, image, imageMemory, 0);
    }


    /**
    * @brief Copy a VkBuffer to a VkImage.
    * @param[in] buffer: The buffer we are copying from.
    * @param[in] image: The image we are copying to.
    * @param[in] width: The image's width.
    * @param[in] height: The image's height.
    */
    void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
        VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = {
            width,
            height,
            1
        };

        vkCmdCopyBufferToImage(
            commandBuffer,
            buffer,
            image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region
        );

        EndSingleTimeCommands(commandBuffer);
    }


    /**
    * @brief Transit the image's layout with a new layout using a pipline barrier.
    * @param[in] image: The image that is affected and the specific part of the image.
    * @param[in] format: The format of the image TODO!!!!!!!!!!!
    * @param[in] oldLayout: The old layout we have.
    * @param[in] newLayout: The new layout we determine.
    */
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
        VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;

        /* Don't transfer the ownership of the queue family */
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        /* Set the barrier access mask */
        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else {
            throw std::invalid_argument("unsupported layout transition!");
        }

        /* Submit the pipline barrier */
        vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage,       // Which pipeline stage the operations occur that should happen before the barrier
            destinationStage,       // The pipeline stage in which operations will wait on the barrier.
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );

        EndSingleTimeCommands(commandBuffer);
    }


    /**
    * @brief Create the texture image with a given texture.
    */
    void CreateTextureImage()
    {
        /* Load the texture image */
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        VkDeviceSize imageSize = texWidth * texHeight * 4;      // 4 means 4 bytes for pixel.

        if (!pixels) {
            throw std::runtime_error("failed to load texture image!");
        }

        /* Create a buffe to store the image data */
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        /* Copy the image to the buffer */
        void* data;
        vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
        memcpy(data, pixels, static_cast<size_t>(imageSize));
        vkUnmapMemory(device, stagingBufferMemory);

        stbi_image_free(pixels);

        CreateImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);
        
        /* Copy the staging buffer to the texture image */
        transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        CopyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
        
        /* Change texture image's layout for the shade access */
        transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    
        /* Clear the stage buffer */
        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
    }


    /**
    * @brief Create the image view to access and present the texture image.
    */
    void CreateTextureImageView()
    {
        /* Create the texture image view */
        textureImageView = CreateImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
    }


    /**
    * @brief Create the texture sampler to access the texture.
    * Apply filtering and transformations to compute the final color that is retrieved.
    */
    void CreateTextureSampler() 
    {

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        /* How to interpolate texels that are magnified or minified */
        /* Option: VK_FILTER_NEAREST or VK_FILTER_LINEAR */
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;

        /* VK_SAMPLER_ADDRESS_MODE_REPEAT: Repeat the texture when going beyond the image dimensions */
        /* VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT: Repeat but mirror the image when beyond the dimensions */
        /* VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE: Take the color of the edge closest to the coordinate */
        /* VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE:  Like clamp but instead uses the edge opposite to the closest edge */
        /* VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER: Return a solid color when beyond the dimension */
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

        /* Get the anisotrpy property from the physical device */
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(physicalDevice, &properties);
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

        /* If unnormalized, use coordinates within the [0, texWidth) and [0, texHeight) range. */
        samplerInfo.unnormalizedCoordinates = VK_FALSE;

        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

        /* Set the mipmap info for the sampler */
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;
        
        /* Create the Sampler with the info */
        if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture sampler!");
        }
    }


    /**
    * @brief Create all the semaphores and fences used for sync updating the command buffers
    */
    void CreateSyncObjects()
    {
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        /* Create the info for the semaphore and the fence */
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;     // Make it signaled to avoid the first-frame dilemma

        /* Loop through the vector to create semaphores */
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {

                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    }


    /**
    * @brief Find the supported vkformat that supports the tiling mode and the features.
    * @param[in] candidates: A list of available formats.
    * @param[in] tiling: The tiling mode we are requiring.
    * @param[in] features: The features we want it to support.
    * @return A desired vkformat.
    */
    VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) 
    {
        /* Loop through all the formats */
        for (VkFormat format : candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

            /* Choose the one that has the tiling feature */
            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            }
            else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }

        throw std::runtime_error("failed to find supported format!");
    }


    /**
    * @brief select a format with a depth component that supports usage as depth attachment.
    * @return The found depth format.
    */
    VkFormat FindDepthFormat() {
        return FindSupportedFormat(
            { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }


    /**
    * @brief Check if a format contains a stencil component (Can be used for the depth test)
    * @param[in] format: The format we want to testify.
    * @return True if it has the component.
    */
    bool HasStencilComponent(VkFormat format) {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }


    /**
    * @brief Create the depth image and the image view
    */
    void CreateDepthResources() 
    {
        VkFormat depthFormat = FindDepthFormat();

        /* Create the image with the given format */
        CreateImage(swapChainExtent.width, swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
        depthImageView = CreateImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
    }


    /**
    * @brief Load a obj model from the given path.
    */
    void LoadModel()
    {
        /* Holds all of the positions, normals and texture coordinates */
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials,  &err, MODEL_PATH.c_str())) {
            throw std::runtime_error(err);
        }

        /* A map of Vertices and Indices to make sure there're no duplicate vertex */
        std::unordered_map<Vertex, uint32_t> uniqueVertices{};

        /* Loop through the shapes */
        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                Vertex vertex{};

                /* Fill the triangle data */
                vertex.pos = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };

                vertex.texCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    /* We need to flip the v so that it can become a top to bottom orientation */
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                };

                vertex.color = { 1.0f, 1.0f, 1.0f };

                /* Check if there's a duplicate vertex */
                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }

                indices.push_back(uniqueVertices[vertex]);
            }
        }
    }


    /**
    * @brief Clean up the swapchain and all the related resources.
    */
    void CleanUpSwapChain()
    {
        vkDestroyImageView(device, depthImageView, nullptr);
        vkDestroyImage(device, depthImage, nullptr);
        vkFreeMemory(device, depthImageMemory, nullptr);

        for (auto framebuffer : swapChainFramebuffers) {
            vkDestroyFramebuffer(device, framebuffer, nullptr);
        }

        for (auto imageView : swapChainImageViews) {
            vkDestroyImageView(device, imageView, nullptr);
        }

        vkDestroySwapchainKHR(device, swapChain, nullptr);
    }


    /**
    * @brief Recreate the swapchain if the window is changed and the previous one become invalid.
    */
    void RecreateSwapChain()
    {
        /* Pause the window if the frame buffer size is 0 (window minimization)*/
        int width = 0, height = 0;

        if (ISWINWINDOW) {
            RECT rect;
            if (GetWindowRect(hwnd, &rect))
            {
                width = rect.right - rect.left;
                height = rect.bottom - rect.top;
            }
            else {
                throw std::runtime_error("failed to get window rect!");
            }
        }
        else {
            glfwGetFramebufferSize(window, &width, &height);
        }

        while (width == 0 || height == 0) {
            if (ISWINWINDOW) {
                // TODO: 
            }
            else {
                glfwGetFramebufferSize(window, &width, &height);
                glfwWaitEvents();
            }
        }

        /* Wait for the using device to be done */
        vkDeviceWaitIdle(device);

        /* Clean up the previous swapchain data */
        CleanUpSwapChain();

        /* Recreate all the resources that depends on the swapchain */
        CreateSwapChain();
        CreateImageViews();
        CreateDepthResources();
        CreateFrameBuffers();
    }


    /**
    * @brief Initialize the Vulkan application and setup.
    */
    void InitVulkan() 
    {
        CreateInstance();
        SetupDebugMessenger();
        CreateSurface();
        PickPhysicalDevice();
        CreateLogicalDevice();
        CreateSwapChain();
        CreateImageViews();
        CreateRenderPass();
        CreateDescriptorSetLayout();
        CreateGraphicsPipeline();
        CreateCommandPool();
        CreateDepthResources();
        CreateFrameBuffers();
        CreateTextureImage();
        CreateTextureImageView();
        CreateTextureSampler();
        LoadModel();
        CreateVertexBuffer();
        CreateIndexBuffer();
        CreateUniformBuffers();
        CreateDescriptorPool();
        CreateDescriptorSets();
        CreateCommandBuffers();
        CreateSyncObjects();
    }


    /**
    * @brief Create a main loop to let the window keep opening.
    */
    void MainLoop()
    {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();       // Check for inputs
            DrawFrame();
        }

        /* Wait for the logical device to finish operations before exiting mainLoop and destroying the window */
        vkDeviceWaitIdle(device);
    }


    /**
    * @brief Create a main loop to let the window keep opening (Looping using the MSG mechanism).
    */
    void MainLoopWIN()
    {
        MSG msg = { };
        while (GetMessage(&msg, NULL, 0, 0) > 0)
        {   
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }


public:

    /**
    * @brief Run the Vulkan application.
    */
    void Run()
    {
        InitWindow();
        InitVulkan();
        MainLoop();
        CleanUp();
    }


    /**
    * @brief Run the Vulkan application using the Windows created window.
    */
    void RunWIN(HINSTANCE new_Instance, HWND new_hwnd)
    {
        InitWindowWIN(new_Instance, new_hwnd);
        InitVulkan();
        MainLoopWIN();
    }


    /**
    * @brief Wait for the previous frame to finish.
    * Acquire an image from the swap chain.
    * Record a command buffer which draws the scene onto that image.
    * Submit the recorded command buffer.
    * Present the swap chain image.
    */
    void DrawFrame()
    {
        /* Wait until the previous frame has finished */
        vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        /* Acquire an image from the swap chain, may need to recreate the swapchain if the image is outdated */
        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            RecreateSwapChain();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        /* Reset the fence after wait. Only reset the fence if we are submitting work */
        vkResetFences(device, 1, &inFlightFences[currentFrame]);

        /* Record the Command Buffer */
        vkResetCommandBuffer(commandBuffers[currentFrame], 0);
        RecordCommandBuffer(commandBuffers[currentFrame], imageIndex);

        /* Update the Uniform Buffer */
        UpdateUniformBuffer(currentFrame);

        /* Create the info to submit the command buffer */
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        /* Submit a single command buffer */
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

        /* Signal the semaphore */
        VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        /* Submit the command buffer */
        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }


        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        /* The swap chains to present images to and the index of the image for each swap chain */
        VkSwapchainKHR swapChains[] = { swapChain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;

        /* Use the present info result to present the image! */
        presentInfo.pResults = nullptr; // Optional
        result = vkQueuePresentKHR(presentQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
            framebufferResized = false;
            RecreateSwapChain();
        }
        else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }

        /* Update the current frame to the next frame index */
        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }


    /**
    * @brief CleanUp used for destroy the instance and related destruction.
    */
    void CleanUp()
    {
        if (ISWINWINDOW) {
            /* Wait for the logical device to finish operations before exiting mainLoop and destroying the window */
            vkDeviceWaitIdle(device);
        }


        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        }

        CleanUpSwapChain();

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(device, inFlightFences[i], nullptr);
        }

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroyBuffer(device, uniformBuffers[i], nullptr);
            vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
        }
        vkDestroyBuffer(device, indexBuffer, nullptr);
        vkFreeMemory(device, indexBufferMemory, nullptr);
        vkDestroyBuffer(device, vertexBuffer, nullptr);
        vkFreeMemory(device, vertexBufferMemory, nullptr);

        vkDestroySampler(device, textureSampler, nullptr);
        vkDestroyImageView(device, textureImageView, nullptr);
        vkDestroyImage(device, textureImage, nullptr);
        vkFreeMemory(device, textureImageMemory, nullptr);

        vkDestroyDescriptorPool(device, descriptorPool, nullptr);
        vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
        vkDestroyPipeline(device, graphicsPipeline, nullptr);
        vkDestroyRenderPass(device, renderPass, nullptr);
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);   // Destroy the pipline layout
        vkDestroyCommandPool(device, commandPool, nullptr);     // Command buffer will be freed when the pool is freed

        vkDestroyDevice(device, nullptr);       // Destory the logical device
        vkDestroySurfaceKHR(instance, surface, nullptr);    // Destroy the surface instance
        vkDestroyInstance(instance, nullptr);   // Destroy the vulkan instance

        if (ISWINWINDOW) {
            DestroyWindow(hwnd);
            PostQuitMessage(0);
        }
        else {
            glfwDestroyWindow(window);      // Close the window
            glfwTerminate();            // GLFW destruction
        }
    }
};


/* An instance of the program */
HelloTriangle* TriInstance = nullptr;


/**
* @brief A callback function captures the event from the window.
*/
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        /* If the window is closed */
        case WM_CLOSE:
        {   
            if (TriInstance != nullptr) {
                TriInstance->CleanUp();
            }
            return 0;
        }
        /* If it's painting on the window */
        case WM_PAINT:
        {
            if (TriInstance != nullptr) {
                TriInstance->DrawFrame();
            }
            return 0;
        }
        default:
        {
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
    }
}


#if ISWINWINDOW


/**
* @brief A program entry for Windows
*/
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow) {

    TriInstance = new HelloTriangle();

    try
    {
        const wchar_t CLASS_NAME[] = L"Sample Window Class";

        WNDCLASS wc = { };

        wc.lpfnWndProc = WindowProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = CLASS_NAME;

        RegisterClass(&wc);

        HWND hwnd = CreateWindowEx(
            0,                              // Optional window styles.
            CLASS_NAME,                     // Window class
            L"Learn to Program Windows",    // Window text
            WS_OVERLAPPEDWINDOW,            // Window style

            // Size and position
            CW_USEDEFAULT, CW_USEDEFAULT, WIDTH, HEIGHT,

            NULL,       // Parent window
            NULL,       // Menu
            hInstance,  // Instance handle
            NULL        // Additional application data
        );

        if (hwnd == nullptr)
        {
            throw std::runtime_error("failed to create the WIN window!");
        }


        ShowWindow(hwnd, nCmdShow);
        TriInstance->RunWIN(hInstance,hwnd);
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        delete TriInstance;
        return EXIT_FAILURE;
    }
    
    delete TriInstance;
    return EXIT_SUCCESS;
}

#else


/**
* @brief A cross-platform program entry if we want to create the window with GLFW
*/
int main() {

    TriInstance = new HelloTriangle();

    try
    {
        TriInstance->Run();
    }
    catch (const std::exception& e)
    {
        std::cout << "Failed to Run the Program: " << e.what() << std::endl;
        delete TriInstance;
        return EXIT_FAILURE;
    }
    delete TriInstance;
    return EXIT_SUCCESS;
}

#endif



