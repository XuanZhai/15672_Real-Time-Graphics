//
// Created by Xuan Zhai on 2024/2/4.
//

#ifndef XUANJAMESZHAI_A1_XZMATH_H
#define XUANJAMESZHAI_A1_XZMATH_H

#include <array>
#include <cmath>

namespace XZM {

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
        vec3 operator- (const vec3& nv) const;
    };

    class mat4{
    public:
        std::array<std::array<float,4>,4> data{};

        mat4();

        mat4(const mat4& nm);
        mat4& operator= (const mat4& nm);
        bool operator== (const mat4& nm) const;
        mat4 operator * (const mat4& nm) ;

        explicit mat4(float);
    };

    /* x, y, z, w */
    class quat{
    public:
        std::array<float,4> data{};
        
        quat();
        quat(float x,float y,float z,float w);
        quat(const quat&);
        quat& operator= (const quat& nq);
        bool operator== (const quat& nq) const;
        quat operator* (const quat& nq);
    };

    mat4 Translate(const mat4& nm, const vec3& nv);

    mat4 Inverse(const mat4& nm);

    mat4 QuatToMat4(const quat&);

    mat4 Perspective(float vfov, float aspect, float near, float far);

    vec3 CrossProduct(const vec3& v1, const vec3& v2);

    float DotProduct(const vec3& v1, const vec3& v2);

    vec3 Normalize(const vec3& nv);

    mat4 LookAt(const vec3& eyePos, const vec3& target, const vec3& up);

    mat4 Scaling(const mat4& nm, const vec3& factor);

    mat4 Translation(const mat4& nm, const vec3& factor);

    quat Normalize(const quat& nq);
}


#endif //XUANJAMESZHAI_A1_XZMATH_H
