#pragma once

#include <string>
#include <vector>
#include <stdint.h>


class Menu;


enum menustates_e
{
    // Enum Starts at 1000, for menu scripting
    MENU_SPLASH = 1000,
    MENU_PROFILE,
    MENU_PROFILECREATE,
    MENU_DISCLAIMER,
    MENU_MAIN,

    MENU_HUNT,
    MENU_HUNT_AREA = MENU_HUNT,
    MENU_HUNT_ANIMALS,
    MENU_HUNT_WEAPONS,
    MENU_HUNT_UTILITY,

    MENU_OPTIONS,
    MENU_OPTIONS_VIDEO,
    MENU_OPTIONS_AUDIO,
    MENU_OPTIONS_CONTROLS,

    MENU_CREDITS,

    MENU_SURVIVAL,

    MENU_MULTIPLAYER,

    MENU_TROPHY,

    MENU_DUMMY
};


enum menuitem_type_e
{
	MI_NONE					= 0,
	MI_BUTTON				= 1,

	MI_MAX,
};


class CMenuItem
{
public:

	virtual void click(){};
	virtual void cursorEnter(){};
	virtual void cursorLeave(){};
	virtual void draw( ) = 0;

	uint32_t	x,
				y,
				w,
				h,
				id,
				type;

	bool		active;
	uint32_t	flags;

	Menu*		owner;
};

class CMenuButton : public CMenuItem
{
public:
    TPicture    *pic_current,
				pic_off,
				pic_on;

	CMenuButton();

	void cursorEnter();
	void cursorLeave();
	void draw( );
};

class CMenuLabel : public CMenuItem
{
public:
    std::string		text;

    CMenuLabel();

	void draw( );
};

class CMenuImage : public CMenuItem
{
public:
	TPicture	pic;

	CMenuImage();

	void draw( );
};

class CMenuRange : public CMenuItem
{
public:
	CMenuRange();
	~CMenuRange();

	void		draw( );
	void		addItem( std::string p_item );
	std::string	getSelectedItem();

private:

	std::vector<std::string>	m_items;
};


class Menu
{
public:
	uint32_t				MenuState = MENU_SPLASH;
	TPicture    			MenuBackground;
	std::vector<CMenuItem*>	MenuItems;
    float					MenuScale;
    int						TargetW, TargetH;

	Menu();
	~Menu();

	bool loadScript( std::string p_scriptname );

	void processEvents();
	void draw();
};
