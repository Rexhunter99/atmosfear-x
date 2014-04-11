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
typedef uint32_t	font_t;
class TPicture;
class i_image;
class i_font;


enum fontstyle_e
{
	FONTSTYLE_NONE		= 0,
	FONTSTYLE_BOLD		= 1,
	FONTSTYLE_ITALIC	= 2,
	FONTSTYLE_UNDERLINE	= 4,
	FONTSTYLE_STRIKE	= 8
};


class VideoGL
{
public:
	VideoGL();
	~VideoGL();

	bool init();
	void shutdown();

	// -- 2D Drawing functions
	void drawBackground( i_image p_pic );
	void drawPicture( i_image p_pic, int p_x = 0, int p_y = 0, float p_scale = 1.0f );
	void drawCircle( int p_x, int p_y, int radius, bool outline = false );
	void drawRect( int p_x1, int p_y1, int p_x2, int p_y2, uint32_t p_color, bool outline = false );
	void drawText( int p_x, int p_y, std::string p_text, uint32_t p_color );

	font_t		loadFont( std::string p_file, uint8_t p_size, fontstyle_e p_style = FONTSTYLE_NONE );
	i_image		loadTexture( std::string p_file );
	i_image		loadTexture( void* p_data, uint32_t p_width = 256, uint32_t p_height = 256, uint32_t p_bits = 16 );

	void		freeFonts();
	void		freeTextures();

private:

	std::vector<i_font>			m_fonts;
	std::vector<i_image>		m_textures;
};
