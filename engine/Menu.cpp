
#include "Hunt.h"

#include "Menu.hpp"

#include <cstdio>

using namespace std;


void CMenuButton::cursorEnter()
{
	this->pic_current = &this->pic_on;
}

void CMenuButton::cursorLeave()
{
	this->pic_current = &this->pic_off;
}

void CMenuButton::draw()
{
	DrawPicture( this->x, this->y, *pic_current );
}

void CMenuLabel::draw()
{
	Draw_Text( this->x, this->y, (char*)text.c_str(), 0xFFFFFFFF );
}

void CMenuImage::draw()
{
	DrawPicture( this->x, this->y, pic );
}


Menu::Menu()
{
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
		fgets( line, 255, fp );
		line_toks.clear();

		// -- Split the line up into tokens
		char* tok = strtok( line, " \t\n" );
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
		}
		else if ( line_toks[0] == "IMAGE" && line_toks.size() >= 3 )
		{
			CMenuImage *mimg;
			MenuItems.push_back( mimg = new CMenuImage() );

			string fn1 = string(string( "huntdat/menu/images/" ) + string(line_toks[1]));

			LoadPicture( mimg->pic, fn1.c_str() );
			mimg->x = atoi( line_toks[2].c_str() );
			mimg->y = atoi( line_toks[3].c_str() );
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

	for ( size_t i=0; i<MenuItems.size(); i++ )
	{
		CMenuItem* it = MenuItems[i];

		if ( cursor.x >= it->x && cursor.y >= it->y && cursor.x <= (it->x+it->w) && cursor.y <= (it->y+it->h) )
		{
			if ( !(it->flags & 1) )
			{
				it->flags |= (1 << 0);
				it->cursorEnter();
			}
		}
		else
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
	DrawPictureTiled( MenuBackground );

	// -- Render all the menu items
	for ( size_t i=0; i<MenuItems.size(); i++ )
	{
		CMenuItem* it = MenuItems[i];
		it->draw();
	}
}
