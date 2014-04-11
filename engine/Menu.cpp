
#include "Hunt.h"
#include "Menu.hpp"

#include <cstdio>

using namespace std;


CMenuButton::CMenuButton()
{
	this->type = MI_BUTTON;
}

CMenuLabel::CMenuLabel()
{
	this->type = MI_NONE;
}

CMenuImage::CMenuImage()
{
	this->type = MI_NONE;
}

void CMenuButton::cursorEnter()
{
	this->pic_current = &this->pic_on;
}

void CMenuButton::cursorLeave()
{
	this->pic_current = &this->pic_off;
}

void CMenuButton::draw( )
{
	DrawPictureExt( this->x*owner->MenuScale, this->y*owner->MenuScale, owner->MenuScale, *pic_current );
}

void CMenuLabel::draw( )
{
	Draw_Text( this->x*owner->MenuScale, this->y*owner->MenuScale, (char*)text.c_str(), 0xFFFFFFFF );
}

void CMenuImage::draw( )
{
	DrawPictureExt( this->x*owner->MenuScale, this->y*owner->MenuScale, owner->MenuScale, pic );
}


Menu::Menu()
{
	this->MenuScale	= 1.0f;
}

Menu::~Menu()
{
	for ( int i=0; i<MenuItems.size(); i++ )
	{
		delete MenuItems[i];
	}
	MenuItems.clear();
}

bool Menu::loadScript( string p_scriptname )
{
	FILE* fp = 0;

	if ( (fp = fopen( p_scriptname.c_str(), "r" )) == NULL )
	{
		DoHalt( "blah" );
		return false;
	}

	vector<string> line_toks;

	// -- Loop through to the end of the file
	while ( !feof( fp ) )
	{
		char line[256];
		char *line_p = line;
		fgets( line, 255, fp );
		line_toks.clear();

		// -- Split the line up into tokens
		char* tok = strtok( line_p, " \t\n" );
		if ( !tok ) continue;

		// -- Check for comments
		if ( strstr( tok, "//" ) ) continue;
		if ( strstr( tok, ";" ) ) continue;
		if ( strstr( tok, "#" ) ) continue;
		if ( strstr( tok, "--" ) ) continue;

		while ( tok )
		{
			fprintf( stderr, "menu tok: \"%s\"\n", tok );
			line_toks.push_back( tok );

			if ( strstr( tok, "LABEL" ) )
			{
				tok = strtok( NULL, "\"\n" );
				//tok = strtok( NULL, "\"\n" );
			}
			else
			{
				tok = strtok( NULL, " \t\n" );
			}
		}

		// -- Interpret the tokens
		if ( line_toks[0] == "BACKGROUND" && line_toks.size() >= 1 )
		{
			string fn = string(string( "huntdat/menu/images/" ) + string(line_toks[1]));

			LoadPicture( MenuBackground, fn.c_str() );
		}
		else if ( line_toks[0] == "PROPS" )
		{
			TargetW = atoi( line_toks[1].c_str() );
			TargetH = atoi( line_toks[2].c_str() );
			int window_w, window_h;
			glfwGetWindowSize( &window_w, &window_h );
			this->MenuScale = (float)window_h / (float)this->TargetH;
		}
		else if ( line_toks[0] == "BUTTON" && line_toks.size() > 5 )	// BUTTON default_image hover_image x y id
		{
			CMenuButton *mbtn;
			MenuItems.push_back( mbtn = new CMenuButton() );

			string fn1 = string(string( "huntdat/menu/images/" ) + string(line_toks[1]));
			string fn2 = string(string( "huntdat/menu/images/" ) + string(line_toks[2]));

			LoadPicture( mbtn->pic_off, fn1.c_str() );
			LoadPicture( mbtn->pic_on, fn2.c_str() );
			mbtn->x = atoi( line_toks[3].c_str() );
			mbtn->y = atoi( line_toks[4].c_str() );
			mbtn->id = atoi( line_toks[5].c_str() );
			mbtn->w = mbtn->pic_on.m_width;
			mbtn->h = mbtn->pic_on.m_height;
			mbtn->pic_current = &mbtn->pic_off;
			mbtn->owner	= this;
		}
		else if ( line_toks[0] == "LABEL" && line_toks.size() > 5 )	// LABEL text x y w h
		{
			CMenuLabel *mlab;
			MenuItems.push_back( mlab = new CMenuLabel() );

			mlab->text = line_toks[1];

			mlab->x = atoi( line_toks[2].c_str() );
			mlab->y = atoi( line_toks[3].c_str() );
			mlab->w = atoi( line_toks[4].c_str() );
			mlab->h = atoi( line_toks[5].c_str() );
			mlab->owner	= this;
		}
		else if ( line_toks[0] == "IMAGE" && line_toks.size() >= 3 )
		{
			CMenuImage *mimg;
			MenuItems.push_back( mimg = new CMenuImage() );

			string fn1 = string(string( "huntdat/menu/images/" ) + string(line_toks[1]));

			LoadPicture( mimg->pic, fn1.c_str() );
			mimg->x = atoi( line_toks[2].c_str() );
			mimg->y = atoi( line_toks[3].c_str() );
			mimg->owner	= this;
		}
	}

	fclose( fp );

	return true;
}

void Menu::processEvents()
{
	vec2i cursor;
	glfwGetMousePos( &cursor.x, &cursor.y );
	int mb[] = {
		glfwGetMouseButton( GLFW_MOUSE_BUTTON_LEFT ),
		glfwGetMouseButton( GLFW_MOUSE_BUTTON_MIDDLE ),
		glfwGetMouseButton( GLFW_MOUSE_BUTTON_RIGHT )
	};

	// -- Process the ratio/scale factor
	int window_w, window_h;
	glfwGetWindowSize( &window_w, &window_h );
	this->MenuScale = (float)window_h / (float)this->TargetH;

	for ( size_t i=0; i<MenuItems.size(); i++ )
	{
		CMenuItem* it = MenuItems[i];

		if ( cursor.x >= (it->x*MenuScale) && cursor.y >= (it->y*MenuScale) &&
			 cursor.x <= ((it->x*MenuScale)+(it->w*MenuScale)) && cursor.y <= ((it->y*MenuScale)+(it->h*MenuScale)) )
		{
			if ( mb[0] == GLFW_PRESS )
			{
				if ( it->type == MI_BUTTON )
				{
					switch ( it->id )
					{
					case 200:
						_GameState = GAMESTATE_GAMESTART;
						break;
					case 202:
						_GameState = GAMESTATE_CLOSE;
					}
				}
			}
			if ( !(it->flags & 1) )
			{
				it->flags |= (1 << 0);
				it->cursorEnter();
			}
		}
		else // We are not hovering over this item
		{
			if ( (it->flags & 1) )
			{
				it->flags &= ~(1 << 0);
				it->cursorLeave();
			}
		}
	}
}

void Menu::draw()
{
	// -- Draw the background
	DrawPictureExt( 0, 0, this->MenuScale, MenuBackground );//DrawPictureTiled( MenuBackground );

	// -- Render all the menu items
	for ( size_t i=0; i<MenuItems.size(); i++ )
	{
		CMenuItem* it = MenuItems[i];
		it->draw( );
	}
}
