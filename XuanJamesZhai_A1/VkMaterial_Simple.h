//
// Created by Xuan Zhai on 2024/2/25.
//

#ifndef XUANJAMESZHAI_A1_VKMATERIAL_SIMPLE_H
#define XUANJAMESZHAI_A1_VKMATERIAL_SIMPLE_H

#include "VkMaterial.h"

struct UniformBufferObject;

class VkMaterial_Simple : public VkMaterial{

public:
    void CreateDescriptorSetLayout() override;

    void CreateDescriptorPool() override;

    void CreateDescriptorSets(const std::vector<VkBuffer>& uniformBuffers);
};


#endif //XUANJAMESZHAI_A1_VKMATERIAL_SIMPLE_H
