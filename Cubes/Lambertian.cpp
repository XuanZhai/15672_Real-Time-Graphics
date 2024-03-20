//
// Created by Xuan Zhai on 2024/3/2.
//

#include "Lambertian.h"

#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


/**
 * @brief Sample a direction. Since it's lambertian we uniformly pick a random direction.
 * @return The sampled direction.
 */
XZM::vec3 Lambertian::MakeSample(){
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0, 1);

    /* Random around phi and r^2. */
    XZM::vec3 rv((float)dis(gen), (float)dis(gen),1);
    float phi = rv.data[0] * 2.0f * (float)M_PI;
    float r = std::sqrt(rv.data[1]);
    return {
            std::cos(phi) * r,
            std::sin(phi) * r,
            std::sqrt(1.0f - rv.data[1])
    };
}


/**
 * @brief Importance sampling around the bright directions/
 * @param dir The direction of the surface normal.
 * @return The accumulated result.
 */
XZM::vec3 Lambertian::SumBrightDirection(const XZM::vec3& dir) {
    XZM::vec3 ret = XZM::vec3();
    float totalWeight = 0;

    for (const auto& bd : brightDirections) {
        float NoL = std::max(0.0f, std::min(1.0f, XZM::DotProduct(dir,bd.dir)));

        if(NoL > 0.995) {
            ret += (bd.light * NoL);
            totalWeight += NoL;
        }
    }
    return ret;
}


/**
 * @brief An override function for doing the Lambertian Monte-Carlo.
 * @param newNSamples The number of samples to take.
 * @param outWidth The output image width.
 * @param outHeight The output image height.
 */
void Lambertian::Processing(uint32_t newNSamples, uint32_t outWidth, uint32_t outHeight){
    outMapWidth = outWidth;
    outMapHeight = outHeight;
    nSamples = newNSamples;

    /* Collect the brightest part. */
    //ProcessBright();

    for(int face = 0; face < 6; face++){
        outMaps[face] = new XZM::vec3*[outMapHeight];
        for(int row = 0; row < outMapHeight; row++){
            outMaps[face][row] = new XZM::vec3[outMapWidth];
        }
        /* Create a new thread to process each face. */
        threads[face] = std::thread(&Lambertian::ProcessingFace,this,static_cast<EFace>(face));
    }
    /* Wait for all the works are done. */
    for(auto face = 0; face < 6; face++){
        threads[face].join();
    }
    /* Save the data. */
    SaveOutput();
}


/**
 * @brief Process the Lambertian Monte-Carlo for a given output face.
 * @param face The face we want to process.
 */
void Lambertian::ProcessingFace(EFace face) {

    printf("Lambertian: Sampling face %d/6... \n", face);
    XZM::vec3 sc;
    XZM::vec3 tc;
    XZM::vec3 rc;

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
            /* Find the Normal, Tangent, and BiTangent to the output pixel. */
            XZM::vec3 N = XZM::Normalize(rc + sc * (2.0f * ((float) u + 0.5f) / (float) outMapHeight - 1.0f) +
                                         tc * (2.0f * ((float) v + 0.5f) / (float) outMapWidth - 1.0f));
            XZM::vec3 temp = (abs(N.data[2]) < 0.99f ? XZM::vec3(0.0f, 0.0f, 1.0f) : XZM::vec3(1.0f, 0.0f, 0.0f));
            XZM::vec3 TX = XZM::Normalize(XZM::CrossProduct(N, temp));
            XZM::vec3 TY = XZM::CrossProduct(N, TX);

            XZM::vec3 V = N;
            XZM::vec3 acc = XZM::vec3(0.0f, 0.0f, 0.0f);
            float totalWeight = 0;

            for (uint32_t i = 0; i < uint32_t(nSamples); ++i) {
                /* Randomly make a sample. */
                XZM::vec3 sampleDir = MakeSample();
                sampleDir = XZM::Normalize(XZM::vec3(TX * sampleDir.data[0] + TY * sampleDir.data[1] + N * sampleDir.data[2]));

                XZM::vec3 L = XZM::Normalize(sampleDir *2 * XZM::DotProduct( V, sampleDir ) - V);
                float NoL = std::max(0.0f, std::min(1.0f, XZM::DotProduct(N,L)));

                if(NoL > 0){
                    /* Find its correspond cube map. */
                    acc += Projection(sampleDir) * NoL;
                    totalWeight ++;
                }
            }
            /* Average the result. */
            acc = acc * M_PI * (1.0f / totalWeight);
           // acc += (SumBrightDirection(N));
            outMaps[face][v][u] = acc;
        }
    }
    printf("Lambertian: Sampling face %d/6 Finished.\n",face+1);
}


/**
 * @brief Save the output as a png file.
 */
void Lambertian::SaveOutput(){

    auto* dst = new stbi_uc[outMapWidth*outMapHeight*4*6];

    /* Save each face. */
    for(int face = 0; face < 6; face++){
        ReadFace(dst, static_cast<EFace>(face), outMapWidth, outMapHeight);
    }

    /* Save to png. */
    std::string outFileName = srcName + "_lam.png";
    printf("Lambertian: Save Output to. %s ...\n",outFileName.c_str());
    stbi_write_png(outFileName.c_str(), (int)outMapWidth, (int)outMapHeight*6, 4, dst, (int)outMapWidth * 4);

    delete[] dst;
}

