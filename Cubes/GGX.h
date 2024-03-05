//
// Created by Xuan Zhai on 2024/3/4.
//

#ifndef CUBES_GGX_H
#define CUBES_GGX_H

#include "Cube.h"

/** Reference: Inspired by https://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
 *  and https://learnopengl.com/PBR/IBL/Specular-IBL
 */


/**
 * @brief A child class of Cube. Create the GGX lookup table.
 */
class GGX : public Cube{

    /* The roughness input of the GGX sampling. from 0.0 to 1.0 */
    float roughness = 0.0f;

    /* The Van Der Corput sequence which mirrors a decimal binary representation around its decimal point. */
    static float RadicalInverse_VdC(unsigned int bits);
    /* The Hammersley Sequence for the low discrepancy sequence. */
    static std::pair<float,float> Hammersley(unsigned int i, unsigned int N);
    /* Make a GGX sample based on the Hammersley Sequence. */
    [[nodiscard]] XZM::vec3 MakeSample(const std::pair<float,float>& Xi) const;

public:
    /* An override function for doing the GGX Monte-Carlo. */
    void Processing(uint32_t newNSamples, uint32_t outWidth, uint32_t outHeight) override;
    /* Process the Lambertian Monte-Carlo for a given output face. */
    void ProcessingFace(EFace face);
    /* Save the output as a png file. */
    void SaveOutput();
};


#endif //CUBES_GGX_H
