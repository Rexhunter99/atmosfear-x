#ifndef _HEADER_HPP_
#define _HEADER_HPP_

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <windows.h>
#include <string>
#include "Resource.h"
#include "math.h"

using namespace std;


// == DEFINITIONS == //
#define MAX_TEXTURES 1024
#define AF1_MAPSIZE 0x00
#define AF2_MAPSIZE 0x00D7FFFF


enum {
	fmReverse	= 0x0010,
	fmNOWAY		= 0x0020,

	fmWater		= 0x0080,
	
	fmWater2	= 0x8000,
	fmWaterA	= 0x8080
};

enum {
	sfDoubleSide	= 1,
	sfDarkBack		= 2,
	sfOpacity		= 4,
	sfTransparent	= 8,
	sfMortal		= 0x0010,
	sfPhong			= 0x0030,
	sfEnvMap		= 0x0050,

	sfNeedVC		= 0x0080,
	sfDark			= 0x8000
};

#define ofPLACEWATER       1
#define ofPLACEGROUND      2
#define ofPLACEUSER        4
#define ofCIRCLE           8
#define ofBOUND            16
#define ofNOBMP            32
#define ofNOLIGHT          64
#define ofDEFLIGHT         128
#define ofGRNDLIGHT        256
#define ofNOSOFT           512
#define ofNOSOFT2          1024
#define ofANIMATED         0x80000000

#define csONWATER          0x00010000


// == TYPES == //
#ifndef __EZ_TYPES__
#define __EZ_TYPES__
typedef unsigned int    EZenum;
typedef bool            EZbool;
typedef unsigned int    EZsize_t;

typedef signed char     EZchar;
typedef signed short    EZshort;
typedef signed int      EZint;
typedef signed long     EZlong;
typedef unsigned char   EZbyte;
typedef unsigned short  EZword;
typedef unsigned int    EZuint;
typedef unsigned long   EZdword;
typedef float           EZfloat;
typedef double          EZdouble;
#endif


// == ENUMERATIONS == //
enum ETOOL{
    TOOL_NONE = 0,
    // -- Terrain tools
    TOOL_TRAISE = 1,
    TOOL_TLOWER = 2,
    TOOL_TLEVEL = 3,
    TOOL_TTEXTURE = 4,
    // -- Object tools
    TOOL_OPLACE,
};


// == STRUCTURES == //
typedef struct {
    EZword  Data[256*256];
} TTEXTURE;

typedef struct {
    EZword  Data[128*128];
} TSPRITE;

typedef struct {
    EZdword V1,V2,V3;
    EZdword TX1,TX2,TX3;
    EZdword TY1,TY2,TY3;
    EZdword Flags;
    EZdword U1;
    EZdword Parent;
    EZdword U2,U3,U4, U5;
} TTRIANGLE;

typedef struct {
    EZfloat x,y,z;
    EZword  Bone,
            Hide;
} TVERTEX;

typedef struct {
    EZchar  Name[32];
    EZfloat x,y,z;
    EZshort Parent;
    EZword  Hide;
} TBONE;

typedef struct {
    EZshort x,y,z;
} TSVERTEX;

typedef struct {
    TSVERTEX Vertices[1024];
} TFRAME;

typedef struct {
    EZchar Name[32];
    EZdword KPS;
    EZdword NumFrames;
    TFRAME  Frames[32];
} TANIMATION;

class TOBJECT
{
public:
    // -> Header Block
	struct {
		EZdword Radius;
		EZdword YLo,YHi;
		EZdword LineLength,LIntensity;
		EZdword CircleRadius,CIntensity;
		EZdword Flags;
		EZdword GrRad;
		EZdword DefaultLight;
		EZdword LastAniTime;
		EZfloat BoundRadius;
		EZbyte  Reserved[16];
	} Header;
    EZdword NumVerts;
    EZdword NumFaces;
    EZdword NumBones;
    EZdword TextureLength;

    // -> Triangle Block
    TTRIANGLE Triangles[1024];

    // -> Vertice Block
    TVERTEX Vertices[1024];

    // -> Bone Block
    TBONE Bones[64];

    // -> Texture Block
    TTEXTURE Texture;

    // -> Sprite Block
    TSPRITE Sprite;

    // -> Animation Block
    TANIMATION Animation;
};

typedef struct {
    EZdword Color;
    EZfloat MinAltitude;
    EZdword Flags;
    EZfloat Opacity;
    EZfloat MaxAltitude;
} TFOG;

class TSOUND {
    public:
    EZdword DataLength;
    EZbyte  *Data;

    TSOUND()
    {
        Data = NULL;
        DataLength = 0;
    }
    ~TSOUND()
    {
        Free();
    }
	inline void Allocate( size_t size )
	{
		Data = new EZbyte[size];
	}
    inline void Free()
    {
        if (Data) delete [] Data;
        Data = NULL;
        DataLength = 0;
    }
};

typedef struct {
	EZdword Index;
    EZdword Volume;
    EZdword Frequency;
    EZword  Environment;
    EZword  Flags;
} TRD;

typedef struct {
    TSOUND	Sound;
    TRD		Random[16];
	EZdword	NumRandomSFX;
	EZdword	Volume;
} TAMBIENT;

typedef struct {
    EZdword Texture,
            Height;
    EZfloat Opacity;
    EZdword FogRGB;
} TWATER;

class TMAP
{
public:
    EZbyte  HMap[1024][1024];   //Heightmap
    EZword  TMap1[1024][1024];  //Texture Indices
    EZword  TMap2[1024][1024];  //Texture Indices LoD
    EZbyte  OMap[1024][1024];   //Object Indices
    EZword  FMap[1024][1024];   //Flags
    EZbyte  LMap1[1024][1024];  //Dawn Lightmap
    EZbyte  LMap2[1024][1024];  //Day Lightmap
    EZbyte  LMap3[1024][1024];  //Night Lightmap
    EZbyte  WMap[1024][1024];   //Water Indices
    EZbyte  OHMap[1024][1024];  //Object Heights
    EZbyte  MMap[512][512];     //Fog/Mist Indices
    EZbyte  SMap[512][512];     //Ambient Sound Indices

	TMAP()
	{
		memset( HMap, 0, 1024*1024 );
		memset( TMap1, 0, 1024*1024*2 );
		memset( TMap2, 0, 1024*1024*2 );
		memset( OMap, 0, 1024*1024 );
		memset( FMap, 0, 1024*1024*2 );
		memset( LMap1, 0, 1024*1024 );
		memset( LMap2, 0, 1024*1024 );
		memset( LMap3, 0, 1024*1024 );
		memset( WMap, 0, 1024*1024 );
		memset( OHMap, 0, 1024*1024 );
		memset( MMap, 0, 512*512 );
		memset( SMap, 0, 512*512 );
	}
};

class TRESOURCE
{
public:
    EZdword     NumTextures;
    EZdword     NumObjects;
    EZbyte      DawnBG_R,DawnBG_G,DawnBG_B;
    EZbyte      DayBG_R,DayBG_G,DayBG_B;
    EZbyte      NightBG_R,NightBG_G,NightBG_B;
    EZbyte      DawnFade_R,DawnFade_G,DawnFade_B;
    EZbyte      DayFade_R,DayFade_G,DayFade_B;
    EZbyte      NightFade_R,NightFade_G,NightFade_B;

    // -> Texture Block
    TSPRITE Textures[ MAX_TEXTURES ];
	EZword	*TextureStrip;

    // -> Object Block
    TOBJECT     Objects[252];

    // -> Sky Block
    TTEXTURE    DawnSky;
    TTEXTURE    DaySky;
    TTEXTURE    NightSky;
    TSPRITE Clouds;

    // -> Fog Block
    EZdword     NumFogs;
    TFOG        Fogs[256];

    // -> Random Ambient Block
	EZdword		NumRandomSounds;
    TSOUND		RandomSounds[256];

    // -> Sound Block
	EZdword		NumAmbientSounds;
    TAMBIENT	AmbientSounds[256];

    // -> Water Block
    EZdword		NumWaters;
    TWATER		Waters[256];

	TRESOURCE() {
		TextureStrip = 0;
	}
	~TRESOURCE() {
		if ( TextureStrip ) delete [] TextureStrip;
	}
};


class TPROJECT
{
public:
	string Name;
	
	string TextureNames[1024];
	string ObjectNames[256];
	string RandSoundNames[32];
	string AmbientSoundNames[256];
	string FogNames[256];
	string WaterNames[256];

	// -- Texture HBITMAPS for list view
	HBITMAP TextureBMP[1024];
	HBITMAP ObjectBMP[256];

	TPROJECT()
	{
		memset( TextureBMP, 0, sizeof(HBITMAP) * 1024 );
		memset( ObjectBMP, 0, sizeof(HBITMAP) * 256 );
	}
	~TPROJECT()
	{
		for (int i=0; i<1024; i++)
		{
			DeleteObject( TextureBMP[i] );
			TextureBMP[i] = NULL;
		}
	}
};


// == VARIABLES == //
extern TMAP  Map;
extern TRESOURCE Resources;
extern TPROJECT Project;


// == FUNCTIONS == //
bool Activate3DHardware(HWND hwnd);
bool Shutdown3DHardware(HWND hwnd);

size_t gpuGetPolyCount();
size_t gpuGetChunkCount();
void gpuUploadTextures( unsigned short* data, size_t size );
void Clear3DBuffers(EZdword ClrColor, EZfloat ClrDepth);
void Scene3DBegin(const float *From, const float *To, float WinW, float WinH);
void Scene3DEnd();
void Scene2DBegin(float WinW, float WinH);
void Scene2DEnd();
void Draw3DGrid( void );

void Draw3DTerrainIndexed(unsigned int Count, float* VtxPtr, unsigned long* ClrPtr, float* TexPtr);
void Draw3DArrays(unsigned int Count, float* VtxPtr, unsigned long* ClrPtr, float* TexPtr);

void Process2DTerrain(unsigned int Zoom, int VX, int VY);
void Process3DTerrain(float x,float y,float z);


#endif
