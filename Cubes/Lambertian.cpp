//
// Created by Xuan Zhai on 2024/3/2.
//

#include "Lambertian.h"

#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


XZM::vec3 Lambertian::MakeSample(){
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0, 1);

    XZM::vec3 rv((float)dis(gen), (float)dis(gen),1);
    float phi = rv.data[0] * 2.0f * (float)M_PI;
    float r = std::sqrt(rv.data[1]);
    return {
            std::cos(phi) * r,
            std::sin(phi) * r,
            std::sqrt(1.0f - rv.data[1])
    };
}


void Lambertian::Processing(uint32_t newNSamples, int outWidth, int outHeight){
    outMapWidth = outWidth;
    outMapHeight = outHeight;
    nSamples = newNSamples;

    ProcessBright();

    for(int face = 0; face < 6; face++){
        outMaps[face] = new XZM::vec3*[outMapHeight];
        //outMaps[face].resize(outMapHeight);
        for(int row = 0; row < outMapHeight; row++){
            //outMaps[face][row].resize(outMapWidth);
            outMaps[face][row] = new XZM::vec3[outMapWidth];
        }

        threads[face] = std::thread(&Lambertian::ProcessingFace,this,static_cast<EFace>(face));
    }

    for(auto face = 0; face < 6; face++){
        threads[face].join();
    }

    SaveOutput();
}


void Lambertian::ProcessingFace(EFace face) {

    std::cout << "Sampling face " << face << "/6 ..." << std::endl;
    XZM::vec3 sc; //maps to rightward axis on face
    XZM::vec3 tc; //maps to upward axis on face
    XZM::vec3 rc; //direction to face

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

    for (uint32_t v = 0; v < (uint32_t) outMapHeight; v++) {
        for (uint32_t u = 0; u < (uint32_t) outMapWidth; u++) {
            XZM::vec3 N = XZM::Normalize(rc + sc * (2.0f * ((float) u + 0.5f) / (float) outMapHeight - 1.0f) +
                                         tc * (2.0f * ((float) v + 0.5f) / (float) outMapWidth - 1.0f));
            XZM::vec3 temp = (abs(N.data[2]) < 0.99f ? XZM::vec3(0.0f, 0.0f, 1.0f) : XZM::vec3(1.0f, 0.0f, 0.0f));
            XZM::vec3 TX = XZM::Normalize(XZM::CrossProduct(N, temp));
            XZM::vec3 TY = XZM::CrossProduct(N, TX);

            XZM::vec3 acc = XZM::vec3(0.0f, 0.0f, 0.0f);

            for (uint32_t i = 0; i < uint32_t(nSamples); ++i) {
                XZM::vec3 sampleDir = MakeSample();
                acc += Projection(XZM::vec3(TX * sampleDir.data[0] + TY * sampleDir.data[1] + N * sampleDir.data[2]));
                //acc += (dir.x * TX + dir.y * TY + dir.z * N) * 0.5f + 0.5f; //DEBUG
                //acc += ((TX*sampleDir.data[0] + TY*sampleDir.data[1] + N*sampleDir.data[2]) * 0.5f + 0.5f);
            }

            acc = acc * (1.0f / float(nSamples));
            acc += SumBrightDirection(N);
            outMaps[face][v][u] = acc;
        }
    }
    std::cout << "Sampling face " << face+1 << "/6 Finished" << std::endl;
}


void Lambertian::SaveOutput(){

    auto* dst = new stbi_uc[outMapWidth*outMapHeight*4*6];

    for(int face = 0; face < 6; face++){
        ReadFace(dst,face,outMapWidth,outMapHeight);
    }

    std::string outFileName = inputName + "_lam.png";
    stbi_write_png(outFileName.c_str(), outMapWidth, outMapHeight*6, 4, dst, outMapWidth * 4);

    delete[] dst;
}

