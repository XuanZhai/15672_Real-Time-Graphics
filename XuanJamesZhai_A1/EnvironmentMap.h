//
// Created by Xuan Zhai on 2024/2/22.
//

#ifndef XUANJAMESZHAI_A1_ENVIRONMENTMAP_H
#define XUANJAMESZHAI_A1_ENVIRONMENTMAP_H

#include <string>
#include <memory>
#include <array>
#include <stdexcept>
#include "stb_image.h"

const int num = 1;

class EnvironmentMap {

public:
    uint32_t mipLevels = 0;

    std::array<std::shared_ptr<stbi_uc[]>,6> cubeMap;


    void LoadEnvironmentTexture(const std::string& filename);
};


#endif //XUANJAMESZHAI_A1_ENVIRONMENTMAP_H
