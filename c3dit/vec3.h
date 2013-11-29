
#ifndef __VEC3_H__
#define __VEC3_H__

class vec3
{
    public:
    float x,y,z;

    // Constructor
    vec3(void) : x(0) , y(0) , z(0) {}
    vec3(const vec3 &vec) : x(vec.x) , y(vec.y) , z(vec.z) {}
    vec3(float X,float Y,float Z) : x(X) , y(Y) , z(Z) {}
    vec3(int X,int Y,int Z) : x(X) , y(Y) , z(Z) {}

    // Operators (Some are overloaded)
    vec3& operator* (const vec3&);
    vec3& operator/ (const vec3&);
    vec3& operator+ (const vec3&);
    vec3& operator- (const vec3&);
    vec3& operator* (float);
    vec3& operator/ (float);
    vec3& operator+ (float);
    vec3& operator- (float);

    vec3& operator*= (const vec3&);
    vec3& operator/= (const vec3&);
    vec3& operator+= (const vec3&);
    vec3& operator-= (const vec3&);
    vec3& operator*= (float);
    vec3& operator/= (float);
    vec3& operator+= (float);
    vec3& operator-= (float);

    // Functions
    void        CrossProduct(vec3*, const vec3 &vec);
    float       DotProduct(const vec3 &vec);
    float       InnerProduct(const vec3 &vec);
    float       Magnitude(void);
    void        Normalize(void);
    void        Rotate(const vec3&);
    //void        Multiply(const mat44&);

    inline void Up() { x=0;y=0;z=1; }
};

#endif //__VEC3_H__
