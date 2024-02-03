
#include <iostream>
#include <string>
#include "VulkanHelper.h"
#include "S72Helper.h"

/* The s72 scene file name passed from the command line. */
static std::string sceneName;

/* The camera name passed from the command line. */
static std::string cameraName;

/* The window width passed from the command line. */
static size_t drawingWidth;

/* The Window height passed from the command line. */
static size_t drawingHeight;


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
            drawingWidth = strtoul(argv[i+1],nullptr,0);
            drawingHeight = strtoul(argv[i+2],nullptr,0);
        }
    }
}


int main(int argc, char** argv) {

    ReadCMDArguments(argc,argv);

    S72Helper newHelper;

    newHelper.ReadS72("sg-Articulation.s72");

    /*
    auto* TriInstance = new VulkanHelper();

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
     */

    return 0;
}
