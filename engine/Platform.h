#pragma once

#ifndef H_PLATFORM_H
#define H_PLATFORM_H


#if defined( AF_PLATFORM_LINUX ) // Unix/Linux/Ubuntu distros

#include <vector>
#include <string>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <X11/Xlib.h>

int GetTextW(const char* s);
int GetTextH(const char* s);


extern XFontStruct* g_SmlFontInfo;


inline int GetScreenSize(int *w, int*h)
{
	Display* pdsp = NULL;
	Screen* pscr = NULL;

	pdsp = XOpenDisplay( NULL );
	if ( !pdsp ) {
		fprintf(stderr, "Failed to open default display.\n");
		return -1;
	}

    pscr = DefaultScreenOfDisplay( pdsp );
	if ( !pscr ) {
		fprintf(stderr, "Failed to obtain the default screen of given display.\n");
		return -2;
	}

	*w = pscr->width;
	*h = pscr->height;

	XCloseDisplay( pdsp );
	return 0;
}

// Emulate CreateDirectory Win32 function
inline bool CreateDirectory( const char* p_pathname, void* p_dummy )
{
	struct stat st = {0};

	if (stat(p_pathname, &st) == -1)
	{
		if ( mkdir(p_pathname, 0x0700) == -1 )
			return false;
		else
			return true;
	}
	else
	{
		return false;
	}
}

inline void InitWorkingDirectory()
{
    char cwd[512];
    getcwd( cwd, 512 );

    //printf( "cwd: \"%s\"\n", cwd );

    readlink( "/proc/self/exe", cwd, 512 );

    char *p = strrchr( cwd, '/' );

    *p = 0;

    chdir( cwd );

    //printf( "new wd: \"%s\"\n", cwd );
}

inline int MessageBox( void* p_hwnd, const char* p_text, const char* p_caption, unsigned int p_type )
{
	Display*	dpy;
	Window		wnd, btn_ok;
	Atom		destroy;
	bool		run = true;
	int32_t		result = 0;
	int32_t		tx = GetTextW(p_text),
				ty = GetTextH(p_text);

	std::vector<std::string> lines;

	if ( p_text )
	{
		char *temp = (char*)malloc( strlen( p_text ) );
		memcpy( temp, p_text, strlen( p_text ) );
		char *tok = strtok( temp, "\n" );

		while ( tok )
		{
			printf( "MessageBox :: tok = \"%s\"\n", tok );
			std::string s = tok;

			for ( int i=0; i<s.size(); i++ )
			{
				if ( s.at( i ) == '\t' )
				{
					s.replace( i, 1, "    " );
				}
			}

			if ( GetTextW( s.c_str() ) > tx ) tx = GetTextW( s.c_str() );

			lines.push_back( s );
			tok = strtok( NULL, "\n" );
		}

		printf( "MessageBox :: lines = %u\n", lines.size() );

		free( temp );
	}
	else
	{
		return -1;
	}

	ty = g_SmlFontInfo->max_bounds.ascent * lines.size();

	dpy = XOpenDisplay( NULL );

	// -- Create a window
	wnd = XCreateSimpleWindow( dpy, DefaultRootWindow(dpy), 0,0, 20 + tx, 32+ty, 4, 0xFF0000, 0xCCCCCC );
	// -- Get the atom for the deletion of a window (X button)
	destroy = XInternAtom( dpy, "WM_DELETE_WINDOW", false );
	// -- Apply that we wish to capture this as a ClientMessage
	XSetWMProtocols( dpy, wnd, &destroy, 1 );
	// -- Store the name
	XStoreName( dpy, wnd, p_caption );
	// -- Transient of the root window (desktop)
	//XSetTransientForHint( dpy, DefaultRootWindow(dpy), wnd );

	XSelectInput(	dpy, wnd,
					ExposureMask | ButtonPressMask | ButtonReleaseMask |
					PointerMotionMask | StructureNotifyMask );

	// -- Map the window to the display
	XMapWindow( dpy, wnd );

	btn_ok		= XCreateSimpleWindow( dpy, wnd, ((tx+20)/2)-(64/2),((ty+20)+8)-(24), 64, 24, 1, 0x000000, 0xDDDDDD );

	XSelectInput(	dpy, btn_ok, ExposureMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | StructureNotifyMask );
	XMapWindow( dpy, btn_ok );

	XFlush( dpy );

	XGCValues values;
    values.background = 0xCCCCCC;
	GC gc = XCreateGC( dpy, wnd, GCBackground, &values );

	while ( run )
	{
		XEvent ev;

		if ( XPending( dpy ) > 0 )
		{
			XNextEvent( dpy, &ev );

			switch ( ev.type )
			{
				case ClientMessage:
					{
						if ( (Atom)ev.xclient.data.l[0] == destroy )
						{
							run = false;
						}
					}
				break;

				case ButtonPress:
					{
						XButtonEvent bev = ev.xbutton;
						if ( bev.window == btn_ok )
						{
							result = 0;
							run = false;
						}
					}
				break;

				case Expose:
					{
						if (ev.xexpose.count != 0)
							break;

						if ( ev.xexpose.window == wnd )
						{
							for ( int l = 0; l<lines.size(); l++ )
							{
								XDrawString( dpy, wnd, gc, 10, 10 + (10*l), lines[l].c_str(), lines[l].size() );
							}
						}
						if ( ev.xexpose.window == btn_ok )
						{
							Window root;
							int x, y;
							unsigned int width, height;
							unsigned int border_width;
							unsigned int depth;

							if ( XGetGeometry( dpy, btn_ok, &root,
								&x, &y, &width, &height, &border_width, &depth) == true )
							{
								XDrawString( dpy, btn_ok, gc, 1 + (width / 2) - 5 , 16, "OK", 2 );
							}
						}
					}
				break;

				default: break;
			}
		}
	}

	XFreeGC( dpy, gc );

	XUnmapWindow( dpy, wnd );
	XDestroyWindow( dpy, btn_ok );
	XDestroyWindow( dpy, wnd );

	XCloseDisplay( dpy );

	return result;
}

#else // Windows

#include <Windows.h>

inline int GetScreenSize( int* x, int* y )
{
	*x = GetSystemMetrics( SM_CXSCREEN );
	*y = GetSystemMetrics( SM_CYSCREEN );
	return 0;
}

inline void InitWorkingDirectory()
{

}

#endif

#endif
