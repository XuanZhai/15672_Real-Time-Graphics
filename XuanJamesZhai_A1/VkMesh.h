//
// Created by Xuan Zhai on 2024/2/25.
//

#ifndef XUANJAMESZHAI_A1_VKMESH_H
#define XUANJAMESZHAI_A1_VKMESH_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include "S72Helper.h"

class VkMesh {

public:

    std::string name;

    VkDevice device;

    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;

    VkBuffer instanceBuffer;
    VkDeviceMemory instanceBufferMemory;

    bool isUseIndex;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    void CleanUp();
};


#endif //XUANJAMESZHAI_A1_VKMESH_H
