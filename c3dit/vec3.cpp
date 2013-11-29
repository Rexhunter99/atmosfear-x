
#include "vec3.h"
#include <math.h>

vec3& vec3::operator* (const vec3 &vec)
{
    this->x *= vec.x;
    this->y *= vec.y;
    this->z *= vec.z;
    return *this;
}
vec3& vec3::operator/ (const vec3 &vec)
{
    this->x /= vec.x;
    this->y /= vec.y;
    this->z /= vec.z;
    return *this;
}
vec3& vec3::operator+ (const vec3 &vec)
{
    this->x += vec.x;
    this->y += vec.y;
    this->z += vec.z;
    return *this;
}
vec3& vec3::operator- (const vec3 &vec)
{
    this->x -= vec.x;
    this->y -= vec.y;
    this->z -= vec.z;
    return *this;
}
vec3& vec3::operator* (float scal)
{
    this->x *= scal;
    this->y *= scal;
    this->z *= scal;
    return *this;
}
vec3& vec3::operator/ (float scal)
{
    this->x /= scal;
    this->y /= scal;
    this->z /= scal;
    return *this;
}
vec3& vec3::operator+ (float scal)
{
    this->x += scal;
    this->y += scal;
    this->z += scal;
    return *this;
}
vec3& vec3::operator- (float scal)
{
    this->x -= scal;
    this->y -= scal;
    this->z -= scal;
    return *this;
}

vec3& vec3::operator*= (const vec3 &vec)
{
    this->x *= vec.x;
    this->y *= vec.y;
    this->z *= vec.z;
    return *this;
}
vec3& vec3::operator/= (const vec3 &vec)
{
    this->x /= vec.x;
    this->y /= vec.y;
    this->z /= vec.z;
    return *this;
}
vec3& vec3::operator+= (const vec3 &vec)
{
    this->x += vec.x;
    this->y += vec.y;
    this->z += vec.z;
    return *this;
}
vec3& vec3::operator-= (const vec3 &vec)
{
    this->x -= vec.x;
    this->y -= vec.y;
    this->z -= vec.z;
    return *this;
}
vec3& vec3::operator*= (float scal)
{
    this->x *= scal;
    this->y *= scal;
    this->z *= scal;
    return *this;
}
vec3& vec3::operator/= (float scal)
{
    this->x /= scal;
    this->y /= scal;
    this->z /= scal;
    return *this;
}
vec3& vec3::operator+= (float scal)
{
    this->x += scal;
    this->y += scal;
    this->z += scal;
    return *this;
}
vec3& vec3::operator-= (float scal)
{
    this->x -= scal;
    this->y -= scal;
    this->z -= scal;
    return *this;
}

// == Functions == //
void vec3::CrossProduct(vec3* _out, const vec3 &vec)
{
    if (_out==0) return;
    // CrossProduct ( v1 x v2 ) Returns a Perpendicular Vector
    _out->x = this->y * vec.z - vec.y * this->z;
    _out->y = this->z * vec.x - vec.z * this->x;
    _out->z = this->x * vec.y - vec.x * this->y;
}

float vec3::DotProduct(const vec3 &vec)
{
    // DotProduct ( v1 . v2 ) Returns the inner product
    return (this->x*vec.x + this->y*vec.y + this->z*vec.z);
}

float vec3::InnerProduct(const vec3 &vec)
{
    // DotProduct ( v1 . v2 ) Returns the dot product
    return (this->x*vec.x + this->y*vec.y + this->z*vec.z);
}

float vec3::Magnitude()
{
    // Length ( |v| ) Returns the length of this vector
    return sqrtf(this->x*this->x + this->y*this->y + this->z*this->z);
}

void vec3::Normalize()
{
    // Convert to Unit Vector: [ v = v / |v| ]
    float m = this->Magnitude();
    this->x = this->x / m;
    this->y = this->y / m;
    this->z = this->z / m;
}

/*void vec3::Multiply(const mat44& mat)
{
    this->x = (mat[0 ]*this->x) + (mat[1 ]*this->y) + (mat[2 ]*this->z);
    this->y = (mat[4 ]*this->x) + (mat[5 ]*this->y) + (mat[6 ]*this->z);
    this->z = (mat[8 ]*this->x) + (mat[9 ]*this->y) + (mat[10]*this->z);
}*/