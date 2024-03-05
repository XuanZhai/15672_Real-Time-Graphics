//
// Created by Xuan Zhai on 2024/3/4.
//

#include "GGX.h"

#include "stb_image.h"
#include "stb_image_write.h"


/**
 * @brief Generate the Van Der Corput sequence
 * @param bits The sample index.
 * @return The sequence.
 */
float GGX::RadicalInverse_VdC(unsigned int bits){
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10f; // / 0x100000000
}


/**
 * @brief Generate the Hammersley sequence.
 * @param i The current sample index.
 * @param N The total number of samples/
 * @return The sequence.
 */
std::pair<float,float> GGX::Hammersley(unsigned int i, unsigned int N){
    return std::make_pair(float(i)/float(N),RadicalInverse_VdC(i));
}


/**
 * @brief Make a GGX sample based on the Hammersley Sequence.
 * @param Xi The Hammersley Sequence.
 * @return The sampled direction.
 */
XZM::vec3 GGX::MakeSample(const std::pair<float,float>& Xi) const{

    float a = roughness * roughness;
    float Phi = 2 * (float)M_PI * Xi.first;
    float CosTheta = sqrt( (1 - Xi.second) / ( 1 + (a*a - 1) * Xi.second ) );
    float SinTheta = sqrt( 1 - CosTheta * CosTheta );

    return {
            SinTheta * cos( Phi ),
            SinTheta * sin( Phi ),
            CosTheta
    };
}


/**
 * @brief An override function for doing the GGX Monte-Carlo.
 * @param newNSamples The number of samples.
 * @param outWidth The output image width.
 * @param outHeight The output image height.
 */
void GGX::Processing(uint32_t newNSamples, uint32_t outWidth, uint32_t outHeight) {

    outMapWidth = outWidth;
    outMapHeight = outHeight;
    nSamples = newNSamples;

    ProcessBright();

    for (int face = 0; face < 6; face++) {
        outMaps[face] = new XZM::vec3 *[outMapHeight];
        for (int row = 0; row < outMapHeight; row++) {
            outMaps[face][row] = new XZM::vec3[outMapWidth];
        }
    }
    /* Create different output based on the roughness values. */
    for(int mip = 0; mip < 10; mip++) {
        /* Roughness is [0.0,1.0). */
        roughness = (float)mip / 10;
        for (int face = 0; face < 6; face++) {
            threads[face] = std::thread(&GGX::ProcessingFace, this, static_cast<EFace>(face));
        }

        for (auto face = 0; face < 6; face++) {
            threads[face].join();
        }

        SaveOutput();
        ClearOutput();
    }
}


/**
 * @brief Process the Lambertian Monte-Carlo for a given output face.
 * @param face The face index.
 */
void GGX::ProcessingFace(EFace face) {

    printf("GGX: Sampling face %d/6 and roughness %f ... \n", face,roughness);
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

            XZM::vec3 acc = XZM::vec3(0.0f, 0.0f, 0.0f);

            for (uint32_t i = 0; i < uint32_t(nSamples); ++i) {
                /* Generate a sample based on the Hammersley sequence. */
                auto Xi = Hammersley(i, nSamples);
                XZM::vec3 sampleDir = MakeSample(Xi);
                sampleDir = XZM::Normalize(XZM::vec3(TX * sampleDir.data[0] + TY * sampleDir.data[1] + N * sampleDir.data[2]));
                /* Find its correspond cube map. */
                acc += Projection(sampleDir);
            }
            /* Average the result. */
            acc = acc * (1.0f / float(nSamples));
            /* Add the bright info. */
            acc += SumBrightDirection(N);
            outMaps[face][v][u] = acc;
        }
    }
    printf("GGX: Sampling face %d/6 and roughness %f Finished.\n",face+1,roughness);
}


/**
 * @brief Save the output as a png file.
 */
void GGX::SaveOutput() {

    auto* dst = new stbi_uc[outMapWidth*outMapHeight*4*6];

    /* Save each face. */
    for(int face = 0; face < 6; face++){
        ReadFace(dst, static_cast<EFace>(face), outMapWidth, outMapHeight);
    }

    /* Save to png. */
    std::string outFileName = srcName + "_ggx_" + std::to_string((int)(roughness*10)) + ".png";
    printf("GGX: Save Output to. %s ...\n",outFileName.c_str());
    stbi_write_png(outFileName.c_str(), (int)outMapWidth, (int)outMapHeight*6, 4, dst, (int)outMapWidth * 4);

    delete[] dst;
}
