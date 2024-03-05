//
// Created by Xuan Zhai on 2024/3/2.
//

#ifndef CUBES_LAMBERTIAN_H
#define CUBES_LAMBERTIAN_H

#include "Cube.h"
#include <random>

/* Reference: Inspired by https://github.com/ixchow/15-466-ibl/blob/master/cubes/blur_cube.cpp */


/**
 * @brief A child class of Cube. Create the Lambertian lookup table.
 */
class Lambertian : public Cube{

    /* Sample a direction. */
    static XZM::vec3 MakeSample();

public:
    /* An override function for doing the Lambertian Monte-Carlo. */
    void Processing(uint32_t nSamples, uint32_t outWidth, uint32_t outHeight) override;
    /* Process the Lambertian Monte-Carlo for a given output face. */
    void ProcessingFace(EFace face);
    /* Save the output as a png file. */
    void SaveOutput();
};


#endif //CUBES_LAMBERTIAN_H
