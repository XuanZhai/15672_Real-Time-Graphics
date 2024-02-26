//
// Created by Xuan Zhai on 2024/2/25.
//

#ifndef XUANJAMESZHAI_A1_VKMATERIAL_ENVIRONMENTMIRROR_H
#define XUANJAMESZHAI_A1_VKMATERIAL_ENVIRONMENTMIRROR_H

#include "VkMesh.h"

class VkMaterial_EnvironmentMirror : public VkMaterial {

public:
    void CreateDescriptorSetLayout() override;

    void CreateDescriptorPool() override;

    void CreateDescriptorSets(const std::vector<VkBuffer>& uniformBuffers, const VkSampler& textureSampler, const VkImageView& cubeMap);
};


#endif //XUANJAMESZHAI_A1_VKMATERIAL_ENVIRONMENTMIRROR_H
