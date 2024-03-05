//
// Created by Xuan Zhai on 2024/3/3.
//

#include "Cube.h"
#include <random>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


/**
 * @brief Load a face data from a loaded RGBE image. Stored as a float images.
 * @param src The source RGBE image.
 * @param face The face we want to load.
 * @param width The image's width.
 * @param height The image's height.
 */
void Cube::LoadFace(const stbi_uc *src, EFace face, int width, int height) {

    int numPixel = width * height;
    int offSet = numPixel * 4 * face;

    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            auto r = static_cast<float>(src[offSet + 4*i*width + 4*j]);
            auto g = static_cast<float>(src[offSet + 4*i*width + 4*j+1]);
            auto b = static_cast<float>(src[offSet + 4*i*width + 4*j+2]);
            auto e = static_cast<int>(src[offSet + 4*i*width + 4*j+3]);

            /* If it is 0. */
            if(r == 0 && g == 0 && b == 0 && e == 0){
                cubeMaps[face][i][j].data[0] = 0;
                cubeMaps[face][i][j].data[1] = 0;
                cubeMaps[face][i][j].data[2] = 0;
                continue;
            }

            r = (r+0.5f)/256;
            g = (g+0.5f)/256;
            b = (b+0.5f)/256;
            e = e - 128;

            cubeMaps[face][i][j].data[0] = ldexp(r,e);
            cubeMaps[face][i][j].data[1] = ldexp(g,e);
            cubeMaps[face][i][j].data[2] = ldexp(b,e);
        }
    }
}


/**
 * @brief Read a RGBE src image from a given path and name.
 * @param fileName The source file path and name.
 */
void Cube::ReadFile(const std::string &fileName) {

    srcName = fileName;

    stbi_uc* src = stbi_load(fileName.c_str(),&cubeMapWidth,&cubeMapHeight,&cubeMapChannel,0);

    if(src == nullptr){
        throw std::runtime_error("Cannot find input file.");
    }

    cubeMapHeight /= 6;

    for(int face = 0; face < 6; face++){
        cubeMaps[face] = new XZM::vec3*[cubeMapHeight];
        for(int row = 0; row < cubeMapHeight; row++){
            cubeMaps[face][row] = new XZM::vec3[cubeMapWidth];
        }
        LoadFace(src, static_cast<EFace>(face), cubeMapWidth, cubeMapHeight);
    }

    stbi_image_free(src);
    printf("Cube: Input cube map loaded.\n");
}


/**
 * @brief Collect the brightest directions.
 */
void Cube::ProcessBright(){
    uint32_t bright = std::min< uint32_t >(cubeMapWidth*cubeMapHeight*6, 10000);

    std::vector<std::pair<float, Pixel_Ref>> pixelList;
    pixelList.resize(cubeMapWidth*cubeMapHeight*6);

    for(size_t face = 0; face < 6; face++){
        for(uint32_t v = 0; v < cubeMapHeight; v++){
            for(uint32_t u = 0; u < cubeMapWidth; u++){
                pixelList.emplace_back();
                pixelList.back().first = std::max({cubeMaps[face][v][u].data[0],cubeMaps[face][v][u].data[1],cubeMaps[face][v][u].data[2]});
                pixelList.back().second.face = static_cast<EFace>(face);
                pixelList.back().second.v = v;
                pixelList.back().second.u = u;
            }
        }
    }

    /* Sort all the pixels with the brightness. */
    std::stable_sort(pixelList.rbegin(),pixelList.rend());

    for(auto i = 0; i < bright; i++){
        XZM::vec3 sc;
        XZM::vec3 tc;
        XZM::vec3 rc;

        auto face = pixelList[i].second.face;
        auto u = pixelList[i].second.u;
        auto v = pixelList[i].second.v;

        if (face == EFace::Right) {
            sc = XZM::vec3(0.0f, 0.0f, -1.0f);
            tc = XZM::vec3(0.0f, -1.0f, 0.0f);
            rc = XZM::vec3(1.0f, 0.0f, 0.0f);
        }
        else if (face == EFace::Left) {
            sc = XZM::vec3(0.0f, 0.0f, 1.0f);
            tc = XZM::vec3(0.0f, -1.0f, 0.0f);
            rc = XZM::vec3(-1.0f, 0.0f, 0.0f);
        }
        else if (face == EFace::Front) {
            sc = XZM::vec3(1.0f, 0.0f, 0.0f);
            tc = XZM::vec3(0.0f, 0.0f, 1.0f);
            rc = XZM::vec3(0.0f, 1.0f, 0.0f);
        }
        else if (face == EFace::Back) {
            sc = XZM::vec3(1.0f, 0.0f, 0.0f);
            tc = XZM::vec3(0.0f, 0.0f, -1.0f);
            rc = XZM::vec3(0.0f, -1.0f, 0.0f);
        }
        else if (face == EFace::Up) {
            sc = XZM::vec3(1.0f, 0.0f, 0.0f);
            tc = XZM::vec3(0.0f, -1.0f, 0.0f);
            rc = XZM::vec3(0.0f, 0.0f, 1.0f);
        }
        else if (face == EFace::Down) {
            sc = XZM::vec3(-1.0f, 0.0f, 0.0f);
            tc = XZM::vec3(0.0f, -1.0f, 0.0f);
            rc = XZM::vec3(0.0f, 0.0f, -1.0f);
        }

        XZM::vec3 N = XZM::Normalize(rc + sc * (2.0f * ((float) u + 0.5f) / (float) cubeMapHeight - 1.0f) +
                                     tc * (2.0f * ((float) v + 0.5f) / (float) cubeMapWidth - 1.0f));

        brightDirections.emplace_back();
        brightDirections.back().dir = N;
        float solid_angle = 4.0f * (float)M_PI / float(6.0f * (float)cubeMapWidth * (float)cubeMapHeight); // approximate, since pixels on cube actually take up different amounts depending on position
        brightDirections.back().light = cubeMaps[face][v][u] * solid_angle;
        cubeMaps[face][v][u] = XZM::vec3(0,0,0);
    }
}


/**
 * @brief Given a direction, sum its project with the brightest directions.
 * @param dir The input direction.
 * @return A cos-weighted of the light info between the input direction and the bright directions.
 */
XZM::vec3 Cube::SumBrightDirection(const XZM::vec3& dir){
    XZM::vec3 ret = XZM::vec3();
    for (auto const &bd : brightDirections) {
        ret += (bd.light * std::max(0.0f, XZM::DotProduct(bd.dir, dir)));
    }
    return ret;
}


/**
 * @brief Project a given direction to the cube map, retrieve its color info.
 * @param dir The input direction.
 * @return The light info of that projected pixel on the cube map.
 */
XZM::vec3 Cube::Projection(const XZM::vec3 &dir) {
    float sc, tc, rc;
    uint32_t face;

    if (std::abs(dir.data[0]) >= std::abs(dir.data[1]) && std::abs(dir.data[0]) >= std::abs(dir.data[2])) {
        if (dir.data[0] >= 0) { sc = -dir.data[2]; tc = -dir.data[1]; rc = dir.data[0]; face = EFace::Right; }
        else            { sc =  dir.data[2]; tc = -dir.data[1]; rc = dir.data[0]; face = EFace::Left; }
    } else if (std::abs(dir.data[1]) >= std::abs(dir.data[2])) {
        if (dir.data[1] >= 0) { sc =  dir.data[0]; tc =  dir.data[2]; rc = dir.data[1]; face = EFace::Front; }
        else            { sc =  dir.data[0]; tc = -dir.data[2]; rc = dir.data[1]; face = EFace::Back; }
    } else {
        if (dir.data[2] >= 0) { sc =  dir.data[0]; tc = -dir.data[1]; rc = dir.data[2]; face = EFace::Up; }
        else            { sc = -dir.data[0]; tc = -dir.data[1]; rc = dir.data[2]; face = EFace::Down; }
    }

    int32_t u = std::floor(0.5f * (sc / std::abs(rc) + 1.0f) * (float)cubeMapWidth);
    u = std::max(0, std::min(int32_t(cubeMapWidth)-1, u));
    int32_t v = std::floor(0.5f * (tc / std::abs(rc) + 1.0f) * (float)cubeMapHeight);
    v = std::max(0, std::min(int32_t(cubeMapHeight)-1, v));

    return cubeMaps[face][v][u];
}


/**
 * @brief Read a face data to a RGBE image.
 * @param dst The target image we want to store to.
 * @param face The face index.
 * @param width The output width.
 * @param height The output height.
 */
void Cube::ReadFace(unsigned char*& dst, EFace face, uint32_t width, uint32_t height){

    uint32_t numPixel = width * height;
    uint32_t offSet = numPixel * 4 * face;

    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            float r = outMaps[face][i][j].data[0];
            float g = outMaps[face][i][j].data[1];
            float b = outMaps[face][i][j].data[2];

            float d = std::max(r, std::max(g, b));
            if (d <= 1e-32f) {
                dst[offSet + 4*i*width + 4*j] = 0;
                dst[offSet + 4*i*width + 4*j+1] = 0;
                dst[offSet + 4*i*width + 4*j+2] = 0;
                dst[offSet + 4*i*width + 4*j+3] = 0;
            }

            int e;
            float fac = 255.999f * (std::frexp(d, &e) / d);

            /* value is too large to represent, clamp. */
            if (e > 127) {
                dst[offSet + 4*i*width + 4*j] = 0xff;
                dst[offSet + 4*i*width + 4*j+1] = 0xff;
                dst[offSet + 4*i*width + 4*j+2] = 0xff;
                dst[offSet + 4*i*width + 4*j+3] = 0xff;
            }

            dst[offSet + 4*i*width + 4*j] = std::max(0, int32_t(r * fac));
            dst[offSet + 4*i*width + 4*j+1] = std::max(0, int32_t(g * fac));
            dst[offSet + 4*i*width + 4*j+2] = std::max(0, int32_t(b * fac));
            dst[offSet + 4*i*width + 4*j+3] = e + 128;
        }
    }
}


/**
 * @brief Reset the output data to 0.
 */
void Cube::ClearOutput(){

    if(outMaps[0] == nullptr || outMaps[0][0] == nullptr) return;

    for(auto face = 0; face < 6; face++){
        for(auto v = 0; v < outMapHeight; v++){
            for(auto u = 0; u < outMapWidth; u++){
                outMaps[face][v][u] = XZM::vec3();
            }
        }
    }
}


/**
 * @brief Destruct all the allocated data.
 */
Cube::~Cube() {
    for(int face = 0; face < 6; face++){
        /* Clear the cube map. */
        for(int row = 0; row < cubeMapHeight; row++){
            delete[] cubeMaps[face][row];
        }
        delete[] cubeMaps[face];

        /* Clear the output map. */
        for(int row = 0; row < outMapHeight; row++){
            delete[] outMaps[face][row];
        }
        delete[] outMaps[face];
    }
}
