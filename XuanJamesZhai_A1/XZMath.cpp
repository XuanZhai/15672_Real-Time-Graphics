//
// Created by Xuan Zhai on 2024/2/4.
//

#include "XZMath.h"


/**
 * @brief Default Constructor for vec3.
 */
XZM::vec3::vec3() {
    data = {0,0,0};
}


/**
 * @brief Construct a vec3 based on three float values.
 * @param nx
 * @param ny
 * @param nz
 */
XZM::vec3::vec3(float nx, float ny, float nz) {
    data[0] = nx;
    data[1] = ny;
    data[2] = nz;
}


/**
 * @brief A copy constructor of vec3.
 * @param nv a copied target.
 */
XZM::vec3::vec3(const XZM::vec3 &nv) {
    data[0] = nv.data[0];
    data[1] = nv.data[1];
    data[2] = nv.data[2];
}


/**
 * @brief An overload assignment operator for vec3.
 * @param nv a copied target.
 * @return A self reference.
 */
XZM::vec3 &XZM::vec3::operator=(const XZM::vec3 &nv) {
    if(data == nv.data){
        return *this;
    }
    data = nv.data;
    return *this;
}


/**
 * @brief A overload equality operator for vec3.
 * @param nv
 * @return True if two vec3 are equal, false if are not equal.
 */
bool XZM::vec3::operator==(const XZM::vec3 &nv) const {
    return data == nv.data;
}


/**
 * @brief A overload add operator for vec3.
 * @param nv A vec3 we want to add with.
 * @return The result vec3.
 */
XZM::vec3 XZM::vec3::operator+(const XZM::vec3 &nv) const {
    return {data[0] + nv.data[0],data[1] + nv.data[1],data[2] + nv.data[2]};
}


/**
 * @brief A overload multiply operator for vec3.
 * @param nv A vec3 we want to multiply with.
 * @return The result vec3.
 */
XZM::vec3 XZM::vec3::operator* (const vec3& nv) const {
    return {data[0] * nv.data[0],data[1] * nv.data[1],data[2] * nv.data[2]};
}


/**
 * @brief A overload subtract operator for vec3.
 * @param nv A vec3 we want to subtract with.
 * @return The result vec3.
 */
XZM::vec3 XZM::vec3::operator- (const vec3& nv) const{
    return {data[0] - nv.data[0],data[1] - nv.data[1],data[2] - nv.data[2]};
}


/**
 * @brief A overload multiply operator for vec3.
 * @param nf A float we want to multiply with.
 * @return The result vec3.
 */
XZM::vec3 XZM::vec3::operator* (float nf) const{
    return {data[0] * nf, data[1] * nf, data[2] * nf};
}


/**
 * @brief A overload divide operator for vec3.
 * @param nf A float we want to divide with.
 * @return The result vec3.
 */
XZM::vec3 XZM::vec3::operator/ (float nf) const{
    return {data[0] / nf, data[1] / nf, data[2] / nf};
}


/**
 * @brief Check if a vec3 is {0,0,0}
 * @return True if a vec3 is {0,0,0}.
 */
bool XZM::vec3::IsEmpty() const{
    return data[0] == 0 && data[1] == 0 && data[2] == 0;
}


/**
 * @brief Overload += operator for vec3.
 * @param nv Add a vec3 to itself.
 * @return A self reference,
 */
XZM::vec3& XZM::vec3::operator+= (const vec3& nv){
    data[0] += nv.data[0];
    data[1] += nv.data[1];
    data[2] += nv.data[2];

    return *this;
}


/**
 * @brief Overload -= operator for vec3.
 * @param nv Subtract a vec3 to itself.
 * @return A self reference,
 */
XZM::vec3& XZM::vec3::operator-= (const vec3& nv){
    data[0] -= nv.data[0];
    data[1] -= nv.data[1];
    data[2] -= nv.data[2];

    return *this;
}


/**
 * @brief Calculate the length of a vec3.
 * @return The length in float.
 */
float XZM::vec3::Length() const {
    return std::sqrt(data[0] * data[0] + data[1] * data[1] + data[2] * data[2]);
}


/* ===================================================================================================== */


/**
 * @brief Default Constructor.
 */
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


/**
 * @brief A constructor with a float apply to the diagonal.
 * @param nf The float factor.
 */
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


/**
 * @brief A copy constructor.
 * @param nm A copied target.
 */
XZM::mat4::mat4(const mat4& nm){
    data = nm.data;
}


/**
 * @brief A overload assignment operator.
 * @param nm A copied target.
 * @return A self reference.
 */
XZM::mat4& XZM::mat4::operator= (const mat4& nm){
    if(data == nm.data){
        return *this;
    }
    data = nm.data;
    return *this;
}


/**
 * @brief A overload equality comparison operator.
 * @param nm A compared target.
 * @return True if two mat4 are equal.
 */
bool XZM::mat4::operator== (const mat4& nm) const {
    return data == nm.data;
}


/**
 * @brief Overload multiplication operator.
 * @param nm A target mat4 we want to multiply with.
 * @return A result mat4.
 */
XZM::mat4 XZM::mat4::operator * (const mat4& nm){
    mat4 result;

    for (size_t i = 0; i < 4; ++i) {
        for (size_t j = 0; j < 4; ++j) {
            result.data[i][j] = 0.0f;
            for (int k = 0; k < 4; ++k) {
                result.data[i][j] += data[i][k] * nm.data[k][j];
            }
        }
    }
    return result;
}


/**
 * @brief A overload multiplication operator that multiply a mat4 with a vec3.
 * @param nv A vec3 we want to multiply with.
 * @return A result vec3.
 */
XZM::vec3 XZM::mat4::operator*(const XZM::vec3 &nv) {
    vec3 result;
    result.data[0] = data[0][0] * nv.data[0] + data[1][0] * nv.data[1] + data[2][0] * nv.data[2] + data[3][0];
    result.data[1] = data[0][1] * nv.data[0] + data[1][1] * nv.data[1] + data[2][1] * nv.data[2] + data[3][1];
    result.data[2] = data[0][2] * nv.data[0] + data[1][2] * nv.data[1] + data[2][2] * nv.data[2] + data[3][2];
    return result;
}


/* ============================================================================================================== */


/**
 * @brief Default constructor for quat.
 */
XZM::quat::quat(){
    data[0] = 0;
    data[1] = 0;
    data[2] = 0;
    data[3] = 1;
}


/**
 * @brief Copy constructor for quat.
 * @param nq A copied quat.
 */
XZM::quat::quat(const quat& nq){
    data = nq.data;
}


/**
 * @brief Constructor for quat that accept four numbers.
 * @param x The x parameter.
 * @param y The y parameter.
 * @param z The z parameter.
 * @param w The w parameter.
 */
XZM::quat::quat(float x,float y,float z,float w){
    data[0] = x;
    data[1] = y;
    data[2] = z;
    data[3] = w;
}


/**
 * @brief A overload assignment operator for quat.
 * @param nq A copied quaternion.
 * @return A self reference.
 */
XZM::quat& XZM::quat::operator= (const quat& nq){
    if(data == nq.data){
        return *this;
    }
    data = nq.data;
    return *this;
}


/**
 * @brief Overload equality comparison operator.
 * @param nq A copied quaternion.
 * @return A self reference.
 */
bool XZM::quat::operator== (const quat& nq) const{
    return data == nq.data;
}


/**
 * @brief Overload addition operator.
 * @param nq A target quat want to add with.
 * @return A result quat.
 */
XZM::quat XZM::quat::operator+ (const quat& nq) const{
    return {data[0]+nq.data[0],data[1]+nq.data[1],data[2]+nq.data[2],data[3]+nq.data[3]};
}


/**
 * @brief Overload multiplication operator.
 * @param nq A target quat we want to multiply with.
 * @return A result quat.
 */
XZM::quat XZM::quat::operator * (const XZM::quat &nq) {
    quat result;

    result.data[0] = data[3] * nq.data[0] + data[0] * nq.data[3] + data[1] * nq.data[2] - data[2] * nq.data[1];
    result.data[1] = data[3] * nq.data[1] - data[0] * nq.data[2] + data[1] * nq.data[3] + data[2] * nq.data[0];
    result.data[2] = data[3] * nq.data[2] + data[0] * nq.data[1] - data[1] * nq.data[0] + data[2] * nq.data[3];
    result.data[3] = data[3] * nq.data[3] - data[0] * nq.data[0] - data[1] * nq.data[1] - data[2] * nq.data[2];

    return result;
}


/**
 * @brief Overload multiplication operator.
 * @param f A multiplication factor.
 * @return A result quat.
 */
XZM::quat XZM::quat::operator*(float f) const {
    return {data[0]*f,data[1]*f,data[2]*f,data[3]*f};
}


/* ============================================================================================================== */


/**
 * @brief Convert a quaternion to a mat4 rotation matrix.
 * @param nq The quaternion we want to convert.
 * @return The result rotation matrix.
 */
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

    mat4 result;
    result.data[0][0] = 1.0f - 2.0f * (yy + zz);
    result.data[1][0] = 2.0f * (xy - wz);
    result.data[2][0] = 2.0f * (xz + wy);

    result.data[0][1] = 2.0f * (xy + wz);
    result.data[1][1] = 1.0f - 2.0f * (xx + zz);
    result.data[2][1] = 2.0f * (yz - wx);

    result.data[0][2] = 2.0f * (xz - wy);
    result.data[1][2] = 2.0f * (yz + wx);
    result.data[2][2] = 1.0f - 2.0f * (xx + yy);

    return result;
}


/**
 * @brief Find the perspective matrix.
 * @param v_fov Vertical field of view.
 * @param aspect Aspect ration,
 * @param near The near z distance.
 * @param far The far z distance.
 * @return The perspective matrix.
 */
XZM::mat4 XZM::Perspective(float v_fov, float aspect, float near, float far){
    mat4 result;
    float scale = tanf(v_fov * 0.5f) * near;
    float right = aspect * scale;
    float left = -right;
    float top = scale;
    float bottom = -top;


    result.data[0][0] = 2 * near / (right - left);
    result.data[0][1] = 0;
    result.data[0][2] = 0;
    result.data[0][3] = 0;

    result.data[1][0] = 0;
    result.data[1][1] = 2 * near / (top - bottom);
    result.data[1][2] = 0;
    result.data[1][3] = 0;

    result.data[2][0] = (right + left) / (right - left);
    result.data[2][1] = (top + bottom) / (top - bottom);
    result.data[2][2] = -(far + near) / (far - near);
    result.data[2][3] = -1;

    result.data[3][0] = 0;
    result.data[3][1] = 0;
    result.data[3][2] = -2 * far * near / (far - near);
    result.data[3][3] = 0;

    return result;
}


/**
 * @brief Do the cross product between two vectors.
 * @param v1 The first vec3.
 * @param v2 The second vec3.
 * @return The result vec3.
 */
XZM::vec3 XZM::CrossProduct(const vec3& v1, const vec3& v2){

    float resultX = v1.data[1] * v2.data[2] - v1.data[2] * v2.data[1];
    float resultY = v1.data[2] * v2.data[0] - v1.data[0] * v2.data[2];
    float resultZ = v1.data[0] * v2.data[1] - v1.data[1] * v2.data[0];

    return {resultX, resultY, resultZ};
}


/**
 * @brief Do the dot product between two vectors.
 * @param v1 The first vec3.
 * @param v2 The second vec3.
 * @return The result in float.
 */
float XZM::DotProduct(const XZM::vec3 &v1, const XZM::vec3 &v2) {

    float ans = v1.data[0] * v2.data[0] + v1.data[1] * v2.data[1] + v1.data[2] * v2.data[2];

    return ans;
}


/**
 * @brief Do the dot product between two quaternions.
 * @param v1 The first quat.
 * @param v2 The second quat.
 * @return The result quat.
 */
float XZM::DotProduct(const quat& q1, const quat& q2){

    float ans = q1.data[0] * q2.data[0] + q1.data[1] * q2.data[1] + q1.data[2] * q2.data[2] + q1.data[3] * q2.data[3];

    if(ans > 1.0f){
        ans = 1.0f;
    }
    else if(ans < -1.0f){
        ans = -1.0f;
    }

    return ans;
}


/**
 * @brief Find the normalized a vec3 to a unit vector.
 * @param nv The source vec3.
 * @return The normalized vector.
 */
XZM::vec3 XZM::Normalize(const vec3& nv){
    float length = std::sqrt(nv.data[0] * nv.data[0] + nv.data[1] * nv.data[1] + nv.data[2] * nv.data[2]);
    return {nv.data[0] / length, nv.data[1] / length, nv.data[2] / length};
}


/**
 * @brief Find the view matrix.
 * @param eyePos The source position.
 * @param target The target position.
 * @param up The up direction as a unit vector.
 * @return The view matrix.
 */
XZM::mat4 XZM::LookAt(const vec3& eyePos, const vec3& target, const vec3& up){

    mat4 result;

    // Compute the forward, right, and up vectors
    vec3 forward = Normalize(target-eyePos);
    vec3 right = Normalize(CrossProduct(forward, up));
    vec3 newUp = CrossProduct(right, forward);

    // Set the view matrix components
    result.data[0][0] = right.data[0];
    result.data[1][0] = right.data[1];
    result.data[2][0] = right.data[2];
    result.data[3][0] = -DotProduct(right, eyePos);

    result.data[0][1] = newUp.data[0];
    result.data[1][1] = newUp.data[1];
    result.data[2][1] = newUp.data[2];
    result.data[3][1] = -DotProduct(newUp, eyePos);

    result.data[0][2] = -forward.data[0];
    result.data[1][2] = -forward.data[1];
    result.data[2][2] = -forward.data[2];
    result.data[3][2] = DotProduct(forward, eyePos);

    result.data[0][3] = 0.0f;
    result.data[1][3] = 0.0f;
    result.data[2][3] = 0.0f;
    result.data[3][3] = 1.0f;

    return result;
}


/**
 * @brief Find the look at direction from the forward given a rotation matrix.
 * @param rotationMatrix The rotation matrix.
 * @return The look at vector as a vec3.
 */
XZM::vec3 XZM::GetLookAtDir(const mat4& rotationMatrix){

    vec3 forward = { 0.0f, 0.0f, -1.0f };
    // Transform forward vector using rotation matrix
    vec3 result = {
            rotationMatrix.data[0][0] * forward.data[0] + rotationMatrix.data[1][0] * forward.data[1] + rotationMatrix.data[2][0] * forward.data[2],
            rotationMatrix.data[0][1] * forward.data[0] + rotationMatrix.data[1][1] * forward.data[1] + rotationMatrix.data[2][1] * forward.data[2],
            rotationMatrix.data[0][2] * forward.data[0] + rotationMatrix.data[1][2] * forward.data[1] + rotationMatrix.data[2][2] * forward.data[2]
    };

    Normalize(result);
    return result;
}


/**
 * @brief Get the scale matrix given a vec3 scale factor.
 * @param factor The vec3 scale factor.
 * @return The scale matrix.
 */
XZM::mat4 XZM::Scaling(const vec3& factor){
    mat4 result;
    result.data[0][0] *= factor.data[0];
    result.data[1][1] *= factor.data[1];
    result.data[2][2] *= factor.data[2];
    return result;
}


/**
 * @brief Get the translation matrix given a vec3 translation factor.
 * @param factor The vec3 translation factor.
 * @return The scale matrix.
 */
XZM::mat4 XZM::Translation(const vec3& nv){
    mat4 result;
    result.data[3][0] += nv.data[0];
    result.data[3][1] += nv.data[1];
    result.data[3][2] += nv.data[2];
    return result;
}


/**
 * @brief Convert a vec3 to a string.
 * @param nv The target vec3.
 * @return The string of vec3 wrapped by {}.
 */
std::string XZM::ToString(const vec3& nv){
    return std::string("{" + std::to_string(nv.data[0]) + "," + std::to_string(nv.data[1]) + "," + std::to_string(nv.data[2]) + "}");
}


/**
 * @brief Get the transpose of a mat4.
 * @param nm The target matrix.
 * @return The transposed matrix.
 */
XZM::mat4 XZM::Transpose(const mat4& nm){
    XZM::mat4 result;

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            result.data[j][i] = nm.data[i][j];
        }
    }
    return result;
}


/**
 * Extract the translation factor from a given matrix.
 * @param nm The transform matrix.
 * @return The translation factor.
 */
XZM::vec3 XZM::ExtractTranslationFromMat(const mat4& nm){
    return {nm.data[3][0],nm.data[3][1],nm.data[3][2]};
}


/**
 * @brief Rotate a vec3 along an axis with radians.
 * @param vector The direction we want to rotate.
 * @param axis The axis we want to rotate along,
 * @param radians The radian we want ot rotate.
 * @return
 */
XZM::vec3 XZM::RotateVec3(const vec3& vector, const vec3& axis, float radians){
    float cosTheta = cos(radians);
    float sinTheta = sin(radians);

    vec3 result;

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

    result.data[0] = vector.data[0] * rotationMatrix.data[0][0] + vector.data[1] * rotationMatrix.data[0][1] + vector.data[2] * rotationMatrix.data[0][2];
    result.data[1] = vector.data[0] * rotationMatrix.data[1][0] + vector.data[1] * rotationMatrix.data[1][1] + vector.data[2] * rotationMatrix.data[1][2];
    result.data[2] = vector.data[0] * rotationMatrix.data[2][0] + vector.data[1] * rotationMatrix.data[2][1] + vector.data[2] * rotationMatrix.data[2][2];

    return result;
}


/**
 * Find the linear interpolated vec3 between the lower bound and the upper bound given an alpha.
 * @param low The lower bound.
 * @param high The upper bound.
 * @param t The alpha factor between 0 and 1.
 * @return The linear interpolated vec3.
 */
XZM::vec3 XZM::Lerp(const vec3& low, const vec3& high, float t){
    float x = low.data[0] * (1.0f - t) + high.data[0] * t;
    float y = low.data[1] * (1.0f - t) + high.data[1] * t;
    float z = low.data[2] * (1.0f - t) + high.data[2] * t;

    return {x,y,z};
}


/**
 * Find the linear interpolated quat between the lower bound and the upper bound given an alpha.
 * @param low The lower bound.
 * @param high The upper bound.
 * @param t The alpha factor between 0 and 1.
 * @return The linear interpolated quat.
 */
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
        // Quaternions are not close, use spherical linear interpolation (square lerp)
        float angle = acos(dot);
        float sinAngle = sin(angle);
        float invSinAngle = 1.0f / sinAngle;
        float factor_1 = sin((1.0f - t) * angle) * invSinAngle;
        float factor_2 = sin(t * angle) * invSinAngle;

        result.data[0] = low.data[0] * factor_1 + high_adj.data[0] * factor_2;
        result.data[1] = low.data[1] * factor_1 + high_adj.data[1] * factor_2;
        result.data[2] = low.data[2] * factor_1 + high_adj.data[2] * factor_2;
        result.data[3] = low.data[3] * factor_1 + high_adj.data[3] * factor_2;
    }
    return result;
}


/**
 * @brief Find the Spherical Linear interpolate two vec3 with a given factor t.
 * @param low The lower bound.
 * @param high The upper bound.
 * @param t The alpha factor between 0 and 1.
 * @return The linear interpolated vec3.
 */
XZM::vec3 XZM::SLerp(const vec3& low, const vec3& high, float t){
    float theta = acosf(XZM::DotProduct(low,high));

    /* If theta is close to 0, change to lerp to avoid zero divide. */
    if(abs(theta) < 0.005f){
        return Lerp(low,high,t);
    }

    return low * (sin((1-t)*theta)/ sin(theta)) + high * (sin(t*theta)/sin(theta));
}


/**
 * @brief Find the Spherical Linear interpolate two quat with a given factor t.
 * @param low The lower bound.
 * @param high The upper bound.
 * @param t The alpha factor between 0 and 1.
 * @return The linear interpolated quat.
 */
/* Spherical Linear interpolate two quaternion with a given factor t. */
XZM::quat XZM::SLerp(const quat& low, const quat& high, float t){
    float dot = DotProduct(low,high);
    /* If the dot product is negative, invert one of the quaternions to take the shortest path. */
    quat high_adj = high;
    if (dot < 0.0f) {
        high_adj.data[0] = -high.data[0];
        high_adj.data[1] = -high.data[1];
        high_adj.data[2] = -high.data[2];
        high_adj.data[3] = -high.data[3];
        dot = -dot;
    }

    float theta = acos(dot);

    /* If theta is close to 0, change to lerp to avoid zero divide. */
    if(abs(sin(theta)) <= 1.0f){
        return Lerp(low,high_adj,t);
    }
    printf("Here %f and %f\n", dot, theta);
    return low * (sin((1-t)*theta)/ sin(theta)) + high_adj * (sin(t*theta)/sin(theta));
}