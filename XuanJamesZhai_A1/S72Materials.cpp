//
// Created by Xuan Zhai on 2024/2/25.
//

#include "S72Materials.h"

/**
 * @brief Read a node and load all the info.
 * @param node The node we want to load.
 */
void S72Object::Material::ProcessMaterial(const std::shared_ptr<ParserNode>& node){


    name = std::get<std::string>(node->GetObjectValue("name")->data);

    // TODO: Process other value like the normal map.

}


/**
 * @brief Read a node and load all the info. Overrode for the lambertian material.
 * @param node The node we want to load.
 */
void S72Object::Material_Lambertian::ProcessMaterial(const std::shared_ptr<ParserNode> &node) {

    name = std::get<std::string>(node->GetObjectValue("name")->data);

    if (node->GetObjectValue("lambertian") == nullptr) {
        throw std::runtime_error("It is not a lambertian material!");
    }

    auto lambertian = node->GetObjectValue("lambertian");
    auto newAlbedo = lambertian->GetObjectValue("albedo");

    if (std::get_if<ParserNode::PNVector>(&newAlbedo->data) != nullptr) {
        ParserNode::PNVector color = std::get<ParserNode::PNVector>(newAlbedo->data);

        float r = std::get<float>(color[0]->data);
        float g = std::get<float>(color[1]->data);
        float b = std::get<float>(color[2]->data);

        albedo = std::string() + (char) (r * 256) + (char) (g * 256) + (char) (b * 256) + (char)(255);
        albedoHeight = 1;
        albedoWidth = 1;
        albedoChannel = 4;
        albedoMipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(albedoWidth, albedoHeight)))) + 1;
    } else {
        // TODO: Read a png albedo color map.
    }
}
