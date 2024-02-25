//
// Created by Xuan Zhai on 2024/2/1.
//
#include "VulkanHelper.h"
#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"
#include <utility>



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

    std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;

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
    file.read(buffer.data(), (std::streamsize)fileSize);
    file.close();

    return buffer;
}


/**
 * @brief React with a key event.
 * @param window The glfw window that it's interacting with.
 * @param key The key is pressed.
 * @param scancode
 * @param action If it is pressed or held down.
 * @param mods
 */
void GLFW_Key_Callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    auto* instance = reinterpret_cast<VulkanHelper*>(glfwGetWindowUserPointer(window));
    if(instance == nullptr) return;

    if (key == GLFW_KEY_W && action == GLFW_REPEAT){
        instance->ProcessGLFWInputCallBack('W');
    }
    else if (key == GLFW_KEY_S && action == GLFW_REPEAT){
        instance->ProcessGLFWInputCallBack('S');
    }
    else if (key == GLFW_KEY_A && action == GLFW_REPEAT){
        instance->ProcessGLFWInputCallBack('A');
    }
    else if (key == GLFW_KEY_D && action == GLFW_REPEAT){
        instance->ProcessGLFWInputCallBack('D');
    }
    else if (key == GLFW_KEY_Q && action == GLFW_REPEAT){
        instance->ProcessGLFWInputCallBack('Q');
    }
    else if (key == GLFW_KEY_E && action == GLFW_REPEAT){
        instance->ProcessGLFWInputCallBack('E');
    }
    else if (key == GLFW_KEY_R && action == GLFW_PRESS){
        instance->ProcessGLFWInputCallBack('R');
    }
    else if (key == GLFW_KEY_T && action == GLFW_PRESS){
        instance->ProcessGLFWInputCallBack('T');
    }
    else if (key == GLFW_KEY_Y && action == GLFW_PRESS){
        instance->ProcessGLFWInputCallBack('Y');
    }
    else if (key == GLFW_KEY_C && action == GLFW_PRESS){
        instance->ProcessGLFWInputCallBack('C');
    }
}


/* ====================================== VulkanHelper ============================================================== */


/**
* @brief A callback function use to detect if a window is resized.
* @param[in] window: The window we are detecting
* @param[in] width: The new width
* @param[in] height: The new height
*/
void VulkanHelper::FramebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<VulkanHelper*>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;
}


/**
* @brief Initialize the GLFW window used for the Vulkan Display.
*/
void VulkanHelper::InitWindow()
{
    /* Not create a window if doing the headless rendering. */
    if(useHeadlessRendering){
        return;
    }

    glfwInit();      // Initialize GLFW

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);       // Tell it to not create an OpenGL context
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);     // Disable resize window

    window = glfwCreateWindow((int)windowWidth, (int)windowHeight, "Vulkan", nullptr, nullptr);    // Create a GLFW window

    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, FramebufferResizeCallback);      // Callback for window resize

    glfwSetKeyCallback(window, GLFW_Key_Callback);
}


/**
* @brief Assign the HWND instance and the HINSTANCE to the member variable.
*/
void VulkanHelper::InitWindowWIN(HINSTANCE new_hInstance, HWND new_hwnd)
{
    hInstance = new_hInstance;
    hwnd = new_hwnd;
}


/**
* @brief Populate the detail of swap chain.
* @param[in] device: The physical device that is using
* @return The Swap Chain Detail struct
*/
VulkanHelper::SwapChainSupportDetails VulkanHelper::QuerySwapChainSupport(VkPhysicalDevice newDevice)
{
    SwapChainSupportDetails details;

    /* Determine the surface capability */
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(newDevice, surface, &details.capabilities);

    /* Determine the surface format */
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(newDevice, surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(newDevice, surface, &formatCount, details.formats.data());
    }

    /* Query the supporting presenting mode */
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(newDevice, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(newDevice, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}


/**
* @brief Select the Swap Chain surface format.
* @param[in] availableFormats: The given available formats.
* @return The format we want to choose from the list.
*/
VkSurfaceFormatKHR VulkanHelper::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{

    /* Set the format (the color channels and types) and the color space (if the SRGB color space is supported or not) */
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_R8G8B8A8_UNORM) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}


/**
* @brief Select the Swap Chain present mode.
* VK_PRESENT_MODE_IMMEDIATE_KHR: Submit right away. may have tearing.
* VK_PRESENT_MODE_FIFO_KHR swap chain with a queue.
* VK_PRESENT_MODE_FIFO_RELAXED_KHR Instead of waiting for the next vertical blank, the image is transferred right away when it finally arrives.
* VK_PRESENT_MODE_MAILBOX_KHR When the queue is full. Instead of waiting, just replace the last one.
* @param[in] availablePresentModes: The given available modes
* @return The mode we want from the list.
*/
VkPresentModeKHR VulkanHelper::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
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
* @brief The swap extent is the resolution of the swap chain images.
* and it’s almost always exactly equal to the resolution of the window that we’re drawing to in pixels (more on that in a moment).
* @param[in] capabilities: The surface capabilities for the swap chain image.
* @return The 2D extent we choose.
*/
VkExtent2D VulkanHelper::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != (std::numeric_limits<uint32_t>::max)() ) {
        return capabilities.currentExtent;
    }
    else {
        int width, height;

        if (USE_WSI) {
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
std::vector<const char*> VulkanHelper::GetRequiredExtensions()
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
bool VulkanHelper::CheckValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    /* Loop through layers, check if all the layers in validationLayers exist in the availableLayers list */
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
void VulkanHelper::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
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
void VulkanHelper::SetupDebugMessenger()
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
void VulkanHelper::CreateInstance()
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


    if (USE_WSI) {
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
VulkanHelper::QueueFamilyIndices VulkanHelper::FindQueueFamilies(VkPhysicalDevice newDevice)
{
    QueueFamilyIndices QFIndices;

    /* Retrieving the list of queue families */
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(newDevice, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(newDevice, &queueFamilyCount, queueFamilies.data());

    /* Find at least one queue family that supports VK_QUEUE_GRAPHICS_BIT */
    int i = 0;
    for (const auto& queueFamily : queueFamilies) {

        if (QFIndices.isComplete(useHeadlessRendering)) {
            break;
        }

        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            QFIndices.graphicsFamily = i;
        }

        if(!useHeadlessRendering) {
            /* Check if the device support surface */
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(newDevice, i, surface, &presentSupport);
            if (presentSupport) {
                QFIndices.presentFamily = i;
            }
        }

        i++;
    }
    return QFIndices;
}


/**
* @brief Check if the physical device is suitable for the vulkan rendering.
* @param[in] device: The physical device we are assessing.
* @return True if this device is suitable for the rendering.
*/
bool VulkanHelper::IsDeviceSuitable(VkPhysicalDevice newDevice)
{

    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(newDevice, &deviceProperties);

    /* The support for optional features like texture compression, 64-bit floats and multi viewport rendering */
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(newDevice, &deviceFeatures);

    QueueFamilyIndices QFIndices = FindQueueFamilies(newDevice);

    /* Check support for swap chain*/
    bool extensionsSupported = CheckDeviceExtensionSupport(newDevice);

    bool swapChainAdequate = false;
    if(useHeadlessRendering){
        swapChainAdequate = true;
    }
    else if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(newDevice);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    bool result = deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
           deviceFeatures.geometryShader && QFIndices.isComplete(useHeadlessRendering) && extensionsSupported && swapChainAdequate && deviceFeatures.samplerAnisotropy;

    if(deviceName.empty()) return result;

    /* Check If the device's name matches with the requirement if it is specified. */
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
    return result && deviceProperties.deviceName == deviceName;
}


/**
* @brief Verify that your graphics card is indeed capable of creating a swap chain.
* @param[in] device: The physical device that's using.
* @return Return true if it is capable.
*/
bool VulkanHelper::CheckDeviceExtensionSupport(VkPhysicalDevice newDevice)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(newDevice, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(newDevice, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}


/**
* @brief Pick the graphics card that fits for the need for the Vulkan rendering.
*/
void VulkanHelper::PickPhysicalDevice()
{

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    /* A list of physical devices handles */
    std::vector<VkPhysicalDevice> pDevices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, pDevices.data());

    /* Loop through those devices and check if you find suitable */
    for (const auto& pDevice : pDevices) {
        if (IsDeviceSuitable(pDevice)) {
            physicalDevice = pDevice;
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
void VulkanHelper::CreateLogicalDevice()
{

    QueueFamilyIndices QFIndices = FindQueueFamilies(physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

    std::set<uint32_t> uniqueQueueFamilies = { QFIndices.graphicsFamily.value()};
    if(!useHeadlessRendering){
        uniqueQueueFamilies = { QFIndices.graphicsFamily.value(), QFIndices.presentFamily.value() };
    }

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

    VkPhysicalDeviceFeatures2 deviceFeatures{};
    deviceFeatures.features.samplerAnisotropy = VK_TRUE;     // Enable the anisotropy feature

    /* Enable the extended dynamic state feature. */
    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extendedDynamicStateFeatures = {};
    extendedDynamicStateFeatures.extendedDynamicState = VK_TRUE;
    extendedDynamicStateFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT;

    /* Enable the dynamic vertex input state feature. */
    VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT extendedVertexInputFeatures = {};
    extendedVertexInputFeatures.vertexInputDynamicState = VK_TRUE;
    extendedVertexInputFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT;
    extendedVertexInputFeatures.pNext = &extendedDynamicStateFeatures;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext = &extendedVertexInputFeatures;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());

    createInfo.pEnabledFeatures = &deviceFeatures.features;

    /* Enable extensions for swap chain */
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if(useHeadlessRendering){
        createInfo.enabledExtensionCount--;
    }

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
    vkGetDeviceQueue(device, QFIndices.graphicsFamily.value(), 0, &graphicsQueue);
    if(!useHeadlessRendering) {
        /* Retrieve queue handles for each present family*/
        vkGetDeviceQueue(device, QFIndices.presentFamily.value(), 0, &presentQueue);
    }
}


/**
* @brief Create the window surface that will be used for display the output.
*/
void VulkanHelper::CreateSurface()
{
    VkWin32SurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;

    if (USE_WSI) {
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
void VulkanHelper::CreateSwapChain()
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

    QueueFamilyIndices QFIndices = FindQueueFamilies(physicalDevice);
    uint32_t queueFamilyIndices[] = { QFIndices.graphicsFamily.value(), QFIndices.presentFamily.value() };

    /* If the queue families differ, then we’ll be using the concurrent mode in this tutorial to avoid having to do the ownership chapters */
    if (QFIndices.graphicsFamily != QFIndices.presentFamily) {
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
 * @brief Create the swap chain (a list of images) used for the headless rendering mode.
 */
void VulkanHelper::CreateHeadlessSwapChain(){

    uint32_t imageCount = MAX_FRAMES_IN_FLIGHT;

    swapChainImages.resize(imageCount);
    headlessImageMemory.resize(imageCount);

    for(size_t i = 0; i < imageCount; i++){
        CreateImage(windowWidth,windowHeight,1,1,VK_FORMAT_R8G8B8A8_UNORM,VK_IMAGE_TILING_OPTIMAL,VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 0, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,swapChainImages[i],headlessImageMemory[i]);
    }

    swapChainImageFormat = VK_FORMAT_R8G8B8A8_UNORM;
    swapChainExtent = {windowWidth,windowHeight};
}


/**
* @brief Create an image view instance based on the image and its format.
* @param[in] image: The image we are using.
* @param[in] format: The format of the image view.
* @param[in] aspectFlags: The aspect mask. Can be color or depth.
*/
VkImageView VulkanHelper::CreateImageView(VkImage image, VkFormat format, VkImageViewType viewType, VkImageAspectFlags aspectFlags, uint32_t newMipLevels, uint32_t layerCount)
{
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = viewType;
    viewInfo.format = format;

    /* The subresourceRange field describes what the image’s purpose is and which part of the image should be accessed. */
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = newMipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = layerCount;


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
void VulkanHelper::CreateImageViews()
{
    swapChainImageViews.resize(swapChainImages.size());

    for (uint32_t i = 0; i < swapChainImages.size(); i++) {
        swapChainImageViews[i] = CreateImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT, 1,1);
    }
}


/**
* @brief Create a module to wrap the info in the binary shader vector.
* @param[in] code The binary char array of the shaders
* @return A shader module with file's data.
*/
VkShaderModule VulkanHelper::CreateShaderModule(const std::vector<char>& code)
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
void VulkanHelper::CreateDescriptorSetLayout()
{
    /*
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;    // The type of descriptor is a uniform buffer object
    uboLayoutBinding.descriptorCount = 1;

    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr;


    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 2;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;     // Use the image sampler in the fragment shader stage.

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };


    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    } */
}


/**
* @brief Read the shaders and create the graphics pipeline.
*/
void VulkanHelper::CreateGraphicsPipeline(const std::string& vertexFileName, const std::string& fragmentFileName, const VkDescriptorSetLayout& descriptorSetLayout, VkPipeline& graphicsPipeline, VkPipelineLayout& pipelineLayout)
{
    /* Read the file into a char array */
    auto vertShaderCode = ReadFile(vertexFileName);
    auto fragShaderCode = ReadFile(fragmentFileName);

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

    /* Describe the rasterizer in the pipeline */
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;     // False = discard and True = Clamp
    rasterizer.rasterizerDiscardEnable = VK_FALSE;  // True = disables any output to the framebuffer.
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;  // Fill the area of the polygon with fragments.
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;   // Face culling mode.
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

    /* Describe the color blending in the pipeline */
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

    /* Specify the layout, the 'uniform' item in the pipeline */
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1; // Optional
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout; // Optional
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
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
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = VK_NULL_HANDLE;    // Vertex Input State is nullptr cuz we do it dynamically.
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

    /* Option to create derived pipeline */
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, VK_NULL_HANDLE, &graphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(device, fragShaderModule, nullptr);
    vkDestroyShaderModule(device, vertShaderModule, nullptr);
}


/**
* @brief Create the render pass to attach the framebuffer.
*/
void VulkanHelper::CreateRenderPass()
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
    if(useHeadlessRendering) {
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    }

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

    /* Describe the subPass. Used for example like post-processing */
    VkSubpassDescription subPass{};
    subPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subPass.colorAttachmentCount = 1;
    subPass.pColorAttachments = &colorAttachmentRef;        // Connect it with the color attachment.
    subPass.pDepthStencilAttachment = &depthAttachmentRef;  // Connect it with the depth attachment. Its count is always 1.

    /* Fill the render pass instance with info */
    std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subPass;

    /* Add the subPass dependency to the render pass */
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
void VulkanHelper::CreateFrameBuffers()
{
    swapChainFramebuffers.resize(swapChainImageViews.size());

    /* Iterate through each image and create the buffer for it */
    for (size_t i = 0; i < swapChainImageViews.size(); i++) {

        /* Attach the color image and the depth image into the frame buffer */
        /* The same depth image can be used by all of them because only a single subPass is running at the same time due to our semaphores.*/
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
* @return The selected memory type index
*/
uint32_t VulkanHelper::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{

    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    /* Find the memory that is suitable for the buffer to use */
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        /* typeFilter specify the bit field of memory types */
        /* The properties define special features of the memory, like being able to map it, so we can write to it from the CPU. */
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
void VulkanHelper::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
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
    allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate buffer memory!");
    }

    /* Bind the memory with the vertex buffer */
    vkBindBufferMemory(device, buffer, bufferMemory, 0);
}


/**
 * @brief Start a command buffer. Will be ended in a separate function.
 * @return The command buffer started.
 */
VkCommandBuffer VulkanHelper::BeginSingleTimeCommands() {

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


/**
 * @brief End a given command buffer.
 * @param commandBuffer The command buffer we need to terminate.
 */
void VulkanHelper::EndSingleTimeCommands(VkCommandBuffer commandBuffer) {
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
* @brief Copy a vulkan buffer from source to destination.
* @param[in] srcBuffer: The source buffer.
* @param[in] dstBuffer: The destination buffer.
* @param[in] size: The copied buffer size.
*/
void VulkanHelper::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    EndSingleTimeCommands(commandBuffer);
}


void VulkanHelper::CreateMeshes(){
    if(s72Instance == nullptr){
        throw std::runtime_error("Vertex Buffer Error: s72Instance is null.");
    }

    for(const auto& mesh : s72Instance->meshes){
        VkMeshes[mesh.first] = std::make_shared<VkMesh>();
        VkMeshes[mesh.first]->device = device;
        CreateVertexBuffer(*mesh.second,*VkMeshes[mesh.first]);

        if(mesh.second->isUseIndex){
            VkMeshes[mesh.first]->isUseIndex = mesh.second->isUseIndex;
            CreateIndexBuffer(*mesh.second,*VkMeshes[mesh.first]);
        }

        CreateInstanceBuffer(*VkMeshes[mesh.first]);
    }
}


/**
* @brief Create the vertex buffer to store the vertex data.
* @param[in] newMesh The mesh data which contains the vertices info.
* @param[in] index The index of that mesh instance in the list.
*/
void VulkanHelper::CreateVertexBuffer(const S72Object::Mesh& newMesh, VkMesh& vkMesh)
{
    //VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
    VkDeviceSize bufferSize = newMesh.src.size();

    /* Create a staging buffer as temporary buffer and use a device local one as actual vertex buffer. */
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    /* Copy the vertex data to the buffer using memory copy */
    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, newMesh.src.data(), (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    /* The vertex buffer is now device local */
    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vkMesh.vertexBuffer, vkMesh.vertexBufferMemory);

    /* Copy the buffer from the staging buffer to the device local vertex buffer */
    CopyBuffer(stagingBuffer, vkMesh.vertexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}


/**
 * @brief For a binding description struct based on the info of a mesh instance.
 * @param newMeshInstance The mesh we are construct from.
 * @return newMeshInstance's binding description info.
 */
std::array<VkVertexInputBindingDescription2EXT,2> VulkanHelper::CreateBindingDescription(const S72Object::Mesh& newMeshInstance){
    std::array<VkVertexInputBindingDescription2EXT,2> bindingDescription{};

    /* Vertex binding info. */
    bindingDescription[0].sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT;
    bindingDescription[0].binding = 0;         // Specifies the index of the binding in the array of bindings.
    bindingDescription[0].stride = newMeshInstance.stride;     // Specifies the number of bytes from one entry to the next.
    bindingDescription[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;  // Move to the next data entry after each vertex.
    bindingDescription[0].divisor = 1;

    /* Instance binding info. */
    bindingDescription[1].sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT;
    bindingDescription[1].binding = 1;         // Specifies the index of the binding in the array of bindings.
    bindingDescription[1].stride = sizeof(S72Object::MeshInstance);     // Specifies the number of bytes from one entry to the next.
    bindingDescription[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
    bindingDescription[1].divisor = 1;

    return bindingDescription;
}


/**
 * @brief Form an attribute description struct based on the info of a mesh instance.
 * @param newMeshInstance The mesh we are construct from.
 * @return newMeshInstance's attribute description info.
 */
std::array<VkVertexInputAttributeDescription2EXT, 7> VulkanHelper::CreateAttributeDescription(const S72Object::Mesh& newMeshInstance){

    std::array<VkVertexInputAttributeDescription2EXT, 7> attributeDescriptions{};

    /* Attribute for the vertex data */
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = newMeshInstance.pFormat;  // Describes the type of data for the attribute
    attributeDescriptions[0].offset = newMeshInstance.pOffset;
    attributeDescriptions[0].sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT;

    /* Attribute for the normal data */
    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = newMeshInstance.nFormat;
    attributeDescriptions[1].offset = newMeshInstance.nOffset;
    attributeDescriptions[1].sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT;

    /* Attribute for the color data */
    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = newMeshInstance.cFormat;
    attributeDescriptions[2].offset = newMeshInstance.cOffset;
    attributeDescriptions[2].sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT;

    /* Per instance data. */
    attributeDescriptions[3].binding = 1;
    attributeDescriptions[3].location = 3;
    attributeDescriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[3].offset = offsetof(S72Object::MeshInstance, model);
    attributeDescriptions[3].sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT;

    attributeDescriptions[4].binding = 1;
    attributeDescriptions[4].location = 4;
    attributeDescriptions[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[4].offset = offsetof(S72Object::MeshInstance, model) + sizeof(float)*4;
    attributeDescriptions[4].sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT;

    attributeDescriptions[5].binding = 1;
    attributeDescriptions[5].location = 5;
    attributeDescriptions[5].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[5].offset = offsetof(S72Object::MeshInstance, model) + sizeof(float)*8;
    attributeDescriptions[5].sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT;

    attributeDescriptions[6].binding = 1;
    attributeDescriptions[6].location = 6;
    attributeDescriptions[6].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[6].offset = offsetof(S72Object::MeshInstance, model) + sizeof(float)*12;
    attributeDescriptions[6].sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT;

    return attributeDescriptions;
}


/**
 * @brief Create the index buffer to store the index relations.
 * @param newMesh The mesh object.
 * @param index The index of the mesh in the array.
 */
void VulkanHelper::CreateIndexBuffer(const S72Object::Mesh& newMesh, VkMesh& vkMesh)
{
    VkDeviceSize bufferSize = newMesh.indicesSrc.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, newMesh.indicesSrc.data(), (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    /* One difference is set its usage to INDEX_BUFFER */
    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vkMesh.indexBuffer, vkMesh.indexBufferMemory);

    CopyBuffer(stagingBuffer, vkMesh.indexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}


/**
 * @brief Create a single dynamic instance buffer for a mesh.
 * @param index The index of the instance in the array.
 */
void VulkanHelper::CreateInstanceBuffer(VkMesh& vkMesh){
    VkDeviceSize bufferSize = s72Instance->instanceCount * sizeof(S72Object::MeshInstance);
    CreateBuffer(bufferSize,VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vkMesh.instanceBuffer,vkMesh.instanceBufferMemory);
}


/**
 * @brief Update an instance buffer with the new instance data.
 * @param newMesh The mesh object which has the instance data.
 * @param index The index of the instance in the array.
 */
void VulkanHelper::UpdateInstanceBuffer(const S72Object::Mesh& newMesh){
    // Map dynamic instance buffer memory
    void* mappedData;
    VkDeviceSize bufferSize = s72Instance->instanceCount * sizeof(S72Object::MeshInstance);

    if(!VkMeshes.count(newMesh.name)){
        throw std::runtime_error("Cannot find the vkMesh!");
    }

    vkMapMemory(device, VkMeshes[newMesh.name]->instanceBufferMemory, 0, bufferSize, 0, &mappedData);

    // Copy visible instance data to the mapped memory
    memcpy(mappedData, newMesh.visibleInstances.data(), newMesh.visibleInstances.size() * sizeof(S72Object::MeshInstance));

    // Unmap dynamic instance buffer memory
    vkUnmapMemory(device, VkMeshes[newMesh.name]->instanceBufferMemory);
}


/**
* @brief Create the uniform buffer to store the uniform data.
*/
void VulkanHelper::CreateUniformBuffers()
{
    /* We use a large uniform buffer to store all mesh instance's ubo data */
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
void VulkanHelper::UpdateUniformBuffer(uint32_t currentImage)
{
    /* Define the model, view and projection transformations in the uniform buffer object.*/
    UniformBufferObject ubo{};

    //ubo = mesh.instances.at(instanceIndex);
    ubo.view = currCamera->viewMatrix;
    ubo.proj = currCamera->projMatrix;;
    ubo.viewPos = currCamera->cameraPos;

    /* Flip the Y-Dir */
    ubo.proj.data[1][1] *= -1;

    /* Copy the data to the current uniform buffer */
    memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}


/**
* @brief Create the command pool which will be used to allocate the memory for the command buffer.
*/
void VulkanHelper::CreateCommandPool()
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
void VulkanHelper::CreateCommandBuffers()
{
    commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;                // Allocate the buffer on the command pool
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

    if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate command buffers!");
    }
}


/**
* @brief Writes the commands we want to execute into a command buffer.
* @param[in] commandBuffer: The buffer we are writing to
* @param[in] imageIndex: The index of the current swap chain image we want to write to
*/
void VulkanHelper::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("Failed to begin recording command buffer!");
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
    /* The depth is between 0 and 1 */
    clearValues[1].depthStencil = { 1.0f, 0 };

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    /* Begin the render pass*/
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

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

    std::array<VkVertexInputBindingDescription2EXT,2> newBindingDescription{};
    std::array<VkVertexInputAttributeDescription2EXT, 7> newAttributeDescription{};
    auto vkCmdSetVertexInputExt = (PFN_vkCmdSetVertexInputEXT)vkGetDeviceProcAddr(device, "vkCmdSetVertexInputEXT");
    auto vkCmdSetPrimitiveTopologyEXT = (PFN_vkCmdSetPrimitiveTopologyEXT)( vkGetDeviceProcAddr( device, "vkCmdSetPrimitiveTopologyEXT" ) );

    UpdateUniformBuffer(currentFrame);

    for(const auto& material : VkMaterials){

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material.second->pipeline);

        /* Bind the descriptor sets */
        uint32_t dynamicOffset = 0 * sizeof(UniformBufferObject);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material.second->pipelineLayout, 0, 1, &material.second->descriptorSets[currentFrame], 1,
                                &dynamicOffset);

        for(auto& mesh : s72Instance->meshesByMaterial[material.first]){
            /* Update the visible instance list. */
            if(currCamera->name == "Debug-Camera"){
                mesh->UpdateInstanceWithCulling(s72Instance->cameras["User-Camera"], cullingMode);
            }
            else{
                mesh->UpdateInstanceWithCulling(currCamera, cullingMode);
            }

            /* If no instance will be drawn, go to the next mesh. */
            if(mesh->visibleInstances.empty()){
                continue;
            }

            /* Update the instance buffer with the new instance data. */
            UpdateInstanceBuffer(*mesh);

            /* Bind its vertex buffer and set its info. */
            VkBuffer newVertexBuffers[] = {  VkMeshes[mesh->name]->vertexBuffer , VkMeshes[mesh->name]->instanceBuffer};
            VkDeviceSize offsets[] = { 0, 0 };
            vkCmdBindVertexBuffers(commandBuffer, 0, 2, newVertexBuffers, offsets);

            newBindingDescription = CreateBindingDescription(*mesh);
            newAttributeDescription = CreateAttributeDescription(*mesh);

            vkCmdSetVertexInputExt(commandBuffer,static_cast<uint32_t>(newBindingDescription.size()),newBindingDescription.data(),static_cast<uint32_t>(newAttributeDescription.size()),newAttributeDescription.data());
            vkCmdSetPrimitiveTopologyEXT(commandBuffer,mesh->topology);

            if(mesh->isUseIndex){
                vkCmdDrawIndexed(commandBuffer,mesh->indicesCount,(uint32_t)mesh->visibleInstances.size(),0,0,0);
            }
            else{
                vkCmdDraw(commandBuffer, mesh->count, (uint32_t)mesh->visibleInstances.size(), 0, 0);
            }
        }
    }

    /* End the render pass */
    vkCmdEndRenderPass(commandBuffer);

    /* Finish recording the command buffer */
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}


/**
* @brief Create the image instance.
* @param[in] width: The image's width.
* @param[in] height: The image's height.
* @param[in] format: Image's format
* @param[in] tiling: Image's tiling mode
* @param[in] usage: The image's usage type.
* @param[in] properties: The image's property.
* @param[out] image: The image instance.
* @param[out] imageMemory: The memory used to allocate the image.
*/
void VulkanHelper::CreateImage(uint32_t width, uint32_t height, uint32_t newMipLevels, uint32_t newArrayLayer, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{
    /* Set the info for the texture */
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;                 // Texture is 2D
    imageInfo.extent.width = static_cast<uint32_t>(width);
    imageInfo.extent.height = static_cast<uint32_t>(height);
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = newMipLevels;                                // Mipmap level is 1
    imageInfo.arrayLayers = newArrayLayer;

    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;              // Used for multi-sampling
    imageInfo.flags = flags; // Optional

    if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    /* Allocate the memory for the image */
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

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
void VulkanHelper::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
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


void VulkanHelper::CopyBufferToImageCube(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height,uint32_t nChannel){
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

    std::vector<VkBufferImageCopy> regions;
    uint64_t offset = 0;

    std::array<uint32_t,6> faceOrder{5,4,2,3,1,0};

    for(uint32_t face = 0; face < 6; face++) {
        VkBufferImageCopy region{};
        region.bufferOffset = offset;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = faceOrder[face];
        region.imageSubresource.layerCount = 1;

        region.imageOffset = {0, 0, 0};
        region.imageExtent = {
                width,
                height,
                1
        };

        offset += width * height * nChannel * sizeof(float);
        regions.emplace_back(region);
    }

    vkCmdCopyBufferToImage(
            commandBuffer,
            buffer,
            image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            static_cast<uint32_t>(regions.size()),
            regions.data()
    );

    EndSingleTimeCommands(commandBuffer);
}



/**
* @brief Transit the image's layout with a new layout using a pipeline barrier.
* @param[in] image: The image that is affected and the specific part of the image.
* @param[in] format: The format of the image TODO!!!!!!!!!!!
* @param[in] oldLayout: The old layout we have.
* @param[in] newLayout: The new layout we determine.
*/
void VulkanHelper::TransitionImageLayout(VkImage image, uint32_t layerCount, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t newMipLevels) {
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
    barrier.subresourceRange.levelCount = newMipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = layerCount;

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

    /* Submit the pipeline barrier */
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


void VulkanHelper::ProcessRGBEImage(const unsigned char* src, float*& dst, int texWidth, int texHeight){

    int numPixel = texWidth * texHeight;

    for(int i = 0; i < numPixel*4; i+=4){
        auto r = static_cast<float>(src[i]);
        auto g = static_cast<float>(src[i+1]);
        auto b = static_cast<float>(src[i+2]);
        auto e = static_cast<int>(src[i+3]);

        if(r == 0 && g == 0 && b == 0 && e == 0){
            dst[i] = 0;
            dst[i+1] = 0;
            dst[i+2] = 0;
            dst[i+3] = 1;
            continue;
        }

        r = (r+0.5f)/256;
        g = (g+0.5f)/256;
        b = (b+0.5f)/256;
        e = e - 128;

        r = ldexp(r,e);
        g = ldexp(g,e);
        b = ldexp(b,e);

        // Scale to 0-255 range and store in sRGB array
        dst[i] = r;
        dst[i+1] = g;
        dst[i+2] = b;
        dst[i+3] = 1;
    }
}


void VulkanHelper::CreateCubeTextureImageAndView(const std::string& filename, VkImage& image, VkDeviceMemory& imageMemory, VkImageView& imageView){

    int texWidth, texHeight, texChannels;

    unsigned char* pixelRGBE = stbi_load(filename.c_str(), &texWidth, &texHeight, &texChannels, 4);

    if (!pixelRGBE) {
        throw std::runtime_error("failed to load texture image!");
    }

    VkDeviceSize imageSize = texWidth * texHeight * texChannels;


    auto* pixelRBG = new float[imageSize];
    ProcessRGBEImage(pixelRGBE,pixelRBG,texWidth,texHeight);

    texHeight /= 6;

    uint32_t envMipLevels = static_cast<uint32_t>(std::floor(std::log2(max(texWidth, texHeight)))) + 1;

    /* Create a buffer to store the image data */
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    CreateBuffer(imageSize*sizeof(float), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    /* Copy the image to the buffer */
    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, imageSize*sizeof(float), 0, &data);
    memcpy(data, pixelRBG, static_cast<size_t>(imageSize)*sizeof(float));
    vkUnmapMemory(device, stagingBufferMemory);

    stbi_image_free(pixelRGBE);
    delete[] pixelRBG;

    CreateImage(texWidth, texHeight, envMipLevels, 6,VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, imageMemory);

    /* Copy the staging buffer to the texture image */
    TransitionImageLayout(image, 6, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, envMipLevels);
    CopyBufferToImageCube(stagingBuffer, image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), static_cast<uint32_t>(texChannels));

    /* Clear the stage buffer */
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);

    GenerateMipmaps(image, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, envMipLevels, 6);

    imageView = CreateImageView(image, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_VIEW_TYPE_CUBE, VK_IMAGE_ASPECT_COLOR_BIT, envMipLevels, 6);
}


void VulkanHelper::CreateEnvironments(){
    if(s72Instance == nullptr || s72Instance->envFileName.empty()){
        throw std::runtime_error("failed to get the environment cube map info from s72!");
    }

    CreateCubeTextureImageAndView(s72Instance->envFileName, envTextureImage,envTextureImageMemory,envTextureImageView);

    std::string lamFileName = "lam_" + s72Instance->envFileName;

    CreateCubeTextureImageAndView(lamFileName,lamTextureImage,lamTextureImageMemory,lamTextureImageView);
}


void VulkanHelper::CreateMaterials(){
    for(auto& material : s72Instance->materials){
        std::shared_ptr<VkMaterial> newVkMaterial = std::make_shared<VkMaterial>();
        newVkMaterial->name = material.first;
        newVkMaterial->device = device;
        newVkMaterial->CreateDescriptorSetLayout();
        newVkMaterial->CreateDescriptorPool();

        if(material.first == "simple"){
            newVkMaterial->CreateDescriptorSets(textureSampler,uniformBuffers,nullptr);
        }
        else if(material.first == "environment" || material.first == "mirror"){
            newVkMaterial->CreateDescriptorSets(textureSampler,uniformBuffers,envTextureImageView);
        }

        std::string vertexShader = shaderMap.at(material.first)[0];
        std::string fragShader = shaderMap.at(material.first)[1];
        CreateGraphicsPipeline(vertexShader, fragShader,newVkMaterial->descriptorSetLayout,newVkMaterial->pipeline,newVkMaterial->pipelineLayout);

        VkMaterials[material.first] = newVkMaterial;
    }
}


/**
* @brief Create the texture image with a given texture.
*/
void VulkanHelper::CreateTextureImage()
{
    uint32_t mipLevels = 0;

    /* Load the texture image */
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;      // 4 means 4 bytes for pixel.

    /* Determine the LOD */
    mipLevels = static_cast<uint32_t>(std::floor(std::log2(max(texWidth, texHeight)))) + 1;

    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    /* Create a buffer to store the image data */
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    /* Copy the image to the buffer */
    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(device, stagingBufferMemory);

    stbi_image_free(pixels);

    CreateImage(texWidth, texHeight, mipLevels, 1, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 0, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

    /* Copy the staging buffer to the texture image */
    TransitionImageLayout(textureImage, 1, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
    CopyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

    /* Change texture image's layout for the shade access */
    //transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevels);

    /* Clear the stage buffer */
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);

    GenerateMipmaps(textureImage, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mipLevels,1);
}


/**
* @brief Create the image view to access and present the texture image.
*/
void VulkanHelper::CreateTextureImageView()
{
    uint32_t mipLevels = 0;
    /* Create the texture image view */
    textureImageView = CreateImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT,mipLevels, 1);
}


/**
* @brief Create the texture sampler to access the texture.
* Apply filtering and transformations to compute the final color that is retrieved.
*/
void VulkanHelper::CreateTextureSampler()
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

    /* Get the anisotropy property from the physical device */
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

    /* If normalized, use coordinates within the [0, texWidth) and [0, texHeight) range. */
    samplerInfo.unnormalizedCoordinates = VK_FALSE;

    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

    /* Set the mipmap info for the sampler */
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.minLod = 0.0f; // Optional
    samplerInfo.maxLod = VK_LOD_CLAMP_NONE;
    samplerInfo.mipLodBias = 0.0f; // Optional

    /* Create the Sampler with the info */
    if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}


/**
* @brief Create all the semaphores and fences used for sync updating the command buffers.
*/
void VulkanHelper::CreateSyncObjects()
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
* @brief Find the supported vkFormat that supports the tiling mode and the features.
* @param[in] candidates: A list of available formats.
* @param[in] tiling: The tiling mode we are requiring.
* @param[in] features: The features we want it to support.
* @return A desired vkFormat.
*/
VkFormat VulkanHelper::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
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
* @brief Select a format with a depth component that supports usage as depth attachment.
* @return The found depth format.
*/
VkFormat VulkanHelper::FindDepthFormat() {
    return FindSupportedFormat(
            { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}


/**
* @brief Check if a format contains a stencil component. (Can be used for the depth test)
* @param[in] format: The format we want to testify.
* @return True if it has the component.
*/
bool VulkanHelper::HasStencilComponent(VkFormat format) {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}


/**
* @brief Create the depth image and the image view.
*/
void VulkanHelper::CreateDepthResources()
{
    VkFormat depthFormat = FindDepthFormat();

    /* Create the image with the given format */
    CreateImage(swapChainExtent.width, swapChainExtent.height, 1,1,depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 0, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
    depthImageView = CreateImageView(depthImage, depthFormat, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_DEPTH_BIT,1, 1);
}


/**
 * @brief Copy a VKImage to data through a staging buffer.
 * @param image The image we want to copy.
 * @param imageMemory The memory allocated for the image.
 * @param data The data we copied to.
 */
void VulkanHelper::CopyImageToData(const VkImage& image, const VkDeviceMemory& imageMemory, void*& data){

    VkMemoryRequirements oldMemoryRequirements;
    vkGetImageMemoryRequirements(device, image, &oldMemoryRequirements);

    /* Allocate staging buffer */
    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = oldMemoryRequirements.size;
    bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkBuffer stagingBuffer;
    vkCreateBuffer(device, &bufferCreateInfo, nullptr, &stagingBuffer);

    /* Allocate memory for staging buffer. */
    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(device, stagingBuffer, &memoryRequirements);

    VkMemoryAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = FindMemoryType(memoryRequirements.memoryTypeBits,VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    VkDeviceMemory stagingBufferMemory;
    vkAllocateMemory(device, &allocateInfo, nullptr, &stagingBufferMemory);
    vkBindBufferMemory(device, stagingBuffer, stagingBufferMemory, 0);

    /* Copy data from image to staging buffer */
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();
    VkBufferImageCopy copyRegion = {};
    copyRegion.bufferOffset = 0;
    copyRegion.bufferRowLength = windowWidth;
    copyRegion.bufferImageHeight = windowHeight;
    copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.imageSubresource.mipLevel = 0;
    copyRegion.imageSubresource.baseArrayLayer = 0;
    copyRegion.imageSubresource.layerCount = 1;
    copyRegion.imageOffset = {0, 0, 0};
    copyRegion.imageExtent = {
            windowWidth,
            windowHeight,
            1
    };

    vkCmdCopyImageToBuffer(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, stagingBuffer, 1, &copyRegion);

    /* Transition image layout back if needed */
    vkMapMemory(device, stagingBufferMemory, 0, VK_WHOLE_SIZE, 0, &data);

    EndSingleTimeCommands(commandBuffer);

    vkUnmapMemory(device, stagingBufferMemory);

    /* Clean up */
    vkFreeMemory(device, stagingBufferMemory, nullptr);
    vkDestroyBuffer(device, stagingBuffer, nullptr);
}


/**
 * @brief Save an image to a PPM file.
 * @param image The image we want to save.
 * @param imageMemory The memory allocated for the image.
 * @param filename The target PPM file name.
 */
void VulkanHelper::SaveImageToPPM(const VkImage& image, const VkDeviceMemory& imageMemory,  const std::string& filename){
    /* Map image memory */
    void *mappedMemory;
    /* Read pixel data from image */
    CopyImageToData(image,imageMemory,mappedMemory);

    /* Write pixel data to PPM file */
    std::ofstream ppmFile(filename, std::ios::binary);
    if (!ppmFile.is_open()) {
        throw std::runtime_error("Failed to open PPM file for writing.");
    }

    /* Write PPM header */
    ppmFile << "P6\n";
    ppmFile << windowWidth << " " << windowHeight << "\n";
    ppmFile << "255\n";

    /* Write pixel data */
    auto *row = reinterpret_cast<unsigned int*>(mappedMemory);
    uint32_t y = 0;
    uint32_t x = 0;

    for (; y < windowHeight; y++){
        for (x = 0; x < windowWidth; x++){
            /* We want to avoid writing the alpha data. */
            ppmFile.write((char*)row,3);
            row++;
        }
    }
    /* Close file and unmap memory */
    ppmFile.close();
}


/**
* @brief Clean up the swap chain and all the related resources.
*/
void VulkanHelper::CleanUpSwapChain()
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

    if(useHeadlessRendering){
        for(size_t i = 0; i < swapChainImages.size(); i++){
            vkDestroyImage(device,swapChainImages[i], nullptr);
            vkFreeMemory(device, headlessImageMemory[i] ,nullptr);
        }
    }
    else {
        vkDestroySwapchainKHR(device, swapChain, nullptr);
    }
}


/**
* @brief Recreate the swap chain if the window is changed and the previous one become invalid.
*/
void VulkanHelper::RecreateSwapChain()
{
    /* Pause the window if the frame buffer size is 0 (window minimization)*/
    int width = 0, height = 0;

    if (USE_WSI) {
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
        if (USE_WSI) {
            // TODO:
        }
        else {
            glfwGetFramebufferSize(window, &width, &height);
            glfwWaitEvents();
        }
    }

    /* Wait for the using device to be done */
    vkDeviceWaitIdle(device);

    /* Clean up the previous swap chain data */
    CleanUpSwapChain();

    /* Recreate all the resources that depends on the swap chain */
    CreateSwapChain();
    CreateImageViews();
    CreateDepthResources();
    CreateFrameBuffers();
}


/**
* @brif Generate the mipmaps through the command buffer.
* @param[in] image: The texture image.
* @param[in] imageFormat: The format of the texture.
* @param[in] texWidth: The texture width.
* @param[in] texHeight: The texture height.
* @param[in] mipLevels: The LOD of the texture.
*/
void VulkanHelper::GenerateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t newMipLevels, uint32_t newLayerCount) {

    /* Check if image format supports linear blitting */
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(physicalDevice, imageFormat, &formatProperties);

    if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        throw std::runtime_error("texture image format does not support linear blitting!");
    }

    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = newLayerCount;
    barrier.subresourceRange.levelCount = 1;

    int32_t mipWidth = texWidth;
    int32_t mipHeight = texHeight;

    /* Record each of the VkCmdBlitImage commands */
    for (uint32_t i = 1; i < newMipLevels; i++) {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                             0, nullptr,
                             0, nullptr,
                             1, &barrier);

        /* Set the blit operation's info */
        VkImageBlit blit{};
        blit.srcOffsets[0] = { 0, 0, 0 };
        blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = newLayerCount;
        blit.dstOffsets[0] = { 0, 0, 0 };
        blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = newLayerCount;

        vkCmdBlitImage(commandBuffer,
                       image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       1, &blit,
                       VK_FILTER_LINEAR);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                             0, nullptr,
                             0, nullptr,
                             1, &barrier);

        /* Check each dimension before the division to ensure that dimension never becomes 0. */
        if (mipWidth > 1) mipWidth /= 2;
        if (mipHeight > 1) mipHeight /= 2;
    }

    barrier.subresourceRange.baseMipLevel = newMipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                         0, nullptr,
                         0, nullptr,
                         1, &barrier);

    EndSingleTimeCommands(commandBuffer);
}


/**
* @brief Initialize the Vulkan application and setup.
*/
void VulkanHelper::InitVulkan()
{
    CreateInstance();
    SetupDebugMessenger();

    if(!useHeadlessRendering) {
        CreateSurface();
    }

    PickPhysicalDevice();
    CreateLogicalDevice();

    if(!useHeadlessRendering) {
        CreateSwapChain();
    }
    else{
        CreateHeadlessSwapChain();
    }

    CreateImageViews();
    CreateRenderPass();
    CreateDescriptorSetLayout();
    //CreateGraphicsPipeline();
    CreateCommandPool();
    CreateDepthResources();
    CreateFrameBuffers();
    //CreateTextureImage();
    //CreateTextureImageView();
    //CreateTextureSampler();
    //CreateEnvTextureImage("ox_bridge_morning_512.png");
    //CreateEnvTextureImageView();
    //CreateCubeTextureImageAndView("ox_bridge_morning_512.png",envTextureImage,envTextureImageMemory,envTextureImageView);
    CreateTextureSampler();
    CreateEnvironments();

    //CreateVertexBuffers();
    //CreateIndexBuffers();
    //CreateInstanceBuffers();
    CreateMeshes();
    CreateUniformBuffers();
    //CreateDescriptorPool();
    //CreateDescriptorSets();
    CreateMaterials();
    CreateCommandBuffers();
    CreateSyncObjects();
}


/**
* @brief Create a main loop to let the window keep opening (Looping using the MSG mechanism).
*/
void VulkanHelper::MainLoopWIN()
{
    MSG msg = { };
    while (GetMessage(&msg, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}


/**
 * @brief Set the s72Instance with the new one.
 * @param newS72Instance
 */
void VulkanHelper::SetS72Instance(const std::shared_ptr<S72Helper>& newS72Instance){
    this->s72Instance = newS72Instance;
}


/**
 * @brief Set the window size.
 * @param width New width.
 * @param height New height.
 */
void VulkanHelper::SetWindowSize(uint32_t width, uint32_t height){
    this->windowWidth = width;
    this->windowHeight = height;
}


/**
 * @brief Set the selected physical device name.
 * @param newDeviceName The selected physical device name.
 */
void VulkanHelper::SetDeviceName(const std::string& newDeviceName){
    this->deviceName = newDeviceName;
}


/**
 * @brief Select the culling mode, can be 'none' or 'frustum'.
 * @param newCullingMode The new culling mode.
 */
void VulkanHelper::SetCullingMode(const std::string& newCullingMode){
    this->cullingMode = newCullingMode;
}


/**
 * @brief Set the first camera we want to use.
 * @param cameraName The target camera.
 */
void VulkanHelper::SetCameraName(const std::string& cameraName){

    if(s72Instance == nullptr){
        throw std::runtime_error("Vulkan initialization error: s72Instance is null");
    }

    /* Find if there's a camera with the target name. */
    if(s72Instance->cameras.count(cameraName)){
        currCamera = s72Instance->cameras[cameraName];
    }
    else{
        std::cout << "Could not find the selected camera" << std::endl;
    }
}


/**
 * @brief Select if we want to use the headless mode.
 * @param isUseHeadless True if we want to use the headless mode.
 */
void VulkanHelper::SetHeadlessMode(bool isUseHeadless){
    this->useHeadlessRendering = isUseHeadless;
}


/**
 * @brief Save the rendered result to a PPM file.
 * @param filename The target PPM's file name.
 */
void VulkanHelper::SaveRenderResult(const std::string& filename){
    if(!useHeadlessRendering){
        std::cout << "Cannot save a render image in a on window mode" << std::endl;
    }

    size_t imageIndex = headlessImageIndex - 1;
    if(headlessImageIndex == 0){
        imageIndex = headlessImageMemory.size()-1;
    }

    SaveImageToPPM(swapChainImages[imageIndex], headlessImageMemory[imageIndex], filename);
}


/**
* @brief Run the Vulkan application using the Windows created window.
*/
void VulkanHelper::RunWIN(HINSTANCE new_Instance, HWND new_hwnd)
{
    InitWindowWIN(new_Instance, new_hwnd);
    InitVulkan();
    MainLoopWIN();
}


/**
 * @brief Handle the GLFW input based on the key the user typed.
 * @param key The key the user typed as a char.
 */
void VulkanHelper::ProcessGLFWInputCallBack(char key){
    if(currCamera == nullptr) return;

    switch (key) {
        case 'E':
            currCamera->MoveCameraUpDown(true);
            break;
        case 'Q':
            currCamera->MoveCameraUpDown(false);
            break;
        case 'W':
            currCamera->MoveCameraForwardBackward(true);
            break;
        case 'S':
            currCamera->MoveCameraForwardBackward(false);
            break;
        case 'A':
            currCamera->MoveCameraLeftRight(true);
            break;
        case 'D':
            currCamera->MoveCameraLeftRight(false);
            break;
        case 'R':
            currCamera->ReFocusToCenter();
            break;
        case 'T': {
            if (s72Instance == nullptr) return;

            auto Camera_iterator = s72Instance->cameras.find(currCamera->name);
            Camera_iterator++;

            if (Camera_iterator == s72Instance->cameras.end()) {
                Camera_iterator = s72Instance->cameras.begin();
            }
            currCamera = Camera_iterator->second;
            break;
        }
        case 'Y': {
            if(!s72Instance->isPlayingAnimation){
                s72Instance->StartAnimation();
            }
            else{
                s72Instance->StopAnimation();
            }
            break;
        }
        case 'C': {
            if(cullingMode == "none") cullingMode = "frustum";
            else cullingMode = "none";
            break;
        }
        default:
            break;
    }
}


/**
* @brief Wait for the previous frame to finish.
* Acquire an image from the swap chain.
* Record a command buffer which draws the scene onto that image.
* Submit the recorded command buffer.
* Present the swap chain image.
*/
void VulkanHelper::DrawFrame()
{
    /* Wait until the previous frame has finished */
    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    /* Acquire an image from the swap chain, may need to recreate the swap chain if the image is outdated */
    uint32_t imageIndex;
    VkResult result;

    if(!useHeadlessRendering) {
        result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame],
                                                VK_NULL_HANDLE, &imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            RecreateSwapChain();
            return;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }
    }
    else{
        imageIndex = headlessImageIndex;
        headlessImageIndex = (headlessImageIndex + 1) % headlessImageMemory.size();
    }

    /* Reset the fence after wait. Only reset the fence if we are submitting work */
    vkResetFences(device, 1, &inFlightFences[currentFrame]);

    /* Record the Command Buffer */
    vkResetCommandBuffer(commandBuffers[currentFrame], 0);
    RecordCommandBuffer(commandBuffers[currentFrame], imageIndex);

    /* Create the info to submit the command buffer */
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    if(!useHeadlessRendering) {
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
    }
    else{
        submitInfo.waitSemaphoreCount = 0;
    }

    /* Submit a single command buffer */
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    if(!useHeadlessRendering) {
        /* Signal the semaphore */
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;
    }
    else {
        submitInfo.signalSemaphoreCount = 0;
    }

    /* Submit the command buffer */
    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    if(useHeadlessRendering){
        /* Update the current frame to the next frame index */
        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
        return;
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
void VulkanHelper::CleanUp()
{
    /* Wait for the logical device to finish operations before exiting mainLoop and destroying the window */
    vkDeviceWaitIdle(device);

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

    for(auto& mesh : VkMeshes){
        mesh.second->CleanUp();
    }

    /**********************************/
    vkDestroyImageView(device, envTextureImageView, nullptr);
    vkDestroyImage(device, envTextureImage, nullptr);
    vkFreeMemory(device, envTextureImageMemory, nullptr);

    vkDestroyImageView(device, lamTextureImageView, nullptr);
    vkDestroyImage(device, lamTextureImage, nullptr);
    vkFreeMemory(device, lamTextureImageMemory, nullptr);

    vkDestroySampler(device, textureSampler, nullptr);
    vkDestroyImageView(device, textureImageView, nullptr);
    vkDestroyImage(device, textureImage, nullptr);
    vkFreeMemory(device, textureImageMemory, nullptr);

    for(auto& material : VkMaterials){
        material.second->CleanUp();
    }

    vkDestroyRenderPass(device, renderPass, nullptr);

    vkDestroyCommandPool(device, commandPool, nullptr);     // Command buffer will be freed when the pool is freed

    vkDestroyDevice(device, nullptr);       // Destroy the logical device

    if(!useHeadlessRendering) {
        vkDestroySurfaceKHR(instance, surface, nullptr);    // Destroy the surface instance
    }
    vkDestroyInstance(instance, nullptr);   // Destroy the vulkan instance

    if (USE_WSI) {
        DestroyWindow(hwnd);
        PostQuitMessage(0);
    }
    else if(!useHeadlessRendering) {
        glfwDestroyWindow(window);      // Close the window
        glfwTerminate();            // GLFW destruction
    }
}