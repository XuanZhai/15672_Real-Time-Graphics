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


XZM::vec3 XZM::vec3::operator* (float nf) const{
    vec3 newvec;
    newvec.data[0] = data[0] * nf;
    newvec.data[1] = data[1] * nf;
    newvec.data[2] = data[2] * nf;
    return newvec;
}


XZM::vec3 XZM::vec3::operator/ (float nf) const{
    return {data[0]/nf, data[1]/nf, data[2]/nf};
}


bool XZM::vec3::IsEmpty() const{
    return data[0] == 0 && data[1] == 0 && data[2] == 0;
}


XZM::vec3& XZM::vec3::operator+= (const vec3& nv){
    data[0] += nv.data[0];
    data[1] += nv.data[1];
    data[2] += nv.data[2];

    return *this;
}


XZM::vec3& XZM::vec3::operator-= (const vec3& nv){
    data[0] -= nv.data[0];
    data[1] -= nv.data[1];
    data[2] -= nv.data[2];

    return *this;
}

float XZM::vec3::Length() const {
    return std::sqrt(data[0] * data[0] + data[1] * data[1] + data[2] * data[2]);
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

XZM::vec3 XZM::mat4::operator*(const XZM::vec3 &nv) {
    vec3 result;
    result.data[0] = data[0][0] * nv.data[0] + data[1][0] * nv.data[1] + data[2][0] * nv.data[2] + data[3][0];
    result.data[1] = data[0][1] * nv.data[0] + data[1][1] * nv.data[1] + data[2][1] * nv.data[2] + data[3][1];
    result.data[2] = data[0][2] * nv.data[0] + data[1][2] * nv.data[1] + data[2][2] * nv.data[2] + data[3][2];
    return result;
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
    rotationMatrix.data[1][0] = 2.0f * (xy - wz);
    rotationMatrix.data[2][0] = 2.0f * (xz + wy);

    rotationMatrix.data[0][1] = 2.0f * (xy + wz);
    rotationMatrix.data[1][1] = 1.0f - 2.0f * (xx + zz);
    rotationMatrix.data[2][1] = 2.0f * (yz - wx);

    rotationMatrix.data[0][2] = 2.0f * (xz - wy);
    rotationMatrix.data[1][2] = 2.0f * (yz + wx);
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


float XZM::DotProduct(const quat& q1, const quat& q2){
    return q1.data[0] * q2.data[0] + q1.data[1] * q2.data[1] + q1.data[2] * q2.data[2] + q1.data[3] * q2.data[3];
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


XZM::vec3 XZM::GetLookAtDir(const vec3& eyePos, const mat4& rotationMatrix){
    // Convert quaternion to rotation matrix
    // Create forward vector
    vec3 forward = { 0.0f, 0.0f, -1.0f };

    // Transform forward vector using rotation matrix
    vec3 lookAtDirection = {
            rotationMatrix.data[0][0] * forward.data[0] + rotationMatrix.data[1][0] * forward.data[1] + rotationMatrix.data[2][0] * forward.data[2],
            rotationMatrix.data[0][1] * forward.data[0] + rotationMatrix.data[1][1] * forward.data[1] + rotationMatrix.data[2][1] * forward.data[2],
            rotationMatrix.data[0][2] * forward.data[0] + rotationMatrix.data[1][2] * forward.data[1] + rotationMatrix.data[2][2] * forward.data[2]
    };

    // Normalize look-at direction
    Normalize(lookAtDirection);
    return lookAtDirection;
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
    newmat.data[3][0] += nv.data[0];
    newmat.data[3][1] += nv.data[1];
    newmat.data[3][2] += nv.data[2];

    return newmat;
}


XZM::vec3 XZM::GetTranslationFromMat(const mat4& nm){
    return vec3(nm.data[0][3],nm.data[1][3],nm.data[2][3]);
}


std::string XZM::ToString(const vec3& nv){
    return std::string("{" + std::to_string(nv.data[0]) + "," + std::to_string(nv.data[1]) + "," + std::to_string(nv.data[2]) + "}");
}


XZM::mat4 XZM::Transpose(const mat4& nm){
    XZM::mat4 result;

    for (int i = 0; i < 4; ++i) {
        for (int j = i + 1; j < 4; ++j) {
            result.data[j][i] = nm.data[i][j];
        }
    }
    return result;
}


XZM::vec3 XZM::ExtractTranslationFromMat(const mat4& nm){
    return vec3(nm.data[3][0],nm.data[3][1],nm.data[3][2]);
}


XZM::quat XZM::ExtractQuatFromMat(const mat4& rotationMatrix){
    quat quaternion;
    float trace = rotationMatrix.data[0][0] + rotationMatrix.data[1][1] + rotationMatrix.data[2][2];
    if (trace > 0) {
        float s = 0.5f / sqrt(trace + 1.0f);
        quaternion.data[3] = 0.25f / s;
        quaternion.data[0] = (rotationMatrix.data[1][2] - rotationMatrix.data[2][1]) * s;
        quaternion.data[1] = (rotationMatrix.data[2][0] - rotationMatrix.data[0][2]) * s;
        quaternion.data[2] = (rotationMatrix.data[0][1] - rotationMatrix.data[1][0]) * s;
    } else {
        if (rotationMatrix.data[0][0] > rotationMatrix.data[1][1] && rotationMatrix.data[0][0] > rotationMatrix.data[2][2]) {
            float s = 2.0f * sqrt(1.0f + rotationMatrix.data[0][0] - rotationMatrix.data[1][1] - rotationMatrix.data[2][2]);
            quaternion.data[3] = (rotationMatrix.data[1][2] - rotationMatrix.data[1][2]) / s;
            quaternion.data[0] = 0.25f * s;
            quaternion.data[1] = (rotationMatrix.data[1][0] + rotationMatrix.data[0][1]) / s;
            quaternion.data[2] = (rotationMatrix.data[2][0] + rotationMatrix.data[0][2]) / s;
        } else if (rotationMatrix.data[1][1] > rotationMatrix.data[2][2]) {
            float s = 2.0f * sqrt(1.0f + rotationMatrix.data[1][1] - rotationMatrix.data[0][0] - rotationMatrix.data[2][2]);
            quaternion.data[3] = (rotationMatrix.data[0][0] - rotationMatrix.data[0][2]) / s;
            quaternion.data[0] = (rotationMatrix.data[1][0] + rotationMatrix.data[0][1]) / s;
            quaternion.data[1] = 0.25f * s;
            quaternion.data[2] = (rotationMatrix.data[2][1] + rotationMatrix.data[1][2]) / s;
        } else {
            float s = 2.0f * sqrt(1.0f + rotationMatrix.data[2][2] - rotationMatrix.data[0][0] - rotationMatrix.data[1][1]);
            quaternion.data[3] = (rotationMatrix.data[0][1] - rotationMatrix.data[1][0]) / s;
            quaternion.data[0] = (rotationMatrix.data[2][0] + rotationMatrix.data[0][2]) / s;
            quaternion.data[1] = (rotationMatrix.data[2][1] + rotationMatrix.data[1][2]) / s;
            quaternion.data[2] = 0.25f * s;
        }
    }
    return quaternion;
}


XZM::vec3 XZM::FindForwardDirection(const quat& quaternion){

    // Rotate the default forward vector [0, 0, -1] by the rotation quaternion
    float forwardX = 2.0f * (quaternion.data[0] * quaternion.data[2] - quaternion.data[3] * quaternion.data[1]);
    float forwardY = 2.0f * (quaternion.data[1] * quaternion.data[2] + quaternion.data[3] * quaternion.data[0]);
    float forwardZ = 1.0f - 2.0f * (quaternion.data[0] * quaternion.data[0] + quaternion.data[1] * quaternion.data[1]);

    // Normalize the resulting vector
    return Normalize(vec3(forwardX, forwardY, forwardZ));
}


XZM::vec3 XZM::ConvertQuatToVec3(const XZM::quat &quaternion) {
    float norm = std::sqrt(quaternion.data[0] * quaternion.data[0] + quaternion.data[1] * quaternion.data[1] + quaternion.data[2] * quaternion.data[2] + quaternion.data[3] * quaternion.data[3]);
    quat normalizedQuat = { quaternion.data[0] / norm, quaternion.data[1] / norm, quaternion.data[2] / norm, quaternion.data[3] / norm };

    // Extract axis of rotation
    vec3 axis = { quaternion.data[0], quaternion.data[1], quaternion.data[2] };
    Normalize(axis);
    return axis;
}


XZM::vec3 XZM::RotateVec3(const vec3& vector, const vec3& axis, float radians){
    float cosTheta = cos(radians);
    float sinTheta = sin(radians);

    vec3 rotatedVec;

    XZM::mat4 rotationMatrix;
    rotationMatrix.data[0][0] = cosTheta + (1 - cosTheta) * axis.data[0] * axis.data[0];
    rotationMatrix.data[0][1] = (1 - cosTheta) * axis.data[0] * axis.data[1] - sinTheta * axis.data[2];
    rotationMatrix.data[0][2] = (1 - cosTheta) * axis.data[0] * axis.data[2] + sinTheta * axis.data[1];

    rotationMatrix.data[1][0] = (1 - cosTheta) * axis.data[0] * axis.data[1] + sinTheta * axis.data[2];
    rotationMatrix.data[1][1] = cosTheta + (1 - cosTheta) * axis.data[1] * axis.data[1];
    rotationMatrix.data[1][2] = (1 - cosTheta) * axis.data[1] * axis.data[2] - sinTheta * axis.data[0];

    rotationMatrix.data[2][0] = (1 - cosTheta) * axis.data[0] * axis.data[2] - sinTheta * axis.data[1];
    rotationMatrix.data[2][1] = (1 - cosTheta) * axis.data[1] * axis.data[2] + sinTheta * axis.data[0];
    rotationMatrix.data[2][2] = cosTheta + (1 - cosTheta) * axis.data[2] * axis.data[2];

    // Apply the rotation matrix to the vector
    rotatedVec.data[0] = vector.data[0] * rotationMatrix.data[0][0] + vector.data[1] * rotationMatrix.data[0][1] + vector.data[2] * rotationMatrix.data[0][2];
    rotatedVec.data[1] = vector.data[0] * rotationMatrix.data[1][0] + vector.data[1] * rotationMatrix.data[1][1] + vector.data[2] * rotationMatrix.data[1][2];
    rotatedVec.data[2] = vector.data[0] * rotationMatrix.data[2][0] + vector.data[1] * rotationMatrix.data[2][1] + vector.data[2] * rotationMatrix.data[2][2];

    return rotatedVec;
}


XZM::vec3 XZM::Lerp(const vec3& low, const vec3& high, float t){
    float x = low.data[0] * (1.0f - t) + high.data[0] * t;
    float y = low.data[1] * (1.0f - t) + high.data[1] * t;
    float z = low.data[2] * (1.0f - t) + high.data[2] * t;

    return {x,y,z};
}


XZM::quat XZM::Lerp(const quat& low, const quat& high, float t){
    float dot = DotProduct(low,high);

    // If the dot product is negative, invert one of the quaternions to take the shortest path
    quat high_adj = high;
    if (dot < 0.0f) {
        high_adj.data[0] = -high.data[0];
        high_adj.data[1] = -high.data[1];
        high_adj.data[2] = -high.data[2];
        high_adj.data[3] = -high.data[3];
        dot = -dot;
    }

    quat result;
    // Interpolate the quaternions
    if (dot > 0.9995f) {
        // Quaternions are very close, perform linear interpolation
        result.data[0] = low.data[0] * (1.0f - t) + high_adj.data[0] * t;
        result.data[1] = low.data[1] * (1.0f - t) + high_adj.data[1] * t;
        result.data[2] = low.data[2] * (1.0f - t) + high_adj.data[2] * t;
        result.data[3] = low.data[3] * (1.0f - t) + high_adj.data[3] * t;
    } else {
        // Quaternions are not close, use spherical linear interpolation (slerp)
        float angle = acos(dot);
        float sinAngle = sin(angle);
        float invSinAngle = 1.0f / sinAngle;
        float coeff1 = sin((1.0f - t) * angle) * invSinAngle;
        float coeff2 = sin(t * angle) * invSinAngle;

        result.data[0] = low.data[0] * coeff1 + high_adj.data[0] * coeff2;
        result.data[1] = low.data[1] * coeff1 + high_adj.data[1] * coeff2;
        result.data[2] = low.data[2] * coeff1 + high_adj.data[2] * coeff2;
        result.data[3] = low.data[3] * coeff1 + high_adj.data[3] * coeff2;
    }
    return result;
}