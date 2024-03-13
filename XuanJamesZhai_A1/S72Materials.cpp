//
// Created by Xuan Zhai on 2024/2/25.
//

#include "S72Materials.h"
#include "stb_image.h"


/**
 * @brief Overload < operator used for the map container.
 * @param newMat The compared material.
 * @return If this is smaller.
 */
bool S72Object::Material::operator < (const Material& newMat) const{
    if(this->type != newMat.type){
        return this->type < newMat.type;
    }
    return this->name < newMat.name;
}


/**
 * @brief Default create pool function.
 * @param device The physical device.
 */
void S72Object::Material::CreateDescriptorPool(const VkDevice& device){}


/**
 * @brief Default create layout function.
 * @param device The physical device.
 */
void S72Object::Material::CreateDescriptorSetLayout(const VkDevice& device) {}


/**
 * @brief Read a node and load all the info.
 * @param node The node we want to load.
 */
void S72Object::Material::ProcessMaterial(const std::shared_ptr<ParserNode>& node){

    if(node == nullptr){
        name = "XZDefault";
    }
    else{
        name = std::get<std::string>(node->GetObjectValue("name")->data);
    }

    if(node != nullptr && node->GetObjectValue("normalMap") != nullptr){
        auto normalObject = node->GetObjectValue("normalMap");
        auto src = normalObject->GetObjectValue("src");

        ReadPNG( S72Helper::s72fileName + "/../" + std::get<std::string>(src->data),normalMap,normalMapWidth,normalMapHeight,normalMapChannel,normalMipLevels);
    }
    else{
        normalMap = std::string() + (char) (128u) + (char) (128u) + (char) (255u) + (char)(255u);
        normalMapWidth = 1;
        normalMapHeight = 1;
        normalMapChannel = 4;
        normalMipLevels = 1;
    }

    if(node != nullptr && node->GetObjectValue("displacementMap") != nullptr){
        auto normalObject = node->GetObjectValue("displacementMap");
        auto src = normalObject->GetObjectValue("src");

        ReadPNG( S72Helper::s72fileName + "/../" + std::get<std::string>(src->data),heightMap,heightMapWidth,heightMapHeight,heightMapChannel,heightMapMipLevels);
    }
    else{
        heightMap = std::string() + (char) (0 * 256);
        heightMapWidth = 1;
        heightMapHeight = 1;
        heightMapChannel = 1;
        heightMapMipLevels = 1;
    }
}


/**
 * @brief Read a PNG from a file path.
 * @param filename The file path and name.
 * @param src The target container for the image.
 * @param width The image width.
 * @param height The image height.
 * @param nChannels The image number of channels.
 * @param mipLevels The image's mip map level.s
 */
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
                src.push_back((char)(255u));
                index+=3;
            }
        }
        nChannels = 4;
    }

    mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
}


/**
 * @brief Deallocate and free the memory of the normal/displacement data, as well as the pool.
 * @param device The physical device.
 */
void S72Object::Material::CleanUp(const VkDevice& device){
    vkDestroyImageView(device, normalImageView, nullptr);
    vkDestroyImage(device, normalImage, nullptr);
    vkFreeMemory(device, normalImageMemory, nullptr);

    vkDestroyImageView(device, heightImageView, nullptr);
    vkDestroyImage(device, heightImage, nullptr);
    vkFreeMemory(device, heightImageMemory, nullptr);

    vkDestroyDescriptorPool(device, MDescriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(device, MDescriptorSetLayout, nullptr);
}
