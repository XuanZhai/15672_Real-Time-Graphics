//
// Created by Xuan Zhai on 2024/2/25.
//

#ifndef XUANJAMESZHAI_A1_S72MATERIALS_H
#define XUANJAMESZHAI_A1_S72MATERIALS_H

#include <string>
#include <cmath>
#include "S72Helper.h"


namespace S72Object{

    /**
     * @brief The S72-side material object. Contain all the data in the .72 files.
     */
    class Material{
        public:
            std::string name;
            // TODO: May have other data.

            /* Read a node and load all the info. */
            virtual void ProcessMaterial(const std::shared_ptr<ParserNode>& node);
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
}


#endif //XUANJAMESZHAI_A1_S72MATERIALS_H
