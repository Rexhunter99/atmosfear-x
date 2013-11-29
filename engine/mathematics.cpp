#include "Hunt.h"
#include <time.h>
#include <stdint.h>
#include <memory.h>


// Global Variables
vec3 TraceA, TraceB, TraceNv;
int      TraceRes;


#if defined( AF_PLATFORM_WINDOWS )

struct timeval
{
    time_t  tv_sec;     // seconds
    long    tv_usec;    // microseconds
};

#define DELTA_EPOCH_IN_MICROSECS 11644473600000000Ui64

int gettimeofday ( struct timeval *tv, void *pDummy )
{
    FILETIME time;
    double timed;
    GetSystemTimeAsFileTime ( &time );

    // Win32 has units of 1e-7 sec (tenths of microseconds)
    // 429496729++ = 2^32. Shifting the high word over 11644473600
    // is the number of seconds between the Windows epoch 1601-Jan-01
    // and the Unix epoch 1970-Jan-01

    timed = ( ( time.dwHighDateTime * 4294967296e-7 ) - 11644473600.0 ) +
            ( time.dwLowDateTime * 1e-7 );
    tv->tv_sec = ( long ) timed;
    tv->tv_usec = ( long ) ( ( timed - tv->tv_sec ) * 1e6 );
    return 0;
}

#endif


//====================================================

/**
* Normalize the vector
**/
void NormVector(vec3& v, float Scale)
{
    float n;
    n=v.x*v.x + v.y*v.y + v.z*v.z;
    if (n<0.000000001f) n=0.000000001f;
    n=Scale / sqrtf(n);
    v.x=v.x*n;
    v.y=v.y*n;
    v.z=v.z*n;
}

float SGN(float f)
{
    if (f<0)
        return -1.f;
    else
        return  1.f;
}

void DeltaFunc(float &a, float b, float d)
{
    if (b > a)
    {
        a+=d;
        if (a > b) a = b;
    }
    else
    {
        a-=d;
        if (a < b) a = b;
    }
}


/**
* Multiply the vectors and return the value through (r)
ToDo: use the function to return the value instead of pass it via variable
**/
//======================================================================
// DotProduct / InnerProduct of vectors
void MulVectorsScal(vec3 v1,vec3 v2, float &r)
{
    r = v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
}

/**
* Multiply the vectors and return the resulting vector by (r)
**/
void MulVectorsVect(vec3 v1, vec3 v2, vec3& r )
{
    r.x= v1.y*v2.z - v2.y*v1.z;
    r.y=-v1.x*v2.z + v2.x*v1.z;
    r.z= v1.x*v2.y - v2.x*v1.y;
}

/**
* Rotate the vector and return the value
**/
vec3 RotateVector(vec3 v)
{
    // Rotate the vector based on the Camera Alpha and Beta
    // cb = cos beta, ca = cos alpha
    // sb = sin beta, sa = sin alpha
    vec3 vv;
    float vx = v.x * ca + v.z * sa;
    float vz = v.z * ca - v.x * sa;
    float vy = v.y;
    vv.x = vx;
    vv.y = vy * cb - vz * sb;
    vv.z = vz * cb + vy * sb;
    return vv;
}

/**
* Subtract the vectors and return the resulting vector
**/
vec3 SubVectors( vec3 v1, vec3 v2 )
{
    // Subtract v2 from v1
    vec3 res;
    res.x = v1.x-v2.x;
    res.y = v1.y-v2.y;
    res.z = v1.z-v2.z;
    return res;
}

/**
* Add the vectors and return the resulting vector
**/
vec3 AddVectors( vec3 v1, vec3 v2 )
{
    vec3 res;
    res.x = v1.x+v2.x;
    res.y = v1.y+v2.y;
    res.z = v1.z+v2.z;
    return res;
}


/**
* Return the magnitude of the vector
**/
float VectorLength(vec3 v)
{
    return sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
}


int siRand(int R)
{
	//int r = (float(rand()) / float(RAND_MAX) ) * R;//(rand() * (R * 2 +1)) / RAND_MAX - R;
    return ((float(rand()) / float(RAND_MAX) ) * (R*2) ) - R;
}


int rRand(int R)
{
    if (!R) return 0;
    int res = ( float(rand()) / float(RAND_MAX) ) * float(R);// / (RAND_MAX);
    return res;
}



void CalcHitPoint(CLIPPLANE& C, vec3 a, vec3 b, vec3 hp)
{
    float SCLN,SCVN;
    vec3 lv = SubVectors(b,a);
    NormVector(lv, 1.0);

    MulVectorsScal(a, C.nv, SCLN);
    MulVectorsScal(lv,C.nv, SCVN);

    SCLN/=SCVN;
    SCLN=(float)fabsf(SCLN);
    hp.x = a.x + lv.x * SCLN;
    hp.y = a.y + lv.y * SCLN;
    hp.z = a.z + lv.z * SCLN;
}


float PointToVectorD(vec3 A, vec3 AB, vec3 C)
{
    vec3 AC = SubVectors(C,A);
    vec3 vm;
    MulVectorsVect(AB, AC, vm);
    return VectorLength(vm);
}


float Mul2dVectors(float vx, float vy, float ux, float uy)
{
    return vx*uy - ux*vy;
}


float _FindVectorAlpha(float vx, float vy)
{
    float adx, ady, alpha, dalpha;

    adx=(float)fabsf(vx);
    ady=(float)fabsf(vy);

    alpha = pi / 4.f;
    dalpha = pi / 8.f;

    for (int i=1; i<=10; i++)
    {
        alpha=alpha-dalpha*SGN(Mul2dVectors(adx,ady, (float)cosf(alpha), (float)sinf(alpha)));
        dalpha/=2;
    }

    if (vx<0)   if (vy<0) alpha+=pi;
        else alpha=pi-alpha;
    else        if (vy<0) alpha=2.f*pi-alpha;

    return alpha;
}

float FindVectorAlpha(float vx, float vy)
{
    float al = atan2f(-vy, -vx)+pi;
    if (al<0) al = 2.f*pi-al;
    if (al>2.f*pi) al = al-2.f*pi;
    //sprintf(logt, "%d   %d", (int)(al*100), (int)(_FindVectorAlpha(vx, vy)*100));
    //AddMessage(logt);
    return al;
}


void CheckBoundCollision(float &px, float &py, float cx, float cy, float oy, TBound *bound, int angle)
{
    float ppx=px-cx;
    float ppy=py-cy;

    float ca = (float) cosf(angle*pi / 2.f);
    float sa = (float) sinf(angle*pi / 2.f);
    float w,h;

    for (int o=0; o<8; o++)
    {

        if (bound[o].a<0) continue;
        if (bound[o].y2 + oy < PlayerY + 128) continue;
        if (bound[o].y1 + oy > PlayerY + 256) continue;

        float ccx = bound[o].cx*ca + bound[o].cy*sa;
        float ccy = bound[o].cy*ca - bound[o].cx*sa;

        if (angle & 1)
        {
            w = bound[o].b+2.f;
            h = bound[o].a+2.f;
        }
        else
        {
            w = bound[o].a+2.f;
            h = bound[o].b+2.f;
        }

        float dw = fabsf(ppx - ccx) - w;
        float dh = fabsf(ppy - ccy) - h;

        if ( (dw > 0) || (dh > 0) ) continue;

        if (dw > dh)
        {
            px = cx+ccx + w * SGN(ppx-ccx);
        }
        else
        {
            py = cy+ccy + h * SGN(ppy-ccy);
        }
    }
}



void CheckCollision(float &cx, float &cz)
{
    if (cx < 36*256) cx = 36*256;
    if (cz < 36*256) cz = 36*256;
    if (cx >980*256) cx =980*256;
    if (cz >980*256) cz =980*256;
    int ccx = (int)cx / 256;
    int ccz = (int)cz / 256;

    for (int z=-4; z<=4; z++)
        for (int x=-4; x<=4; x++)
            if (OMap[ccz+z][ccx+x]!=255)
            {
                int ob = OMap[ccz+z][ccx+x];
                float CR = (float)MObjects[ob].info.Radius;

                float oz = (ccz+z) * 256.f + 128.f;
                float ox = (ccx+x) * 256.f + 128.f;

                float LandY = GetLandOH(ccx+x, ccz+z);

                if (!(MObjects[ob].info.flags & ofBOUND))
                {
                    if (MObjects[ob].info.YHi + LandY < PlayerY + 128) continue;
                    if (MObjects[ob].info.YLo + LandY > PlayerY + 256) continue;
                }

                if (MObjects[ob].info.flags & ofBOUND)
                {
                    CheckBoundCollision(cx, cz, ox, oz, LandY, MObjects[ob].bound, ((FMap[ccz+z][ccx+x] >> 2) & 3)  );
                }
                else if (MObjects[ob].info.flags & ofCIRCLE)
                {
                    float r = (float) sqrtf( (ox-cx)*(ox-cx) + (oz-cz)*(oz-cz) );
                    if (r<CR)
                    {
                        cx = cx - (ox - cx) * (CR-r)/r;
                        cz = cz - (oz - cz) * (CR-r)/r;
                    }
                }
                else
                {
                    float r = (float) std::max( fabsf(ox-cx) , fabsf(oz-cz) );
                    if (r<CR)
                    {
                        if (fabsf(ox-cx) > fabsf(oz-cz) )
                            cx = cx - (ox - cx) * (CR-r)/r;
                        else
                            cz = cz - (oz - cz) * (CR-r)/r;
                    }
                }
            }

    if (!TrophyMode) return;
    for (int c=0; c<ChCount; c++)
    {
        float px = Characters[c].pos.x;
        float pz = Characters[c].pos.z;
        float CR = DinoInfo[ Characters[c].CType ].Radius;
        float r = (float) sqrtf( (px-cx)*(px-cx) + (pz-cz)*(pz-cz) );
        if (r<CR)
        {
            cx = cx - (px - cx) * (CR-r)/r;
            cz = cz - (pz - cz) * (CR-r)/r;
        }

    }

}



int TraceCheckPlane(vec3 a, vec3 b, vec3 c)
{
    vec3 pnv,hp;
    float sa, sb;
    MulVectorsVect(SubVectors(b,a), SubVectors(c,a), pnv);
    NormVector(pnv, 1.f);

    MulVectorsScal(SubVectors(TraceA,a), pnv, sa);
    MulVectorsScal(SubVectors(TraceB,a), pnv, sb);
    if (sa*sb>-1.f) return 0;

//========= calc hit point =======//
    float SCLN,SCVN;

    MulVectorsScal(SubVectors(TraceA,a), pnv, SCLN);
    MulVectorsScal(TraceNv, pnv, SCVN);

    SCLN/=SCVN;
    SCLN=(float)fabsf(SCLN);
    hp.x = TraceA.x + TraceNv.x * SCLN;
    hp.y = TraceA.y + TraceNv.y * SCLN;
    hp.z = TraceA.z + TraceNv.z * SCLN;

    vec3 vm;
    MulVectorsVect( SubVectors(b,a), SubVectors(hp,a), vm);
    MulVectorsScal( vm, pnv, sa);
    if (sa<0) return 0;

    MulVectorsVect( SubVectors(c,b), SubVectors(hp,b), vm);
    MulVectorsScal( vm, pnv, sa);
    if (sa<0) return 0;

    MulVectorsVect( SubVectors(a,c), SubVectors(hp,c), vm);
    MulVectorsScal( vm, pnv, sa);
    if (sa<0) return 0;


    if (VectorLength(SubVectors(hp, TraceA)) <
            VectorLength(SubVectors(TraceB, TraceA)) )
    {
        TraceB = hp;
        return 1;
    }

    return 0;
}


void TraceModel(int xx, int zz, int o)
{
    float dist = 0.0f;
    TModel *mptr = MObjects[o].model;
    v[0].x = xx * 256.f + 128.f;
    v[0].z = zz * 256.f + 128.f;
    v[0].y = (float)(HMapO[zz][xx]) * ctHScale;

    v[0].y+=700.f;
    if ( (dist = PointToVectorD(TraceA, TraceNv, v[0]) ) > 1024.f * WeapInfo[CurrentWeapon].Range ) return;
    v[0].y-=700.f;

    float malp = (float)((FMap[zz][xx] >> 2) & 7) * 2.f*pi / 8.f;

    float ca = (float)cosf(malp);
    float sa = (float)sinf(malp);

    for (int vv=0; vv<mptr->VCount; vv++)
    {
        rVertex[vv].x = mptr->gVertex[vv].x * ca + mptr->gVertex[vv].z * sa  + v[0].x;
        rVertex[vv].y = mptr->gVertex[vv].y + v[0].y;
        rVertex[vv].z = mptr->gVertex[vv].z * ca - mptr->gVertex[vv].x * sa  + v[0].z;
    }

    for (int f=0; f<mptr->FCount; f++)
    {
        TFacef *fptr = &mptr->gFace[f];
        if (fptr->Flags & (sfOpacity + sfTransparent) ) continue;
        v[0] = rVertex[fptr->v1];
        v[1] = rVertex[fptr->v2];
        v[2] = rVertex[fptr->v3];
        if (TraceCheckPlane(v[0], v[1], v[2]) )
            TraceRes = tresModel;
    }
}




void TraceCharacter(int c)
{
    float dist = 0.0f;
    TCharacter *cptr = &Characters[c];

    if ( (dist = PointToVectorD(TraceA, TraceNv, cptr->pos) ) > 1024.f * WeapInfo[CurrentWeapon].Range ) return;

    TModel *mptr = cptr->pinfo->mptr;
    CreateChMorphedModel(cptr);
    float ca = (float)cosf(-cptr->alpha + pi / 2.f);
    float sa = (float)sinf(-cptr->alpha + pi / 2.f);
    for (int vv=0; vv<mptr->VCount; vv++)
    {
        rVertex[vv].x = mptr->gVertex[vv].x * ca + mptr->gVertex[vv].z * sa  + cptr->pos.x;
        rVertex[vv].y = mptr->gVertex[vv].y + cptr->pos.y;
        rVertex[vv].z = mptr->gVertex[vv].z * ca - mptr->gVertex[vv].x * sa  + cptr->pos.z;
    }

    for (int f=0; f<mptr->FCount; f++)
    {
        TFacef *fptr = &mptr->gFace[f];
        if (fptr->Flags & (sfOpacity + sfTransparent) ) continue;
        v[0] = rVertex[fptr->v1];
        v[1] = rVertex[fptr->v2];
        v[2] = rVertex[fptr->v3];
        if (TraceCheckPlane(v[0], v[1], v[2]) )
        {
            TraceRes = tresChar;
            ShotDino = c;
            if (fptr->Flags & sfMortal) TraceRes |= 0x8000;
        }

    }
}


void FillVGround(vec3 &v, int xx, int zz)
{
    v.x = xx*256.f;
    v.z = zz*256.f;
    v.y = (float)HMap[zz][xx]*ctHScale;
}


void FillWGround(vec3 &v, int xx, int zz)
{
    v.x = xx*256.f;
    v.z = zz*256.f;
    v.y = (float)WaterList[ WMap[zz][xx] ].wlevel*ctHScale;
}


int  TraceLook(float ax, float ay, float az,
               float bx, float by, float bz)
{
    TraceA.x = ax;
    TraceA.y = ay;
    TraceA.z = az;
    TraceB.x = bx;
    TraceB.y = by;
    TraceB.z = bz;

    TraceNv =  SubVectors(TraceB, TraceA);

    vec3 TraceNvP;

    TraceNvP = TraceNv;
    TraceNvP.y = 0;

    NormVector(TraceNv, 1.0f);
    NormVector(TraceNvP, 1.0f);
    ObjectsOnLook=0;

    int axi = (int)(ax/256.f);
    int azi = (int)(az/256.f);

    int bxi = (int)(bx/256.f);
    int bzi = (int)(bz/256.f);

    int xm1 = std::min(axi, bxi) - 2;
    int xm2 = std::max(axi, bxi) + 2;
    int zm1 = std::min(azi, bzi) - 2;
    int zm2 = std::max(azi, bzi) + 2;

//======== trace ground model and static objects ============//
    for (int zz=zm1; zz<=zm2; zz++)
        for (int xx=xm1; xx<=xm2; xx++)
        {
            if (xx<2 || xx>1010) continue;
            if (zz<2 || zz>1010) continue;

            bool ReverseOn = (FMap[zz][xx] & fmReverse);

            FillVGround(v[0], xx, zz);
            FillVGround(v[1], xx+1, zz);
            if (ReverseOn) FillVGround(v[2], xx, zz+1);
            else FillVGround(v[2], xx+1, zz+1);
            if (TraceCheckPlane(v[0], v[1], v[2])) return 1;

            if (ReverseOn)
            {
                v[0] = v[2];
                FillVGround(v[2], xx+1, zz+1);
            }
            else
            {
                v[1] = v[2];
                FillVGround(v[2], xx, zz+1);
            }
            if (TraceCheckPlane(v[0], v[1], v[2])) return 1;

            int o = OMap[zz][xx];
            if ( o!=255)
            {
                float s1,s2;
                v[0].x = xx * 256.f + 128.f;
                v[0].z = zz * 256.f + 128.f;
                v[0].y = TraceB.y;
                MulVectorsScal( SubVectors(v[0], TraceB), TraceNv, s1);
                s1*=-1;
                v[0].y = TraceA.y;
                MulVectorsScal( SubVectors(v[0], TraceA), TraceNv, s2);

                if (s1>0 && s2>0)
                    if (PointToVectorD(TraceA, TraceNvP, v[0]) < 180.f)
                    {
                        ObjectsOnLook++;
                        if (MObjects[o].info.Radius > 32)	ObjectsOnLook++;
                    }
            }
        }

    return 0;
}



int  TraceShot(float  ax, float  ay, float az,
               float &bx, float &by, float &bz)
{
    TraceA.x = ax;
    TraceA.y = ay;
    TraceA.z = az;
    TraceB.x = bx;
    TraceB.y = by;
    TraceB.z = bz;

    TraceNv =  SubVectors(TraceB, TraceA);
    NormVector(TraceNv, 1.0f);
    TraceRes = -1;

    int axi = (int)(ax/256.f);
    int azi = (int)(az/256.f);

    int bxi = (int)(bx/256.f);
    int bzi = (int)(bz/256.f);

    int xm1 = std::min(axi, bxi) - 2;
    int xm2 = std::max(axi, bxi) + 2;
    int zm1 = std::min(azi, bzi) - 2;
    int zm2 = std::max(azi, bzi) + 2;

//======== trace ground model and static objects ============//
    for (int zz=zm1; zz<=zm2; zz++)
        for (int xx=xm1; xx<=xm2; xx++)
        {
            if (xx<2 || xx>1010) continue;
            if (zz<2 || zz>1010) continue;

            bool ReverseOn = (FMap[zz][xx] & fmReverse);

            FillVGround(v[0], xx, zz);
            FillVGround(v[1], xx+1, zz);

            if (ReverseOn) FillVGround(v[2], xx, zz+1);
            else FillVGround(v[2], xx+1, zz+1);
            if (TraceCheckPlane(v[0], v[1], v[2])) TraceRes = tresGround;

            if (ReverseOn)
            {
                v[0] = v[2];
                FillVGround(v[2], xx+1, zz+1);
            }
            else
            {
                v[1] = v[2];
                FillVGround(v[2], xx, zz+1);
            }
            if (TraceCheckPlane(v[0], v[1], v[2])) TraceRes = tresGround;

            if ( (FMap[zz][xx] & fmWaterA)>0)
            {
                FillWGround(v[0], xx, zz);
                FillWGround(v[1], xx+1, zz);
                FillWGround(v[2], xx+1, zz+1);
                if (TraceCheckPlane(v[0], v[1], v[2])) TraceRes = tresWater;
                v[1] = v[2];
                FillWGround(v[2], xx, zz+1);
                if (TraceCheckPlane(v[0], v[1], v[2])) TraceRes = tresWater;
            }

            if (OMap[zz][xx] !=255)
                TraceModel(xx, zz, OMap[zz][xx]);
        }

//======== trace characters ============//
    for (int c=0; c<ChCount; c++)
        TraceCharacter(c);

    float l;
    if ((TraceRes & 0xFF)==tresChar) l = 32.f;
    else l=16.f;
    bx = TraceB.x - TraceNv.x * l;
    by = TraceB.y - TraceNv.y * l;
    bz = TraceB.z - TraceNv.z * l;

    return TraceRes;
}




void InitClips2()
{
    ClipA.v1.x = - (float)sinf(pi/4-0.11); // 0.741
    ClipA.v1.y = 0;
    ClipA.v1.z =   (float)cosf(pi/4-0.11); // 0.820
    ClipA.v2.x = 0;
    ClipA.v2.y = 1;
    ClipA.v2.z = 0;
    MulVectorsVect(ClipA.v1, ClipA.v2, ClipA.nv);

    ClipC.v1.x = + (float)sinf(pi/4-0.11);
    ClipC.v1.y = 0;
    ClipC.v1.z =   (float)cosf(pi/4-0.11);
    ClipC.v2.x = 0;
    ClipC.v2.y =-1;
    ClipC.v2.z = 0;
    MulVectorsVect(ClipC.v1, ClipC.v2, ClipC.nv);


    ClipB.v1.x = 0;
    ClipB.v1.y =   (float)sinf(pi/5-.02);
    ClipB.v1.z =   (float)cosf(pi/5-.02);
    ClipB.v2.x = 1;
    ClipB.v2.y = 0;
    ClipB.v2.z = 0;
    MulVectorsVect(ClipB.v1, ClipB.v2, ClipB.nv);

    ClipD.v1.x = 0;
    ClipD.v1.y = - (float)sinf(pi/5-.02);
    ClipD.v1.z =   (float)cosf(pi/5-.02);
    ClipD.v2.x =-1;
    ClipD.v2.y = 0;
    ClipD.v2.z = 0;
    MulVectorsVect(ClipD.v1, ClipD.v2, ClipD.nv);

    ClipZ.v1.x = 0;
    ClipZ.v1.y = 1;
    ClipZ.v1.z = 0;
    ClipZ.v2.x = 1;
    ClipZ.v2.y = 0;
    ClipZ.v2.z = 0;
    MulVectorsVect(ClipZ.v1, ClipZ.v2, ClipZ.nv);

}



void InitClips()
{
//	float XFOV = atan(CameraW , VideoCX); //1.6
//	float YFOV = atan(CameraH , VideoCY);
    //float xx = (VideoCX+1) / (CameraW);
    //float yy = (VideoCY+2) / (CameraH);
    float xx = (VideoCX+1) / (CameraW);
    float yy = (VideoCY+2) / (CameraH);
    float LX = sqrtf(1.0f + xx * xx);
    float LY = sqrtf(1.0f + yy * yy);

    ClipA.v1.x = - (float)xx / LX;
    ClipA.v1.y = 0;
    ClipA.v1.z =   (float)1 / LX;
    ClipA.v2.x = 0;
    ClipA.v2.y = 1;
    ClipA.v2.z = 0;
    MulVectorsVect(ClipA.v1, ClipA.v2, ClipA.nv);

    ClipC.v1.x = + (float)xx / LX;
    ClipC.v1.y = 0;
    ClipC.v1.z =   (float)1 / LX;
    ClipC.v2.x = 0;
    ClipC.v2.y =-1;
    ClipC.v2.z = 0;
    MulVectorsVect(ClipC.v1, ClipC.v2, ClipC.nv);


    ClipB.v1.x = 0;
    ClipB.v1.y =   (float)yy / LY;
    ClipB.v1.z =   (float)1  / LY;
    ClipB.v2.x = 1;
    ClipB.v2.y = 0;
    ClipB.v2.z = 0;
    MulVectorsVect(ClipB.v1, ClipB.v2, ClipB.nv);

    ClipD.v1.x = 0;
    ClipD.v1.y = - (float)yy / LY;
    ClipD.v1.z =   (float)1  / LY;
    ClipD.v2.x =-1;
    ClipD.v2.y = 0;
    ClipD.v2.z = 0;
    MulVectorsVect(ClipD.v1, ClipD.v2, ClipD.nv);

    ClipZ.v1.x = 0;
    ClipZ.v1.y = 1;
    ClipZ.v1.z = 0;
    ClipZ.v2.x = 1;
    ClipZ.v2.y = 0;
    ClipZ.v2.z = 0;
    MulVectorsVect(ClipZ.v1, ClipZ.v2, ClipZ.nv);

    ClipW.nv.x =  0;
    ClipW.nv.y = cb;
    ClipW.nv.z = sb;

}





void CalcLights(TModel* mptr)
{
    int VCount = mptr->VCount;
    int FCount = mptr->FCount;
    int FUsed;
    float c;
    vec3 norms[1024];
    vec3 a, b, nv, rv;
    vec3 slight;
    slight.x =-Sun3dPos.x;
    slight.y =-Sun3dPos.y/2;
    slight.z =-Sun3dPos.z;


    NormVector(slight, 1.0f);

    for (int f=0; f<FCount; f++)
    {
        int v1 = mptr->gFace[f].v1;
        int v2 = mptr->gFace[f].v2;
        int v3 = mptr->gFace[f].v3;

        a.x = mptr->gVertex[v2].x - mptr->gVertex[v1].x;
        a.y = mptr->gVertex[v2].y - mptr->gVertex[v1].y;
        a.z = mptr->gVertex[v2].z - mptr->gVertex[v1].z;

        b.x = mptr->gVertex[v3].x - mptr->gVertex[v1].x;
        b.y = mptr->gVertex[v3].y - mptr->gVertex[v1].y;
        b.z = mptr->gVertex[v3].z - mptr->gVertex[v1].z;

        MulVectorsVect(a, b, norms[f]);
        NormVector(norms[f], 1.0f);
    }

    for (int VT=0; VT<4; VT++)
    {
        float ca = (float)cos (VT * pi / 2);
        float sa = (float)sin (VT * pi / 2);
        for (int v=0; v<VCount; v++)
        {
            FUsed = 0;
            nv.x=0;
            nv.y=0;
            nv.z=0;
            for (int f=0; f<FCount; f++)
                if (!(mptr->gFace[f].Flags & sfDoubleSide) )
                    if (mptr->gFace[f].v1 == v || mptr->gFace[f].v2 == v || mptr->gFace[f].v3 == v )
                    {
                        FUsed++;
                        nv = AddVectors(nv, norms[f]);
                    }

            if (!FUsed) mptr->VLight[VT][v] = 0;
            else
            {
                NormVector(nv, 1.0f);
                rv.y = nv.y;
                rv.x = nv.x * sa + nv.z * ca;
                rv.z = nv.z * sa - nv.x * ca;
                MulVectorsScal(rv, slight, c);
                c = c * 64;
                //if (c>64) c=64;
                //if (c<-64) c=-64;
#ifdef _d3d
                mptr->VLight[VT][v] = (int)c;
#else
                mptr->VLight[VT][v] = c;
#endif
            }
        }
    }
}


/*
void CalcGouraud(TModel* mptr, Vecto3d *nvs[])
{
	int VCount = mptr->VCount;
	int FCount = mptr->FCount;
    int FUsed;
	float c;
	vec3 norms[1024];
	vec3 a, b, nv, rv;
    vec3 slight;
	slight.x =-Sun3dPos.x;
	slight.y =-Sun3dPos.y;
	slight.z =-Sun3dPos.z;

	NormVector(slight, 1.0f);

    for (int f=0; f<FCount; f++) {
		int v1 = mptr->gFace[f].v1;
		int v2 = mptr->gFace[f].v2;
		int v3 = mptr->gFace[f].v3;

		a.x = mptr->gVertex[v2].x - mptr->gVertex[v1].x;
		a.y = mptr->gVertex[v2].y - mptr->gVertex[v1].y;
		a.z = mptr->gVertex[v2].z - mptr->gVertex[v1].z;

		b.x = mptr->gVertex[v3].x - mptr->gVertex[v1].x;
		b.y = mptr->gVertex[v3].y - mptr->gVertex[v1].y;
		b.z = mptr->gVertex[v3].z - mptr->gVertex[v1].z;

		MulVectorsVect(a, b, norms[f]);
		NormVector(norms[f], 1.0f);
	}

	for (int v=0; v<VCount; v++) {
		FUsed = 0;
		nv.x=0; nv.y=0; nv.z=0;
		for (f=0; f<FCount; f++)
		  if (!(mptr->gFace[f].Flags & sfDoubleSide) )
			if (mptr->gFace[f].v1 == v || mptr->gFace[f].v2 == v || mptr->gFace[f].v3 == v )
			{ FUsed++;  nv = AddVectors(nv, norms[f]); }

		if (!FUsed) mptr->VLight[0][v] = 0;
		else {
           NormVector(nv, 1.0f);
           MulVectorsScal(nv, slight, c);
		   if (c<0) c=0; c=(c-0.5)*2;
		   c=c*c*c;
		   c = c * 98;
		   if (c>96) c=96;
		   if (c<-96) c=-96;
		   mptr->VLight[0][v] = c;
		}

	}
}
*/


void CalcGouraud(TModel* mptr, vec3 *nvs)
{
    int VCount = mptr->VCount;
    float c;
    vec3 slight;
    slight.x =-Sun3dPos.x;
    slight.y =-Sun3dPos.y;
    slight.z =-Sun3dPos.z;

    NormVector(slight, 1.0f);

    for (int v=0; v<VCount; v++)
    {
        MulVectorsScal(nvs[v], slight, c);
        if (c<0) c=0;
        c=(c-0.5f)*2;
        c=c*c*c;
        c = c * 96;
        if (c>96) c=96;
        if (c<-64) c=-64;
#ifdef _d3d
        mptr->VLight[0][v] = (int)c;
#else
        mptr->VLight[0][v] = (float)c;
#endif
    }
}



void CalcNormals(TModel* mptr, vec3 *nvs)
{
    int VCount = mptr->VCount;
    int FCount = mptr->FCount;
    vec3 a, b, nv;
    memset( nvs, 0, 3*4*VCount );

    for (int f=0; f<FCount; f++)
    {
        if (mptr->gFace[f].Flags & sfDoubleSide) continue;
        int v1 = mptr->gFace[f].v1;
        int v2 = mptr->gFace[f].v2;
        int v3 = mptr->gFace[f].v3;

        a.x = mptr->gVertex[v2].x - mptr->gVertex[v1].x;
        a.y = mptr->gVertex[v2].y - mptr->gVertex[v1].y;
        a.z = mptr->gVertex[v2].z - mptr->gVertex[v1].z;

        b.x = mptr->gVertex[v3].x - mptr->gVertex[v1].x;
        b.y = mptr->gVertex[v3].y - mptr->gVertex[v1].y;
        b.z = mptr->gVertex[v3].z - mptr->gVertex[v1].z;

        MulVectorsVect(a, b, nv);
        NormVector(nv, 1000.0f);
        nvs[v1]=AddVectors(nvs[v1], nv);
        nvs[v2]=AddVectors(nvs[v2], nv);
        nvs[v3]=AddVectors(nvs[v3], nv);
    }

    for (int v=0; v<VCount; v++)
        NormVector(nvs[v], 1.0);
}


/*
void CalcNormals(TModel* mptr, vec3 *nvs)
{
	int VCount = mptr->VCount;
	int FCount = mptr->FCount;
    int FUsed;
	float c;
	vec3 norms[1024];
	vec3 a, b, nv, rv;
    vec3 slight;
	slight.x =-Sun3dPos.x;
	slight.y =-Sun3dPos.y;
	slight.z =-Sun3dPos.z;

	NormVector(slight, 1.0f);

    for (int f=0; f<FCount; f++) {
		int v1 = mptr->gFace[f].v1;
		int v2 = mptr->gFace[f].v2;
		int v3 = mptr->gFace[f].v3;

		a.x = mptr->gVertex[v2].x - mptr->gVertex[v1].x;
		a.y = mptr->gVertex[v2].y - mptr->gVertex[v1].y;
		a.z = mptr->gVertex[v2].z - mptr->gVertex[v1].z;

		b.x = mptr->gVertex[v3].x - mptr->gVertex[v1].x;
		b.y = mptr->gVertex[v3].y - mptr->gVertex[v1].y;
		b.z = mptr->gVertex[v3].z - mptr->gVertex[v1].z;

		MulVectorsVect(a, b, norms[f]);
		NormVector(norms[f], 1.0f);
	}

	for (int v=0; v<VCount; v++) {
		FUsed = 0;
		nv.x=0; nv.y=0; nv.z=0;
		for (f=0; f<FCount; f++)
		  if (!(mptr->gFace[f].Flags & sfDoubleSide) )
			if (mptr->gFace[f].v1 == v || mptr->gFace[f].v2 == v || mptr->gFace[f].v3 == v )
			{ FUsed++;  nv = AddVectors(nv, norms[f]); }

		if (!FUsed) { nv.x=0; nv.y=1; nv.z=0; }
        NormVector(nv, 1.0f);
        nvs[v] = nv;
	}
} */



void CalcPhongMapping(TModel* mptr, vec3 *nv)
{
    vec3 l,v,m, tx, ty, tv;
    float x,y;
    l.x =-Sun3dPos.x;
    l.y =-Sun3dPos.y;
    l.z =-Sun3dPos.z;

    tv.z = 0;
    tv.x = 1;
    tv.y = 1;

    NormVector(l, 1.0f);
    for (int i=0; i<mptr->VCount; i++)
    {
        v.x = mptr->gVertex[i].x;
        v.y = mptr->gVertex[i].y;
        v.z = mptr->gVertex[i].z;
        NormVector(v, 1.0f);
        m = AddVectors(l, v);
        NormVector(m, 1.0f);


        if (l.z < 0) MulVectorsVect(m, tv, tx);
        else MulVectorsVect(m, v, tx);

        NormVector(tx, 1.0f);
        MulVectorsVect(m, tx, ty);
        NormVector(ty, 1.0f);
        MulVectorsScal(tx, nv[i], x);
        MulVectorsScal(ty, nv[i], y);
        PhongMapping[i].x = 128 + x *127;
        PhongMapping[i].y = 128 + y *127;
    }
}


void CalcEnvMapping(TModel* mptr, vec3 *nv)
{
    vec3 l,v,m, tx, ty;
    float x,y,s;
    float cc = cosf((PlayerX + PlayerZ) / 500);
    float ss = sinf((PlayerX + PlayerZ) / 500);
    tx.x = cc;
    tx.y = 0.0f;
    tx.z =-ss;
    ty.x = ss;
    ty.y = 0.0f;
    ty.z = cc;

    tx = RotateVector(tx);
    ty = RotateVector(ty);

    NormVector(l, 1.0f);
    for (int i=0; i<mptr->VCount; i++)
    {
        v.x = mptr->gVertex[i].x;
        v.y = mptr->gVertex[i].y;
        v.z = mptr->gVertex[i].z;
        MulVectorsScal(v, nv[i], s);
        m = nv[i];
        NormVector(m, s*2);
        m = AddVectors(m, v);
        NormVector(m, 1.0f);

        MulVectorsScal(tx, m, x);
        MulVectorsScal(ty, m, y);
        PhongMapping[i].x = 128 + x * 122;
        PhongMapping[i].y = 128 + y * 122;
    }
}



void CalcBoundBox(TModel* mptr, TBound *bound)
{
    float x1, x2, y1, y2, z1, z2;
    bool first;

    for (int o=0; o<8; o++)
    {
        first = true;
        bound[o].a=-1;


        for (int v=0; v<mptr->VCount; v++)
        {
            TPoint3d p = mptr->gVertex[v];
            if (p.hide) continue;
            if (p.owner!=o) continue;

            if (first)
            {
                x1 = p.x-1.0f;
                x2 = p.x+1.0f;
                y1 = p.y-1.0f;
                y2 = p.y+1.0f;
                z1 = p.z-1.0f;
                z2 = p.z+1.0f;
                first = false;
            }

            if (p.x < x1) x1=p.x;
            if (p.x > x2) x2=p.x;

            if (p.y < y1) y1=p.y;
            if (p.y > y2) y2=p.y;

            if (p.z < z1) z1=p.z;
            if (p.z > z2) z2=p.z;
        }

        if (first) continue;

        x1-=72.f;
        x2+=72.f;
        z1-=72.f;
        z2+=72.f;

        bound[o].y1 = y1;
        bound[o].y2 = y2;
        bound[o].cx = (x1+x2) / 2;
        bound[o].cy = (z1+z2) / 2;
        bound[o].a  = (x2-x1) / 2;
        bound[o].b  = (z2-z1) / 2;

    }

}
