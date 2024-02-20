//
// Created by Xuan Zhai on 2024/2/15.
//

#ifndef XUANJAMESZHAI_A1_RENDERHELPER_H
#define XUANJAMESZHAI_A1_RENDERHELPER_H

#include <string>
#include <memory>
#include "VulkanHelper.h"

/**
 * @brief The render mode we can use.
 * PerformanceTest mode is for doing the performance & bottleneck analysis.
 */
enum RenderMode{
    OnWindow,
    Headless,
    PerformanceTest
};


/**
 * @brief A helper class that will connect the Vulkan, S72 and the event handler.
 */
class RenderHelper {

private:
    std::shared_ptr<VulkanHelper> vulkanHelper = nullptr;
    std::shared_ptr<S72Helper> s72Helper = nullptr;
    std::shared_ptr<EventHelper> eventHelper = nullptr;

    /* The render mode we want to use for rendering. */
    RenderMode renderMode = OnWindow;

    /* A start time for the headless rendering. */
    std::chrono::system_clock::time_point eventStartTimePoint;

    /* The iteration count to do the performance test. */
    size_t performanceTestCount = 0;

public:
    RenderHelper();

    /* Read a s72 file to the s72 instance. */
    void ReadS72(const std::string& fileName);

    /* Attach the s72 instance to the vulkan helper. */
    void AttachS72ToVulkan();

    /* Process the event file. */
    void SetEventFile(const std::string& eventFileName);

    /* Set the performance test iteration count to decide if we do the performance test. */
    void SetPerformanceTest(size_t count);

    /* Set the vulkan data from the command line arguments. */
    void SetVulkanData(uint32_t width,uint32_t height, const std::string& deviceName, const std::string& cameraName,
                        const std::string& cullingMode);

    /* Initialize the vulkan helper. */
    void InitVulkan();

    /* Run the vulkan renderer. */
    void RunVulkan();

    /* Clean the vulkan data when stopped. */
    void ClearVulkan();
};


#endif //XUANJAMESZHAI_A1_RENDERHELPER_H
