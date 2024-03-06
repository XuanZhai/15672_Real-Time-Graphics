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

/* Reference: Inspired by https://github.com/ixchow/15-466-ibl/blob/master/cubes/blur_cube.cpp */

/* Correspond to the order of the face. */
enum EFace{
    Right,
    Left,
    Front,
    Back,
    Up,
    Down
};

/**
 * @brief The data of a pixel in a image. Will have its face index, and its uv index.
 */
struct Pixel_Ref{
    EFace face = EFace::Right;
    size_t u = 0;
    size_t v = 0;

    bool operator < (const Pixel_Ref& obj) const {
        return face < obj.face;
    }
};


/**
 * @brief Used to calculate the brightest direction.
 */
struct BrightDirection{
    XZM::vec3 dir = XZM::vec3();
    XZM::vec3 light = XZM::vec3();
};


class Cube {

protected:
    std::string srcName;

    /* Order: Right, Left, Front, Back, Up, Down */
    std::array<XZM::vec3**,6> cubeMaps;
    int cubeMapWidth = 0;
    int cubeMapHeight = 0;
    int cubeMapChannel = 0;

    /* A list of brightest direction, sort by the brightness. */
    std::vector<BrightDirection> brightDirections;

    /* Number of samples when doing the Monte-Carlo. */
    uint32_t nSamples = 0;

    /* Used multi-threading to process each face. */
    std::array<std::thread,6> threads;

    std::array<XZM::vec3**,6> outMaps;
    uint32_t outMapWidth = 0;
    uint32_t outMapHeight = 0;

    /* Load a face data. from a loaded RGBE image. */
    void LoadFace(const unsigned char* src, EFace face, int width, int height);
    /* Collect the brightest directions. */
    void ProcessBright();
    /* Given a direction, sum its project with the brightest directions. */
    virtual XZM::vec3 SumBrightDirection(const XZM::vec3& dir);
    /* Project a given direction to the cube map, retrieve its color info. */
    XZM::vec3 Projection(const XZM::vec3& dir);
    /* Read a face data to a RGBE image. */
    void ReadFace(unsigned char*& dst, EFace face, uint32_t width, uint32_t height);
    /* Reset the output data to 0. */
    void ClearOutput();

public:
    /* Read a RGBE src image from a given path and name. */
    void ReadFile(const std::string& fileName);
    /* Process the Monte-Carlo estimation. Will be inherited by child classes. */
    virtual void Processing(uint32_t nSamples, uint32_t outWidth, uint32_t outHeight) = 0;
    /* Destruct all the allocated data. */
    virtual ~Cube();
};


#endif //CUBES_CUBE_H
