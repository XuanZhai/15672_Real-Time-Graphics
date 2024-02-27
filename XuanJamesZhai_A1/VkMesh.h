//
// Created by Xuan Zhai on 2024/2/25.
//

#ifndef XUANJAMESZHAI_A1_VKMESH_H
#define XUANJAMESZHAI_A1_VKMESH_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include "S72Helper.h"

/**
 * @brief A Vulkan-side mesh object. Contains all the buffers it needs to create mesh.
 */
class VkMesh {

public:

    std::string name;

    /* The physical device it is working on. */
    VkDevice device;

    /* The vertex buffer. */
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;

    /* The instance buffer. */
    VkBuffer instanceBuffer;
    VkDeviceMemory instanceBufferMemory;

    /* The index info. */
    bool isUseIndex;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    /* Destructor. */
    void CleanUp();
};


#endif //XUANJAMESZHAI_A1_VKMESH_H
