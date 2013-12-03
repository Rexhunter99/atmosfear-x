/************************************************************************
* OpenGL Video Driver
* Version:	1.0
* By:		James Ogden
*/

#pragma once

#include <stdint.h>
#include <cstdio>
#include <vector>
#include <string>

typedef uint32_t	texture_t;
class TPicture;


class VideoGL
{
public:
	VideoGL();
	~VideoGL();

	bool init();

	void shutdown();

	void drawBackground( TPicture &p_pic );
	void drawPicture( TPicture &p_pic, int p_x = 0, int p_y = 0, float p_scale = 1.0f );

	texture_t	loadTexture( std::string p_file );
	texture_t	loadTexture( void* p_data, uint32_t p_width = 256, uint32_t p_height = 256, uint32_t p_bits = 16 );

};
