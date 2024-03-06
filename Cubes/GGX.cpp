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


XZM::vec3 GGX::SumBrightDirection(const XZM::vec3& dir){

    XZM::vec3 ret = XZM::vec3();
    float totalWeight = 0;

    for (const auto& bd : brightDirections) {
        float NoL = std::max(0.0f, std::min(1.0f, XZM::DotProduct(dir,bd.dir)));

        if(NoL > 0.995) {
            ret += (bd.light * NoL);
            totalWeight += NoL;
        }
    }

    //if(totalWeight != 0){
        return ret;
    //}
    std::cout << "Here" << std::endl;
    return ret;
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

    brdf = new XZM::vec3*[10];
    for(auto i = 0; i < 10; i++){
        brdf[i] = new XZM::vec3[10];
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
        ProcessBRDF();
    }
    SaveBRDF();
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
            XZM::vec3 TX = XZM::Normalize(XZM::CrossProduct(temp, N));
            XZM::vec3 TY = XZM::Normalize(XZM::CrossProduct(N, TX));

            XZM::vec3 V = N;

            XZM::vec3 acc = XZM::vec3(0.0f, 0.0f, 0.0f);
            float totalWeight = 0;

            for (uint32_t i = 0; i < uint32_t(nSamples); ++i) {
                /* Generate a sample based on the Hammersley sequence. */
                auto Xi = Hammersley(i, nSamples);
                XZM::vec3 sampleDir = MakeSample(Xi);
                sampleDir = XZM::Normalize(XZM::vec3(TX * sampleDir.data[0] + TY * sampleDir.data[1] + N * sampleDir.data[2]));

                XZM::vec3 L = XZM::Normalize(sampleDir *2 * XZM::DotProduct( V, sampleDir ) - V);
                float NoL = std::max(0.0f, std::min(1.0f, XZM::DotProduct(N,L)));

                if(NoL > 0){
                    /* Find its correspond cube map. */
                    acc += Projection(sampleDir) * NoL;
                    totalWeight += NoL;
                }
            }
            /* Average the result. */
            acc = acc * (1.0f / totalWeight);
            acc += (SumBrightDirection(N));
            outMaps[face][v][u] = acc;


        }
    }
    printf("GGX: Sampling face %d/6 and roughness %f Finished.\n",face+1,roughness);
}


float GGX::GeometrySchlickGGX(float NoV){
    float a = roughness;
    float k = (a * a) / 2.0f;

    float nom   = NoV;
    float denom = NoV * (1.0f - k) + k;

    return nom / denom;
}


float GGX::GeometrySmith(const XZM::vec3& N, const XZM::vec3& V, const XZM::vec3& L){
    float NoV = std::max(XZM::DotProduct(N, V), 0.0f);
    float NoL = std::max(XZM::DotProduct(N, L), 0.0f);
    float ggx2 = GeometrySchlickGGX(NoV);
    float ggx1 = GeometrySchlickGGX(NoL);
    return ggx1 * ggx2;
}


void GGX::ProcessBRDF(){

    for(int i = 0; i < 10; i++){
        float NoV = (float)i / 10;
        XZM::vec3 V;
        V.data[0] = sqrt( 1.0f - NoV * NoV ); // sin
        V.data[1] = 0;
        V.data[2] = NoV; // cos

        float A = 0;
        float B = 0;
        XZM::vec3 N = XZM::vec3(0.0, 0.0, 1.0);
        XZM::vec3 temp = (abs(N.data[2]) < 0.99f ? XZM::vec3(0.0f, 0.0f, 1.0f) : XZM::vec3(1.0f, 0.0f, 0.0f));
        XZM::vec3 TX = XZM::Normalize(XZM::CrossProduct(temp, N));
        XZM::vec3 TY = XZM::Normalize(XZM::CrossProduct(N, TX));

        for(auto j = 0; j < nSamples; j++ ){
            auto Xi = Hammersley( j, nSamples );
            XZM::vec3 sampleDir = MakeSample(Xi);
            sampleDir = XZM::Normalize(XZM::vec3(TX * sampleDir.data[0] + TY * sampleDir.data[1] + N * sampleDir.data[2]));
            XZM::vec3 L = sampleDir * 2 * XZM::DotProduct( V, sampleDir ) - V;
            float NoL = std::max(0.0f, std::min(1.0f, L.data[2]));
            float NoH = std::max(0.0f, std::min(1.0f, sampleDir.data[2]));
            float VoH = std::max(0.0f, std::min(1.0f, XZM::DotProduct(V,sampleDir)));

            if( NoL > 0 ){
                float G = GeometrySmith(N, V, L);
                float G_Vis = (G * VoH) / (NoH * std::max(NoV,0.00001f));
                float Fc = pow(1.0f - VoH, 5.0f);
                A += (1.0f - Fc) * G_Vis;
                B += Fc * G_Vis;
            }
        }
        A /= float(nSamples);
        B /= float(nSamples);
        brdf[(size_t)(roughness*10)][i] = XZM::vec3(A,B,0);
    }
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


void GGX::SaveBRDF(){
    auto* dst = new stbi_uc[10*10*3];

    for(auto i = 0; i < 10; i++){
        for(auto j = 0; j < 10; j++){
            dst[i*30 + j*3    ] =  (stbi_uc)(255*brdf[i][j].data[0]);
            dst[i*30 + j*3 + 1] = (stbi_uc)(255*brdf[i][j].data[1]);
            dst[i*30 + j*3 + 2] = (stbi_uc)(255*brdf[i][j].data[2]);
        }
    }

    /* Save to png. */
    std::string outFileName = srcName + "_ggx_brdf.png";
    stbi_write_png(outFileName.c_str(), 10, 10, 3, dst, 30);

    delete[] dst;
}

GGX::~GGX() {
    if(brdf == nullptr){
        return;
    }

    for(auto i = 0; i < 10; i++){
        delete[] brdf[i];
    }
    delete[] brdf;
}
