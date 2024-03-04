//
// Created by Xuan Zhai on 2024/3/3.
//

#ifndef CUBES_CUBE_H
#define CUBES_CUBE_H

#define _USE_MATH_DEFINES
#include <cmath>
#include <memory>
#include <string>
#include <stdexcept>
#include <array>
#include <iostream>
#include <vector>
#include <thread>
#include <algorithm>
#include "XZMath.h"

enum EFace{
    Right,
    Left,
    Front,
    Back,
    Up,
    Down
};


struct Pixel_Ref{
    uint32_t face = 0;
    size_t u = 0;
    size_t v = 0;

    bool operator < (const Pixel_Ref& obj) const {
        return face < obj.face;
    }
};


struct BrightDirection{
    XZM::vec3 dir = XZM::vec3();
    XZM::vec3 light = XZM::vec3();
};


class Cube {

protected:
    std::string inputName;

    /* Order: Right, Left, Front, Back, Up, Down */
    //std::array<std::vector<std::vector<XZM::vec3>>,6> cubeMaps;
    std::array<XZM::vec3**,6> cubeMaps;
    int cubeMapWidth = 0;
    int cubeMapHeight = 0;
    int cubeMapChannel = 0;

    std::vector<BrightDirection> brightDirections;

    uint32_t nSamples = 0;
    std::array<std::thread,6> threads;

    //std::array<std::vector<std::vector<XZM::vec3>>,6> outMaps;
    std::array<XZM::vec3**,6> outMaps;
    int outMapWidth = 0;
    int outMapHeight = 0;

    void LoadFace(const unsigned char* src, int face, int width, int height);

    void ProcessBright();

    XZM::vec3 SumBrightDirection(const XZM::vec3& dir);

    virtual XZM::vec3 MakeSample() = 0;

    XZM::vec3 Projection(const XZM::vec3& dir);

    void ReadFace(unsigned char*& dst, int face, int width, int height);

    void ClearOutput();

public:

    void ReadFile(const std::string& fileName);

    virtual void Processing(uint32_t nSamples, int outWidth, int outHeight) = 0;

    virtual void SaveOutput() = 0;

    virtual ~Cube();
};


#endif //CUBES_CUBE_H
