#ifndef C3DIT_H
#define C3DIT_H

#include <windows.h>
#include <WindowsX.h>
#include <commctrl.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <gl/glext.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <vector>
#include "vec3.h"

#include "version.h"
#include "resource.h"


typedef unsigned char   byte;

#define MAX_VERTICES		2048
#define MAX_TRIANGLES		2048
#define pi					3.141592653f

// Face Flags
enum EFaceFlags {
	sfDoubleSide		= 1,
	sfDarkBack			= 2,
	sfOpacity			= 4,
	sfTransparent		= 8,
	sfMortal			= 16,
	sfPhong				= 32,
	sfEnvMap			= 64,
	sfNeedVC			= 128,
	sfUnused1			= 256,
	sfUnused2			= 512,
	sfUnused3			= 1024,
	sfUnused4			= 2048,
	sfUnused5			= 4096,
	sfUnused6			= 8192,
	sfUNused7			= 16384,
	sfDark				= 32768,
	sfAll				= 0xFFFF, // 65535
};

// Object Flags
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

enum EView
{
    VIEW_PERSPECTIVE = 0,
	VIEW_FIRSTPERSON,
    VIEW_LEFT,
    VIEW_RIGHT,
    VIEW_FRONT,
    VIEW_BACK,
    VIEW_TOP,
    VIEW_BOTTOM
};

//==Type definitions==//
typedef struct VERTEX {
    //--Vertex
    float x,y,z;
    short bone;
	short hide;
} VERTEX;

class NORM
{
public:
    //--Normals
	union {
		struct { float x,y,z; };
		float n[3];
	};

    inline const NORM& operator+= (const NORM& norm)
    {
        this->x += norm.x;
		this->y += norm.y;
		this->z += norm.z;
        return *this;
    }
    inline const NORM& operator/= (float scalar)
    {
        for (int i=0; i<3; i++)
        n[i] /= scalar;
        return *this;
    }
};

typedef struct _Tri {
    //--Triangle
    int v1,v2,v3;
    int tx1,tx2,tx3,ty1,ty2,ty3;
    int flags,u1;
    int parent;
    // Triangle data? (from 3D Designer)
    int u2,u3,u4,u5;
} TRIANGLE;

typedef struct _Bone {
    char    name[32];
    float   x;
    float   y;
    float   z;
    short   parent;
    short   hide;
} BONE;

class TVtl
{
    public:

    //--Animation
    char    Name[32];
    int     KPS;
    int     FrameCount;
    short*  Data;

    TVtl(){ Data = 0; }
    ~TVtl(){ if (Data) delete [] Data; }
};

class SOUND
{
    public:
    //--Sound
    char name[32];
    int len;
    BYTE *data;

    SOUND() { data = 0; }
    ~SOUND() { if ( data ) delete [] data; }
};

typedef struct _rgb {
    byte r,g,b;
} RGB;

typedef struct _OBJ
{ // OBJECT Information
    int  Radius;
    int  YLo, YHi;
    int  linelenght, lintensity;
    int  circlerad, cintensity;
    int  flags;
    int  GrRad;
    int  DefLight;
    int  LastAniTime;
    float BoundR;
    BYTE res[16];
} OBJ;

class TModel
{
public:
	OBJ		mObjInfo;

	// -- Header
	char	mName[32];
	char	mTag[8];
	
	long	mAnimCount;
	long	mSoundCount;
	long	mVertCount;
	long	mFaceCount;
	long	mBoneCount;
	
	long	mTextureSize;
	long	mTextureWidth;
	long	mTextureHeight;
	long	mTextureColor;

	// -- Data
	void*	mVertices;
	void*	mFaces;
	void*	mBones;
	void*	mTexture;
	void*	mSprite;

	int		mAnimSFXTable[64];

	TModel()
	{
		mTexture = 0;
		mSprite = 0;
	}

	~TModel()
	{
		delete [] mTexture;
		mTexture = 0;
	}

	void FindTextureDimensions()
	{
		if ( mTextureSize <= 256*256*2 )
		{
			mTextureWidth = 256;
			mTextureHeight = (mTextureSize/2)/256;
			mTextureColor = 16;
		}
		else if ( mTextureSize == 256*256*2 )
		{
			mTextureWidth = 256;
			mTextureHeight = 256;
			mTextureColor = 16;
		}
		else if ( mTextureSize == 256*256*3 )
		{
			mTextureWidth = 256;
			mTextureHeight = 256;
			mTextureColor = 24;
		}
		else if ( mTextureSize == 256*256*4 )
		{
			mTextureWidth = 256;
			mTextureHeight = 256;
			mTextureColor = 32;
		}
		else if ( mTextureSize == 512*512*2 )
		{
			mTextureWidth = 512;
			mTextureHeight = 512;
			mTextureColor = 16;
		}
		else if ( mTextureSize == 512*512*3 )
		{
			mTextureWidth = 512;
			mTextureHeight = 512;
			mTextureColor = 24;
		}
		else if ( mTextureSize == 512*512*4 )
		{
			mTextureWidth = 512;
			mTextureHeight = 512;
			mTextureColor = 32;
		}
		else if ( mTextureSize == 1024*1024*2 )
		{
			mTextureWidth = 1024;
			mTextureHeight = 1024;
			mTextureColor = 16;
		}
		else if ( mTextureSize == 1024*1024*3 )
		{
			mTextureWidth = 1024;
			mTextureHeight = 1024;
			mTextureColor = 24;
		}
		else if ( mTextureSize == 1024*1024*2 )
		{
			mTextureWidth = 1024;
			mTextureHeight = 1024;
			mTextureColor = 32;
		}
	}

	void ApplyTextureOpacity()
	{
		if ( mTexture == 0 ) return;

		if ( mTextureColor == 16 )
		for (int i=0; i<mTextureWidth * mTextureHeight; i++)
		{
			if ( *( (WORD*) mTexture + i ) == 0 || *( (WORD*) mTexture + i ) == 0x8000 )
				*( (WORD*) mTexture + i ) = 0;
			else
				*( (WORD*) mTexture + i ) |= 0x8000;
		}
		if ( mTextureColor == 24 )
		for (int i=0; i<mTextureWidth * mTextureHeight; i++)
		{
			/*if ( *( (LONG*) mTexture + i ) == 0 || *( (LONG*) mTexture + i ) == (255<<24) )
				*( (LONG*) mTexture + i ) = 0;
			else
				*( (LONG*) mTexture + i ) |= (255<<24);*/
		}
	}

};

typedef struct _tagCamera {
    float x ,y ,z;
    float xt,yt,zt;
    float yaw,pitch;
    float dist;
} CAMERA;


//==Variables==//
extern std::vector<int> g_TriSelection;
extern CAMERA       cam,ortho;
extern TRIANGLE     g_Triangles[MAX_TRIANGLES];
extern VERTEX       g_Verticies[MAX_VERTICES];
extern NORM         g_Normals[MAX_VERTICES],
					g_VNormals[MAX_VERTICES];
extern BONE         g_Bones[32];
extern TVtl         g_Animations[64];
extern SOUND        g_Sounds[64];
extern GLuint       g_TextureID[5];
extern TModel		Model;
extern WORD         g_Sprite[128*128];
extern HWND         g_hMain,gHTool,gHStatus,g_hAbout,g_hTools;
extern bool         key[256],
					mouse[3],
                    ISCAR,
					ISC2O,
					COLOR,
					ANIMPLAY;
extern char         fileName[260];
extern int          CUR_FRAME,CUR_ANIM;
extern int     Ani,Snd;
extern long         LastTick,RealTime,PrevTime,TimeDt,FTime;
extern int          FPS,Frames;
extern int          WinX,WinY,WinW,WinH,CurX,CurY;
extern GLbyte       WAVhead[];
extern EView		CameraView;

extern HWND    g_hAniDlg,g_hSndDlg,g_hCarDlg, g_DrawArea,g_FileView,g_AniTrack, g_AniDlgTrack, embedTest;
//==Functions==//
LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);

void    EnableOpenGL(HWND hwnd, HDC*, HGLRC*);
void    DisableOpenGL(HWND, HDC, HGLRC);
void    RenderMesh();
void    DrawTextGL(float, float, float, char*, unsigned int);
void    tga16_to_bgr24(WORD *A, byte *bgr, int L);
float   lengthdir_x(float len,float yaw,float pitch);
float   lengthdir_y(float len,float yaw,float pitch);
float   lengthdir_z(float len, float pitch);
float   degtorad(float deg);
//bool    rayIntersectsTriangle( EZvector &Origin, EZvector &Direction, EZvector &v1, EZvector &v2, EZvector &v3);
bool	rayIntersectsPoint(vec3 &Origin, vec3 &Direction, const vec3 &Point);
void	PlayWave( SOUND* );
void    importModel();
void    exportSound();
void    importSound();
void    SaveProject();
NORM    ComputeNormals(VERTEX vt1, VERTEX vt2, VERTEX vt3);
BOOL    OpenFileDlg(const char *filter, const char *ext);
BOOL    SaveFileDlg(const char *filter, const char *ext);
BOOL    SaveFileDlg(char*,char*,char*);

#endif // __HEADER__
