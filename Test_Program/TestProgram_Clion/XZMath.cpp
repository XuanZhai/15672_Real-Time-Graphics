//
// Created by Xuan Zhai on 2024/2/4.
//

#include "XZMath.h"

XZM::vec3::vec3() {
    data[0] = 0;
    data[1] = 0;
    data[2] = 0;
}

XZM::vec3::vec3(float nx, float ny, float nz) {
    data[0] = nx;
    data[1] = ny;
    data[2] = nz;
}

XZM::vec3::vec3(const XZM::vec3 &nv) {
    data[0] = nv.data[0];
    data[1] = nv.data[1];
    data[2] = nv.data[2];
}

XZM::vec3 &XZM::vec3::operator=(const XZM::vec3 &nv) {
    if(data == nv.data){
        return *this;
    }
    data = nv.data;
    return *this;
}

bool XZM::vec3::operator==(const XZM::vec3 &nv) const {
    return data == nv.data;
}

XZM::vec3 XZM::vec3::operator+(const XZM::vec3 &nv) const {

    vec3 newvec;
    newvec.data[0] = data[0] + nv.data[0];
    newvec.data[1] = data[1] + nv.data[1];
    newvec.data[2] = data[2] + nv.data[2];
    return newvec;
}


XZM::vec3 XZM::vec3::operator* (const vec3& nv) const {
    vec3 newvec;
    newvec.data[0] = data[0] * nv.data[0];
    newvec.data[1] = data[1] * nv.data[1];
    newvec.data[2] = data[2] * nv.data[2];
    return newvec;
}


XZM::vec3 XZM::vec3::operator- (const vec3& nv) const{
    vec3 newvec;
    newvec.data[0] = data[0] - nv.data[0];
    newvec.data[1] = data[1] - nv.data[1];
    newvec.data[2] = data[2] - nv.data[2];
    return newvec;
}





/* ===================================================================================================== */







XZM::mat4::mat4() {
    data[0][0] = 1;
    data[0][1] = 0;
    data[0][2] = 0;
    data[0][3] = 0;

    data[1][0] = 0;
    data[1][1] = 1;
    data[1][2] = 0;
    data[1][3] = 0;

    data[2][0] = 0;
    data[2][1] = 0;
    data[2][2] = 1;
    data[2][3] = 0;

    data[3][0] = 0;
    data[3][1] = 0;
    data[3][2] = 0;
    data[3][3] = 1;
}


XZM::mat4::mat4(float nf){
    data[0][0] = nf;
    data[0][1] = 0;
    data[0][2] = 0;
    data[0][3] = 0;

    data[1][0] = 0;
    data[1][1] = nf;
    data[1][2] = 0;
    data[1][3] = 0;

    data[2][0] = 0;
    data[2][1] = 0;
    data[2][2] = nf;
    data[2][3] = 0;

    data[3][0] = 0;
    data[3][1] = 0;
    data[3][2] = 0;
    data[3][3] = nf;
}


XZM::mat4::mat4(const mat4& nm){
    data = nm.data;
}


XZM::mat4& XZM::mat4::operator= (const mat4& nm){
    if(data == nm.data){
        return *this;
    }
    data = nm.data;
    return *this;
}


bool XZM::mat4::operator== (const mat4& nm) const {
    return data == nm.data;
}


XZM::mat4 XZM::mat4::operator * (const mat4& nm){
    mat4 newMat;

    for (size_t i = 0; i < 4; ++i) {
        for (size_t j = 0; j < 4; ++j) {
            newMat.data[i][j] = 0.0f;
            for (int k = 0; k < 4; ++k) {
                newMat.data[i][j] += data[i][k] * nm.data[k][j];
            }
        }
    }

    return newMat;
}















/* ============================================================================================================== */


XZM::quat::quat(){
    data[0] = 0;
    data[1] = 0;
    data[2] = 0;
    data[3] = 1;
}


XZM::quat::quat(const quat& nq){
    data = nq.data;
}


XZM::quat::quat(float x,float y,float z,float w){
    data[0] = x;
    data[1] = y;
    data[2] = z;
    data[3] = w;
}


XZM::quat& XZM::quat::operator= (const quat& nq){
    if(data == nq.data){
        return *this;
    }
    data = nq.data;
    return *this;
}


bool XZM::quat::operator== (const quat& nq) const{
    return data == nq.data;
}


XZM::quat XZM::quat::operator * (const XZM::quat &nq) {
    quat result;

    result.data[0] = data[3] * nq.data[0] + data[0] * nq.data[3] + data[1] * nq.data[2] - data[2] * nq.data[1];
    result.data[1] = data[3] * nq.data[1] - data[0] * nq.data[2] + data[1] * nq.data[3] + data[2] * nq.data[0];
    result.data[2] = data[3] * nq.data[2] + data[0] * nq.data[1] - data[1] * nq.data[0] + data[2] * nq.data[3];
    result.data[3] = data[3] * nq.data[3] - data[0] * nq.data[0] - data[1] * nq.data[1] - data[2] * nq.data[2];

    return result;
}








/* ============================================================================================================== */


XZM::mat4 XZM::Inverse(const XZM::mat4 &nm) {

    mat4 tmp;
    mat4 cur;

    for (unsigned int y = 0; y < 4; ++y)
    {
        for (unsigned int x = 0; x < 4; ++x)
        {
            cur.data[y][x] = nm.data[y][x];
        }
    }

    for (unsigned int x = 0; x < 4; x++)
    {
        if (cur.data[x][x] != 0)
        {
            float denominator = cur.data[x][x];
            for (unsigned int a = x; a < 4; ++a)
            {
                cur.data[x][a] = cur.data[x][a] / denominator;
            }
            for (unsigned int a = 0; a < 4; ++a)
            {
                tmp.data[x][a] = tmp.data[x][a] / denominator;
            }
        }

        for (unsigned int y = 0; y < 4; ++y)
        {
            if (y != x && cur.data[y][x] != 0)
            {
                float difference = cur.data[y][x];
                for (unsigned int a = x; a < 4; ++a)
                {
                    cur.data[y][a] = (cur.data[y][a] - difference) * cur.data[x][a];
                }
                for (unsigned int a = 0; a < 4; ++a)
                {
                    tmp.data[y][a] = (tmp.data[y][a] - difference) * tmp.data[x][a];
                }
            }
        }
    }

    return tmp;
}


XZM::mat4 XZM::QuatToMat4(const XZM::quat & nq) {
    float x = nq.data[0];
    float y = nq.data[1];
    float z = nq.data[2];
    float w = nq.data[3];

    float xx = x * x;
    float yy = y * y;
    float zz = z * z;
    float xy = x * y;
    float xz = x * z;
    float yz = y * z;
    float wx = w * x;
    float wy = w * y;
    float wz = w * z;

    mat4 rotationMatrix;
    rotationMatrix.data[0][0] = 1.0f - 2.0f * (yy + zz);
    rotationMatrix.data[0][1] = 2.0f * (xy - wz);
    rotationMatrix.data[0][2] = 2.0f * (xz + wy);

    rotationMatrix.data[1][0] = 2.0f * (xy + wz);
    rotationMatrix.data[1][1] = 1.0f - 2.0f * (xx + zz);
    rotationMatrix.data[1][2] = 2.0f * (yz - wx);

    rotationMatrix.data[2][0] = 2.0f * (xz - wy);
    rotationMatrix.data[2][1] = 2.0f * (yz + wx);
    rotationMatrix.data[2][2] = 1.0f - 2.0f * (xx + yy);

    return rotationMatrix;
}


XZM::mat4 XZM::Perspective(float vfov, float aspect, float near, float far){
    mat4 projMatrix;
    float scale = tanf(vfov * 0.5f) * near;
    float right = aspect * scale;
    float left = -right;
    float top = scale;
    float bottom = -top;


    projMatrix.data[0][0] = 2 * near / (right - left);
    projMatrix.data[0][1] = 0;
    projMatrix.data[0][2] = 0;
    projMatrix.data[0][3] = 0;

    projMatrix.data[1][0] = 0;
    projMatrix.data[1][1] = 2 * near / (top - bottom);
    projMatrix.data[1][2] = 0;
    projMatrix.data[1][3] = 0;

    projMatrix.data[2][0] = (right + left) / (right - left);
    projMatrix.data[2][1] = (top + bottom) / (top - bottom);
    projMatrix.data[2][2] = -(far + near) / (far - near);
    projMatrix.data[2][3] = -1;

    projMatrix.data[3][0] = 0;
    projMatrix.data[3][1] = 0;
    projMatrix.data[3][2] = -2 * far * near / (far - near);
    projMatrix.data[3][3] = 0;


    return projMatrix;
}


XZM::vec3 XZM::CrossProduct(const vec3& v1, const vec3& v2){

    float resultX = v1.data[1] * v2.data[2] - v1.data[2] * v2.data[1];
    float resultY = v1.data[2] * v2.data[0] - v1.data[0] * v2.data[2];
    float resultZ = v1.data[0] * v2.data[1] - v1.data[1] * v2.data[0];

    return vec3(resultX, resultY, resultZ);
}


float XZM::DotProduct(const XZM::vec3 &v1, const XZM::vec3 &v2) {
        return v1.data[0] * v2.data[0] + v1.data[1] * v2.data[1] + v1.data[2] * v2.data[2];
}


XZM::vec3 XZM::Normalize(const vec3& nv){
    float length = std::sqrt(nv.data[0] * nv.data[0] + nv.data[1] * nv.data[1] + nv.data[2] * nv.data[2]);
    return vec3(nv.data[0] / length, nv.data[1] / length, nv.data[2] / length);
}


XZM::quat XZM::Normalize(const quat& nq){
    float magnitude = std::sqrt(nq.data[0] * nq.data[0] + nq.data[1] * nq.data[1] + nq.data[2] * nq.data[2] + nq.data[3] * nq.data[3]);
    return quat(nq.data[0] / magnitude, nq.data[1] / magnitude, nq.data[2] / magnitude, nq.data[3] / magnitude);
}


XZM::mat4 XZM::LookAt(const vec3& eyePos, const vec3& target, const vec3& up){

    mat4 viewMatrix;

    // Compute the forward, right, and up vectors
    vec3 forward = Normalize(target-eyePos);
    vec3 right = Normalize(CrossProduct(forward, up));
    vec3 newUp = CrossProduct(right, forward);

    // Set the view matrix components
    viewMatrix.data[0][0] = right.data[0];
    viewMatrix.data[1][0] = right.data[1];
    viewMatrix.data[2][0] = right.data[2];
    viewMatrix.data[3][0] = -DotProduct(right, eyePos);

    viewMatrix.data[0][1] = newUp.data[0];
    viewMatrix.data[1][1] = newUp.data[1];
    viewMatrix.data[2][1] = newUp.data[2];
    viewMatrix.data[3][1] = -DotProduct(newUp, eyePos);

    viewMatrix.data[0][2] = -forward.data[0];
    viewMatrix.data[1][2] = -forward.data[1];
    viewMatrix.data[2][2] = -forward.data[2];
    viewMatrix.data[3][2] = DotProduct(forward, eyePos);

    viewMatrix.data[0][3] = 0.0f;
    viewMatrix.data[1][3] = 0.0f;
    viewMatrix.data[2][3] = 0.0f;
    viewMatrix.data[3][3] = 1.0f;

    return viewMatrix;
}


XZM::mat4 XZM::Scaling(const mat4& nm, const vec3& factor){
    mat4 result = nm;

    result.data[0][0] *= factor.data[0];
    result.data[1][1] *= factor.data[1];
    result.data[2][2] *= factor.data[2];

    return result;
}


XZM::mat4 XZM::Scaling(const vec3& factor){
    mat4 result;

    result.data[0][0] *= factor.data[0];
    result.data[1][1] *= factor.data[1];
    result.data[2][2] *= factor.data[2];

    return result;
}


XZM::mat4 XZM::Translation(const mat4& nm, const vec3& factor){
    mat4 result = nm;

    result.data[0][3] += factor.data[0];
    result.data[1][3] += factor.data[1];
    result.data[2][3] += factor.data[2];

    return result;
}


XZM::mat4 XZM::Translation(const vec3& nv){
    mat4 newmat;
    newmat.data[0][3] = nv.data[0];
    newmat.data[1][3] = nv.data[1];
    newmat.data[2][3] = nv.data[2];

    return newmat;
}


XZM::vec3 XZM::GetTranslationFromMat(const mat4& nm){
    return vec3(nm.data[0][3],nm.data[1][3],nm.data[2][3]);
}


std::string XZM::ToString(const vec3& nv){
    return std::string("{" + std::to_string(nv.data[0]) + "," + std::to_string(nv.data[1]) + "," + std::to_string(nv.data[2]) + "}");
}

void FromGLMtoXZM(const glm::mat4& nm, XZM::mat4 viewMatrix){

    glm::mat4 newMat =  glm::transpose(nm);

    viewMatrix.data[0][0] = newMat[0][0];
    viewMatrix.data[0][1] = newMat[0][1];
    viewMatrix.data[0][2] = newMat[0][2];
    viewMatrix.data[0][3] = newMat[0][3];

    viewMatrix.data[1][0] = newMat[1][0];
    viewMatrix.data[1][1] = newMat[1][1];
    viewMatrix.data[1][2] = newMat[1][2];
    viewMatrix.data[1][3] = newMat[1][3];

    viewMatrix.data[2][0] = newMat[2][0];
    viewMatrix.data[2][1] = newMat[2][1];
    viewMatrix.data[2][2] = newMat[2][2];
    viewMatrix.data[2][3] = newMat[2][3];

    viewMatrix.data[3][0] = newMat[3][0];
    viewMatrix.data[3][1] = newMat[3][1];
    viewMatrix.data[3][2] = newMat[3][2];
    viewMatrix.data[3][3] = newMat[3][3];
}