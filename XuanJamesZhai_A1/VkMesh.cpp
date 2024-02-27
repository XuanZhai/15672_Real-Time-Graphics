//
// Created by Xuan Zhai on 2024/2/25.
//

#include "VkMesh.h"


/**
 * @brief Destruct the buffers and free the memory.
 */
void VkMesh::CleanUp(){

        vkDestroyBuffer(device, vertexBuffer, nullptr);
        vkFreeMemory(device, vertexBufferMemory, nullptr);

        if(isUseIndex) {
            vkDestroyBuffer(device, indexBuffer, nullptr);
            vkFreeMemory(device, indexBufferMemory, nullptr);
        }

        vkDestroyBuffer(device, instanceBuffer, nullptr);
        vkFreeMemory(device, instanceBufferMemory, nullptr);
}