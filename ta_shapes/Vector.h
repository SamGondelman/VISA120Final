#ifndef __VECTOR_H__
#define __VECTOR_H__

#include <math.h>

class Vector {
public:
    float x, y, z;

    Vector(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f) : x(_x), y(_y), z(_z) {}
    Vector(const Vector &vec) : x(vec.x), y(vec.y), z(vec.z) {}

    Vector operator+(const Vector &vec) const { return Vector(x + vec.x, y + vec.y, z + vec.z); }
    Vector operator-(const Vector &vec) const { return Vector(x - vec.x, y - vec.y, z - vec.z); }
    Vector operator*(float s) const { return Vector(x * s, y * s, z * s); }
    Vector operator*(const Vector &vec) const { return Vector(x * vec.x, y * vec.y, z * vec.z); }
    Vector operator/(float s) const { return Vector(x / s, y / s, z / s); }

    float Length() { return sqrtf(x * x + y * y + z * z); }
    float Dot(const Vector &vec) { return x * vec.x + y * vec.y + z * vec.z; }
    Vector Cross(const Vector &vec) const {
        return Vector(y * vec.z - z * vec.y, z * vec.x - x * vec.z, x * vec.y - y * vec.x); }
    Vector Unit() { return *this / Length(); }
    void Normalize() { *this = Unit(); }
};

#endif
