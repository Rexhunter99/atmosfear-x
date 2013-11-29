
#pragma comment(lib, "Comctl32.lib")

#include "Header.hpp"
#include "EZWindows.h"
#include <stdio.h>
#include <stdlib.h>


#define WINDOWTITLE "C2 Area Editor - "


class CCamera
{
public:
	float	Position[3];
	float	LookAt[3];
	float	Yaw,Pitch;
} Camera;


long			g_WindowW = 0,
				g_WindowH = 0;
ETOOL           ToolKind;
EZlong          ViewX=0,
				ViewY=0;
EZdword         Zoom=32;
HWND            g_Window,
				g_hAbtDlg,
				g_hAbtLink;
EZWScrollBar    g_DAScrollH,
				g_DAScrollV;
EZWTab          g_TabControl,
				g_ToolTab;
EZWStatic       g_DrawArea,
				g_ToolPanel,
				g_TestBitmap;
EZWMenuBar      g_Menu;
EZWStatusBar	g_StatBar;
EZWProgressBar	g_StatusProgress;
EZWButton       g_BCursor,
				g_BRaise,
				g_BLower,
				g_BLevel;
EZWGroup		g_GFlags;
EZWToolTip		g_ToolTip[12];

HBITMAP ButtonBMP[32];
HBITMAP AboutBMP;


LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK AboutDlgProc(HWND, UINT, WPARAM, LPARAM);
WNDPROC ToolTabProc;


void SaveAsProject( LPSTR );
void LoadProject( LPSTR );
void LoadMap( LPSTR );
bool LoadResources( LPSTR );


void GetFileName(LPSTR File, LPSTR Output)
{
	// Get the name of the file, strip the directory and extension
	if (Output == NULL) return;

	char* start = strrchr( File, '/' );
	if (start == NULL)
	{
		start = strrchr( File, '\\' );
	}
	start++;

	char* end = strrchr( File, '.' );
	if (end == NULL)
	{
		return;
	}

	DWORD Length = (end - start);
	DWORD Start = start - File;

	strncpy( Output, File + Start, Length );
}


HRESULT CALLBACK _ToolTabProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case BN_CLICKED:
                {
                    if (HWND(lParam) == HWND(g_BCursor)) ToolKind = TOOL_NONE;
                    if (HWND(lParam) == HWND(g_BRaise)) ToolKind = TOOL_TRAISE;
                    if (HWND(lParam) == HWND(g_BLower)) ToolKind = TOOL_TLOWER;
					if (HWND(lParam) == HWND(g_BLevel)) MessageBox( hwnd, "Clicked Level Tool", "Notification", 0);
                }
                break;
			}
		}
		break;
    }

    return CallWindowProc(ToolTabProc, hwnd, uMsg, wParam, lParam); // do NOT call the proc directly, use this method
}

float speed = 0.1f;

void KeyboardProcess()
{
	if (GetKeyState(VK_SHIFT) & 0x8000)
	{
		if (GetKeyState('1') & 0x8000) speed = 0.1f;
		if (GetKeyState('2') & 0x8000) speed = 0.25f;
		if (GetKeyState('3') & 0x8000) speed = 1.0f;
		if (GetKeyState('4') & 0x8000) speed = 2.0f;
		if (GetKeyState('5') & 0x8000) speed = 3.0f;
	}

	if (GetKeyState(VK_RIGHT) & 0x8000) {
		Camera.Yaw += 1.0f;
		if ( Camera.Yaw >= 360.0f ) Camera.Yaw = 0.0f;
	}
	if (GetKeyState(VK_LEFT) & 0x8000) {
		Camera.Yaw -= 1.0f;
		if ( Camera.Yaw <= -1.0f ) Camera.Yaw = 360.0f;
	}
	if (GetKeyState(VK_UP) & 0x8000) { 
		Camera.Pitch += 1.0f;
		if ( Camera.Pitch >= 89.0f ) Camera.Pitch = 89.0f;
	}
	if (GetKeyState(VK_DOWN) & 0x8000) {
		Camera.Pitch -= 1.0f;
		if ( Camera.Pitch <= -89.0f ) Camera.Pitch = -89.0f;
	}
	if (GetKeyState('S') & 0x8000) {
		Camera.Position[0] -= sin(Camera.Yaw/180*M_PI) * cos(Camera.Pitch/180*M_PI) * speed;
		Camera.Position[1] -= cos(Camera.Yaw/180*M_PI) * cos(Camera.Pitch/180*M_PI) * speed;
		Camera.Position[2] -= sin(Camera.Pitch/180.0f*M_PI) * speed;
		char str[256];
		sprintf( str, "%d,%d",  (int)Camera.Position[0], (int)Camera.Position[1] );
		g_StatBar.SetBarSection( 1, str );
	}
	if (GetKeyState('W') & 0x8000) {
		Camera.Position[0] += sin(Camera.Yaw/180*M_PI) * cos(Camera.Pitch/180*M_PI) * speed;
		Camera.Position[1] += cos(Camera.Yaw/180*M_PI) * cos(Camera.Pitch/180*M_PI) * speed;
		Camera.Position[2] += sin(Camera.Pitch/180.0f*M_PI) * speed;
		char str[256];
		sprintf( str, "%d,%d",  (int)Camera.Position[0], (int)Camera.Position[1] );
		g_StatBar.SetBarSection( 1, str );
	}

	if (true)
	{
		char str[256];
		sprintf( str, "Tris: %u | Chunks: %u", gpuGetPolyCount(), gpuGetChunkCount() );
		g_StatBar.SetBarSection( 0, str );
	}
}


#ifdef l_DEBUG
int main()
#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#endif
{
    WNDCLASSEX wcex;
    HWND hwnd;
    MSG msg;
    BOOL bQuit = FALSE;
#ifdef l_DEBUG
	HINSTANCE hInstance = GetModuleHandle( NULL );
	LPSTR lpCmdLine = NULL;
	int nCmdShow = SW_SHOW;
#endif

    /* register window class */
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_OWNDC;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = "CADIT";
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);;


    if (!RegisterClassEx(&wcex))
        return 0;

    ezwInitComCtl();

    HFONT hfDefault = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

    /* create main window */
    hwnd = CreateWindowEx(0,
                          "CADIT",
                          "C2 Area Editor - <New Project>",
                          WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          920,
                          640,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);

    ShowWindow(hwnd, nCmdShow);
    SendMessage(hwnd, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));
	g_Window = hwnd;

    RECT Rc,cRc;
    GetClientRect(hwnd,&cRc);
    g_WindowW = cRc.right;
    g_WindowH = cRc.bottom;
    SetRect(&Rc,0,0,100,100);

    // -- Create resources
    AboutBMP = LoadBitmap(hInstance, MAKEINTRESOURCE(1));
    ButtonBMP[0] = LoadBitmap(hInstance,MAKEINTRESOURCE(2));
    ButtonBMP[1] = LoadBitmap(hInstance,MAKEINTRESOURCE(3));
    ButtonBMP[2] = LoadBitmap(hInstance,MAKEINTRESOURCE(4));
    ButtonBMP[3] = LoadBitmap(hInstance,MAKEINTRESOURCE(5));

    // -- Create Controls
    g_Menu.Create();
    g_Menu.AddPopup(-1, "&File" );
    g_Menu.AddPopup(-1, "&Edit" );
    g_Menu.AddPopup(-1, "&Windows" );
    g_Menu.AddPopup(-1, "&Help" );
	// File
	g_Menu.AddItem(0, "New\tCtrl+N", ID_NEW );
    g_Menu.AddItem(0,"Open\tCtrl+O",ID_OPEN );
    g_Menu.AddItem(0,"Save\tCtrl+S",ID_SAVE );
    g_Menu.AddItem(0,"Save As\tCtrl+A",ID_SAVEAS );
    g_Menu.AddSeparator(0);
	
	// File->Recent Files
	g_Menu.AddPopup(0, "Recent Files" );
	g_Menu.AddItem(4, "AREA2.prj", ID_RECENT1);

    g_Menu.AddSeparator(0);
    g_Menu.AddItem(0,"Exit\tCtrl+X",ID_EXIT );
	//Edit
	g_Menu.AddItem( 1, "Undo\tCtrl+Z",ID_UNDO );
	g_Menu.AddItem( 1, "Redo\tCtrl+Shift+Z",ID_REDO );
	g_Menu.AddSeparator( 1 );
	g_Menu.AddPopup( 1, "Project" );
	g_Menu.AddItem( 5, "Name", ID_UNDO );
	//Tools
	g_Menu.AddItem( 2, "Textures\tShift+T",ID_TEXTURES );
	g_Menu.AddItem( 2, "Objects\tShift+O",ID_TEXTURES );
	g_Menu.AddItem( 2, "Random Sounds\tShift+R",ID_TEXTURES );
	g_Menu.AddItem( 2, "Ambient Sounds\tShift+A",ID_TEXTURES );
	g_Menu.AddItem( 2, "Fogs\tShift+F",ID_TEXTURES );
	g_Menu.AddItem( 2, "Waters\tShift+W",ID_TEXTURES );
	g_Menu.AddSeparator( 2 );
	g_Menu.AddItem( 2, "Test", ID_TEST );

	//Help
	g_Menu.AddItem(3, "Help\tCtrl+F1",ID_HELP );
	g_Menu.AddSeparator( 3 );
	g_Menu.AddItem(3, "About",ID_ABOUT );

    SetMenu(hwnd,g_Menu);

	if (g_ToolTab.Create(hwnd, Rc, 0, hInstance))
    {
        SendMessage(g_ToolTab, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));
        g_ToolTab.AddTab("Terrain");
        g_ToolTab.AddTab("Flags");
        g_ToolTab.AddTab("Textures");
    }

	ToolTabProc = (WNDPROC)SetWindowLongPtr( g_ToolTab, GWLP_WNDPROC, (LONG_PTR)_ToolTabProc );

    g_DrawArea.Create(hwnd, NULL, SS_NOTIFY, Rc);
    SetRect(&Rc, 32,32, 64,48);
    //g_DAScrollH.Create(g_DrawArea,SBS_HORZ, Rc);
    //g_DAScrollV.Create(g_DrawArea,SBS_VERT, Rc);

    //g_ToolPanel.Create(hwnd, NULL, 0, Rc);

    g_BRaise.Create( g_ToolTab, NULL, BS_BITMAP | BS_PUSHLIKE | BS_AUTOCHECKBOX , Rc);
    g_BRaise.SetBitmap(ButtonBMP[0]);

    g_BLower.Create( g_ToolTab, NULL, BS_BITMAP | BS_PUSHLIKE | BS_AUTOCHECKBOX, Rc);
    g_BLower.SetBitmap(ButtonBMP[1]);

    g_BCursor.Create( g_ToolTab, NULL, BS_BITMAP | BS_PUSHLIKE | BS_AUTOCHECKBOX, Rc);
    g_BCursor.SetBitmap(ButtonBMP[2]);

	g_BLevel.Create( g_ToolTab, NULL, BS_BITMAP | BS_PUSHLIKE | BS_AUTOCHECKBOX, Rc);
	g_BLevel.SetBitmap( ButtonBMP[3] );

	g_GFlags.Create(g_ToolTab, "Flags", 0, Rc);
	SendMessage(g_GFlags, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));

	g_TestBitmap.Create( g_ToolTab, NULL, SS_BITMAP, Rc );
	g_TestBitmap.SetBitmap( Project.TextureBMP[0] );
	MoveWindow( g_TestBitmap, 10, 300, 128, 128, TRUE );


    if (g_TabControl.Create(hwnd, Rc, 0, hInstance))
    {
        SendMessage(g_TabControl, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));
        g_TabControl.AddTab("Field");
        g_TabControl.AddTab("Textures");
		g_TabControl.AddTab("Objects");
    }
    
	int StatusWidths[] = {200, 250, 450, -1};
	int StatusHeight = 0;
	g_StatBar.Create( hwnd, 2500 );
	g_StatBar.SetupBar( sizeof(StatusWidths)/sizeof(int), StatusWidths );
	g_StatBar.SetBarSection( 0, "Idle" );
	g_StatBar.SetBarSection( 1, "0,0" );
	g_StatBar.SetBarSection( 2, "<New Project>" );
	g_StatBar.SetBarSection( 3, "" );
	StatusHeight = g_StatBar.Size();

	g_StatusProgress.Create( g_StatBar, PBS_MARQUEE, 300 ); //PBS_MARQUEE
	g_StatusProgress.SetMarquee( false, 0 );
	MoveWindow( g_StatusProgress, 450, 4, 200, StatusHeight-8, TRUE);

    // -- Resize everything so it's nice and perfect
	GetWindowRect( hwnd, &Rc );
	MoveWindow( hwnd, Rc.left, Rc.top, Rc.right, Rc.bottom, TRUE);

	g_ToolTip[0].Create( hwnd, g_BCursor, "Selection Cursor" );
	g_ToolTip[1].Create( hwnd, g_BRaise, "Terrain Raise" );
	g_ToolTip[2].Create( hwnd, g_BLower, "Terrain Lower" );

	Camera.Position[0] = 1;
	Camera.Position[1] = 1;
	Camera.Position[2] = 1;

    // -- Enable 3D Hardware
    if (!Activate3DHardware(g_DrawArea)) printf("Failed to Start 3D Mode!\n");

    /* program main loop */
    while (!bQuit)
    {
        /* check for messages */
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            /* handle or dispatch messages */
            if (msg.message == WM_QUIT)
            {
                bQuit = TRUE;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            /* OpenGL animation code goes here */
            RECT vRc;
            GetClientRect(g_DrawArea, &vRc);

			if ( GetFocus()==g_Window ) KeyboardProcess();

            float Focal[] = {
				(cos((-Camera.Yaw+90.0f)/180.0f*M_PI) * cos(Camera.Pitch/180.0f*M_PI)),
				(sin((-Camera.Yaw+90.0f)/180.0f*M_PI) * cos(Camera.Pitch/180.0f*M_PI)),
				sin(Camera.Pitch/180.0f*M_PI)
			};

            Clear3DBuffers(0xFF500000, 1.0f);

            if (TabCtrl_GetCurSel(g_TabControl)==0)
            {
				Scene3DBegin( Camera.Position, Focal, vRc.right, vRc.bottom);
				Draw3DGrid();
				Process3DTerrain( Camera.Position[0], Camera.Position[1], 0 );
				Scene3DEnd();
			}

            Sleep (1);
        }
    }

    // -- Shut down the 3D Hardware
    Shutdown3DHardware(g_DrawArea);

    //
    DeleteObject(AboutBMP);
    DeleteObject(ButtonBMP[0]);
    DeleteObject(ButtonBMP[1]);
    DeleteObject(ButtonBMP[2]);

    // -- Destroy the Window
    DestroyWindow(hwnd);

    // -- Unregister the Window Class
    UnregisterClass(wcex.lpszClassName,hInstance);

    return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CREATE:
        {
        }
        break;

        case WM_CLOSE:
            PostQuitMessage(0);
        break;

        case WM_DESTROY:
            return 0;

        case WM_SIZE:
        {
			int StatusHeight = g_StatBar.Size();

            g_WindowW = GET_X_LPARAM(lParam);
            g_WindowH = GET_Y_LPARAM(lParam);

            RECT rc;
            SetRect(&rc,0,0,g_WindowW-160,g_WindowH - StatusHeight);
            g_TabControl.Resize(&rc);

            MoveWindow(g_TabControl,0,0,g_WindowW-160,g_WindowH - StatusHeight,TRUE);
			MoveWindow(g_ToolTab,g_WindowW-160,0,160,g_WindowH - StatusHeight,TRUE);
            MoveWindow(g_DrawArea,rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top,TRUE);
            MoveWindow(g_BCursor, 10,24,32,32,TRUE);
            MoveWindow(g_BRaise, 44,24,32,32,TRUE);
            MoveWindow(g_BLower, 78,24,32,32,TRUE);
			MoveWindow(g_BLevel, 112,24,32,32,TRUE);

			MoveWindow(g_GFlags,10,80,140,140,TRUE);
        }
        break;

        case WM_VSCROLL:
        {
            SetWindowText(hwnd,"VScroll");
        }
        break;

        case WM_HSCROLL:
        {
            SetWindowText(hwnd,"HScroll");
        }
        break;

        case WM_KEYDOWN:
        {
            switch (wParam)
            {
            }
        }
        break;

        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                //default: MessageBox(hwnd,"WM_COMMAND","Button",MB_OK); break;
                case ID_NEW:
                {
                    memset(&Map, 0, sizeof(TMAP));
					memset(&Resources, 0, sizeof(TRESOURCE));
					Project.Name = "";
					for (int i=0; i<256; i++)
					{
						Project.TextureNames[i] = "";
						Project.ObjectNames[i] = "";
						Project.FogNames[i] = "";
						Project.WaterNames[i] = "";
					}
                }
                break;

                case ID_OPEN:
                {
					char FileName[MAX_PATH] = "";

					EZWFileDialog LoadFile;
					LoadFile.SetOwnerWindow( hwnd );
					LoadFile.SetOutputString( FileName );
					LoadFile.SetFilters( "Carnivores Area Project (*.PRJ)\0*.PRJ\0" );
					LoadFile.SetDefaultExt( "PRJ" );
					LoadFile.BasicOpenDialog();

					if ( LoadFile.OpenFile() )
					{
						LoadProject( FileName );
					}
                }
                break;

				case ID_SAVE:
				case ID_SAVEAS:
				{
					char FileName[MAX_PATH] = "";

					EZWFileDialog SaveFile;
					SaveFile.SetOwnerWindow( hwnd );
					SaveFile.SetOutputString( FileName );
					SaveFile.SetFilters( "Carnivores Area Project (*.PRJ)\0*.PRJ\0" );
					SaveFile.SetDefaultExt( "PRJ" );
					SaveFile.BasicSaveDialog();

					if ( SaveFile.SaveFile() )
					{
						SaveAsProject( FileName );
					}
				}
				break;

                case ID_EXIT:
                    PostQuitMessage(0);
                break;

				case ID_ABOUT:
					if (DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(100), hwnd, (DLGPROC)AboutDlgProc))
					{

					}
					else
					{

					}
					break;


                case BN_CLICKED: //STN_CLICKED
                {
					if (HWND(lParam) == HWND(g_DrawArea)) SetFocus( g_Window ); // Set Keyboard Focus back to the main window
                }
                break;
            }
        }
        break;

		case WM_NOTIFY:
		{
			switch ( ((LPNMHDR)lParam)->code )
			{
				case STN_DBLCLK:
				{
					char str[256];
					//if ( ((LPNMHDR)lParam)->hwndFrom == g_DrawArea )
					sprintf( str, "%d", ((LPNMHDR)lParam)->hwndFrom);
					MessageBox(hwnd, str, "WM_NOTIFY", 0);
				}
				break;
			}
		}
		break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

BOOL CALLBACK AboutDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch (Message)
    {
        case WM_INITDIALOG:
        {
            g_hAbtLink = CreateWindowEx(
                                0, "SysLink",
                                "Report bugs at: \r\n<A HREF=\"url\">The Carnivores Saga</A>",
                                WS_VISIBLE | WS_CHILD | WS_TABSTOP,
                                5,80-16, 150,32,
                                hwnd, NULL, GetModuleHandle(NULL), NULL
                                );
            SendMessage(g_hAbtLink, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));

			SendMessage((HWND)GetDlgItem(hwnd, 2),STM_SETIMAGE,(WPARAM)IMAGE_BITMAP, (LPARAM)AboutBMP);

			RECT rc,wrc;
            GetWindowRect(hwnd,&rc);
            GetWindowRect(g_Window,&wrc);

            int w = wrc.right-wrc.left;
            int h = wrc.bottom-wrc.top;
            int w2 = rc.right-rc.left;
            int h2 = rc.bottom-rc.top;

            MoveWindow(hwnd,
                        wrc.left + (w / 2) - (w2 / 2),
                        wrc.top + (h / 2) - (h2 / 2),
                        rc.right-rc.left, rc.bottom-rc.top,
                        TRUE);
        }
        break;

        case WM_CLOSE:
            ShowWindow(g_hAbtDlg, SW_HIDE);
        break;

        case WM_NOTIFY:
        {
            switch (((LPNMHDR)lParam)->code)
            {
                case NM_CLICK:
                case NM_RETURN:
                {
                    ShellExecute(NULL,
                                 "open",
                                 "http://z3.invisionfree.com/The_Carnivores_Saga/index.php?showtopic=1814",
                                 NULL, NULL, SW_SHOW);
                }
                break;
            }
            break;
        }
        break;

        case WM_COMMAND:
        switch (LOWORD(wParam))
        {
            case ABT_OK:
				EndDialog(hwnd, wParam); 
                return TRUE;
        }
        break;

        default: return FALSE;
    }
    return TRUE;
}

void SaveAsProject(LPSTR Name)
{
	FILE *fp = fopen( Name, "w" );

	if (fp == NULL) return;

	fprintf( fp, "Version=6\r" );

	fprintf( fp, "Total textures=%u\r", Resources.NumTextures );
	for (int i=0; i<Resources.NumTextures; i++)
	{
		fprintf( fp, "Texture%003u=%s\r", (UINT)i, "Blank_Texture" );
	}
	fprintf( fp, "Total objects=%u\r", Resources.NumObjects );
	for (int i=0; i<Resources.NumObjects; i++)
	{
		fprintf( fp, "Object%003u=%s\r", (UINT)i, "New_Object" );
	}

	fprintf( fp, "Sky loaded=1\r" );
	fprintf( fp, "Sky map loaded=1\r" );

	fprintf( fp, "Total fog=%u\r", Resources.NumFogs );
	for (int i=0; i<Resources.NumFogs; i++)
	{
		fprintf( fp, "Fog%003u=%s\r", (UINT)i, "Default_fog" );
	}

	fprintf( fp, "Total random sounds=%u\r", Resources.NumRandomSounds );
	for (int i=0; i<Resources.NumRandomSounds; i++)
	{
		fprintf( fp, "Random%003u=%s\r", (UINT)i, "Default_sound" );
	}

	fprintf( fp, "Total ambient sounds=%u\r", Resources.NumAmbientSounds );
	for (int i=0; i<Resources.NumAmbientSounds; i++)
	{
		fprintf( fp, "Ambient%003u=%s\r", (UINT)i, "Default_ambient_sound" );
	}

	fclose( fp );
}

void SaveAsMap(LPSTR Name)
{
	FILE *fp = fopen( Name, "wb" );

	fwrite(&Map, sizeof(TMAP), 1, fp);

	fclose( fp );
}


void SaveAsResource(LPSTR Name)
{
	FILE *fp = fopen( Name, "wb" );

	fwrite(&Resources.NumTextures, 4, 1, fp);
	fwrite(&Resources.NumObjects, 4, 1, fp);

	fwrite(&Resources.DawnBG_R, 4, 1, fp);
	fwrite(&Resources.DawnBG_G, 4, 1, fp);
	fwrite(&Resources.DawnBG_B, 4, 1, fp);
	fwrite(&Resources.DayBG_R, 4, 1, fp);
	fwrite(&Resources.DayBG_G, 4, 1, fp);
	fwrite(&Resources.DayBG_B, 4, 1, fp);
	fwrite(&Resources.NightBG_R, 4, 1, fp);
	fwrite(&Resources.NightBG_G, 4, 1, fp);
	fwrite(&Resources.NightBG_B, 4, 1, fp);

	fwrite(&Resources.DawnFade_R, 4, 1, fp);
	fwrite(&Resources.DawnFade_G, 4, 1, fp);
	fwrite(&Resources.DawnFade_B, 4, 1, fp);
	fwrite(&Resources.DayFade_R, 4, 1, fp);
	fwrite(&Resources.DayFade_G, 4, 1, fp);
	fwrite(&Resources.DayFade_B, 4, 1, fp);
	fwrite(&Resources.NightFade_R, 4, 1, fp);
	fwrite(&Resources.NightFade_G, 4, 1, fp);
	fwrite(&Resources.NightFade_B, 4, 1, fp);

	for (int i=0; i<Resources.NumTextures; i++)
	{
		fwrite( Resources.Textures[i].Data, 128*128, 1, fp );
	}

	for (int i=0; i<Resources.NumObjects; i++)
	{
		fwrite( &Resources.Objects[i].Header, sizeof(Resources.Objects[i].Header), 1, fp );

		fwrite( &Resources.Objects[i].NumVerts, 4, 1, fp );
		fwrite( &Resources.Objects[i].NumFaces, 4, 1, fp );
		fwrite( &Resources.Objects[i].NumBones, 4, 1, fp );
		fwrite( &Resources.Objects[i].TextureLength, 4, 1, fp );

		fwrite(  Resources.Objects[i].Triangles, sizeof(TTRIANGLE), Resources.Objects[i].NumFaces, fp );
		fwrite(  Resources.Objects[i].Vertices, sizeof(TVERTEX), Resources.Objects[i].NumVerts, fp );
		fwrite(  Resources.Objects[i].Bones, sizeof(TBONE), Resources.Objects[i].NumBones, fp );

		fwrite(  Resources.Objects[i].Texture.Data, Resources.Objects[i].TextureLength, 1, fp );
		fwrite(  Resources.Objects[i].Sprite.Data, 128*128*2, 1, fp );

		fwrite(  Resources.Objects[i].Animation.Name, 32, 1, fp );
		fwrite( &Resources.Objects[i].Animation.KPS, 4, 1, fp );
		fwrite( &Resources.Objects[i].Animation.NumFrames, 4, 1, fp );
		for (int i=0; i<Resources.Objects[i].Animation.NumFrames; i++)
		{
			fwrite( Resources.Objects[i].Animation.Frames[i].Vertices, sizeof(TSVERTEX), Resources.Objects[i].NumVerts, fp );
		}
	}

	fclose( fp );
}


void LoadProject( LPSTR FileName )
{
	char MAPName[MAX_PATH] = "";
	char RSCName[MAX_PATH] = "";
	char PRJName[MAX_PATH] = "";
	memset(PRJName, 0, sizeof(PRJName));
	memset(MAPName, 0, sizeof(MAPName));
	memset(RSCName, 0, sizeof(RSCName));

	GetFileName(FileName, PRJName);
	Project.Name = PRJName;

	strcpy(RSCName, FileName);
	char* ptr = strrchr(RSCName, '.');
	ptr++;
	*ptr = 'R'; ptr++;
	*ptr = 'S'; ptr++;
	*ptr = 'C';
	strcpy(MAPName, FileName);
	ptr = strrchr(MAPName, '.');
	ptr++;
	*ptr = 'M'; ptr++;
	*ptr = 'A'; ptr++;
	*ptr = 'P';

	g_StatusProgress.SetMarquee( true, 1000 );

	LoadMap( MAPName );

	LoadResources( RSCName );

	g_StatusProgress.SetMarquee( false, 0 );

	char Title[512] = "";
	sprintf( Title, "%s%s", WINDOWTITLE, PRJName);
	SetWindowText( g_Window, Title );

	g_StatBar.SetBarSection( 2, PRJName );
	g_StatBar.SetBarSection( 0, "Idle" );
}


void LoadMap( LPSTR FileName )
{
	g_StatBar.SetBarSection( 0, "Opening .MAP" );
    FILE *fp;
    fp = fopen(FileName, "rb");

	if (fp == NULL)
	{
		MessageBox( g_Window, "Failed to open the .MAP file...", "CAdit: File Error", MB_ICONERROR);
		return;
	}

    fread(Map.HMap, 1024*1024, 1, fp);
	g_StatBar.SetBarSection( 0, "MAP->Heights" );
	fread(Map.TMap1, 1024*1024*2, 1, fp);
	g_StatBar.SetBarSection( 0, "MAP->Texture1" );
	fread(Map.TMap2, 1024*1024*2, 1, fp);
	g_StatBar.SetBarSection( 0, "MAP->Texture2" );
	fread(Map.OMap, 1024*1024, 1, fp);
	g_StatBar.SetBarSection( 0, "MAP->Objects" );
	fread(Map.FMap, 1024*1024*2, 1, fp);
	g_StatBar.SetBarSection( 0, "MAP->Flags" );
	fread(Map.LMap1, 1024*1024, 1, fp);
	g_StatBar.SetBarSection( 0, "MAP->Dawn" );
	fread(Map.LMap2, 1024*1024, 1, fp);
	g_StatBar.SetBarSection( 0, "MAP->Noon" );
	fread(Map.LMap3, 1024*1024, 1, fp);
	g_StatBar.SetBarSection( 0, "MAP->Night" );
	fread(Map.WMap, 1024*1024, 1, fp);
	g_StatBar.SetBarSection( 0, "MAP->Water" );
	fread(Map.OHMap, 1024*1024, 1, fp);
	g_StatBar.SetBarSection( 0, "MAP->Object Heights" );
	fread(Map.MMap, 512*512, 1, fp);
	g_StatBar.SetBarSection( 0, "MAP->Fog" );
	fread(Map.SMap, 512*512, 1, fp);
	g_StatBar.SetBarSection( 0, "MAP->Sounds" );

    fclose(fp);
}


bool LoadResources( LPSTR FileName )
{
	char str[256];

	g_StatBar.SetBarSection( 0, "Opening .RSC" );

	FILE *fp = fopen( FileName, "rb" );

	if (fp == NULL)
	{
		MessageBox( g_Window, "The resource file does not exist!", "File Error", MB_OK|MB_ICONERROR);
		return false;
	}

	g_StatBar.SetBarSection( 0, "RSC->Header" );

	fread( &Resources.NumTextures, 4, 1, fp );
	fread( &Resources.NumObjects, 4, 1, fp );
	
	fread( &Resources.DawnBG_R, 4, 1, fp);
	fread( &Resources.DawnBG_G, 4, 1, fp);
	fread( &Resources.DawnBG_B, 4, 1, fp);
	fread( &Resources.DayBG_R, 4, 1, fp);
	fread( &Resources.DayBG_G, 4, 1, fp);
	fread( &Resources.DayBG_B, 4, 1, fp);
	fread( &Resources.NightBG_R, 4, 1, fp);
	fread( &Resources.NightBG_G, 4, 1, fp);
	fread( &Resources.NightBG_B, 4, 1, fp);

	fread( &Resources.DawnFade_R, 4, 1, fp);
	fread( &Resources.DawnFade_G, 4, 1, fp);
	fread( &Resources.DawnFade_B, 4, 1, fp);
	fread( &Resources.DayFade_R, 4, 1, fp);
	fread( &Resources.DayFade_G, 4, 1, fp);
	fread( &Resources.DayFade_B, 4, 1, fp);
	fread( &Resources.NightFade_R, 4, 1, fp);
	fread( &Resources.NightFade_G, 4, 1, fp);
	fread( &Resources.NightFade_B, 4, 1, fp);

	g_StatBar.SetBarSection( 0, "RSC->Textures" );

	HDC compatHDC = CreateCompatibleDC( GetDC( g_Window ) );

	//int maxWidth = gpuGetMaxTextureWidth();

	if ( Resources.TextureStrip ) delete [] Resources.TextureStrip;
	Resources.TextureStrip = new WORD[ 4096*4096 ];

	for (int i=0; i<Resources.NumTextures; i++)
	{
		fread(  Resources.Textures[i].Data, 128*128*2, 1, fp );

		if ( Project.TextureBMP[i] != NULL )
		{
			DeleteObject( Project.TextureBMP[i] );
		}

		BITMAPINFOHEADER bmih;
		bmih.biSize =		sizeof( BITMAPINFOHEADER );
		bmih.biWidth  =		128;
		bmih.biHeight =		128;
		bmih.biPlanes =		1;
		bmih.biBitCount =	16;
		bmih.biCompression = BI_RGB;
		bmih.biSizeImage =	0;
		bmih.biXPelsPerMeter = 400;
		bmih.biYPelsPerMeter = 400;
		bmih.biClrUsed =	0;
		bmih.biClrImportant = 0;
		BITMAPINFO binfo;
		binfo.bmiHeader = bmih;

		void* Bitmap = 0;

		Project.TextureBMP[i] = CreateDIBSection(GetDC( g_Window) , &binfo, DIB_RGB_COLORS, &Bitmap, NULL, 0);
		memcpy( Bitmap, Resources.Textures[i].Data, 128*128*2 );

		// Now copy to the atlas
		for ( int y=0; y<128; ++y )
		{
			/*
			z_x == horizontal index
			z_y == vertical index
			row == row page width
			*/
			int z_x = i % 32;
			int z_y = i / 32;
			int row = 4096;
			memcpy( Resources.TextureStrip + (  ((y * row) + (z_y * (128 * row))) + (z_x * 128) ), Resources.Textures[i].Data + ( y * 128 ), 256 );
		}
	}
	g_TestBitmap.SetBitmap( (HBITMAP)Project.TextureBMP[1] );
	gpuUploadTextures( Resources.TextureStrip, 0 );
	DeleteDC( compatHDC );

	// -- Objects Block
	for ( int i=0; i<Resources.NumObjects; ++i )
	{
		fread( &Resources.Objects[i].Header, 64, 1, fp );
		fread( &Resources.Objects[i].NumVerts, 4, 1, fp );
		fread( &Resources.Objects[i].NumFaces, 4, 1, fp );
		fread( &Resources.Objects[i].NumBones, 4, 1, fp );
		fread( &Resources.Objects[i].TextureLength, 4, 1, fp );

		fread( Resources.Objects[i].Triangles, sizeof(TTRIANGLE) * Resources.Objects[i].NumFaces, 1, fp );

		fread( Resources.Objects[i].Vertices, sizeof(TVERTEX) * Resources.Objects[i].NumVerts, 1, fp );

		fread( Resources.Objects[i].Bones, sizeof(TBONE) * Resources.Objects[i].NumBones, 1, fp );

		fread( Resources.Objects[i].Texture.Data, Resources.Objects[i].TextureLength, 1, fp );

		fread( Resources.Objects[i].Sprite.Data, 128*128*2, 1, fp );

		if ( Resources.Objects[i].Header.Flags & ofANIMATED )
		{
			fread( Resources.Objects[i].Animation.Name, 32, 1, fp );
			fread( &Resources.Objects[i].Animation.KPS, 4, 1, fp );
			fread( &Resources.Objects[i].Animation.NumFrames, 4, 1, fp );
			for ( int a=0; a<Resources.Objects[i].Animation.NumFrames; ++a )
			{
				fread( Resources.Objects[i].Animation.Frames[a].Vertices, sizeof(TSVERTEX), Resources.Objects[i].NumVerts, fp ); 
			}
		}
	}

	// -- Sky Block
	fread( Resources.DawnSky.Data, 256*256*2, 1, fp );
	fread( Resources.DaySky.Data, 256*256*2, 1, fp );
	fread( Resources.NightSky.Data, 256*256*2, 1, fp );
	fread( Resources.Clouds.Data, 128*128, 1, fp );

	// -- Fog Block
	fread( &Resources.NumFogs, 4, 1, fp );
	fread( Resources.Fogs, sizeof(TFOG), Resources.NumFogs, fp );

	// -- Random Sounds Block
	fread( &Resources.NumRandomSounds, 4, 1, fp );
	for ( int i=0; i<Resources.NumRandomSounds; ++i )
	{
		Resources.RandomSounds[i].Free();
		fread( &Resources.RandomSounds[i].DataLength, 4, 1, fp );
		Resources.RandomSounds[i].Allocate( Resources.RandomSounds[i].DataLength );
		fread( Resources.RandomSounds[i].Data, Resources.RandomSounds[i].DataLength, 1, fp );
	}

	// -- Ambient Sounds Block
	fread( &Resources.NumAmbientSounds, 4, 1, fp );
	for ( int i=0; i<Resources.NumAmbientSounds; ++i )
	{
		Resources.AmbientSounds[i].Sound.Free();
		fread( &Resources.AmbientSounds[i].Sound.DataLength, 4, 1, fp );
		Resources.AmbientSounds[i].Sound.Allocate( Resources.AmbientSounds[i].Sound.DataLength );
		fread( Resources.AmbientSounds[i].Sound.Data, Resources.AmbientSounds[i].Sound.DataLength, 1, fp );

		fread( Resources.AmbientSounds[i].Random, sizeof(TRD), 16, fp );

		fread( &Resources.AmbientSounds[i].NumRandomSFX, 4, 1, fp );
		fread( &Resources.AmbientSounds[i].Volume, 4, 1, fp );
	}

	// -- Water Block
	fread( &Resources.NumWaters, 4, 1, fp );
	fread( Resources.Waters, sizeof(TWATER), Resources.NumWaters, fp );

	fclose( fp );

	return true;
}