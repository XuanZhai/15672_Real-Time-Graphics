//
// Created by Xuan Zhai on 2024/3/3.
//

#include "Cube.h"
#include <random>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


void Cube::LoadFace(const stbi_uc *src, int face, int width, int height) {

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


void Cube::ReadFile(const std::string &fileName) {

    inputName = fileName;

    stbi_uc* src = stbi_load(fileName.c_str(),&cubeMapWidth,&cubeMapHeight,&cubeMapChannel,0);

    if(src == nullptr){
        throw std::runtime_error("Cannot find input file.");
    }

    cubeMapHeight /= 6;

    for(int face = 0; face < 6; face++){
        cubeMaps[face] = new XZM::vec3*[cubeMapHeight];
        //cubeMaps[face].resize(cubeMapHeight);
        for(int row = 0; row < cubeMapHeight; row++){
            cubeMaps[face][row] = new XZM::vec3[cubeMapWidth];
            //cubeMaps[face][row].resize(cubeMapWidth);
        }

        LoadFace(src,face,cubeMapWidth,cubeMapHeight);
    }

    stbi_image_free(src);
    std::cout << "Input loaded" << std::endl;
}


void Cube::ProcessBright(){
    uint32_t bright = std::min< uint32_t >(cubeMapWidth*cubeMapHeight*6, 10000);

    std::vector<std::pair<float, Pixel_Ref>> pixelList;
    pixelList.resize(cubeMapWidth*cubeMapHeight*6);

    for(size_t face = 0; face < 6; face++){
        for(uint32_t v = 0; v < cubeMapHeight; v++){
            for(uint32_t u = 0; u < cubeMapWidth; u++){
                pixelList.emplace_back();
                pixelList.back().first = std::max({cubeMaps[face][v][u].data[0],cubeMaps[face][v][u].data[1],cubeMaps[face][v][u].data[2]});
                pixelList.back().second.face = face;
                pixelList.back().second.v = v;
                pixelList.back().second.u = u;
            }
        }
    }

    std::stable_sort(pixelList.rbegin(),pixelList.rend());

    for(auto i = 0; i < bright; i++){
        XZM::vec3 sc; //maps to rightward axis on face
        XZM::vec3 tc; //maps to upward axis on face
        XZM::vec3 rc; //direction to face

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


XZM::vec3 Cube::SumBrightDirection(const XZM::vec3& dir){
    XZM::vec3 ret = XZM::vec3();
    for (auto const &bd : brightDirections) {
        ret += (bd.light * std::max(0.0f, XZM::DotProduct(bd.dir, dir)));
    }
    return ret;
}


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


void Cube::ReadFace(unsigned char*& dst, int face, int width, int height){

    int numPixel = width * height;
    int offSet = numPixel * 4 * face;

    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            float r = outMaps[face][i][j].data[0];
            float g = outMaps[face][i][j].data[1];
            float b = outMaps[face][i][j].data[2];

            float d = std::max(r, std::max(g, b));
            if (d <= 1e-32f) {
                //return glm::u8vec4(0,0,0,0);
                dst[offSet + 4*i*width + 4*j] = 0;
                dst[offSet + 4*i*width + 4*j+1] = 0;
                dst[offSet + 4*i*width + 4*j+2] = 0;
                dst[offSet + 4*i*width + 4*j+3] = 0;
            }

            int e;
            float fac = 255.999f * (std::frexp(d, &e) / d);

            //value is too large to represent, clamp to bright white:
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


Cube::~Cube() {
    for(int face = 0; face < 6; face++){
        for(int row = 0; row < cubeMapHeight; row++){
            delete[] cubeMaps[face][row];
        }
        delete[] cubeMaps[face];

        for(int row = 0; row < outMapHeight; row++){
            delete[] outMaps[face][row];
        }
        delete[] outMaps[face];
    }
}
