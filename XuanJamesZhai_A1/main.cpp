
#include <iostream>
#include <string>
#include <memory>
#include "VulkanHelper.h"

/* The s72 scene file name passed from the command line. */
static std::string sceneName;

/* The camera name passed from the command line. */
static std::string cameraName;

/* The name of the physical device that will be used for Vulkan. */
static std::string deviceName;

/* The window width passed from the command line. */
static size_t windowWidth = 1920;

/* The Window height passed from the command line. */
static size_t windowHeight = 1080;

/* The culling mode applied when render the scene. */
static std::string cullingMode = "none";

/* The file path and the name of the events. */
static std::string eventFileName;

/* A dynamic allocated instance of the VKHelper. */
static std::shared_ptr<VulkanHelper> newVKHelper = std::make_shared<VulkanHelper>();


/**
 * @brief Read the command line arguments and record their values.
 * @param argc The number of arguments
 * @param argv The char array of the arguments
 */
void ReadCMDArguments(int argc, char** argv){
    for(int i = 1; i < argc; i++){
        if(strcmp(argv[i],"--scene") == 0){
            sceneName = argv[i+1];
        }
        else if(strcmp(argv[i],"--camera") == 0){
            cameraName = argv[i+1];
        }
        else if(strcmp(argv[i],"--drawing-size") == 0){
            windowWidth = strtoul(argv[i+1],nullptr,0);
            windowHeight = strtoul(argv[i+2],nullptr,0);
        }
        else if(strcmp(argv[i],"--culling") == 0){
            cullingMode = argv[i+1];
        }
        else if(strcmp(argv[i],"--headless") == 0){
            eventFileName = argv[i+1];
        }
    }
}


int main(int argc, char** argv) {

    ReadCMDArguments(argc,argv);

    /* Parse the s72 file. */
    std::shared_ptr<S72Helper> newS72Helper(new S72Helper());
    newS72Helper->ReadS72(sceneName);

    try
    {
        /* Initialize the vulkan helper and pass data from the command line arguments. */
        newVKHelper->InitializeData(newS72Helper,windowWidth,windowHeight,deviceName,cameraName,cullingMode);
        newVKHelper->Run();
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(e.what());
    }
    return EXIT_SUCCESS;
}
