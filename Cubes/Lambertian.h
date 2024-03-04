//
// Created by Xuan Zhai on 2024/3/2.
//

#ifndef CUBES_LAMBERTIAN_H
#define CUBES_LAMBERTIAN_H

#include "Cube.h"
#include <random>


class Lambertian : public Cube{


    XZM::vec3 MakeSample() override;

public:

    void Processing(uint32_t nSamples, int outWidth, int outHeight) override;

    void ProcessingFace(EFace face);

    void SaveOutput() override;
};


#endif //CUBES_LAMBERTIAN_H
