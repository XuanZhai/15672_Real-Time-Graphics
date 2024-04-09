//
// Created by Xuan Zhai on 2024/2/4.
//

#ifndef XUANJAMESZHAI_A1_XZMATH_H
#define XUANJAMESZHAI_A1_XZMATH_H

#include <array>
#include <cmath>
#include <string>


namespace XZM {

    /**
     * @brief A custom vec3 class, each element is a float variable.
     */
    class vec3{
        public:
            std::array<float,3> data{};

            vec3();
            vec3(float nx,float ny,float nz);
            vec3(const vec3& nv);
            vec3& operator= (const vec3& nv);
            bool operator== (const vec3& nv) const;
            vec3 operator+ (const vec3& nv) const;
            vec3 operator* (const vec3& nv) const;
            vec3 operator* (float nf) const;
            vec3 operator- (const vec3& nv) const;
            vec3 operator/ (float nf) const;
            vec3& operator+= (const vec3& nv);
            vec3& operator-= (const vec3& nv);

            /* Check if a vec3 is 0,0,0. */
            [[nodiscard]] bool IsEmpty() const;
            /* Get the length of the vec3. */
            [[nodiscard]] float Length() const;
    };


    /**
     * @brief A custom mat4 class, each element is a float variable.
     */
    class mat4{
        public:
            std::array<std::array<float,4>,4> data{};

            mat4();
            mat4(const mat4& nm);
            mat4& operator= (const mat4& nm);
            bool operator== (const mat4& nm) const;
            mat4 operator * (const mat4& nm) ;
            vec3 operator * (const vec3& nv);

            explicit mat4(float);
    };


    /**
     * @brief A custom quaternion class, each element is a float variable.
     * The four element is in the order of x, y, z, w.
     */
    class quat{
        public:
            std::array<float,4> data{};

            quat();
            quat(float x,float y,float z,float w);
            quat(const quat&);
            quat& operator= (const quat& nq);
            bool operator== (const quat& nq) const;
            quat operator+ (const quat& nq) const;
            quat operator* (const quat& nq);
            quat operator* (float f) const;
    };

    /* Convert a quat to a mat4 rotation matrix. */
    mat4 QuatToMat4(const quat&);

    /* Find the perspective matrix. */
    mat4 Perspective(float v_fov, float aspect, float near, float far);

    /* Calculate the cross product of two vectors. */
    vec3 CrossProduct(const vec3& v1, const vec3& v2);

    /* Calculate the dot product of two vectors. */
    float DotProduct(const vec3& v1, const vec3& v2);

    /* Calculate the dot product of two quaternions. */
    float DotProduct(const quat& q1, const quat& q2);

    /* Normalize a vec3. */
    vec3 Normalize(const vec3& nv);

    /* Find the view matrix. */
    mat4 LookAt(const vec3& eyePos, const vec3& target, const vec3& up);

    /* Find the 'forward' direction given the rotation matrix. */
    vec3 GetLookAtDir(const mat4& rotationMatrix);

    /* Given a scale vec3, convert it to a scale matrix. */
    mat4 Scaling(const vec3& factor);

    /* Given a translation vec3, convert it to a translation matrix. */
    mat4 Translation(const vec3& nv);

    /* Convert a vec3 to a string, used for output debug. */
    std::string ToString(const vec3&);

    /* Transpose a mat4. */
    mat4 Transpose(const mat4&);

    /* Given a transform matrix, extract the translation factor within. */
    vec3 ExtractTranslationFromMat(const mat4&);

    /* Rotate a vec3 along an axis for a given radians. */
    vec3 RotateVec3(const vec3& vector, const vec3& axis, float radians);

    mat4 RotateMat4(const mat4& mat, const vec3& axis, float radians);

    /* Compute the orthographic matrix. */
    mat4 Ortho(float left, float right, float bottom, float top, float near, float far);

    /* Linear interpolate two vec3 with a given factor t. */
    vec3 Lerp(const vec3& low, const vec3& high, float t);

    /* Linear interpolate two quaternion with a given factor t. */
    quat Lerp(const quat& low, const quat& high, float t);

    /* Spherical Linear interpolate two vec3 with a given factor t. */
    vec3 SLerp(const vec3& low, const vec3& high, float t);

    /* Spherical Linear interpolate two quaternion with a given factor t. */
    quat SLerp(const quat& low, const quat& high, float t);
}


#endif //XUANJAMESZHAI_A1_XZMATH_H
