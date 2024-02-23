//
// Created by Xuan Zhai on 2024/2/22.
//

#include "EnvironmentMap.h"



void EnvironmentMap::LoadEnvironmentTexture(const std::string& filename) {

    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(filename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

    if(pixels == nullptr){
        throw std::runtime_error("failed to load the cube map image!");
    }

    texHeight /= 6;
    int faceSize = texWidth * texHeight * texChannels;

    for(int i = 0; i < 6; i++){
        cubeMap[i] = std::shared_ptr<stbi_uc[]>(new stbi_uc[faceSize]);
        int xOffset = (i % texChannels) * texWidth;
        int yOffset = (i / texChannels) * texHeight;
        for (int y = 0; y < texHeight; ++y) {
            for (int x = 0; x < texWidth; ++x) {
                int srcIndex = ((yOffset + y) * texWidth + (xOffset + x)) * texChannels;
                int dstIndex = (y * texWidth + x) * texChannels;
                for (int c = 0; c < texChannels; ++c) {
                    cubeMap[i][dstIndex + c] = pixels[srcIndex + c];
                }
            }
        }
    }
}
