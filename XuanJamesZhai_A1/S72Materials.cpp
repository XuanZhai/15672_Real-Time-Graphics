//
// Created by Xuan Zhai on 2024/2/25.
//

#include "S72Materials.h"
#include "stb_image.h"

/**
 * @brief Read a node and load all the info.
 * @param node The node we want to load.
 */
void S72Object::Material::ProcessMaterial(const std::shared_ptr<ParserNode>& node){


    name = std::get<std::string>(node->GetObjectValue("name")->data);

    if(node->GetObjectValue("normalMap") != nullptr){
        auto normalObject = node->GetObjectValue("normalMap");
        auto src = normalObject->GetObjectValue("src");

        ReadPNG( S72Helper::s72fileName + "/../" + std::get<std::string>(src->data),normal,normalWidth,normalHeight,normalChannel,normalMipLevels);
    }
    else{
        normal = std::string() + (char) (0.5f * 256) + (char) (0.5f * 256) + (char) (255) + (char)(255);
        normalWidth = 1;
        normalHeight = 1;
        normalChannel = 4;
        normalMipLevels = 1;
    }

}


void S72Object::Material::ReadPNG(const std::string& filename, std::string& src, int& width, int& height, int& nChannels, uint32_t& mipLevels){

    unsigned char* image = stbi_load(filename.c_str(), &width, &height, &nChannels, 0);

    if (!image) {
        throw std::runtime_error("failed to load texture image!");
    }

    int size = width * height * nChannels;

    if(nChannels != 3){
        src = std::string( reinterpret_cast< char const* >(image),size);
    }
    else{
        src.reserve(width * height * 4);
        int index = 0;
        for(int i = 0; i < width; i++){
            for(int j = 0; j < height; j++){
                src.push_back((char)image[index]);
                src.push_back((char)image[index+1]);
                src.push_back((char)image[index+2]);
                src.push_back((char)(255));
                index+=3;
            }
        }
        nChannels = 4;
    }

    mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
}


/**
 * @brief Read a node and load all the info. Overrode for the lambertian material.
 * @param node The node we want to load.
 */
void S72Object::Material_Lambertian::ProcessMaterial(const std::shared_ptr<ParserNode> &node) {

    Material::ProcessMaterial(node);

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
    }
    else {
        std::string src = std::get<std::string>(newAlbedo->GetObjectValue("src")->data);
        ReadPNG(src,albedo,albedoWidth,albedoHeight,albedoChannel,albedoMipLevels);
    }
}


void S72Object::Material_PBR::ProcessMaterial(const std::shared_ptr<ParserNode>& node){

    Material::ProcessMaterial(node);

    if (node->GetObjectValue("pbr") == nullptr) {
        throw std::runtime_error("It is not a pbr material!");
    }

    auto pbr = node->GetObjectValue("pbr");

    /* Read the albedo value. */
    auto albedoNode = pbr->GetObjectValue("albedo");
    if (std::get_if<ParserNode::PNVector>(&albedoNode->data) != nullptr) {
        ParserNode::PNVector color = std::get<ParserNode::PNVector>(albedoNode->data);

        float r = std::get<float>(color[0]->data);
        float g = std::get<float>(color[1]->data);
        float b = std::get<float>(color[2]->data);

        albedo = std::string() + (char) (r * 256) + (char) (g * 256) + (char) (b * 256) + (char)(255);
        albedoHeight = 1;
        albedoWidth = 1;
        albedoChannel = 4;
        albedoMipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(albedoWidth, albedoHeight)))) + 1;
    }
    else {
        std::string src = std::get<std::string>(albedoNode->GetObjectValue("src")->data);
        ReadPNG(src,albedo,albedoWidth,albedoHeight,albedoChannel,albedoMipLevels);
    }

    /* Read the roughness value. */
    auto roughnessNode = pbr->GetObjectValue("roughness");
    if(std::get_if<float>(&roughnessNode->data) != nullptr){
        float r = std::get<float>(roughnessNode->data);
        roughness = std::string() + (char) (r * 256);
        roughnessWidth = 1;
        roughnessHeight = 1;
        roughnessMipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(roughnessWidth, roughnessHeight)))) + 1;
    }
    else{
        std::string src = std::get<std::string>(roughnessNode->GetObjectValue("src")->data);
        int tempChannel = 0;
        ReadPNG(src,roughness,roughnessWidth,roughnessHeight,tempChannel,roughnessMipLevels);
    }

    /* Read the metallic value. */
    auto metallicNode = pbr->GetObjectValue("metalness");
    if(std::get_if<float>(&metallicNode->data) != nullptr){
        float r = std::get<float>(metallicNode->data);
        metallic = std::string() + (char) (r * 256);
        metallicWidth = 1;
        metallicHeight = 1;
        metallicMipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(metallicWidth, metallicHeight)))) + 1;
    }
    else{
        std::string src = std::get<std::string>(metallicNode->GetObjectValue("src")->data);
        int tempChannel = 0;
        ReadPNG(src,metallic,metallicWidth,metallicHeight,tempChannel,metallicMipLevels);
    }
}
