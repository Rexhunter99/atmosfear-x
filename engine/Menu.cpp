
#include "Hunt.h"

#include "Menu.hpp"

#include <cstdio>

using namespace std;


void CMenuButton::draw()
{
	DrawPicture( this->x, this->y, *pic_current );
}

void CMenuLabel::draw()
{
	Draw_Text( this->x, this->y, (char*)text.c_str(), 0xFFFFFFFF );
}


Menu::Menu()
{
    LoadPictureTGA(MenuBackground, "MENUM.TGA");
}

bool Menu::loadScript( const char* p_scriptname )
{
	FILE* fp = 0;

	if ( (fp = fopen( p_scriptname, "r" )) == NULL )
	{
		// DoHalt( "blah" );
		return false;
	}

	vector<char*> line_toks;

	while ( !feof( fp ) )
	{
		char line[256];
		fgets( line, 255, fp );
		line_toks.clear();

		char* tok = strtok( line, " \t\n" );
		while ( tok )
		{
			line_toks.push_back( tok );
			tok = strtok( NULL, " \t\n" );
		}

		if ( line_toks[0] == "BACKGROUND" )
		{
			LoadPicture( MenuBackground, line_toks[1] );
		}
		else if ( line_toks[0] == "BUTTON" && line_toks.size() > 5 )	// BUTTON <default_image_file> <hover_image_file> x y id
		{
			CMenuButton *mbtn;
			MenuItems.push_back( mbtn = new CMenuButton() );

			LoadPicture( mbtn->pic_off, line_toks[1] );
			LoadPicture( mbtn->pic_on, line_toks[2] );
			mbtn->x = atoi( line_toks[3] );
			mbtn->y = atoi( line_toks[4] );
			mbtn->id = atoi( line_toks[5] );
			mbtn->w = mbtn.pic_on.m_width;
			mbtn->h = mbtn.pic_on.m_height;
		}
		else if ( line_toks[0] == "LABEL" && line_toks.size() > 5 )	// LABEL text x y w h
		{
			CMenuLabel *mlab;
			MenuItems.push_back( mlab = new CMenuLabel() );

			mlab->text = line_toks[1];

			mlab->x = atoi( line_toks[2] );
			mlab->y = atoi( line_toks[3] );
			mlab->w = atoi( line_toks[4] );
			mlab->h = atoi( line_toks[5] );
		}
	}

	fclose( fp );

	return true;
}

void Menu::draw()
{
	// -- Draw the background
	DrawPicture( 0,0, MenuBackground );

	// -- Render all the menu items
	for ( size_t i=0; i<MenuItems.size(); i++ )
	{
		CMenuItem* it = MenuItems[i];
		it->draw();
	}
}
