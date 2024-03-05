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

    /**
     * @brief The S72-side material object. Contain all the data in the .72 files.
     */
    class Material{
        public:
            std::string name;
            std::string normal;
            int normalHeight = 0;
            int normalWidth = 0;
            int normalChannel = 0;
            uint32_t normalMipLevels;

            /* Read a node and load all the info. */
            virtual void ProcessMaterial(const std::shared_ptr<ParserNode>& node);

            static void ReadPNG(const std::string& filename, std::string& src, int& width, int& height, int& nChannels, uint32_t& mipLevels);
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

            void ProcessMaterial(const std::shared_ptr<ParserNode>& node) override;
    };


    class Material_PBR : public Material{
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

            void ProcessMaterial(const std::shared_ptr<ParserNode>& node) override;
    };
}


#endif //XUANJAMESZHAI_A1_S72MATERIALS_H
