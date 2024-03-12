//
// Created by Xuan Zhai on 2024/2/25.
//

#ifndef XUANJAMESZHAI_A1_S72MATERIALS_H
#define XUANJAMESZHAI_A1_S72MATERIALS_H

#include <string>
#include <cmath>
#include "S72Helper.h"
#include "stb_image.h"

namespace S72Object{

    /* The material types. */
    enum class EMaterial{
        simple,
        environment,
        mirror,
        lambertian,
        pbr
    };

    /**
     * @brief The S72-side material object. Contain all the data in the .72 files,
     * as well as the descriptor sets.
     */
    class Material{
        public:
            std::string name;
            EMaterial type = EMaterial::simple;
            std::string normalMap;
            int normalMapHeight = 0;
            int normalMapWidth = 0;
            int normalMapChannel = 0;
            uint32_t normalMipLevels;

            std::string heightMap;
            int heightMapHeight = 0;
            int heightMapWidth = 0;
            int heightMapChannel = 0;
            uint32_t heightMapMipLevels;

            VkImage normalImage = VK_NULL_HANDLE;
            VkDeviceMemory normalImageMemory = VK_NULL_HANDLE;
            VkImageView normalImageView = VK_NULL_HANDLE;

            VkImage heightImage = VK_NULL_HANDLE;
            VkDeviceMemory heightImageMemory = VK_NULL_HANDLE;
            VkImageView heightImageView = VK_NULL_HANDLE;

            VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
            std::vector<VkDescriptorSet> descriptorSets;

            /* A list of meshes that use this material. */
            std::vector<std::shared_ptr<S72Object::Mesh>> meshes;

            /* Overload < operator used for the map container. */
            bool operator < (const Material& newMat) const;
            /* Read a node and load all the info. */
            virtual void ProcessMaterial(const std::shared_ptr<ParserNode>& node);
            /* Read a PNG from a file path. */
            static void ReadPNG(const std::string& filename, std::string& src, int& width, int& height, int& nChannels, uint32_t& mipLevels);
            /* Default create pool function. */
            virtual void CreateDescriptorPool(const VkDevice& device);
            /* Deallocate and free the memory of the normal/displacement data, as well as the pool. */
            virtual void CleanUp(const VkDevice& device);
    };


    /**
     * @brief Overload material for the Simple Material type.
     */
    class Material_Simple : public Material{
    public:
        void CreateDescriptorPool(const VkDevice& device) override;
        void CreateDescriptorSets(const VkDevice& device, const VkDescriptorSetLayout& descriptorSetLayout,
                                  const std::vector<VkBuffer>& uniformBuffers,VkSampler const &textureSampler);
    };


    /**
    * @brief Overload material for the Environment/Mirror Material type.
    */
    class Material_EnvMirror : public Material{
    public:
        void CreateDescriptorPool(const VkDevice& device) override;
        void CreateDescriptorSets(const VkDevice& device, const VkDescriptorSetLayout& descriptorSetLayout,
                                  const std::vector<VkBuffer>& uniformBuffers,VkSampler const &textureSampler, VkImageView const &cubeMap);
    };


    /**
     * @brief A subclass of Material, used for the lambertian material.
     */
    class Material_Lambertian : public Material{

        public:
            std::string albedo;
            int albedoHeight = 0;
            int albedoWidth = 0;
            int albedoChannel = 0;
            uint32_t albedoMipLevels;

            VkImage albedoImage = VK_NULL_HANDLE;
            VkDeviceMemory albedoImageMemory = VK_NULL_HANDLE;
            VkImageView albedoImageView = VK_NULL_HANDLE;

            void ProcessMaterial(const std::shared_ptr<ParserNode>& node) override;

            void CreateDescriptorPool(const VkDevice& device) override;

            void CreateDescriptorSets(const VkDevice& device, const VkDescriptorSetLayout& descriptorSetLayout,
                                      const std::vector<VkBuffer> &uniformBuffers,VkSampler const &textureSampler,
                                      const VkImageView& cubeMap);

            void CleanUp(const VkDevice& device) override;
    };


    /**
    * @brief Overload material for the PBR Material type.
    */
    class Material_PBR : public Material {
        public:
            std::string albedo;
            int albedoHeight = 0;
            int albedoWidth = 0;
            int albedoChannel = 0;
            uint32_t albedoMipLevels;

            std::string roughness;
            int roughnessHeight = 0;
            int roughnessWidth = 0;
            uint32_t roughnessMipLevels;

            std::string metallic;
            int metallicHeight = 0;
            int metallicWidth = 0;
            uint32_t metallicMipLevels;

            VkImage albedoImage = VK_NULL_HANDLE;
            VkDeviceMemory albedoImageMemory = VK_NULL_HANDLE;
            VkImageView albedoImageView = VK_NULL_HANDLE;

            VkImage roughnessImage = VK_NULL_HANDLE;
            VkDeviceMemory roughnessImageMemory = VK_NULL_HANDLE;
            VkImageView roughnessImageView = VK_NULL_HANDLE;

            VkImage metallicImage = VK_NULL_HANDLE;
            VkDeviceMemory metallicImageMemory = VK_NULL_HANDLE;
            VkImageView metallicImageView = VK_NULL_HANDLE;

            void ProcessMaterial(const std::shared_ptr<ParserNode>& node) override;

            void CreateDescriptorPool(const VkDevice& device) override;

            void CreateDescriptorSets(const VkDevice& device, const VkDescriptorSetLayout& descriptorSetLayout,
                                      const std::vector<VkBuffer>& uniformBuffers, const VkSampler& textureSampler,
                                      const std::vector<VkImageView>& cubeMaps, const VkImageView& brdfLUT);

            void CleanUp(const VkDevice& device) override;
    };
}


#endif //XUANJAMESZHAI_A1_S72MATERIALS_H
