
#include "Header.hpp"
#include <gl/gl.h>
#include <gl/glu.h>
#include <gl/glext.h>
#include <gl/WGLEXT.h>

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")


class vec2
{
public:
	float x,y;

	vec2(){}
	vec2( float a, float b ) : x(a), y(b) {}
};

class vec3
{
public:
	float x, y, z;

	vec3(){}
	vec3( float a, float b, float c ) : x(a), y(b), z(c) {}
	vec3( vec2 v ) : x(v.x), y(v.y), z(0) {}
	vec3( vec2 v, float c ) : x(v.x), y(v.y), z(c) {}
};


class plane
{
public:
	float A,B,C,D, mag;

	void Normalize()
	{
		mag = sqrtf( A*A + B*B + C*C );
		A /= mag;
		B /= mag;
		C /= mag;
		D /= mag;
	}

	float DistanceToPoint(float x,float y,float z)
	{
		return A*x + B*y + C*z + D;
	}
	float DistanceToPoint( vec3 v )
	{
		return A*v.x + B*v.y + C*v.z + D;
	}
};


class vert
{
public:
	float x,y,z;
	DWORD color;
	float s,t,texture;

	vert(){}
	vert( float a, float b, float c, DWORD d, float e, float f, float g ) : x(a), y(b), z(c), color(d), s(e), t(f), texture(g) {}
};

typedef struct {

} RObject;

int ObjectList[16384];
int ObjectCount = 0;


vert	TerrainVertArray[4096];
int		TerrainVertCount = 0;
const int TerBufSize = 4094;
HWND    hwndMain;
HDC     hdcMain;
HGLRC   hrcMain;
size_t	triCount = 0, chunkCount = 0;


vec2 GridArray[ (256) + (256) ];


PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = NULL;
PFNGLCREATEPROGRAMPROC	glCreateProgram = NULL;
PFNGLDELETEPROGRAMPROC	glDeleteProgram = NULL;
PFNGLLINKPROGRAMPROC	glLinkProgram = NULL;
PFNGLUSEPROGRAMPROC		glUseProgram = NULL;
PFNGLCREATESHADERPROC	glCreateShader = NULL;
PFNGLDELETESHADERPROC	glDeleteShader = NULL;
PFNGLATTACHSHADERPROC	glAttachShader = NULL;
PFNGLDETACHSHADERPROC	glDetachShader = NULL;
PFNGLSHADERSOURCEPROC	glShaderSource = NULL;
PFNGLCOMPILESHADERPROC	glCompileShader = NULL;


GLuint	TextureAtlas;
GLuint	ObjectTextures[256];


bool Activate3DHardware(HWND hwnd)
{
    PIXELFORMATDESCRIPTOR pfd;

    int iFormat;

    hwndMain = hwnd;

    /* get the device context (DC) */
    hdcMain = GetDC(hwnd);
    if (hdcMain == NULL) return false;

    /* set the pixel format for the DC */
    ZeroMemory(&pfd, sizeof(pfd));

    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW |
                  PFD_SUPPORT_OPENGL |
                  PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;    //Carnivores only uses 16-bit textures
    pfd.cDepthBits = 32;    //My Computer sucks
    pfd.iLayerType = PFD_MAIN_PLANE;

    iFormat = ChoosePixelFormat(hdcMain, &pfd);

    if (!SetPixelFormat(hdcMain, iFormat, &pfd)) return false;

    /* create and enable the render context (RC) */
    hrcMain = wglCreateContext(hdcMain);
    if (hrcMain == NULL) return false;

    wglMakeCurrent(hdcMain, hrcMain);
	
	// Get extensions
	wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress( "wglSwapIntervalEXT" );
	glCreateProgram = (PFNGLCREATEPROGRAMPROC)wglGetProcAddress( "glCreateProgram" );
	glDeleteProgram = (PFNGLDELETEPROGRAMPROC)wglGetProcAddress( "glDeleteProgram" );
	glLinkProgram = (PFNGLLINKPROGRAMPROC)wglGetProcAddress( "glLinkProgram" );
	glUseProgram = (PFNGLUSEPROGRAMPROC)wglGetProcAddress( "glUseProgram" );
	glCreateShader = (PFNGLCREATESHADERPROC)wglGetProcAddress( "glCreateShader" );
	glDeleteShader = (PFNGLDELETESHADERPROC)wglGetProcAddress( "glDeleteShader" );
	glAttachShader = (PFNGLATTACHSHADERPROC)wglGetProcAddress( "glAttachShader" );
	glDetachShader = (PFNGLDETACHSHADERPROC)wglGetProcAddress( "glDetachShader" );
	glShaderSource = (PFNGLSHADERSOURCEPROC)wglGetProcAddress( "glShaderSource" );
	glCompileShader = (PFNGLCOMPILESHADERPROC)wglGetProcAddress( "glCompileShader" );

	// Final initialization
	glEnable( GL_TEXTURE_2D );
	glEnable( GL_COLOR_MATERIAL );
	//wglSwapIntervalEXT( 1 );
	glGenTextures( 1, &TextureAtlas );

	for (int x=0; x<128; ++x)
	{
		GridArray[ (x*2)+0 ].x = x;
		GridArray[ (x*2)+0 ].y = 0.0f;
		GridArray[ (x*2)+1 ].x = x;
		GridArray[ (x*2)+1 ].y = 128.0f;
	}
	for (int y=0; y<128; ++y)
	{
		GridArray[ 256 + ((y*2)+0) ].x = 0.0f;
		GridArray[ 256 + ((y*2)+0) ].y = y;
		GridArray[ 256 + ((y*2)+1) ].x = 128.0f;
		GridArray[ 256 + ((y*2)+1) ].y = y;
	}


    return true;
}

bool Shutdown3DHardware (HWND hwnd)
{
	glDeleteTextures( 1, &TextureAtlas );

    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hrcMain);
    ReleaseDC(hwnd, hdcMain);
    return true;
}

size_t gpuGetPolyCount()
{
	return triCount;
}
size_t gpuGetChunkCount()
{
	return chunkCount;
}

void gpuUploadTextures( unsigned short* data, size_t size )
{
	glBindTexture( GL_TEXTURE_2D, TextureAtlas );
	glTexEnvi( GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_POINT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_POINT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	//glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE );
	//glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB5, 4096, 4096, 0, GL_BGR, GL_UNSIGNED_SHORT_1_5_5_5_REV, data );
	//glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB5, 128, 128, 0, GL_BGR, GL_UNSIGNED_SHORT_1_5_5_5_REV, data );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB5_A1, 4096, 4096, 0, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, data );
}


float projMat[16];
float mdlvMat[16];
float clipMat[16];
plane clipPlanes[6];


void gpuMatrixMultiply( float* dest, float *m1, float *m2 )
{
	dest[0 ]  = m1[0 ]*m2[0] + m1[1 ]*m2[4] + m1[2 ]*m2[8 ] + m1[3 ]*m2[12];
    dest[1 ]  = m1[0 ]*m2[1] + m1[1 ]*m2[5] + m1[2 ]*m2[9 ] + m1[3 ]*m2[13];
    dest[2 ]  = m1[0 ]*m2[2] + m1[1 ]*m2[6] + m1[2 ]*m2[10] + m1[3 ]*m2[14];
    dest[3 ]  = m1[0 ]*m2[3] + m1[1 ]*m2[7] + m1[2 ]*m2[11] + m1[3 ]*m2[15];

	dest[4 ]  = m1[4 ]*m2[0] + m1[5 ]*m2[4] + m1[6 ]*m2[8 ] + m1[7 ]*m2[12];
    dest[5 ]  = m1[4 ]*m2[1] + m1[5 ]*m2[5] + m1[6 ]*m2[9 ] + m1[7 ]*m2[13];
    dest[6 ]  = m1[4 ]*m2[2] + m1[5 ]*m2[6] + m1[6 ]*m2[10] + m1[7 ]*m2[14];
    dest[7 ]  = m1[4 ]*m2[3] + m1[5 ]*m2[7] + m1[6 ]*m2[11] + m1[7 ]*m2[15];

	dest[8 ]  = m1[8 ]*m2[0] + m1[9 ]*m2[4] + m1[10]*m2[8 ] + m1[11]*m2[12];
    dest[9 ]  = m1[8 ]*m2[1] + m1[9 ]*m2[5] + m1[10]*m2[9 ] + m1[11]*m2[13];
    dest[10]  = m1[8 ]*m2[2] + m1[9 ]*m2[6] + m1[10]*m2[10] + m1[11]*m2[14];
    dest[11]  = m1[8 ]*m2[3] + m1[9 ]*m2[7] + m1[10]*m2[11] + m1[11]*m2[15];

	dest[12]  = m1[12]*m2[0] + m1[13]*m2[4] + m1[14]*m2[8 ] + m1[15]*m2[12];
    dest[13]  = m1[12]*m2[1] + m1[13]*m2[5] + m1[14]*m2[9 ] + m1[15]*m2[13];
    dest[14]  = m1[12]*m2[2] + m1[13]*m2[6] + m1[14]*m2[10] + m1[15]*m2[14];
    dest[15]  = m1[12]*m2[3] + m1[13]*m2[7] + m1[14]*m2[11] + m1[15]*m2[15];
}


void gpuFrustumUpdate()
{
	glGetFloatv(GL_PROJECTION_MATRIX, projMat);
    glGetFloatv(GL_MODELVIEW_MATRIX, mdlvMat);

    // Multiply the modelview matrix by the projection matrix
    gpuMatrixMultiply(clipMat, mdlvMat, projMat);

    // Calculate the clip planes
    // -- Right side Plane
    clipPlanes[0].A = clipMat[ 3] - clipMat[ 0];
    clipPlanes[0].B = clipMat[ 7] - clipMat[ 4];
    clipPlanes[0].C = clipMat[11] - clipMat[ 8];
    clipPlanes[0].D = clipMat[15] - clipMat[12];
    clipPlanes[0].Normalize();
    // -- Left side Plane
    clipPlanes[1].A = clipMat[ 3] + clipMat[ 0];
    clipPlanes[1].B = clipMat[ 7] + clipMat[ 4];
    clipPlanes[1].C = clipMat[11] + clipMat[ 8];
    clipPlanes[1].D = clipMat[15] + clipMat[12];
    clipPlanes[1].Normalize();
    // -- Bottom side Plane
    clipPlanes[2].A = clipMat[ 3] + clipMat[ 1];
    clipPlanes[2].B = clipMat[ 7] + clipMat[ 5];
    clipPlanes[2].C = clipMat[11] + clipMat[ 9];
    clipPlanes[2].D = clipMat[15] + clipMat[13];
    clipPlanes[2].Normalize();
    // -- Top side Plane
    clipPlanes[3].A = clipMat[ 3] - clipMat[ 1];
    clipPlanes[3].B = clipMat[ 7] - clipMat[ 5];
    clipPlanes[3].C = clipMat[11] - clipMat[ 9];
    clipPlanes[3].D = clipMat[15] - clipMat[13];
    clipPlanes[3].Normalize();
    // -- Far side Plane
    clipPlanes[4].A = clipMat[ 3] - clipMat[ 2];
    clipPlanes[4].B = clipMat[ 7] - clipMat[ 6];
    clipPlanes[4].C = clipMat[11] - clipMat[10];
    clipPlanes[4].D = clipMat[15] - clipMat[14];
    clipPlanes[4].Normalize();
    // -- Near side Plane
    clipPlanes[5].A = clipMat[ 3] + clipMat[ 2];
    clipPlanes[5].B = clipMat[ 7] + clipMat[ 6];
    clipPlanes[5].C = clipMat[11] + clipMat[10];
    clipPlanes[5].D = clipMat[15] + clipMat[14];
    clipPlanes[5].Normalize();
}


float gpuFrustumCheckSphere( vec3 pos, float radius )
{
    /*
    * Check if the sphere is within the scene Frustum
    * return: distance to the NEAR clip plane of the nearest point of the sphere's radius
    */
   float d;

   for(int p = 0; p < 6; p++)
   {
      d = clipPlanes[p].DistanceToPoint( pos.x, pos.y, pos.z );
      if( d <= -radius ) return 0.0f;
   }

   return d + radius;
}


float gpuFrustumCheckBox( vec3 pos, vec3 boxMin, vec3 boxMax )
{
    float d = 0.0f;
    for (int p = 0; p < 6; p++)
    {
        if( (d = clipPlanes[p].DistanceToPoint( pos.x+boxMin.x, pos.y+boxMin.y, pos.z+boxMin.z)) > 0 ) continue;
        if( (d = clipPlanes[p].DistanceToPoint( pos.x+boxMax.x, pos.y+boxMin.y, pos.z+boxMin.z)) > 0 ) continue;
        if( (d = clipPlanes[p].DistanceToPoint( pos.x+boxMin.x, pos.y+boxMax.y, pos.z+boxMin.z)) > 0 ) continue;
        if( (d = clipPlanes[p].DistanceToPoint( pos.x+boxMax.x, pos.y+boxMax.y, pos.z+boxMin.z)) > 0 ) continue;
        if( (d = clipPlanes[p].DistanceToPoint( pos.x+boxMin.x, pos.y+boxMin.y, pos.z+boxMax.z)) > 0 ) continue;
        if( (d = clipPlanes[p].DistanceToPoint( pos.x+boxMax.x, pos.y+boxMin.y, pos.z+boxMax.z)) > 0 ) continue;
        if( (d = clipPlanes[p].DistanceToPoint( pos.x+boxMin.x, pos.y+boxMax.y, pos.z+boxMax.z)) > 0 ) continue;
        if( (d = clipPlanes[p].DistanceToPoint( pos.x+boxMax.x, pos.y+boxMax.y, pos.z+boxMax.z)) > 0 ) continue;

        return 0.0f;
    }
    return d;
}


void Clear3DBuffers(EZdword ClrColor, EZfloat ClrDepth)
{
	float R = float(Resources.DayBG_R) / 255.0f;
    float G = float(Resources.DayBG_G) / 255.0f;
    float B = float(Resources.DayBG_B) / 255.0f;
    float A = 1.0f;
    glClearColor(R,G,B,A);
    glClearDepth(ClrDepth);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

	triCount = 0;
	chunkCount = 0;
}


void Scene3DBegin(const float *From, const float *To, float WinW, float WinH)
{
	glEnable( GL_TEXTURE_2D );
	glEnable( GL_CULL_FACE );
    //glEnable(GL_BLEND);
    //glEnable(GL_ALPHA_TEST);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    //glAlphaFunc(GL_GREATER,0.8f);
    //glBlendFunc(GL_SRC_COLOR, GL_SRC_COLOR);

    glViewport(0,0,int(WinW),int(WinH));

	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0f,int(WinW),int(WinH),0.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	/*glBindTexture( GL_TEXTURE_2D, TextureAtlas );
	glBegin( GL_QUADS );
		glColor4f(1,1,1,1);
		glTexCoord2f(  1,  0); glVertex2f(512,0);
		glTexCoord2f(  0,  0); glVertex2f(0,0);
		glTexCoord2f(  0,  1); glVertex2f(0,512);
		glTexCoord2f(  1,  1); glVertex2f(512,512);
	glEnd();
	glBindTexture( GL_TEXTURE_2D, 0 );*/

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, WinW/WinH, 1.0f, 10000.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(From[0],From[1],From[2], From[0]+To[0],From[1]+To[1],From[2]+To[2], 0,0,1);


	// Update the view frustum
	gpuFrustumUpdate();
}

void Scene3DEnd()
{
    glDisable( GL_ALPHA_TEST );
    glDisable( GL_DEPTH_TEST );
    glDisable( GL_BLEND );
    glDisable( GL_TEXTURE_2D );
	glDisable( GL_CULL_FACE );

    SwapBuffers(hdcMain);
}

void Scene2DBegin(float WinW, float WinH)
{
    glViewport(0,0,int(WinW),int(WinH));

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0f,int(WinW),int(WinH),0.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
}

void Scene2DEnd()
{
    glDisable(GL_DEPTH_TEST);

    SwapBuffers(hdcMain);
}


void Draw3DArrays(unsigned int Count, float* VtxPtr, unsigned long* ClrPtr, float* TexPtr)
{
    glBegin(GL_TRIANGLES);
    for (int v=0; v<Count*3; v++)
    {
        glColor4ubv((EZbyte*)&ClrPtr[v]);
        glTexCoord2fv(&TexPtr[v*2]);
        glVertex3fv(&VtxPtr[v*3]);
    }
    glEnd();
}

// -- Specicially Carnivores Orientated Code:
void Draw3DTerrainBuffer()
{
	if ( TerrainVertCount == 0 ) return;
	chunkCount++;

	glColor4f( 1,1,1,1 );

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(3, GL_FLOAT, sizeof(vert), &TerrainVertArray[0].x);
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(vert), &TerrainVertArray[0].color);
    glTexCoordPointer(2, GL_FLOAT, sizeof(vert), &TerrainVertArray[0].s);

    glDrawArrays(GL_QUADS, 0, TerrainVertCount);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	TerrainVertCount = 0;
}

#define MakeByte24( b ) ( (int)( (b) | (b<<8) | (b<<16) ) )
vec2 v[4];


void ProcessLeaf(int x, int y, int w)
{
	float at_w = 128.0f / 4096.0f;
	float pix = 1.0f / 4096.0f;

	if ( w == 1 )
	{
		//if ( gpuFrustumCheckSphere( vec3(x+0.5f,y+0.5f,Map.HMap[x][y]/4.0f), 0.86602f ) == 0 ) return;
		if ( x < 0 || y < 0 ) return;
		if ( x+w >= 1024 || y+w >= 1024 ) return;

		DWORD c1 = MakeByte24(Map.LMap2[x+0][y+0]) | (0xFF<<24);
		DWORD c2 = MakeByte24(Map.LMap2[x+w][y+0]) | (0xFF<<24);
		DWORD c3 = MakeByte24(Map.LMap2[x+w][y+w]) | (0xFF<<24);
		DWORD c4 = MakeByte24(Map.LMap2[x+0][y+w]) | (0xFF<<24);

		int t1 = Map.TMap1[x][y];
		int v_x = t1 % 32;
		int v_y = t1 / 32;
		float xmin = v_x * at_w + pix;
		float xmax = ( v_x * at_w ) + at_w - pix;
		float ymin = v_y * at_w + pix;
		float ymax = ( v_y * at_w ) + at_w - pix;

		int TDirection = Map.FMap[x][y] & 3;
		BOOL ReverseOn = Map.FMap[x][y] & fmReverse;

		if (ReverseOn)
		{
			switch (TDirection)
			{
			case 0://done
				v[0].x = xmin;		v[0].y = ymin;
				v[1].x = xmin;		v[1].y = ymax;
				v[2].x = xmax;		v[2].y = ymax;
				v[3].x = xmax;		v[3].y = ymin;
			break;
			case 1://done
				v[0].x = xmin;		v[0].y = ymax;
				v[1].x = xmax;		v[1].y = ymax;
				v[2].x = xmax;		v[2].y = ymin;
				v[3].x = xmin;		v[3].y = ymin;
			break;
			case 2://done
				v[0].x = xmax;		v[0].y = ymax;
				v[1].x = xmax;		v[1].y = ymin;
				v[2].x = xmin;		v[2].y = ymin;
				v[3].x = xmin;		v[3].y = ymax;
			break;
			case 3://done
				v[0].x = xmax;		v[0].y = ymin;
				v[1].x = xmin;		v[1].y = ymin;
				v[2].x = xmin;		v[2].y = ymax;
				v[3].x = xmax;		v[3].y = ymax;
			break;
			default:
				v[0].x = xmin;		v[0].y = ymin;
				v[1].x = xmin;		v[1].y = ymin;
				v[2].x = xmin;		v[2].y = ymin;
				v[3].x = xmin;		v[3].y = ymin;
			break;
			}
		}
		else
		{
			switch (TDirection)
			{
			case 0://done
				v[0].x = xmin;		v[0].y = ymin;
				v[1].x = xmin;		v[1].y = ymax;
				v[2].x = xmax;		v[2].y = ymax;
				v[3].x = xmax;		v[3].y = ymin;
			break;
			case 1://done
				v[0].x = xmin;		v[0].y = ymax;
				v[1].x = xmax;		v[1].y = ymax;
				v[2].x = xmax;		v[2].y = ymin;
				v[3].x = xmin;		v[3].y = ymin;
			break;
			case 2://done
				v[3].x = xmin;		v[3].y = ymax;
				v[0].x = xmax;		v[0].y = ymax;
				v[1].x = xmax;		v[1].y = ymin;
				v[2].x = xmin;		v[2].y = ymin;
			break;
			case 3://done
				v[1].x = xmin;		v[1].y = ymin;
				v[2].x = xmin;		v[2].y = ymax;
				v[3].x = xmax;		v[3].y = ymax;
				v[0].x = xmax;		v[0].y = ymin;
			break;
			default:
				v[0].x = xmin;		v[0].y = ymin;
				v[1].x = xmin;		v[1].y = ymin;
				v[2].x = xmin;		v[2].y = ymin;
				v[3].x = xmin;		v[3].y = ymin;
			break;
			}
		}

		if ( ReverseOn )
		{
			TerrainVertArray[ TerrainVertCount+0 ] = vert( x,	y,		Map.HMap[x][y] / 4.0f,		c1, v[0].x,v[0].y,0 );
			TerrainVertArray[ TerrainVertCount+1 ] = vert( x+w,	y,		Map.HMap[x+1][y] / 4.0f,	c2, v[1].x,v[1].y,0 );
			TerrainVertArray[ TerrainVertCount+2 ] = vert( x+w,	y+w,	Map.HMap[x+1][y+1] / 4.0f,	c3, v[2].x,v[2].y,0 );
			TerrainVertArray[ TerrainVertCount+3 ] = vert( x,	y+w,	Map.HMap[x][y+1] / 4.0f,	c4, v[3].x,v[3].y,0 );
			TerrainVertCount+=4;
			triCount+=1;
		}
		else
		{
			TerrainVertArray[ TerrainVertCount+0 ] = vert( x,	y+w,	Map.HMap[x][y+1] / 4.0f,	c4, v[3].x,v[3].y,0 );
			TerrainVertArray[ TerrainVertCount+1 ] = vert( x,	y,		Map.HMap[x][y] / 4.0f,		c1, v[0].x,v[0].y,0 );
			TerrainVertArray[ TerrainVertCount+2 ] = vert( x+w,	y,		Map.HMap[x+1][y] / 4.0f,	c2, v[1].x,v[1].y,0 );
			TerrainVertArray[ TerrainVertCount+3 ] = vert( x+w,	y+w,	Map.HMap[x+1][y+1] / 4.0f,	c3, v[2].x,v[2].y,0 );
			TerrainVertCount+=4;
			triCount+=1;
		}

		if ( (Map.FMap[x+0][y+0] & fmWaterA) &&
			 (Map.FMap[x+1][y+0] & fmWaterA) &&
			 (Map.FMap[x+1][y+1] & fmWaterA) &&
			 (Map.FMap[x+0][y+1] & fmWaterA) )
		{
			int water = Map.WMap[x][y];
			v_x = Resources.Waters[ water ].Texture % 32;
			v_y = Resources.Waters[ water ].Texture / 32;
			xmin = v_x * at_w + pix;
			ymin = v_y * at_w + pix;
			xmax = ( v_x * at_w ) + at_w - pix;
			ymax = ( v_y * at_w ) + at_w - pix;
			DWORD water_h = Resources.Waters[ water ].Height / 4.0f;

			TerrainVertArray[ TerrainVertCount+0 ] = vert( x,	y,		water_h,	0xFFFFFFFF, xmax,ymin,0 );
			TerrainVertArray[ TerrainVertCount+1 ] = vert( x+w,	y,		water_h,	0xFFFFFFFF, xmin,ymin,0 );
			TerrainVertArray[ TerrainVertCount+2 ] = vert( x+w,	y+w,	water_h,	0xFFFFFFFF, xmin,ymax,0 );
			TerrainVertArray[ TerrainVertCount+3 ] = vert( x,	y+w,	water_h,	0xFFFFFFFF, xmax,ymax,0 );
			TerrainVertCount+=4;
			triCount+=1;
		}

		if ( TerrainVertCount >= TerBufSize ) Draw3DTerrainBuffer();
	}
	else
	{
		float h = w / 2.0f;
		float r = h + ( h / 2.0f );

		//if ( gpuFrustumCheckSphere( vec3( x+h, y+h, Map.HMap[x][y]/4.0f ), r ) == 0 ) return;

		if ( gpuFrustumCheckBox( vec3( x, y, 0 ), vec3(0,0,0), vec3(w, w, 255.f/4.0f) ) == 0 ) return;

		int l = x + ( w/2 );
		int m = y + ( w/2 );
		//if ( gpuFrustumCheckSphere( vec3( x+h, y+h, Map.HMap[x][y]/4.0f ), r ) )
		ProcessLeaf( x, y, w / 2 );
		//if ( gpuFrustumCheckSphere( vec3( l+h, y+h, Map.HMap[x][y]/4.0f ), r ) )
		ProcessLeaf( l, y, w / 2 );
		//if ( gpuFrustumCheckSphere( vec3( x+h, y+h, Map.HMap[x][y]/4.0f ), r ) )
		ProcessLeaf( l, m, w / 2 );
		//if ( gpuFrustumCheckSphere( vec3( x+h, y+h, Map.HMap[x][y]/4.0f ), r ) )
		ProcessLeaf( x, m, w / 2 );
	}
}

void RenderObject( int id )
{
	TOBJECT *obj = &Resources.Objects[id];

	int f = 0;
}

void Process3DTerrain(float x,float y,float z)
{
	// -- Use QuadTree to recurse through terrain, discard any that are 128 away from the camera
	glBindTexture( GL_TEXTURE_2D, TextureAtlas );

    ProcessLeaf( (int)floor(x - 128), (int)floor(y - 128), 256 );
	Draw3DTerrainBuffer();

	if ( ObjectCount )
	{
		for (int i=0; i<ObjectCount; ++i )
		RenderObject( ObjectList[i] );
	}
	ObjectCount = 0;

	glBindTexture( GL_TEXTURE_2D, 0 );
}

void Draw3DGrid( void )
{
	return;
	glColor4f( 0,0,0, 1.0f );
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, GridArray);
    glDrawArrays(GL_LINES, 0, 256);
    glDisableClientState(GL_VERTEX_ARRAY);
}