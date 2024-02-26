//
// Created by Xuan Zhai on 2024/2/25.
//

#ifndef XUANJAMESZHAI_A1_S72MATERIALS_H
#define XUANJAMESZHAI_A1_S72MATERIALS_H

#include <string>
#include <cmath>
#include "S72Helper.h"


namespace S72Object{

    class Material{
        public:
            std::string name;
            // TODO: May have other data.

            virtual void ProcessMaterial(const std::shared_ptr<ParserNode>& node);
    };


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
