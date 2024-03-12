#include <iostream>
#include <memory>
#include "Cube.h"
#include "Lambertian.h"
#include "GGX.h"

/* The source file path and name. */
std::string src = "src.png";
/* The mode of processing. Can be 'Lambertian' or 'GGX'. */
std::string mode = "Lambertian";
/* The number of sample we take when doing the Monte Carlo. */
uint32_t numSample = 7000;
/* The output images' pixel size for each face. */
uint32_t outputSize = 16;

/**
 * @brief Read the arguments from the command line.
 * @param argc The number of arguments.
 * @param argv The arguments.
 */
void ReadCMDArguments(int argc, char** argv){
    for(int i = 1; i < argc; i++){
        if(strcmp(argv[i],"--src") == 0){
            src = argv[i+1];
        }
        if(strcmp(argv[i],"--mode") == 0){
            mode = argv[i+1];
        }
        if(strcmp(argv[i],"--sample") == 0){
            numSample = strtoul(argv[i+1],nullptr,0);
        }
        if(strcmp(argv[i],"--output") == 0){
            outputSize = strtoul(argv[i+1],nullptr,0);
        }
    }
}


int main(int argc, char** argv) {

    ReadCMDArguments(argc,argv);

    std::shared_ptr<Cube> obj;
    if(mode == "GGX"){
        obj = std::make_shared<GGX>();
    }
    else{
        obj = std::make_shared<Lambertian>();
    }

    obj->ReadFile(src);
    obj->Processing(numSample,outputSize,outputSize);

    return 0;
}
