//
// Created by Xuan Zhai on 2024/2/15.
//

#include "RenderHelper.h"


/**
 * @brief Default constructor will allocate the helpers.
 */
RenderHelper::RenderHelper() {
    vulkanHelper = std::make_shared<VulkanHelper>();
    s72Helper = std::make_shared<S72Helper>();
}


/**
 * @brief Read and parse a s72 file.
 * @param fileName The target file path and name.
 */
void RenderHelper::ReadS72(const std::string& fileName){
    s72Helper->ReadS72(fileName);
}


/**
 * @brief Set the vulkan helper's referenced s72 instance.
 */
void RenderHelper::AttachS72ToVulkan(){
    vulkanHelper->SetS72Instance(s72Helper);
}


/**
 * @brief Process the event file.
 * @param eventFileName The event file's path and name.
 */
void RenderHelper::SetEventFile(const std::string& eventFileName){
    if(eventFileName.empty()){
        return;
    }

    /* We want to set it to headless so that it will do headless rendering if there's an event file. */
    renderMode = RenderMode::Headless;
    eventHelper = std::make_shared<EventHelper>();
    eventHelper->ReadEventFile(eventFileName);
}


/**
 * @brief Update the performance test count and mode;
 * @param count
 */
void RenderHelper::SetPerformanceTest(size_t count){
    performanceTestCount = count;

    if(performanceTestCount != 0){
        renderMode = RenderMode::PerformanceTest;
    }
}


/**
 * @brief Set the vulkan instance with the data from the command line arguments.
 * @param width new window width.
 * @param height new window height.
 * @param deviceName selected physical device name.
 * @param cameraName selected camera's name.
 * @param cullingMode selected culling mode.
 */
void RenderHelper::SetVulkanData(uint32_t  width, uint32_t  height, const std::string &deviceName,
                                 const std::string &cameraName, const std::string &cullingMode) {
    vulkanHelper->SetWindowSize(width,height);

    if(!deviceName.empty()){
        vulkanHelper->SetDeviceName(deviceName);
    }

    if(!cullingMode.empty()){
        vulkanHelper->SetCullingMode(cullingMode);
    }

    if(!cameraName.empty()){
        vulkanHelper->SetCameraName(cameraName);
    }
    else{
        vulkanHelper->SetCameraName("User-Camera");
    }

    /* Both the headless mode and the performance test mode will do the headless rendering. */
    vulkanHelper->SetHeadlessMode(renderMode == RenderMode::Headless || renderMode == RenderMode::PerformanceTest);
}


/**
 * @brief Run the vulkan renderer.
 */
void RenderHelper::RunVulkan(){
    /* If in the headless mode.*/
    if(renderMode == RenderMode::Headless){
        eventStartTimePoint = std::chrono::system_clock::now();
        while(true){
            /* If reach to the end of the event file. */
            if(eventHelper->EventAllFinished()){
                break;
            }
            float duration = std::chrono::duration<float, std::chrono::microseconds::period>(std::chrono::system_clock::now() - eventStartTimePoint).count();
            /* Find a list of matched action. */
            eventHelper->GetMatchedNode(duration);
            /* Run those actions. */
            for(size_t i = eventHelper->startIndex; i < eventHelper->endIndex; i++){
                if(eventHelper->events[i].eventType == EventType::AVAILABLE){
                    s72Helper->UpdateObjects();
                    vulkanHelper->DrawFrame();
                }
                else if(eventHelper->events[i].eventType == EventType::PLAY){
                    s72Helper->StopAnimation();
                    s72Helper->currDuration = 0;
                    s72Helper->StartAnimation();
                    s72Helper->UpdateObjects();
                    vulkanHelper->DrawFrame();
                }
                else if(eventHelper->events[i].eventType == EventType::SAVE){
                    std::string ppmFileName = std::get<std::string>(eventHelper->events[i].data);
                    s72Helper->UpdateObjects();
                    vulkanHelper->DrawFrame();
                    vulkanHelper->SaveRenderResult(ppmFileName);
                }
                else if(eventHelper->events[i].eventType == EventType::MARK){
                    std::cout << std::get<std::string>(eventHelper->events[i].data) << std::endl;
                }
            }
            /* Update the action list as a sliding window. */
            eventHelper->startIndex = eventHelper->endIndex;
        }
    }
    /* If it is the performance test mode. */
    else if(renderMode == RenderMode::PerformanceTest){
        float totalUpdate = 0;
        float totalRender = 0;
        for(size_t i = 0; i < performanceTestCount; i++) {
            auto beforeUpdate = std::chrono::system_clock::now();
            s72Helper->UpdateObjects();
            auto beforeRender = std::chrono::system_clock::now();
            vulkanHelper->DrawFrame();
            auto afterRender = std::chrono::system_clock::now();
            totalUpdate += std::chrono::duration<float, std::chrono::milliseconds::period>(beforeRender - beforeUpdate).count();
            totalRender += std::chrono::duration<float, std::chrono::milliseconds::period>(afterRender - beforeRender).count();
            std::cout << "Finish " << i << std::endl;
        }
        std::cout << "The average time to update the scene graph is: " << totalUpdate/(float)performanceTestCount << "ms" << std::endl;
        std::cout << "The average time to draw the scene is: " << totalRender/(float)performanceTestCount << "ms" << std::endl;
    }
    /* If it is the on window mode. */
    else {
        while (!glfwWindowShouldClose(vulkanHelper->window)) {
            glfwPollEvents();       // Check for inputs
            /* Conditions for updating the scene graph. */
            if(s72Helper->isPlayingAnimation) {
                s72Helper->UpdateObjects();
            }
            vulkanHelper->DrawFrame();
        }
    }
}


/**
 * @brief Initialize the GLFW window and the vulkan instance.
 */
void RenderHelper::InitVulkan() {
    vulkanHelper->InitWindow();
    vulkanHelper->InitVulkan();
}


/**
 * @brief Clean the vulkan usage and free the memory.
 */
void RenderHelper::ClearVulkan() {
    vulkanHelper->CleanUp();
}
