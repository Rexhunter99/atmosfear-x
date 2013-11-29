
#include "C3Dit.h"
#include "vec3.h"

/** Math functions **/
float lengthdir_x(float len,float yaw,float pitch)
{
    return(sinf(degtorad(yaw-90))*cosf(degtorad(pitch))*len);
}


float lengthdir_y(float len,float yaw,float pitch)
{
    return(cosf(degtorad(yaw-90))*cosf(degtorad(pitch))*len);
}


float lengthdir_z(float len, float pitch)
{
    return(sinf(degtorad(pitch))*len);
}

float degtorad(float deg)
{
    return deg * pi / 180.0f;
}

/* Collision/Intersection */

bool rayIntersectsPoint(vec3 &Origin, vec3 &Direction, const vec3 &Point)
{
    vec3 P(Origin),D(Direction),V(Point);

    vec3 vss = V - P;
    vss.Normalize();

	float MinDistance = 0.0f;
    float TestDist = sqrtf( powf(D.x-vss.x,2) + powf(D.z-vss.z,2) + powf(D.z-vss.z,2) );
    if ( TestDist > (-MinDistance) &&TestDist < MinDistance )
    {
        //MinDistance = TestDist;
        return true;
    }
    return false;
}

bool rayIntersectsTriangle( vec3 &Origin, vec3 &Direction,
                            vec3 &v1, vec3 &v2, vec3 &v3)
{
    vec3 p(Origin),d(Direction);
    vec3 e1,e2,h,s,q;
    vec3 V0(v1),V1(v2),V2(v3);
    float a,f,u,v,t;

    e1 = V1 - V0;
    e2 = V2 - V0;

    d.CrossProduct(&h,e2);

    a = e1.DotProduct(h);

    //if (a > -0.00001 && a < 0.00001) return(false);
    if (a == 0.0f) return false;

    f = 1.0f / a;

    s = p - V0;

    u = f * (s.DotProduct(h));

    if (u < 0.0 || u > 1.0) return(false);

    s.CrossProduct(&q,e1);

    v = f * d.DotProduct(q);

    if (v < 0.0 || u + v > 1.0) return(false);

    // at this stage we can compute t to find out where
    // the intersection point is on the line

    t = f * e2.DotProduct(q);

    if (t > 0.00001f) return(true);
    else return (false);
}