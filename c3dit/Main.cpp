#ifdef _WIN32_IE
#undef _WIN32_IE
#endif
#define _WIN32_IE	0x0700
#ifdef WINVER
#undef WINVER
#endif
#define WINVER 0x0501

#include "C3Dit.h"
#include "IniFile.h"
#include "version.h"
#include "Global.h"
#include <vector>
#include <string>

using namespace std;


#define STATMSG_SIZE 11
string statMessages[] = {
    "Holding the right mouse button lets you orbit the object!",
    "Scrolling the mouse wheel will zoom in and out of the scene!",
    "Holding the middle mouse button will let you pan the camera!",
    "Left clicking the scene will refocus the keyboard input!",
    "Make sure to save your files!  Backup frequently!",
	"There are keyboard shortcuts for most actions!",
	"Hold Z and press Up/Down to zoom in and out alternatively.",
	"Holding CTRL and clicking a triangle will add it to the selection!",
	"+/- on the NumPad will zoom in and out as well!",
	"Report bugs to the forum!",
    "The cake is a lie..."
};


#include "Targa.h"

void loadCAR(void);
void loadCAR(char*);
void newScene();
void mouseE();
void keyboardE();
void exportTexture();
void exportModel();
void importTexture();
void PlayWave( SOUND* );
void LoadCARData(char*);
void LoadC2OData(char*);
void Load3DFData(char*);
void Load3DNData(char*);
void LoadOBJData(char*);


INT_PTR CALLBACK AniDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK SndDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK CarDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK AboutDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK WinToolsProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
HTREEITEM TreeView_AddNode(HWND, HTREEITEM, const char*);
HTREEITEM TreeView_AddResource(HWND, HTREEITEM, const char*, LPARAM);


enum
{
    TV_MESHES = 0,
    TV_TEXTURES = 1,
    TV_ANIMATIONS = 2,
    TV_SOUNDS = 3,
    TV_SPRITES = 4,
    TV_TRIANGLES = 5,
    TV_VERTICES = 6,
    TV_BONES = 7,
};


Globals				*g = 0;
char				szTitle[256];
char				str[255];       //For messages/text
int					mx,my;
vector<HTREEITEM>	g_TVItems;
HTREEITEM			rootNodes[12];
RECT				g_TVRect;
char				lpszExePath[MAX_PATH];
char				lpszIniPath[MAX_PATH];

void InitializeCommonControls()
{
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = 0;
    icex.dwICC |= ICC_ANIMATE_CLASS;
    icex.dwICC |= ICC_BAR_CLASSES;
    icex.dwICC |= ICC_COOL_CLASSES;
    icex.dwICC |= ICC_DATE_CLASSES;
    icex.dwICC |= ICC_HOTKEY_CLASS;
    icex.dwICC |= ICC_INTERNET_CLASSES;
    icex.dwICC |= ICC_LINK_CLASS;
    icex.dwICC |= ICC_LISTVIEW_CLASSES;
    icex.dwICC |= ICC_NATIVEFNTCTL_CLASS;
    icex.dwICC |= ICC_PAGESCROLLER_CLASS;
    icex.dwICC |= ICC_PROGRESS_CLASS;
    icex.dwICC |= ICC_STANDARD_CLASSES;
    icex.dwICC |= ICC_TAB_CLASSES;
    icex.dwICC |= ICC_TREEVIEW_CLASSES;
    icex.dwICC |= ICC_UPDOWN_CLASS;
    InitCommonControlsEx(&icex);
}

#ifdef _DEBUG
int main( int argc, char *argv[] )
{
	HINSTANCE hInstance = GetModuleHandle( 0 );
	LPSTR lpCmdLine = "";
	int nCmdShow = SW_SHOW;
#else
int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
#endif
    WNDCLASSEX  wcex;
    HWND        hwnd;
    HDC         hDC;
    HGLRC       hRC;
    MSG         msg;
    BOOL        bQuit = FALSE;
    RECT        rc;

	g = GlobalContainer::GlobalVar();

	GetModuleFileName( 0, lpszExePath, MAX_PATH );
	
	// make the static path of the settings.ini
	strcpy( lpszIniPath, lpszExePath );
	char* pos = strrchr( lpszIniPath, '\\' ) + 1;
	strcpy( pos, "settings.ini" );
	printf( "Ini file: %s\n", lpszIniPath );

	// Remove the exe filename
	pos = strrchr( lpszExePath, '\\' ) + 1;
	*pos = 0;

	// -- Load the settings
	IniFile* ini = new IniFile();
	ini->Load( lpszIniPath );

	g->sGameDirectory	= ini->ReadKeyString( "General", "game", "C:\\Program Files (x86)\\Carnivores 2\\" );
	g->sEnvironmentMap	= ini->ReadKeyString( "General", "envmap", "ENVMAP.TGA" );
	g->sSpecularMap		= ini->ReadKeyString( "General", "specular", "SPECULAR.TGA" );
	
	WinX	= CW_USEDEFAULT;//ini->ReadKeyInt( "Window", "left", 100 );
	WinY	= 0;//ini->ReadKeyInt( "Window", "top", 80 );
	WinW	= 800;//ini->ReadKeyInt( "Window", "width", 800 );
	WinH	= 600;//ini->ReadKeyInt( "Window", "height", 600 );
	
	g->sRecentFile[0] = ini->ReadKeyString( "RecentFiles", "file1", "" );
	g->sRecentFile[1] = ini->ReadKeyString( "RecentFiles", "file2", "" );
	g->sRecentFile[2] = ini->ReadKeyString( "RecentFiles", "file3", "" );
	g->sRecentFile[3] = ini->ReadKeyString( "RecentFiles", "file4", "" );
	g->sRecentFile[4] = ini->ReadKeyString( "RecentFiles", "file5", "" );

	g->iColorBits	= ini->ReadKeyInt( "OpenGL", "color", 24 );
	g->iDepthBits	= ini->ReadKeyInt( "OpenGL", "depth", 16 );

	delete ini;
	ini = 0;

	/*WinX = CW_USEDEFAULT;
	WinY = 0;
	WinW = 900;
	WinH = 600;*/

	cam.x = -24.0f;
    cam.y = -24.0f;
    cam.z = 0.0f;
    cam.xt = 0.0f;
    cam.yt = 0.0f;
    cam.zt = 0.0f;
    cam.dist = 24.0f;

    /* register window class */
	ZeroMemory( &wcex, sizeof( WNDCLASSEX ) );
    wcex.cbSize =			sizeof(WNDCLASSEX);
    wcex.style =			CS_OWNDC;
    wcex.lpfnWndProc =		WindowProc;
    wcex.hInstance =		hInstance;
    wcex.hIcon =			LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor =			LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground =	(HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName =		MAKEINTRESOURCE(IDR_MENU);
    wcex.lpszClassName =	"C3DIT_CLASS";
    wcex.hIconSm =			LoadIcon(NULL, IDI_APPLICATION);


    if (!RegisterClassEx(&wcex))
        return 0;

    InitializeCommonControls();

    /* create main window */
    sprintf(szTitle,"Carnivores 3D Editor [%u.%u.%u]", VERSION_MAJOR, VERSION_MINOR, VERSION_RELEASE );
    hwnd = CreateWindowEx(  WS_EX_ACCEPTFILES,
                            wcex.lpszClassName,
                            szTitle,
                            WS_OVERLAPPEDWINDOW,
                            WinX, WinY,
                            WinW, WinH,
                            NULL,
                            NULL,
                            hInstance,
                            NULL);

    //ShowWindow(hwnd, nCmdShow);
    g_hMain = hwnd;
	GetClientRect(hwnd,&rc);

    g_DrawArea = CreateWindow(
                     WC_STATIC,
                     NULL,
                     WS_CHILD | WS_VISIBLE | SS_BLACKFRAME,
                     rc.left,rc.top+16,rc.right-128,rc.bottom-48,
                     g_hMain,
                     NULL,
                     hInstance,
                     NULL
                 );

    g_AniTrack = CreateWindow(
                    TRACKBAR_CLASS,
                    NULL,
                    WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS,
                    rc.left, rc.bottom-48, rc.right-128, 16,
                    g_hMain,
                    NULL,
                    hInstance,
                    NULL
                    );

    SendMessage(g_AniTrack, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(0,1) );
    SendMessage(g_AniTrack, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)0 );

    g_FileView = CreateWindowEx(
                     WS_EX_CLIENTEDGE,
                     WC_TREEVIEW,
                     NULL,
                     WS_CHILD | WS_VISIBLE |
                     TVS_LINESATROOT | TVS_HASLINES |
                     TVS_HASBUTTONS | TVS_EDITLABELS,
                     rc.left+rc.right-128,rc.top+16,128,rc.bottom-32,
                     g_hMain,
                     NULL,
                     hInstance,
                     NULL
                 );

    SetRect(&g_TVRect, rc.right-128, 0, 128, 0);

    rootNodes[0] = (TreeView_AddNode(g_FileView, 0, "Mesh"));
    rootNodes[1] = (TreeView_AddNode(g_FileView, 0, "Images"));
    rootNodes[2] = (TreeView_AddNode(g_FileView, 0, "Animations"));
    rootNodes[3] = (TreeView_AddNode(g_FileView, 0, "Sounds"));
    rootNodes[4] = (TreeView_AddNode(g_FileView, 0, "LoD"));
    rootNodes[5] = (TreeView_AddNode(g_FileView, rootNodes[0], "Triangles"));
    rootNodes[6] = (TreeView_AddNode(g_FileView, rootNodes[0], "Vertices"));
    rootNodes[7] = (TreeView_AddNode(g_FileView, rootNodes[0], "Bones"));

    /* create the tool bar */
    gHTool = CreateWindowEx(0,
                            TOOLBARCLASSNAME,
                            NULL,
                            WS_CHILD | WS_VISIBLE | WS_BORDER,
                            0,0,0,0,
                            g_hMain,
                            (HMENU)IDM_TOOL,
                            hInstance,
                            NULL);

    /* create the status bar */
    gHStatus = CreateWindowEx(0,
                              STATUSCLASSNAME,
                              NULL,
                              WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
                              0,0,0,0,
                              g_hMain,
                              (HMENU)IDM_STATUS,
                              hInstance,
                              NULL);

    /* useful for backwards compatability */
    SendMessage(gHTool,TB_BUTTONSTRUCTSIZE,(WPARAM)sizeof(TBBUTTON),0);

    /* prepare the toolbar buttons */
    TBBUTTON tbb[7];
    TBADDBITMAP tbab;

    tbab.hInst = hInstance;//GetModuleHandle(NULL);
    tbab.nID = ID_TB_IMAGE;

    SendMessage(gHTool, TB_ADDBITMAP, 4, (LPARAM)&tbab);

    /* Make and define the toolbar buttons */
    ZeroMemory(tbb, sizeof(tbb));
    tbb[0].iBitmap = 0;
    tbb[0].fsState = TBSTATE_ENABLED;
    tbb[0].fsStyle = TBSTYLE_BUTTON;
    tbb[0].idCommand = IDF_NEW;
    tbb[1].iBitmap = 1;
    tbb[1].fsState = TBSTATE_ENABLED;
    tbb[1].fsStyle = TBSTYLE_BUTTON;
    tbb[1].idCommand = IDF_OPEN;
    tbb[2].iBitmap = 2;
    tbb[2].fsState = TBSTATE_ENABLED;
    tbb[2].fsStyle = TBSTYLE_BUTTON;
    tbb[2].idCommand = IDF_SAVEAS;

    tbb[3].iBitmap = 0;
    tbb[3].fsState = TBSTATE_ENABLED;
    tbb[3].fsStyle = TBSTYLE_SEP;
    tbb[3].idCommand = 0;

    tbb[4].iBitmap = 3;
    tbb[4].fsState = TBSTATE_ENABLED;
    tbb[4].fsStyle = TBSTYLE_BUTTON;
    tbb[4].idCommand = ID_ANIM_PLAY;
    tbb[5].iBitmap = 4;
    tbb[5].fsState = TBSTATE_ENABLED;
    tbb[5].fsStyle = TBSTYLE_BUTTON;
    tbb[5].idCommand = ID_ANIM_PAUSE;

    tbb[6].iBitmap = 0;
    tbb[6].fsState = TBSTATE_ENABLED;
    tbb[6].fsStyle = TBSTYLE_SEP;
    tbb[6].idCommand = 0;

    /*tbb[7].iBitmap = 5;
    tbb[7].fsState = TBSTATE_ENABLED;
    tbb[7].fsStyle = TBSTYLE_BUTTON;
    tbb[7].idCommand = ID_UPLOAD;*/

    SendMessage(gHTool, TB_ADDBUTTONS, sizeof(tbb)/sizeof(TBBUTTON), (LPARAM)&tbb);

    /* prepare the status bar */
    srand( timeGetTime() );
    int statwidths[] = {180, -1};
    SendMessage(gHStatus, SB_SETPARTS, sizeof(statwidths)/sizeof(int),(LPARAM)statwidths);
    SendMessage(gHStatus, SB_SETTEXT, 0, (LPARAM)"No Model!");
    SendMessage(gHStatus, SB_SETTEXT, 1, (LPARAM)statMessages[ rand() % 6 ].c_str());

	//SetWindowPos( g_hMain, HWND_TOP, WinX,WinY, WinW, WinH, SWP_SHOWWINDOW );

	//GetWindowRect( g_hMain, &rc );
	int PrevW = WinW,
		PrevH = WinH;
	ShowWindow( g_hMain, SW_SHOW );

    /* enable OpenGL for the window */
    EnableOpenGL(g_DrawArea, &hDC, &hRC);

	//MoveWindow( g_hMain, 0,0, WinW,WinH, true );

	// There is a bug with ShowWindow, it sends a WM_SIZE without my permission
	SendMessage( g_hMain, WM_SIZE, 0, (LPARAM)MAKELPARAM(WinW,WinH) );
	WinW = PrevW;
	WinH = PrevH;

	DWORD messageTime = timeGetTime();
	
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
            // Get the mouse co-ords on the screen
            if (GetForegroundWindow()==g_hMain)
            {
                POINT ms;
                RECT rc;
                GetCursorPos(&ms);
                CurX = ms.x;
                CurY = ms.y;

                mouseE();
                keyboardE();

                /* OpenGL animation code goes here */

                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                // Camera
                GetClientRect(g_DrawArea, &rc);
                glViewport( 0,0, rc.right-rc.left,rc.bottom-rc.top );

				if ( CameraView == VIEW_PERSPECTIVE )
				{
					glMatrixMode(GL_PROJECTION);
					glLoadIdentity();
					gluPerspective(45.0f, (float)rc.right/(float)rc.bottom, 1.0f, 1000.0f);

					glMatrixMode(GL_MODELVIEW);
					glLoadIdentity();
					gluLookAt(cam.x,cam.z,cam.y,cam.xt,cam.zt,cam.yt,0.0f,1.0f,0.0f);
				}
				else if ( CameraView == VIEW_FIRSTPERSON )
				{
					glMatrixMode(GL_PROJECTION);
					glLoadIdentity();
					gluPerspective(60.0f, (float)rc.right/(float)rc.bottom, 0.1f, 1000.0f);

					glMatrixMode(GL_MODELVIEW);
					glLoadIdentity();
					gluLookAt(0,0,0, 0,0,1, 0.0f,1.0f,0.0f);
				}
				else
				{
					glMatrixMode(GL_PROJECTION);
					glLoadIdentity();
					glOrtho(0,rc.right, rc.bottom,0, 1000.0f, -1000.0f);

					glMatrixMode(GL_MODELVIEW);
					glLoadIdentity();

					glTranslatef( (rc.right/2) + ortho.xt, (rc.bottom/2) + ortho.yt, 0 );
					
					glRotatef( 180.0f, 0, 0, 1 );

					glScalef( (cam.dist*2)-cam.dist, (cam.dist*2)-cam.dist, (cam.dist*2)-cam.dist );
					
					if ( CameraView == VIEW_RIGHT )
						glRotatef(-90.0f, 0, 1, 0 );
					if ( CameraView == VIEW_LEFT )
						glRotatef( 90.0f, 0,1,0 );
					if ( CameraView == VIEW_BACK )
						glRotatef( 180.0f, 0,1,0 );
					if ( CameraView == VIEW_FRONT )
						glRotatef( 0.0f, 0,1,0 );
					if ( CameraView == VIEW_TOP)
						glRotatef(-90.0f, 1,0,0 );
					if ( CameraView == VIEW_BOTTOM )
						glRotatef( 90.0f, 1,0,0 );


					//glScalef( 16,16,16 );
				}

				double mvmatrix[16], projmatrix[16];
				int viewport[4];
				glGetIntegerv( GL_VIEWPORT, viewport );
				glGetDoublev( GL_MODELVIEW_MATRIX, mvmatrix );
				glGetDoublev( GL_PROJECTION_MATRIX, projmatrix );

                // Draw the 3d model (mesh) ... duh
                RenderMesh();

                // Draw text overlay
                glMatrixMode(GL_PROJECTION);
                glLoadIdentity();
                glOrtho(rc.left,rc.right,rc.bottom,rc.top, -1280,1280);

                glMatrixMode(GL_MODELVIEW);
                glLoadIdentity();

				// -- Render a vertex in 2D space, mapped to the 3D location
				if ( g->bShowAxis )
				{
					double _outX,_outY,_outZ;

					gluProject (5.5,0,0, mvmatrix, projmatrix, viewport, &_outX, &_outY, &_outZ);
					DrawTextGL( (float)_outX, (float)rc.bottom-_outY, 0, "X", 0xFF0000FF);
					gluProject (0,5.5,0, mvmatrix, projmatrix, viewport, &_outX, &_outY, &_outZ);
					DrawTextGL( (float)_outX, (float)rc.bottom-_outY, 0, "Y", 0xFF00FF00);
					gluProject (0,0,5.5, mvmatrix, projmatrix, viewport, &_outX, &_outY, &_outZ);
					DrawTextGL( (float)_outX, (float)rc.bottom-_outY, 0, "Z", 0xFFFF0000);
				}

				switch ( CameraView )
				{
				case VIEW_PERSPECTIVE: DrawTextGL(13,13,0, "Perspective", 0xFF9000FF); break;
				case VIEW_FIRSTPERSON: DrawTextGL(13,13,0, "First Person", 0xFF9000FF); break;
				case VIEW_LEFT: DrawTextGL(13,13,0, "Left", 0xFF9000FF); break;
				case VIEW_RIGHT: DrawTextGL(13,13,0, "Right", 0xFF9000FF); break;
				case VIEW_FRONT: DrawTextGL(13,13,0, "Front", 0xFF9000FF); break;
				case VIEW_BACK: DrawTextGL(13,13,0, "Back", 0xFF9000FF); break;
				case VIEW_TOP: DrawTextGL(13,13,0, "Top", 0xFF9000FF); break;
				case VIEW_BOTTOM: DrawTextGL(13,13,0, "Bottom", 0xFF9000FF); break;
				}

				// -- Draw focus rectangle
                if ( GetFocus() == g_hMain )
                {
                    glBegin( GL_LINE_LOOP );
                        glColor4ub(255,0,0,255);
                        glVertex2f( (float)rc.left+2,  (float)rc.top+3 );
                        glVertex2f( (float)rc.right-3, (float)rc.top+3 );
                        glVertex2f( (float)rc.right-3, (float)rc.bottom-3 );
                        glVertex2f( (float)rc.left+3,  (float)rc.bottom-3);
                    glEnd();
                }

                RealTime = timeGetTime();
                //RealTime/=4;
                TimeDt = RealTime - PrevTime;
                if (TimeDt<0) TimeDt = 10;
                if (TimeDt>10000) TimeDt = 10;
                if (TimeDt>1000) TimeDt = 1000;
                PrevTime = RealTime;

                if (GetTickCount()-LastTick >=1000)
                {
                    LastTick = GetTickCount();
                    FPS = Frames;
                    Frames = 0;
                }
                Frames++;

				if ( RealTime-messageTime >= 5000 )
				{
					srand( RealTime );
					SendMessage(gHStatus, SB_SETTEXT, 1, (LPARAM)statMessages[ rand() % STATMSG_SIZE ].c_str());
					messageTime = RealTime;
				}

                //Make sure the title is modified
                if (Model.mName[0]!=0)
                {
                    sprintf(str,"%s - %s - FPS: %d",szTitle,Model.mName,FPS);
                    SetWindowText(g_hMain,str);
                }
				else
				{
                    sprintf(str,"%s - FPS: %d",szTitle,FPS);
                    SetWindowText(g_hMain,str);
                }

                // Copy the buffer (Double Buffering)
                SwapBuffers(hDC);
                Sleep(1);
            }
            else
            {
                Sleep (100);
            }
        }
    }

    g_TVItems.clear();

	//ini save
	ini = new IniFile();

	ini->WriteKeyString( "General", "game", "C:\\Program Files (x86)\\Carnivores 2\\" );
	ini->WriteKeyString( "General", "envmap", g->sEnvironmentMap );
	ini->WriteKeyString( "General", "specular", g->sSpecularMap );
	
	ini->WriteKeyInt( "Window", "left", WinX );
	ini->WriteKeyInt( "Window", "top", WinY );
	ini->WriteKeyInt( "Window", "width", WinW );
	ini->WriteKeyInt( "Window", "height", WinH );
	
	ini->WriteKeyString( "RecentFiles", "file1", "ALLO.CAR" );
	ini->WriteKeyString( "RecentFiles", "file2", "" );
	ini->WriteKeyString( "RecentFiles", "file3", "" );
	ini->WriteKeyString( "RecentFiles", "file4", "" );
	ini->WriteKeyString( "RecentFiles", "file5", "" );

	ini->WriteKeyInt( "OpenGL", "color", 24 );
	ini->WriteKeyInt( "OpenGL", "depth", 16 );

	ini->Save( lpszIniPath );
	delete ini;

    /* shutdown OpenGL */
    DisableOpenGL(g_DrawArea, hDC, hRC);

    /* destroy the window explicitly */
    DestroyWindow(g_FileView);
    DestroyWindow(g_DrawArea);
    DestroyWindow(hwnd);

    /* unregister class */
    UnregisterClass( wcex.lpszClassName, wcex.hInstance );

    return msg.wParam;
}


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
    {
        HMENU hMenu, hResMenu;
        hMenu = GetMenu(g_hMain);
        hResMenu = GetSubMenu(hMenu, 1);
        CheckMenuItem(hResMenu, IDF_LIGHT, MF_UNCHECKED | MF_BYCOMMAND);
		CheckMenuItem(hResMenu, IDV_BONES, MF_UNCHECKED | MF_BYCOMMAND);
		CheckMenuItem(hResMenu, IDV_JOINTS, MF_UNCHECKED | MF_BYCOMMAND);
		CheckMenuItem(hResMenu, IDV_BONES, MF_CHECKED | MF_BYCOMMAND);
		CheckMenuItem(hResMenu, IDV_JOINTS, MF_CHECKED | MF_BYCOMMAND);
		UpdateWindow( g_hMain );

        g_hAniDlg	= CreateDialog(GetModuleHandle(NULL),
									MAKEINTRESOURCE(ID_dlg_ANI),hwnd, AniDlgProc);
        g_hSndDlg	= CreateDialog(GetModuleHandle(NULL),
									MAKEINTRESOURCE(ID_dlg_SND),hwnd, SndDlgProc);
        g_hCarDlg	= CreateDialog(GetModuleHandle(NULL),
									MAKEINTRESOURCE(ID_DLG_CAR),hwnd, CarDlgProc);
		g_hAbout	= CreateDialog(GetModuleHandle(NULL),
									MAKEINTRESOURCE(ID_ABOUT_DIALOG),hwnd, AboutDlgProc);
		g_hTools	= CreateDialog(GetModuleHandle( NULL ),
									MAKEINTRESOURCE(ID_DLG_TOOLS),hwnd, WinToolsProc );

    }
    break;

    case WM_CLOSE:
        PostQuitMessage(0);
        break;

    case WM_SIZE:
    {
        WinW = LOWORD(lParam);
        WinH = HIWORD(lParam);
        //glViewport( 0, 0, LOWORD(lParam), HIWORD(lParam) );
        RECT rcTool,rcStatus,rcClient,rc;
        int iToolHeight,iStatusHeight;

        // Get Rects
        GetClientRect(g_hMain, &rcClient);
        GetWindowRect(g_FileView, &g_TVRect);

        // Size toolbar and get height
        gHTool = GetDlgItem(hwnd, IDM_TOOL);
        SendMessage(gHTool, TB_AUTOSIZE, 0, 0);
        GetWindowRect(gHTool, &rcTool);
        iToolHeight = rcTool.bottom - rcTool.top;

        // Size status bar and get height
        gHStatus = GetDlgItem(hwnd, IDM_STATUS);
        SendMessage(gHStatus, WM_SIZE, 0, 0);
        GetWindowRect(gHStatus, &rcStatus);
        iStatusHeight = rcStatus.bottom - rcStatus.top;

        // Calculate remaining height and size draw area
        SetRect(&rc, rcClient.left, rcClient.top+iToolHeight, rcClient.right - 200, rcClient.bottom-iToolHeight-iStatusHeight);
        MoveWindow(g_DrawArea, rc.left,rc.top, rc.right, rc.bottom-32, TRUE);

        MoveWindow(g_AniTrack, rc.left,rc.top+rc.bottom-32,rc.right,32, TRUE);

        // File Components View
        SetRect(&rc, rc.left+rc.right, rc.top, 200, rc.bottom);
        MoveWindow(g_FileView, rc.left,rc.top,rc.right,rc.bottom, TRUE);

        // Resize OpenGL
        RECT rcGL;
        GetClientRect(g_DrawArea, &rcGL);
    }
    break;

    case WM_NOTIFY:
    {
        switch (((LPNMHDR)lParam)->code)
        {
            case TVN_SELCHANGED:
            {
                LPNM_TREEVIEW nmtv = (LPNM_TREEVIEW)lParam;
                HTREEITEM ParItem = TreeView_GetParent(g_FileView, nmtv->itemNew.hItem);

                if ( ParItem == rootNodes[TV_TRIANGLES] )
                if ( GetKeyState( VK_CONTROL ) & 0x8000 )
				{
                    // -- Selection
                    g_TriSelection.push_back( ((UINT)nmtv->itemNew.lParam - (UINT)&g_Triangles[0]) / sizeof(TRIANGLE) );
                }
				else
				{
					g_TriSelection.clear();
					g_TriSelection.push_back( ((UINT)nmtv->itemNew.lParam - (UINT)&g_Triangles[0]) / sizeof(TRIANGLE) );
				}

                if (ParItem == rootNodes[TV_ANIMATIONS]) // Animations
                {
                    CUR_ANIM = ((UINT)nmtv->itemNew.lParam - (UINT)&g_Animations[0]) / sizeof(TVtl);
					CUR_FRAME = 0;
                    SendMessage(g_AniTrack, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(0, ( g_Animations[CUR_ANIM].FrameCount-1 ) ) );
                    SendMessage(g_AniTrack, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)0 );

                }

                if (ParItem == rootNodes[TV_SOUNDS]) // Animations
                {
                    Snd = ((UINT)nmtv->itemNew.lParam - (UINT)&g_Sounds[0]) / sizeof(SOUND);
                }
            }
            break;

            case NM_RCLICK:
            {
                LPNMHDR nmh = (LPNMHDR)lParam;
                if (nmh->hwndFrom == g_FileView)
                {
                    printf("NM_RCLICK:\n");

                    TVHITTESTINFO   tvhti;
                    HTREEITEM       SelItem, ParItem;
                    POINT           p;

                    GetCursorPos(&tvhti.pt);
                    ScreenToClient(g_FileView, &tvhti.pt);

                    SelItem = TreeView_HitTest(g_FileView, &tvhti);//MAX_VERTICES
                    ParItem = TreeView_GetParent(g_FileView, SelItem);

                    if (SelItem == rootNodes[TV_MESHES]) // Mesh
                    {
                        /*GetCursorPos(&p);
                        HMENU hPopupMenu = CreatePopupMenu();
                        InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, 200, "Rename");
                        InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, 200, "Rename");
                        InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, 201, "Open");
                        SetForegroundWindow(hwnd);
                        TrackPopupMenu(hPopupMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, p.x, p.y, 0, hwnd, NULL);*/
                    }
                    if (SelItem == rootNodes[TV_ANIMATIONS]) // Animations
                    {
                        GetCursorPos(&p);
                        HMENU hPopupMenu = CreatePopupMenu();
                        InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, MPM_CLEAR, "Clear");
                        InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, MPM_ADD, "Add");
                        SetForegroundWindow(hwnd);
                        TrackPopupMenu(hPopupMenu, TPM_TOPALIGN | TPM_LEFTALIGN, p.x, p.y, 0, hwnd, NULL);
                    }
                    if (SelItem == rootNodes[TV_SOUNDS]) // Sounds
                    {
                        GetCursorPos(&p);
                        HMENU hPopupMenu = CreatePopupMenu();
                        InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, MPM_CLEAR+10, "Clear");
                        InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, MPM_SFX_ADD, "Add");
                        SetForegroundWindow(hwnd);
                        TrackPopupMenu(hPopupMenu, TPM_TOPALIGN | TPM_LEFTALIGN, p.x, p.y, 0, hwnd, NULL);
                    }
                    if (ParItem == rootNodes[TV_ANIMATIONS]) // Animations
                    {
                        TVITEM ti;
                        ZeroMemory(&ti, sizeof(TVITEM));
                        ti.hItem = SelItem;
                        ti.mask = TVIF_HANDLE | TVIF_PARAM;
                        TreeView_GetItem(g_FileView, &ti);

                        CUR_ANIM = ((UINT)ti.lParam - (UINT)&g_Animations[0]) / sizeof(TVtl);
                        printf("SelAnim = %u\n", Ani);

                        GetCursorPos(&p);
                        HMENU hPopupMenu = CreatePopupMenu();
                        InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, MPM_RENAME+20, "Rename");
                        InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, IDB_APLAY, "Play");
                        InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
                        InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, MPM_REMOVE+20, "Remove");
                        InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, MPM_ADD+20, "Insert");
                        SetForegroundWindow(hwnd);
                        TrackPopupMenu(hPopupMenu, TPM_TOPALIGN | TPM_LEFTALIGN, p.x, p.y, 0, hwnd, NULL);
                    }
                    if (ParItem == rootNodes[TV_SOUNDS]) // Sounds
                    {
                        TVITEM ti;
                        ZeroMemory(&ti, sizeof(TVITEM));
                        ti.hItem = SelItem;
                        ti.mask = TVIF_HANDLE | TVIF_PARAM;
                        TreeView_GetItem(g_FileView, &ti);

                        Snd = ((UINT)ti.lParam - (UINT)&g_Sounds[0]) / sizeof(SOUND);
                        printf("SelSound = %u\n", Snd);

                        GetCursorPos(&p);
                        HMENU hPopupMenu = CreatePopupMenu();
                        InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, 200, "Rename");
                        InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, IDB_SPLAY, "Play");
                        InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
                        InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, 200, "Remove");
                        InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, 201, "Insert");
                        SetForegroundWindow(hwnd);
                        TrackPopupMenu(hPopupMenu, TPM_TOPALIGN | TPM_LEFTALIGN, p.x, p.y, 0, hwnd, NULL);
                    }
                }
            }
            break;

			case NM_CLICK:
			{
				LPNMHDR nh = (LPNMHDR)lParam;
				printf( "BN_CLICKED %u\n",nh->idFrom );
			}	
			break;
		
            case TVN_ENDLABELEDIT:
            {
                LPNMTVDISPINFO ed = (LPNMTVDISPINFO)lParam;
                TVITEM ti;

                ZeroMemory(&ti, sizeof(TVITEM));
                ti.hItem = ed->item.hItem;
                ti.mask = TVIF_HANDLE | TVIF_PARAM;

                TreeView_GetItem(g_FileView, &ti);

                // -- Animations
                if ((LPARAM)&g_Animations[0] <= ti.lParam <= (LPARAM)&g_Animations[Model.mAnimCount])
                {
					if ( ed->item.mask & TVIF_TEXT )
					{
						size_t len = ed->item.cchTextMax;
						TVtl* lpAnim = (TVtl*)ti.lParam;
						strncpy( lpAnim->Name, ed->item.pszText, (len<=32) ? len: 32 );
					}
                }
                // -- Sounds
                if ((LPARAM)&g_Sounds[0] <= ti.lParam <= (LPARAM)&g_Sounds[Model.mSoundCount])
                {
                    size_t len = strlen(ed->item.pszText);
                    SOUND* lpSound = (SOUND*)ti.lParam;
                    strncpy( lpSound->name, ed->item.pszText, (len<=32) ? strlen(ed->item.pszText) : 32 );
                }

                ti = ed->item;
                TreeView_SetItem(g_FileView, &ti);
            }
            break;

        }
    }
    break;

    case WM_MOVE:
        WinX = LOWORD(lParam);
        WinY = HIWORD(lParam);
        break;

    case WM_DESTROY:
		{
			DestroyWindow(embedTest);
			DestroyWindow(g_hAniDlg);
			DestroyWindow(g_hSndDlg);
			DestroyWindow(g_hCarDlg);
		}
        return 0;

    case WM_COMMAND:
    {
        switch (wParam)
        {
        case MPM_ADD:
			{
                //MessageBox(hwnd, "Open Context Item", "Context Command", MB_OK);
				uint32_t a = Model.mAnimCount;
				sprintf( g_Animations[a].Name, "<New>" );
				g_Animations[a].FrameCount = 0;
				g_Animations[a].KPS = 0;
				if ( g_Animations[a].Data ) delete [] g_Animations[a].Data;
				g_Animations[a].Data = 0;

				g_TVItems.push_back(TreeView_AddResource(g_FileView, rootNodes[2], g_Animations[a].Name, LPARAM( &g_Animations[a] ) ));
				Model.mAnimCount++;
			}
			break;
		case MPM_SFX_ADD:
			{
                //MessageBox(hwnd, "Open Context Item", "Context Command", MB_OK);
				uint32_t s = Model.mSoundCount;
				sprintf( g_Sounds[s].name, "<New>" );
				g_Sounds[s].len = 0;
				if ( g_Sounds[s].data ) delete [] g_Sounds[s].data;
				g_Sounds[s].data = 0;

				g_TVItems.push_back(TreeView_AddResource(g_FileView, rootNodes[3], g_Sounds[s].name, LPARAM( &g_Sounds[s] ) ));
				Model.mSoundCount++;
			}
			break;

        case IDF_NEW:
                newScene();
            break;

        case IDF_OPEN:
                loadCAR();
            break;

        case IDF_SAVEAS:
                SaveProject();
            break;

        case IDF_ABOUT:
			{
				ShowWindow( g_hAbout, SW_SHOW );
				//char About[512];
				//sprintf(About, "C3Dit (Designer 3 - D3)\r\nCarnivores 3D Editor - Version: %s\r\nBy James Ogden\r\nContact: rexhunter99@gmail.com", AutoVersion::FULLVERSION_STRING);
				//MessageBox(hwnd,About,"About C3Dit",MB_OK|MB_ICONINFORMATION);
			}
			break;

        case IDF_UPDATE:
        {

        }
        break;

        case IDF_EXIT:
            PostQuitMessage(0);
            break;

        case IDF_FLAGS:
        {
            // Enable Menu items
            HMENU hMenu, hResMenu;
            hMenu = GetMenu(g_hMain);
            hResMenu = GetSubMenu(hMenu, 1);

            COLOR = !COLOR;
            if (COLOR)
                CheckMenuItem(hResMenu, IDF_FLAGS, MF_CHECKED | MF_BYCOMMAND);
            else
                CheckMenuItem(hResMenu, IDF_FLAGS, MF_UNCHECKED | MF_BYCOMMAND);
        }
        break;

        case IDF_WIRE:
        {
            // Enable Menu items
            HMENU hMenu, hResMenu;
            hMenu = GetMenu(g_hMain);
            hResMenu = GetSubMenu(hMenu, 1);

			g->bShowWireframe = !g->bShowWireframe;
            if ( g->bShowWireframe )
                CheckMenuItem(hResMenu, IDF_WIRE, MF_CHECKED | MF_BYCOMMAND);
            else
                CheckMenuItem(hResMenu, IDF_WIRE, MF_UNCHECKED | MF_BYCOMMAND);
        }
        break;

        case IDF_LIGHT:
        {
            // Enable Menu items
            HMENU hMenu, hResMenu;
            hMenu = GetMenu(g_hMain);
            hResMenu = GetSubMenu(hMenu, 1);

			g->bUseLighting = !g->bUseLighting;
            if ( g->bUseLighting )
            {
                glEnable(GL_LIGHTING);
                CheckMenuItem(hResMenu, IDF_LIGHT, MF_CHECKED | MF_BYCOMMAND);
            }
            else
            {
                glDisable(GL_LIGHTING);
                CheckMenuItem(hResMenu, IDF_LIGHT, MF_UNCHECKED | MF_BYCOMMAND);
            }
        }
        break;

		case IDV_BONES:
		{
			// Enable Menu items
			HMENU hMenu, hResMenu;
			hMenu = GetMenu(g_hMain);
			hResMenu = GetSubMenu(hMenu, 1);

			if ( g->bShowBones == true )
				g->bShowBones = false;
			else
				g->bShowBones = true;

			if ( g->bShowBones )
			{
				CheckMenuItem(hResMenu, IDV_BONES, MF_CHECKED | MF_BYCOMMAND);
			}
			else
			{
				CheckMenuItem(hResMenu, IDV_BONES, MF_UNCHECKED | MF_BYCOMMAND);
			}
		}
		break;

		case IDV_JOINTS:
		{
			// Enable Menu items
			HMENU hMenu, hResMenu;
			hMenu = GetMenu(g_hMain);
			hResMenu = GetSubMenu(hMenu, 1);
			
			if ( g->bShowJoints == true )
				g->bShowJoints = false;
			else
				g->bShowJoints = true;

			if ( g->bShowJoints )
			{
				CheckMenuItem(hResMenu, IDV_JOINTS, MF_CHECKED | MF_BYCOMMAND);
			}
			else
			{
				CheckMenuItem(hResMenu, IDV_JOINTS, MF_UNCHECKED | MF_BYCOMMAND);
			}
		}
		break;

		case IDV_GRID:
		{
			// Enable Menu items
			HMENU hMenu, hResMenu;
			hMenu = GetMenu(g_hMain);
			hResMenu = GetSubMenu(hMenu, 1);

			if ( g->bShowGrid == true )
				g->bShowGrid = false;
			else
				g->bShowGrid = true;

			if ( g->bShowGrid )
			{
				CheckMenuItem(hResMenu, IDV_GRID, MF_CHECKED | MF_BYCOMMAND);
			}
			else
			{
				CheckMenuItem(hResMenu, IDV_GRID, MF_UNCHECKED | MF_BYCOMMAND);
			}
		}
		break;

		case IDV_AXIS:
		{
			// Enable Menu items
			HMENU hMenu, hResMenu;
			hMenu = GetMenu(g_hMain);
			hResMenu = GetSubMenu(hMenu, 1);

			if ( g->bShowAxis == true )
				g->bShowAxis = false;
			else
				g->bShowAxis = true;

			if ( g->bShowAxis )
			{
				CheckMenuItem(hResMenu, IDV_AXIS, MF_CHECKED | MF_BYCOMMAND);
			}
			else
			{
				CheckMenuItem(hResMenu, IDV_AXIS, MF_UNCHECKED | MF_BYCOMMAND);
			}
		}
		break;

        case IDT_FLIPTRIS:
        {
            for (int t=0; t<Model.mFaceCount; t++)
            {
                int v1 = g_Triangles[t].v1;
                int v2 = g_Triangles[t].v2;
                int v3 = g_Triangles[t].v3;
                g_Triangles[t].v1 = v3;
                g_Triangles[t].v2 = v2;
                g_Triangles[t].v3 = v1;

                int tx1 = g_Triangles[t].tx1;
                int ty1 = g_Triangles[t].ty1;
                int tx2 = g_Triangles[t].tx2;
                int ty2 = g_Triangles[t].ty2;
                int tx3 = g_Triangles[t].tx3;
                int ty3 = g_Triangles[t].ty3;
                g_Triangles[t].tx1 = tx3;
                g_Triangles[t].ty1 = ty3;
                g_Triangles[t].tx2 = tx2;
                g_Triangles[t].ty2 = ty2;
                g_Triangles[t].tx3 = tx1;
                g_Triangles[t].ty3 = ty1;
            }

            // Normals
            bool vlist[1024];
            ZeroMemory(vlist,sizeof(bool));
            for (int f=0; f<Model.mFaceCount; f++)
            {
                int v1 = g_Triangles[f].v1;
                int v2 = g_Triangles[f].v2;
                int v3 = g_Triangles[f].v3;
                g_Normals[f] = ComputeNormals(g_Verticies[v1],g_Verticies[v2],g_Verticies[v3]);
                if (!vlist[v1])
                {
                    g_VNormals[v1] = g_Normals[f];
                    vlist[v1] = true;
                }
                else
                {
                    g_VNormals[v1] += g_Normals[f];
                    g_VNormals[v1] /= 2;
                }
                if (!vlist[v2])
                {
                    g_VNormals[v2] = g_Normals[f];
                    vlist[v2] = true;
                }
                else
                {
                    g_VNormals[v2] += g_Normals[f];
                    g_VNormals[v2] /= 2;
                }
                if (!vlist[v3])
                {
                    g_VNormals[v3] = g_Normals[f];
                    vlist[v3] = true;
                }
                else
                {
                    g_VNormals[v3] += g_Normals[f];
                    g_VNormals[v3] /= 2;
                }
            }
        }
        break;

        case IDF_EXTEX:
            exportTexture();
            break;

        case IDF_EXMOD:
            exportModel();
            break;

        case IDF_IMMOD:
            importModel();
            break;

        case IDF_IMTEX:
            importTexture();
            break;

		case IDF_TOOLS:
			{
				ShowWindow(g_hTools, SW_SHOW);
			}
			break;

        case IDF_ANI:
        {
            ShowWindow(g_hAniDlg, SW_SHOW);
            if (Model.mAnimCount > 0)
            {
                unsigned int KPS,AniTime,FrameCount;

                KPS =           g_Animations[CUR_ANIM].KPS;
                FrameCount =    g_Animations[CUR_ANIM].FrameCount;
				if ( KPS > 0 )
                AniTime =       (FrameCount * 1000) / KPS;

                SetDlgItemText(g_hAniDlg, ID_NAME, g_Animations[CUR_ANIM].Name);
                SetDlgItemInt(g_hAniDlg, ID_AKPS, g_Animations[CUR_ANIM].KPS, FALSE);
                SetDlgItemInt(g_hAniDlg, ID_ATIME, AniTime, FALSE);
                SetDlgItemInt(g_hAniDlg, ID_AFRM, g_Animations[CUR_ANIM].FrameCount, FALSE);
            }
        }
        break;

        case IDF_SND:
        {
            ShowWindow(g_hSndDlg, SW_SHOW);
            sprintf(str,"%d Kbs", (int)g_Sounds[Snd].len/1000);

            SetDlgItemText(g_hSndDlg, ID_NAME, g_Sounds[Snd].name);
            SetDlgItemText(g_hSndDlg, ID_SIZE, str);
        }
        break;

        case IDF_PROPERTIES:
        {
            char sz[256];
            ShowWindow(g_hCarDlg, SW_SHOW);

            SetDlgItemText(g_hCarDlg, ID_NAME, Model.mName);
            sprintf(sz, "%d", Model.mObjInfo.Radius);
            SetDlgItemText(g_hCarDlg, ID_O_RADIUS, sz);
            sprintf(sz, "%d", Model.mObjInfo.YLo);
            SetDlgItemText(g_hCarDlg, ID_O_YLO, sz);
            sprintf(sz, "%d", Model.mObjInfo.YHi);
            SetDlgItemText(g_hCarDlg, ID_O_YHI, sz);
        }
        break;

        case IDB_SPLAY:
        {
            PlayWave(&g_Sounds[Snd]);
        }
        break;



		case IDV_PERSPECTIVE:
			CameraView = VIEW_PERSPECTIVE;
			break;
		case IDV_FIRSTPERSON:
			CameraView = VIEW_FIRSTPERSON;
			break;
		case IDV_PERSPECTIVE+1:
			CameraView = VIEW_LEFT;
			break;
		case IDV_PERSPECTIVE+2:
			CameraView = VIEW_RIGHT;
			break;
		case IDV_PERSPECTIVE+3:
			CameraView = VIEW_FRONT;
			break;
		case IDV_PERSPECTIVE+4:
			CameraView = VIEW_BACK;
			break;
		case IDV_PERSPECTIVE+5:
			CameraView = VIEW_TOP;
			break;
		case IDV_PERSPECTIVE+6:
			CameraView = VIEW_BOTTOM;
			break;

        case ID_ANIM_PLAY:
        {
            ANIMPLAY = TRUE;
        }
        break;

        case ID_ANIM_PAUSE:
        {
            ANIMPLAY = FALSE;
        }
        break;

        break;
        }
    }
    break;

    case WM_MOUSEWHEEL:
    {
        float zDelta = (float)(GET_WHEEL_DELTA_WPARAM(wParam)) / (float)(WHEEL_DELTA);
        cam.dist -= zDelta;
    }
    break;

    case WM_KEYDOWN:
    {
        key[wParam]=TRUE;

        switch (wParam)
        {
        case VK_ESCAPE:
            PostQuitMessage(0);
            break;
        }
    }
    break;

    case WM_KEYUP:
        key[wParam]=FALSE;
        break;

    case WM_DROPFILES:
    {
        HDROP drop = (HDROP) wParam;
        char file[256];

        for (UINT i=0; i<DragQueryFile(drop, 0xFFFFFFFF, file, 256); i++)
        {
            if (!DragQueryFile(drop, i, file, 256)) continue;
           MessageBox(hwnd, file, "Dropped", MB_OK);
        }

        DragQueryFile(drop, 0, file, 256);

        DragFinish(drop);

        //loadCAR(file);
    }
    break;

    case WM_LBUTTONDOWN:
        mouse[0]=TRUE;
        break;
    case WM_LBUTTONUP:
        mouse[0]=FALSE;
        break;
    case WM_MBUTTONDOWN:
        mouse[1]=TRUE;
        break;
    case WM_MBUTTONUP:
        mouse[1]=FALSE;
        break;
    case WM_RBUTTONDOWN:
        mouse[2]=TRUE;
        break;
    case WM_RBUTTONUP:
        mouse[2]=FALSE;
        break;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

INT_PTR CALLBACK AniDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch (Message)
    {
        case WM_INITDIALOG:
        {

        }
        break;

        case WM_SHOWWINDOW:
        {
			HWND sndList = GetDlgItem( hwnd, IDCB_SOUND);
			
			ComboBox_ResetContent( sndList );

			for ( unsigned s=0; s<Model.mSoundCount; s++ )
			{
				SendMessage( sndList, CB_ADDSTRING, (WPARAM)0, (LPARAM)g_Sounds[s].name );
			}

			SendMessage( sndList, CB_SETCURSEL , (WPARAM)Model.mAnimSFXTable[CUR_ANIM], (LPARAM)0 );
        }
        break;

    case WM_CLOSE:
        ShowWindow(g_hAniDlg, SW_HIDE);
        break;

        case WM_NOTIFY:
        {

        }
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDB_CLOSE:
        {
			// Apply the AnimSFX table update
			Model.mAnimSFXTable[CUR_ANIM] = ComboBox_GetCurSel( GetDlgItem( hwnd, IDCB_SOUND) );
			// Apply the Animation name
            GetDlgItemText(g_hAniDlg, ID_NAME, g_Animations[CUR_ANIM].Name, 32);
			// Apply the Animation KeysPerSecond
            g_Animations[CUR_ANIM].KPS = GetDlgItemInt(g_hAniDlg, ID_AKPS, NULL, FALSE);
			// Hide the window
            ShowWindow(g_hAniDlg, SW_HIDE);
        }
        break;

        case IDB_SAVE:
        {
            if (!SaveFileDlg("Vertex Table (*.vtl)\0*.vtl\0", "vtl"))
                break;

            FILE *afp;
            afp = fopen(fileName,"wb");
            if (afp==NULL)
                break;
            fwrite(&Model.mVertCount,1,4,afp);
            fwrite(&g_Animations[CUR_ANIM].KPS,1,4,afp);
            fwrite(&g_Animations[CUR_ANIM].FrameCount,1,4,afp);
            fwrite( g_Animations[CUR_ANIM].Data, Model.mVertCount * g_Animations[CUR_ANIM].FrameCount * 3, sizeof(short), afp );
            fclose(afp);
        }
        break;

        case IDB_OPEN:
        {
            if (!OpenFileDlg("Vertex Table (*.vtl)\0*.vtl\0Animation (*.ani)\0*.ani\0", "vtl"))
                break;

            FILE *afp;

            if ( (afp = fopen(fileName,"rb") ) == NULL )
			{
				MessageBox( hwnd, "Failed to open the file for reading.\r\nDoes it exist?", "File Error", MB_ICONWARNING );
                break;
			}

			if ( strstr( strlwr( fileName ), ".ani" ) != NULL )
			{
				long mVertCount = 0;
				fread(&g_Animations[CUR_ANIM].KPS,1,4,afp);
				fread(&g_Animations[CUR_ANIM].FrameCount,1,4,afp);
				fread(&mVertCount,1,4,afp);
				if (mVertCount != Model.mVertCount)
				{
					MessageBox( hwnd, "The vertex count in the animation does not match the model.", "File Error", MB_ICONWARNING );
					fclose(afp);
					break;
				}
				if ( g_Animations[CUR_ANIM].Data ) delete [] g_Animations[CUR_ANIM].Data;
				g_Animations[CUR_ANIM].Data = new short [ Model.mVertCount * g_Animations[CUR_ANIM].FrameCount * 3 ];
				fread( g_Animations[CUR_ANIM].Data, Model.mVertCount * g_Animations[CUR_ANIM].FrameCount * 3, sizeof(uint16_t), afp );
			}
			else if ( strstr( strlwr( fileName ), ".vtl" ) != NULL )
			{
				long mVertCount = 0;
				fread(&mVertCount,1,4,afp);
				if (mVertCount != Model.mVertCount)
				{
					MessageBox( hwnd, "The vertex count in the animation does not match the model.", "File Error", MB_ICONWARNING );
					fclose(afp);
					break;
				}
				fread(&g_Animations[CUR_ANIM].KPS,1,4,afp);
				fread(&g_Animations[CUR_ANIM].FrameCount,1,4,afp);
				if ( g_Animations[CUR_ANIM].Data ) delete [] g_Animations[CUR_ANIM].Data;
				g_Animations[CUR_ANIM].Data = new short [ Model.mVertCount * g_Animations[CUR_ANIM].FrameCount * 3 ];
				fread( g_Animations[CUR_ANIM].Data, Model.mVertCount * g_Animations[CUR_ANIM].FrameCount * 3, sizeof(uint16_t), afp );
			}
            
            fclose(afp);

            unsigned int time = (g_Animations[CUR_ANIM].FrameCount * 1000) / g_Animations[CUR_ANIM].KPS;

            SetDlgItemText(g_hAniDlg, ID_NAME, g_Animations[CUR_ANIM].Name);
            SetDlgItemInt(g_hAniDlg, ID_AKPS, g_Animations[CUR_ANIM].KPS, FALSE);
            SetDlgItemInt(g_hAniDlg, ID_ATIME, time, FALSE);
            SetDlgItemInt(g_hAniDlg, ID_AFRM, g_Animations[CUR_ANIM].FrameCount, FALSE);
        }
        break;

        case IDB_NEXT:
        {
            if (Ani<Model.mAnimCount-1)
            {
                Ani++;

                unsigned int div,time,frames;

                //-- Get the information
                GetDlgItemText(g_hAniDlg, ID_NAME, g_Animations[Ani-1].Name, 32);
                g_Animations[Ani-1].KPS = GetDlgItemInt(g_hAniDlg, ID_AKPS, NULL, FALSE);

                div = g_Animations[CUR_ANIM].KPS;
                frames = g_Animations[CUR_ANIM].FrameCount;
                time = (frames * 1000) / div;

                SetDlgItemText(g_hAniDlg, ID_NAME, g_Animations[CUR_ANIM].Name);
                SetDlgItemInt(g_hAniDlg, ID_AKPS, div, FALSE);
                SetDlgItemInt(g_hAniDlg, ID_ATIME, time, FALSE);
                SetDlgItemInt(g_hAniDlg, ID_AFRM, frames, FALSE);

                if (Ani==Model.mAnimCount-1)
                {
                    SetDlgItemText(g_hAniDlg, IDB_NEXT, "Add");
                }
            }
            else
            {
                //Add Animation
                Model.mAnimCount++;
                Ani++;

                sprintf(g_Animations[CUR_ANIM].Name,"New_Anim");
                g_Animations[CUR_ANIM].KPS = 0;//KPS
                g_Animations[CUR_ANIM].FrameCount = 0;

                SetDlgItemText(g_hAniDlg, ID_NAME, g_Animations[CUR_ANIM].Name);
                SetDlgItemInt(g_hAniDlg, ID_AKPS, 0, FALSE);
                SetDlgItemInt(g_hAniDlg, ID_ATIME, 0, FALSE);
                SetDlgItemInt(g_hAniDlg, ID_AFRM, 0, FALSE);

            }
        }
        break;

        case IDB_PREV:
            if (Ani>0)
            {
                if (Ani<Model.mAnimCount-1)
                {
                    SetDlgItemText(g_hAniDlg, IDB_NEXT, "Next");
                }

                Ani-=1;

                unsigned int div,time,frames;
                div = g_Animations[CUR_ANIM].KPS;
                frames = g_Animations[CUR_ANIM].FrameCount;
                time = (frames * 1000) / div;

                SetDlgItemText(g_hAniDlg, ID_NAME, g_Animations[CUR_ANIM].Name);
                //SetDlgItemText(g_hAniDlg, ID_NAME, "rap_run1");
                SetDlgItemInt(g_hAniDlg, ID_AKPS, div, FALSE);
                SetDlgItemInt(g_hAniDlg, ID_ATIME, time, FALSE);
                SetDlgItemInt(g_hAniDlg, ID_AFRM, frames, FALSE);
            }
            break;
        }
        break;

    default:
        return FALSE;
    }
    return TRUE;
}


INT_PTR CALLBACK SndDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch (Message)
    {
    case WM_CLOSE:
        ShowWindow(g_hSndDlg, SW_HIDE);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDB_CLOSE:
        {
            GetDlgItemText(g_hSndDlg, ID_NAME, g_Sounds[Snd].name, 32);
            ShowWindow(g_hSndDlg, SW_HIDE);
        }
        break;

        case IDB_OPEN:
        {
            importSound();
        }
        break;

        case IDB_SAVE:
        {
            exportSound();
        }
        break;

        case IDB_NEXT:
            if (Snd<Model.mSoundCount-1)
            {
                GetDlgItemText(g_hSndDlg, ID_NAME, g_Sounds[Snd].name, 32);
                Snd+=1;

                sprintf(str,"%d Kbs", (int)g_Sounds[Snd].len/1000);

                SetDlgItemText(g_hSndDlg, ID_NAME, g_Sounds[Snd].name);
                SetDlgItemText(g_hSndDlg, ID_SIZE, str);

                if (Snd==Model.mSoundCount-1)
                {
                    SetDlgItemText(g_hAniDlg, IDB_NEXT, "Add");
                }
            }
            else
            {
                //Add Animation
                Model.mSoundCount++;
                Snd++;

                sprintf(g_Sounds[Snd].name,"New_Sound");
                sprintf(str,"%d Kbs", (int)0);
                SetDlgItemText(g_hSndDlg, ID_NAME, g_Sounds[Snd].name);
                SetDlgItemText(g_hSndDlg, ID_SIZE, str);
            }
            break;

        case IDB_PREV:
            if (Snd>0)
            {
                GetDlgItemText(g_hSndDlg, ID_NAME, g_Sounds[Snd].name, 32);
                Snd-=1;

                sprintf(str,"%d Kbs", (int)g_Sounds[Snd].len/1000);

                SetDlgItemText(g_hSndDlg, ID_NAME, g_Sounds[Snd].name);
                SetDlgItemText(g_hSndDlg, ID_SIZE, str);
            }
            break;

        case IDB_SPLAY:
        {
            PlayWave(&g_Sounds[Snd]);
        }
        break;
        }
        break;

    default:
        return FALSE;
    }
    return TRUE;
}

INT_PTR CALLBACK WinToolsProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch (Message)
    {
	case WM_INITDIALOG:
		{
			RECT rc;
			GetClientRect( hwnd, &rc );

			HINSTANCE hInst = GetModuleHandle( 0 );
			HWND hw = CreateWindow( WC_BUTTON, "Apply", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 0,0, 80,20, hwnd, (HMENU)IDB_APPLY, hInst, 0 );
			SendMessage( hw, WM_ENABLE, (WPARAM)TRUE, 0 );
			SendMessage( hw, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));
			MoveWindow( hw, rc.left + (rc.right/3), rc.bottom - 26, rc.right/3, 24, TRUE );

			hw = CreateWindow( WC_BUTTON, "Two Sided", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 5, 30, 200, 20, hwnd, (HMENU)IDC_FLAG1, hInst, 0 );
			SendMessage( hw, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));
			hw = CreateWindow( WC_BUTTON, "Dark Back", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 5, 50, 200, 20, hwnd, (HMENU)IDC_FLAG2, hInst, 0 );
			SendMessage( hw, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));
			hw = CreateWindow( WC_BUTTON, "Opacity", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 5, 70, 200, 20, hwnd, (HMENU)IDC_FLAG3, hInst, 0 );
			SendMessage( hw, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));
			hw = CreateWindow( WC_BUTTON, "Transparent", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 5, 90, 200, 20, hwnd, (HMENU)IDC_FLAG4, hInst, 0 );
			SendMessage( hw, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));
			hw = CreateWindow( WC_BUTTON, "Mortal", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 5,110, 200, 20, hwnd, (HMENU)IDC_FLAG5, hInst, 0 );
			SendMessage( hw, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));
			hw = CreateWindow( WC_BUTTON, "Specular", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 5,130, 200, 20, hwnd, (HMENU)IDC_FLAG6, hInst, 0 );
			SendMessage( hw, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));
			hw = CreateWindow( WC_BUTTON, "EnvMap", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 5,150, 200, 20, hwnd, (HMENU)IDC_FLAG7, hInst, 0 );
			SendMessage( hw, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));
		}
		break;

    case WM_CLOSE:
		{
			if ( g_TriSelection.size() >=1 )
			SendMessage( hwnd, WM_COMMAND, (IDB_APPLY<<16) | (IDB_APPLY), (LPARAM)GetDlgItem( hwnd, IDB_APPLY) );

			ShowWindow(hwnd, SW_HIDE);
		}
		break;

	case WM_NOTIFY:
		{
			LPNMHDR nh = ((LPNMHDR)lParam);
			switch( nh->code )
			{
			case NM_CLICK:
				printf( "BN_CLICKED %u\n",nh->idFrom ); break;
			}
		}
		break;

    case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDB_APPLY:
				{
					printf( "Face Flags applied.\n" );
					// Apply the new settings
					WORD flags = 0;

					if ( IsDlgButtonChecked( hwnd, IDC_FLAG1 ) == BST_CHECKED )
						flags |= 1;
					if ( IsDlgButtonChecked( hwnd, IDC_FLAG2 ) == BST_CHECKED )
						flags |= 2;
					if ( IsDlgButtonChecked( hwnd, IDC_FLAG3 ) == BST_CHECKED )
						flags |= 4;
					if ( IsDlgButtonChecked( hwnd, IDC_FLAG4 ) == BST_CHECKED )
						flags |= 8;
					if ( IsDlgButtonChecked( hwnd, IDC_FLAG5 ) == BST_CHECKED )
						flags |= 16;
					if ( IsDlgButtonChecked( hwnd, IDC_FLAG6 ) == BST_CHECKED )
						flags |= 32;
					if ( IsDlgButtonChecked( hwnd, IDC_FLAG7 ) == BST_CHECKED )
						flags |= 64;
					if ( IsDlgButtonChecked( hwnd, IDC_FLAG8 ) == BST_CHECKED )
						flags |= 128;
					if ( IsDlgButtonChecked( hwnd, IDC_FLAG9 ) == BST_CHECKED )
						flags |= 256;
					if ( IsDlgButtonChecked( hwnd, IDC_FLAG10 ) == BST_CHECKED )
						flags |= 512;
					if ( IsDlgButtonChecked( hwnd, IDC_FLAG11 ) == BST_CHECKED )
						flags |= 1024;
					if ( IsDlgButtonChecked( hwnd, IDC_FLAG12 ) == BST_CHECKED )
						flags |= 2048;
					if ( IsDlgButtonChecked( hwnd, IDC_FLAG13 ) == BST_CHECKED )
						flags |= 4096;
					if ( IsDlgButtonChecked( hwnd, IDC_FLAG14 ) == BST_CHECKED )
						flags |= 8192;
					if ( IsDlgButtonChecked( hwnd, IDC_FLAG15 ) == BST_CHECKED )
						flags |= 16384;
					if ( IsDlgButtonChecked( hwnd, IDC_FLAG16 ) == BST_CHECKED )
						flags |= 32768;

					for ( unsigned i=0; i<g_TriSelection.size(); i++ )
					{
						unsigned f = g_TriSelection[i];
						g_Triangles[f].flags = flags;
					}
				}
				break;
			}
		}
        break;

    default:
        return FALSE;
    }
    return TRUE;
}

INT_PTR CALLBACK CarDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch (Message)
    {
    case WM_CLOSE:
    {
        ShowWindow(g_hCarDlg, SW_HIDE);
        GetDlgItemText(g_hCarDlg, ID_NAME, Model.mName, 24);
    }
    break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
            /*case IDB_CLOSE:
            {
                GetDlgItemText(g_hSndDlg, ID_NAME, g_Sounds[Snd].name, 32);
                ShowWindow(g_hSndDlg, SW_HIDE);
            }
            break;*/
		default:
			break;
        }
        break;

    default:
        return FALSE;
    }
    return TRUE;
}

INT_PTR CALLBACK AboutDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch (Message)
    {
	case WM_INITDIALOG:
		{
			HWND hcontrol = GetDlgItem( hwnd, ID_ABOUT_TEXT );
			
			// set the about text
			sprintf( str, "C3Dit (Designer 3 - D3)\r\nCarnivores 3D Editor\r\nVersion: %u.%u.%u\r\nBy James Ogden\r\nContact: rexhunter99@gmail.com", VERSION_MAJOR, VERSION_MINOR, VERSION_RELEASE );
			SetWindowText( hcontrol, str );

			// Resize and position the control
			MoveWindow( hwnd, (GetSystemMetrics(SM_CXSCREEN)/2)-180,(GetSystemMetrics(SM_CYSCREEN)/2)-72, 360,144, true );

			RECT rc;
			GetClientRect( hwnd, &rc );

			MoveWindow( hcontrol, rc.left+96,rc.top+5,rc.right-10,rc.bottom-24-5, true );

			hcontrol = GetDlgItem( hwnd, ID_ABOUT_OK );

			MoveWindow( hcontrol, (rc.right/2)-40,rc.bottom-22, 80,20, true );
		}
		break;

	case WM_SHOWWINDOW:
		{
			// Resize and position the control
			MoveWindow( hwnd, (GetSystemMetrics(SM_CXSCREEN)/2)-180,(GetSystemMetrics(SM_CYSCREEN)/2)-72, 360,144, true );
		}
		break;

    case WM_CLOSE:
		{
			ShowWindow(hwnd, SW_HIDE);
		}
		break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
            case ID_ABOUT_OK:
            {
                ShowWindow(hwnd, SW_HIDE);
            }
            break;
		default:
			break;
        }
        break;

    default:
        return FALSE;
    }
    return TRUE;
}



void LoadCARData(char* fname)
{
    FILE *fp = fopen(fname,"rb");
    if (fp==NULL)
    {
        MessageBox(NULL, "There was an error when opening the file.",
                   "FILE ERROR",
                   MB_ICONERROR | MB_OK);
    }
    ISCAR = TRUE;
    ISC2O = FALSE;

    newScene();

    //--Header
    fread(Model.mName,1,24,fp);
    fseek(fp,24,SEEK_SET);
    fread(Model.mTag,1,8,fp);
    fseek(fp,32,SEEK_SET);
    fread(&Model.mAnimCount,1,4,fp);
    fread(&Model.mSoundCount,1,4,fp);
    fread(&Model.mVertCount,1,4,fp);
    fread(&Model.mFaceCount,1,4,fp);
    fread(&Model.mTextureSize,1,4,fp);

    //Triangles
    for (int t=0; t<Model.mFaceCount; t++)
    {
        fread(&g_Triangles[t],64,1,fp);

        char str[256];
        sprintf(str,"Triangles #%d",t+1);
        g_TVItems.push_back(TreeView_AddResource(g_FileView, rootNodes[TV_TRIANGLES], str, LPARAM( &g_Triangles[t] ) ));
    }

    //Verticies
    float hiZ=0.0f;
    for (int v=0; v<Model.mVertCount; v++)
    {
        fread(&g_Verticies[v],16,1,fp);

        char str[256];
        sprintf(str,"Vertices #%d",v+1);
        g_TVItems.push_back(TreeView_AddResource(g_FileView, rootNodes[TV_VERTICES], str, LPARAM( &g_Verticies[v] ) ));

        if (g_Verticies[v].y>hiZ)
            hiZ = g_Verticies[v].y;
    }

    bool vlist[ MAX_VERTICES ];
    ZeroMemory(vlist,sizeof(bool)*MAX_VERTICES);
    for (int f=0; f<Model.mFaceCount; f++)
    {
        int v1 = g_Triangles[f].v1;
        int v2 = g_Triangles[f].v2;
        int v3 = g_Triangles[f].v3;
        g_Normals[f] = ComputeNormals(g_Verticies[v1],g_Verticies[v2],g_Verticies[v3]);
        if (!vlist[v1])
        {
            g_VNormals[v1] = g_Normals[f];
            vlist[v1] = true;
        }
        else
        {
            g_VNormals[v1] += g_Normals[f];
            g_VNormals[v1] /= 2;
        }
        if (!vlist[v2])
        {
            g_VNormals[v2] = g_Normals[f];
            vlist[v2] = true;
        }
        else
        {
            g_VNormals[v2] += g_Normals[f];
            g_VNormals[v2] /= 2;
        }
        if (!vlist[v3])
        {
            g_VNormals[v3] = g_Normals[f];
            vlist[v3] = true;
        }
        else
        {
            g_VNormals[v3] += g_Normals[f];
            g_VNormals[v3] /= 2;
        }
    }

    // Set the camera to look at the middle of the model
    cam.xt = 0.0f;
    cam.yt = 0.0f;
    cam.zt = (hiZ/10.0f)/2.0f;

    //Texture
	delete [] Model.mTexture;
	Model.mTexture = 0;
	Model.mTexture = new uint8_t [ Model.mTextureSize ];
    fread( Model.mTexture, Model.mTextureSize,1, fp );
	Model.FindTextureDimensions();
	Model.ApplyTextureOpacity();

    g_TVItems.push_back(TreeView_AddResource(g_FileView, rootNodes[TV_TEXTURES], "Diffuse", LPARAM( Model.mTexture ) ));

    glBindTexture(GL_TEXTURE_2D, g_TextureID[0]);
    glTexEnvi(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	if ( Model.mTextureColor == 16 )
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB5_A1, Model.mTextureWidth, Model.mTextureHeight, 0, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, Model.mTexture );
	if ( Model.mTextureColor == 24 )
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, Model.mTextureWidth, Model.mTextureHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, Model.mTexture );
	if ( Model.mTextureColor == 32 )
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, Model.mTextureWidth, Model.mTextureHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, Model.mTexture );

    //Animations
    for (int a=0; a<Model.mAnimCount; a++)
    {
        fread(&g_Animations[a].Name,32,1,fp);
        fread(&g_Animations[a].KPS,1,4,fp);
        fread(&g_Animations[a].FrameCount,1,4,fp);

		if ( g_Animations[a].Data ) delete [] g_Animations[a].Data;
        g_Animations[a].Data = new short[Model.mVertCount * g_Animations[a].FrameCount * 3];
        fread( g_Animations[a].Data, Model.mVertCount * g_Animations[a].FrameCount * 3, sizeof(short), fp );

        g_TVItems.push_back(TreeView_AddResource(g_FileView, rootNodes[2], g_Animations[a].Name, LPARAM( &g_Animations[a] ) ));
    }
    SendMessage(g_AniTrack, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(0,g_Animations[0].FrameCount-1) );
    SendMessage(g_AniTrack, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)0 );

    //Sound effects
    for (int s=0; s<Model.mSoundCount; s++)
    {
        fread(&g_Sounds[s].name,32,1,fp);
        fread(&g_Sounds[s].len,1,4,fp);

        if (g_Sounds[s].data) delete [] g_Sounds[s].data;
        g_Sounds[s].data = new BYTE[g_Sounds[s].len];

        fread(g_Sounds[s].data,g_Sounds[s].len,1,fp);
        g_TVItems.push_back(TreeView_AddResource(g_FileView, rootNodes[3], g_Sounds[s].name, LPARAM( &g_Sounds[s] ) ));
    }

    //Animation-Sound Table
    fread(&Model.mAnimSFXTable,64,4,fp);

    // Set Status bar
    char sstr[64];
    sprintf(sstr,"Triangles: %u", (UINT)Model.mFaceCount);
    SendMessage(gHStatus, SB_SETTEXT, 0, (LPARAM)sstr);

    // Enable Menu items
    /*HMENU hMenu, hResMenu;
    hMenu = GetMenu(g_hMain);
    hResMenu = GetSubMenu(hMenu, 1);
    EnableMenuItem(hResMenu, IDF_IMTEX, MF_BYCOMMAND | (TRUE ? MF_ENABLED : MF_GRAYED));
    EnableMenuItem(hResMenu, IDF_IMMOD, MF_BYCOMMAND | (TRUE ? MF_ENABLED : MF_GRAYED));
    EnableMenuItem(hResMenu, IDF_EXTEX, MF_BYCOMMAND | (TRUE ? MF_ENABLED : MF_GRAYED));
    EnableMenuItem(hResMenu, IDF_EXMOD, MF_BYCOMMAND | (TRUE ? MF_ENABLED : MF_GRAYED));
    EnableMenuItem(hResMenu, IDF_ANI, MF_BYCOMMAND | (TRUE ? MF_ENABLED : MF_GRAYED));
    EnableMenuItem(hResMenu, IDF_SND, MF_BYCOMMAND | (TRUE ? MF_ENABLED : MF_GRAYED));
    EnableMenuItem(hResMenu, IDF_PROPERTIES, MF_BYCOMMAND | (TRUE ? MF_ENABLED : MF_GRAYED));
    hResMenu = GetSubMenu(hMenu, 0);
    EnableMenuItem(hResMenu, IDF_SAVEAS, MF_BYCOMMAND | (TRUE ? MF_ENABLED : MF_GRAYED));*/

	UpdateWindow(g_FileView);
    UpdateWindow(g_hMain);
}



void LoadC2OData(char *fname)
{
    FILE *fp = fopen(fname,"rb");
    if (fp==NULL)
    {
        MessageBox(NULL, "There was an error when opening the file.",
                   "FILE ERROR",
                   MB_ICONERROR | MB_OK);
    }

    ISCAR = TRUE;
    ISC2O = FALSE;

    //--Header
    sprintf(Model.mName, "<Object>");
    fread(&Model.mObjInfo,1,48,fp);
    fread(&Model.mObjInfo.res,1,16,fp);
    fread(&Model.mVertCount,1,4,fp);
    fread(&Model.mFaceCount ,1,4,fp);
    fread(&Model.mBoneCount,1,4,fp);
    fread(&Model.mTextureSize,1,4,fp);

    //Triangles
    for (int t=0; t<Model.mFaceCount; t++)
    {
        fread(&g_Triangles[t],64,1,fp);

        char str[256];
        sprintf(str,"Triangles #%d",t+1);
        g_TVItems.push_back(TreeView_AddResource(g_FileView, rootNodes[TV_TRIANGLES], str, LPARAM( &g_Triangles[t] ) ));
    }

    //Verticies
    float hiZ=0.0f;
    for (int v=0; v<Model.mVertCount; v++)
    {
        fread(&g_Verticies[v],sizeof(VERTEX),1,fp);

        char str[256];
        sprintf(str,"Vertices #%d",v+1);
        g_TVItems.push_back(TreeView_AddResource(g_FileView, rootNodes[TV_VERTICES], str, LPARAM( &g_Verticies[v] ) ));

        if (g_Verticies[v].y>hiZ)
            hiZ = g_Verticies[v].y;
    }

    // Normals
    bool vlist[ MAX_VERTICES ];
    ZeroMemory(vlist,sizeof(bool));
    for (int f=0; f<Model.mFaceCount; f++)
    {
        int v1 = g_Triangles[f].v1;
        int v2 = g_Triangles[f].v2;
        int v3 = g_Triangles[f].v3;
        g_Normals[f] = ComputeNormals(g_Verticies[v1],g_Verticies[v2],g_Verticies[v3]);
        if (!vlist[v1])
        {
            g_VNormals[v1] = g_Normals[f];
            vlist[v1] = true;
        }
        else
        {
            g_VNormals[v1] += g_Normals[f];
            g_VNormals[v1] /= 2;
        }
        if (!vlist[v2])
        {
            g_VNormals[v2] = g_Normals[f];
            vlist[v2] = true;
        }
        else
        {
            g_VNormals[v2] += g_Normals[f];
            g_VNormals[v2] /= 2;
        }
        if (!vlist[v3])
        {
            g_VNormals[v3] = g_Normals[f];
            vlist[v3] = true;
        }
        else
        {
            g_VNormals[v3] += g_Normals[f];
            g_VNormals[v3] /= 2;
        }
    }

    // Set the camera to look at the middle of the model
    cam.xt = 0.0f;
    cam.yt = 0.0f;
    cam.zt = (hiZ/10.0f)/2.0f;

    //Bones
    for (int b=0; b<Model.mBoneCount; b++)
    {
        fread(&g_Bones[b],sizeof(BONE),1,fp);

        char str[256];
        //sprintf(str,"Bone #%d",b+1);
        sprintf(str,"%s",g_Bones[b].name);
        g_TVItems.push_back(TreeView_AddResource(g_FileView, rootNodes[TV_BONES], str, LPARAM( &g_Bones[b] ) ));
    }

    //Texture
	delete [] Model.mTexture;
	Model.mTexture = 0;
	Model.mTexture = new char [ Model.mTextureSize ];
    fread( Model.mTexture, Model.mTextureSize,1, fp );
	Model.FindTextureDimensions();
	Model.ApplyTextureOpacity();

    g_TVItems.push_back(TreeView_AddResource(g_FileView, rootNodes[TV_TEXTURES], "Diffuse", LPARAM( Model.mTexture ) ));

    glBindTexture(GL_TEXTURE_2D, g_TextureID[0]);
    glTexEnvi(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	if ( Model.mTextureColor == 16 )
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB5_A1, Model.mTextureWidth, Model.mTextureHeight, 0, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, Model.mTexture );
	if ( Model.mTextureColor == 32 )
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, Model.mTextureWidth, Model.mTextureHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, Model.mTexture );

    //Sprite
    fread(&g_Sprite,128*128,2,fp);

    //Animations
    /*
        We can skip these for now since Objects don't
        have animations (though they support having animations)
    */

    fclose(fp);

    // Set Status bar
    char sstr[64];
    sprintf(sstr,"Triangles: %u", (UINT)Model.mFaceCount);
    SendMessage(gHStatus, SB_SETTEXT, 0, (LPARAM)sstr);

    // Enable Menu items
    /*HMENU hMenu, hResMenu;
    hMenu = GetMenu(g_hMain);
    hResMenu = GetSubMenu(hMenu, 1);
    EnableMenuItem(hResMenu, IDF_IMTEX, MF_BYCOMMAND | (TRUE ? MF_ENABLED : MF_GRAYED));
    EnableMenuItem(hResMenu, IDF_IMMOD, MF_BYCOMMAND | (TRUE ? MF_ENABLED : MF_GRAYED));
    EnableMenuItem(hResMenu, IDF_EXTEX, MF_BYCOMMAND | (TRUE ? MF_ENABLED : MF_GRAYED));
    EnableMenuItem(hResMenu, IDF_EXMOD, MF_BYCOMMAND | (TRUE ? MF_ENABLED : MF_GRAYED));
    //Should check if the object has an animation :S
    if (Model.oInfo.flags & ofANIMATED)
        EnableMenuItem(hResMenu, IDF_ANI, MF_BYCOMMAND | (TRUE ? MF_ENABLED : MF_GRAYED));
    else
        EnableMenuItem(hResMenu, IDF_ANI, MF_BYCOMMAND | (FALSE ? MF_DISABLED : MF_GRAYED));
    EnableMenuItem(hResMenu, IDF_SND, MF_BYCOMMAND | (FALSE ? MF_DISABLED : MF_GRAYED));
    EnableMenuItem(hResMenu, IDF_PROPERTIES, MF_BYCOMMAND | (TRUE ? MF_ENABLED : MF_GRAYED));

    hResMenu = GetSubMenu(hMenu, 0);
    EnableMenuItem(hResMenu, IDF_SAVEAS, MF_BYCOMMAND | (TRUE ? MF_ENABLED : MF_GRAYED));*/
}


void newScene()
{
    //--Empty TreeView
    for (UINT i=0; i<g_TVItems.size(); i++)
    {
        TreeView_DeleteItem(g_FileView, g_TVItems[i]);
    }
    g_TVItems.clear();

    //Empty the texture
	delete [] Model.mTexture;
	Model.mTexture = 0;
	Model.mTextureSize = 0;

	// -- Free and empty animations
	for ( int i=0; i<Model.mAnimCount; i++ )
	{
		if ( g_Animations[i].Data ) delete [] g_Animations[i].Data;
		g_Animations[i].Data = 0;
		g_Animations[i].FrameCount = 0;
		g_Animations[i].KPS = 0;
		sprintf( g_Animations[i].Name, "" );
	}

    //Empty the vertices
    ZeroMemory(g_Verticies,sizeof(VERTEX)*MAX_VERTICES);

    //Empty the triangles
    ZeroMemory(g_Triangles,sizeof(TRIANGLE)*MAX_TRIANGLES);

    //Empty the header
    ZeroMemory(&Model,sizeof(TModel));

	ANIMPLAY = false;
    CUR_ANIM = 0;
    CUR_FRAME = 0;
    //CUR_SOUND = 0;

	SendMessage(gHStatus, SB_SETTEXT, 0, (LPARAM)"No Model!");
    SendMessage(gHStatus, SB_SETTEXT, 1, (LPARAM)statMessages[ rand() % 6 ].c_str());

    cam.x = -24;
    cam.y = -24;
    cam.z = 24;
    cam.xt = 0;
    cam.yt = 0;
    cam.zt = 0;
    cam.dist = 24;
    cam.yaw = 0;
    cam.pitch = 0;
}


void loadCAR()
{
    /* loadCAR()
    * Loads one of the supported geometry mediums to a blank scene.
    * Supports:
        CAR - Character Format
        C2O - Carnivores 2 Object Format
        3DF - Action Forms 3D File Format
        OBJ - Wavefront OBJ Format
    */
    if (!OpenFileDlg("All Supported (*.car;*.c2o;*.3df;*.obj;)\0*.car;*.c2o;*.3df;*.3dn;*.obj\0"
                     "Character files (*.car)\0*.car\0"
                     "Carnivores 2 Object (*.c2o)\0*.c2o\0"
                     "3D File (*.3df)\0*.3df\0"
					 "iOS/AOS 3D model (*.3dn)\0*.3dn\0"
                     "Wavefront OBJ File (.obj)\0*.obj\0", "CAR"))
    return;

    newScene();

    if (strstr(_strlwr(fileName),".car")!=NULL)
    {
        LoadCARData(fileName);
    }
	if (strstr(_strlwr(fileName),".3dn")!=NULL)
    {
        Load3DNData(fileName);
    }
    if (strstr(_strlwr(fileName),".c2o")!=NULL)
    {
        LoadC2OData(fileName);
    }
	if (strstr(_strlwr(fileName),".c1o")!=NULL)
    {
        //LoadC1OData(fileName);
    }
    if (strstr(_strlwr(fileName),".3df")!=NULL)
    {
        Load3DFData(fileName);
    }
    if (strstr(strlwr(fileName),".obj")!=NULL)
    {
        LoadOBJData(fileName);
    }

    // Set Status bar
    char sstr[64];
    sprintf(sstr,"Triangles: %u",(UINT)Model.mFaceCount);
    SendMessage(gHStatus, SB_SETTEXT, 0, (LPARAM)sstr);
    MessageBox(g_hMain,sstr,"triangles",MB_OK);
    return;
}

void loadCAR(char *fname)
{
    /* loadCAR()
    * Loads one of the supported geometry mediums to a blank scene.
    * Supports:
        CAR - Character Format
        C2O - Carnivores 2 Object Format
        3DF - Action Forms 3D File Format
        OBJ - Wavefront OBJ Format
    */

    newScene();

    if (strstr(_strlwr(fname),".car")!=NULL)
    {
        LoadCARData(fname);
    }
    if (strstr(_strlwr(fname),".c2o")!=NULL)
    {
        LoadC2OData(fname);
    }
    if (strstr(_strlwr(fname),".3df")!=NULL)
    {
        Load3DFData(fname);
    }
    if (strstr(_strlwr(fname),".obj")!=NULL)
    {
        LoadOBJData(fname);
    }

    // Set Status bar
    char sstr[64];
    sprintf(sstr,"Triangles: %u",(UINT)Model.mFaceCount);
    SendMessage(gHStatus, SB_SETTEXT, 0, (LPARAM)sstr);
    MessageBox(g_hMain,sstr,"triangles",MB_OK);
    return;
}

void SaveCARData(char *fname)
{
    FILE *fp = fopen(fname,"wb");

    // -- C3Dit watermark
    sprintf( Model.mTag, "C3DIT" );

    //--Header
    fwrite( Model.mName, 1,24,fp);
    fwrite( Model.mTag, 1,8,fp);
    fwrite( &Model.mAnimCount, 1,4, fp );
    fwrite( &Model.mSoundCount, 1,4, fp );
    fwrite( &Model.mVertCount, 1,4, fp );
    fwrite( &Model.mFaceCount, 1,4, fp );
	fwrite( &Model.mTextureSize, 1,4, fp );

    //Triangles
    for (int t=0; t<Model.mFaceCount; t++)
    {
        fwrite(&g_Triangles[t],64,1,fp);
    }

    //Verticies
    for (int v=0; v<Model.mVertCount; v++)
    {
        fwrite(&g_Verticies[v],16,1,fp);
    }

    //Texture
    for (int i=0; i<256*256; i++)
    {
        WORD w = *((WORD*)Model.mTexture + i);
        int B = ((w>> 0) & 31);
        int G = ((w>> 5) & 31);
        int R = ((w>>10) & 31);
        int A = 0;
        *((WORD*)Model.mTexture + i) = (B) + (G<<5) + (R<<10) + (A<<15);
    }
    fwrite( Model.mTexture, Model.mTextureSize,1, fp);

    //Animations
    for (int a=0; a<Model.mAnimCount; a++)
    {
        fwrite(&g_Animations[a].Name,32,1,fp);
        fwrite(&g_Animations[a].KPS,1,4,fp);
        fwrite(&g_Animations[a].FrameCount,1,4,fp);
        fwrite( g_Animations[a].Data, Model.mVertCount * g_Animations[a].FrameCount * 3, sizeof(short), fp );
    }

    //Sound effects
	for (int s=0; s<Model.mSoundCount; s++)
    {
        fwrite(&g_Sounds[s].name,32,1,fp);
        fwrite(&g_Sounds[s].len,1,4,fp);
        fwrite(g_Sounds[s].data,g_Sounds[s].len,1,fp);
    }

    //Animation-Sound Table
	fwrite(&Model.mAnimSFXTable,64,4,fp);

    //Close the file
    fclose(fp);
}


void SaveOBJData(char *fname)
{
    // -- Saves the mesh as a triangluated Wavefront OBJ
    FILE *fp = fopen(fname,"w");

    fprintf(fp, "# Created by C3Dit\n");
    fprintf(fp, "# C3Dit (c) Rexhunter99 2009\n");
    fprintf(fp, "# C3Dit - Wavefront OBJ Exporter Version: 1.0\n\n");

    fprintf(fp, "# Vertices: %u\n", (UINT)Model.mVertCount);
    fprintf(fp, "# Triangles: %u\n", (UINT)Model.mFaceCount);

    fprintf(fp, "\no %s\n\n", Model.mName);

    for (int v=0; v<Model.mVertCount; v++)
    {
        fprintf(fp, "v %f %f %f\n", g_Verticies[v].x, g_Verticies[v].z, g_Verticies[v].y );
    }

    for (int t=0; t<Model.mFaceCount; t++)
    {
        fprintf(fp, "vt %f %f 0.0\n",(float)g_Triangles[t].tx1/256.0f,(float)g_Triangles[t].ty1/256.0f);
        fprintf(fp, "vt %f %f 0.0\n",(float)g_Triangles[t].tx2/256.0f,(float)g_Triangles[t].ty2/256.0f);
        fprintf(fp, "vt %f %f 0.0\n",(float)g_Triangles[t].tx3/256.0f,(float)g_Triangles[t].ty3/256.0f);
    }

    for (int t=0; t<Model.mFaceCount; t++)
    {
        fprintf(fp, "f %u/%u %u/%u %u/%u\n",
                                        g_Triangles[t].v1+1, (t*3)+1,
                                        g_Triangles[t].v2+1, (t*3)+2,
                                        g_Triangles[t].v3+1, (t*3)+3 );
    }

    fclose(fp);

    return;
}

void LoadOBJData(char *fname)
{
    // -- Supports Triangulated (preferred) or quadulated geometry

    UINT num_texcoords = 0;
    vector<VERTEX> l_TexCoord;

    // -- Clear the scene
    newScene();

    FILE *fp = fopen(fname,"r");

    while (!feof(fp))
    {
        // # == comment
        // \n == newline
        // \r\n == newline
        // \0 == newline
        // v == vertex
        // vn == vertex normal
        // vt == vertex texture coord
        // f == face

        char str[256];
        fgets(str, 256, fp);
        // -- End Of Line reached

        char *tok = strtok( str, " " );
        //tok = strtok( NULL, " " );

        if (tok[0] == '#') continue;
        if (tok[0] == '\n') continue;
        // skip unused tokens
        if (tok[0] == 'o') continue;
        if (tok[0] == 's') continue;

        if (strcmp(tok, "mtllib")==0) continue;
        if (strcmp(tok, "usemtl")==0) continue;

        // Group
        if (tok[0] == 'g')
        {
            tok = strtok( NULL, " " );

            strncpy(Model.mName, tok, 24);
        }

        // Vertex
        if (tok[0] == 'v' && tok[1] == '\0')
        {
            if ( Model.mVertCount >= MAX_VERTICES ) continue;

            g_Verticies[Model.mVertCount].x = atof( tok = strtok( NULL, " " ) ) * 4.0f;
            g_Verticies[Model.mVertCount].y = atof( tok = strtok( NULL, " " ) ) * 4.0f;
            g_Verticies[Model.mVertCount].z = atof( tok = strtok( NULL, " " ) ) * 4.0f;
            char name[256];
            sprintf(name,"Vertices #%u", (UINT)Model.mVertCount);
            g_TVItems.push_back(TreeView_AddResource(g_FileView, rootNodes[TV_VERTICES], name, LPARAM( &g_Verticies[Model.mVertCount] ) ));
            Model.mVertCount++;
        }

        // Texture Coord
        if (tok[0] == 'v' && tok[1] == 't')
        {
            VERTEX Coords;
            Coords.x = atof( tok = strtok( NULL, " " ) );
            Coords.y = atof( tok = strtok( NULL, " " ) );
            l_TexCoord.push_back( Coords );

            num_texcoords++;
        }

        // Normal
        if (tok[0] == 'v' && tok[1] == 'n')
        {
            continue;
        }

        // Face
        if (tok[0] == 'f' && tok[1] == '\0')
        {
            if ( Model.mFaceCount >= MAX_TRIANGLES ) continue;
            char* slash;

            tok = strtok( NULL, " " );
			if ( tok )
			{
				g_Triangles[Model.mFaceCount].v1 = atoi(&tok[0]) - 1;

				if ( slash = strchr(tok, '/') )
				{
					g_Triangles[Model.mFaceCount].tx1 = l_TexCoord[atoi(&slash[1]) - 1].x * 256;
					g_Triangles[Model.mFaceCount].ty1 = l_TexCoord[atoi(&slash[1]) - 1].y * 256;
				}
			}

            tok = strtok( NULL, " " );
			if ( tok )
			{
				g_Triangles[Model.mFaceCount].v2 = atoi(&tok[0]) - 1;

				if ( slash = strchr(tok, '/') )
				{
					g_Triangles[Model.mFaceCount].tx2 = l_TexCoord[atoi(&slash[1]) - 1].x * 256;
					g_Triangles[Model.mFaceCount].ty2 = l_TexCoord[atoi(&slash[1]) - 1].y * 256;
				}
			}

            tok = strtok( NULL, " " );
			if ( tok )
			{
				g_Triangles[Model.mFaceCount].v3 = atoi(&tok[0]) - 1;

				if ( slash = strchr(tok, '/') )
				{
					g_Triangles[Model.mFaceCount].tx3 = l_TexCoord[atoi(&slash[1]) - 1].x * 256;
					g_Triangles[Model.mFaceCount].ty3 = l_TexCoord[atoi(&slash[1]) - 1].y * 256;
				}
			}

            // -- Add-To-TreeView
            char name[256];
            sprintf(name,"Triangles #%u", (UINT)Model.mFaceCount);
            g_TVItems.push_back(TreeView_AddResource(g_FileView, rootNodes[TV_TRIANGLES], name, LPARAM( &g_Triangles[Model.mFaceCount] ) ));
            Model.mFaceCount++;

			// -- Quad
            tok = strtok( NULL, " \r\n" );
            if ( tok != NULL )
            {
				TRIANGLE* prev = &g_Triangles[Model.mFaceCount-1];

                g_Triangles[Model.mFaceCount].v1 = prev->v3;
				g_Triangles[Model.mFaceCount].v2 = atoi(&tok[0]) - 1;
				g_Triangles[Model.mFaceCount].v3 = prev->v1;

                if ( slash = strchr(tok, '/') )
                {
                    g_Triangles[Model.mFaceCount].tx1 = prev->tx3;
                    g_Triangles[Model.mFaceCount].ty1 = prev->ty3;
                    g_Triangles[Model.mFaceCount].tx2 = l_TexCoord[atoi(&slash[1]) - 1].x * 256;
                    g_Triangles[Model.mFaceCount].ty2 = l_TexCoord[atoi(&slash[1]) - 1].y * 256;
                    g_Triangles[Model.mFaceCount].tx3 = prev->tx1;
                    g_Triangles[Model.mFaceCount].ty3 = prev->ty1;
                }

                // -- Add-To-TreeView
                char name[256];
                sprintf(name,"Triangles #%u", (UINT)Model.mFaceCount);
                g_TVItems.push_back(TreeView_AddResource(g_FileView, rootNodes[TV_TRIANGLES], name, LPARAM( &g_Triangles[Model.mFaceCount] ) ));
                Model.mFaceCount++;
            }
        }
    }

    // Normals
	//Normals
    bool vlist[ MAX_VERTICES ];
    ZeroMemory(vlist,sizeof(bool));
    for (int f=0; f<Model.mFaceCount; f++)
    {
        int v1 = g_Triangles[f].v1;
        int v2 = g_Triangles[f].v2;
        int v3 = g_Triangles[f].v3;
        g_Normals[f] = ComputeNormals(g_Verticies[v1],g_Verticies[v2],g_Verticies[v3]);
        if (!vlist[v1])
        {
            g_VNormals[v1] = g_Normals[f];
            vlist[v1] = true;
        }
        else
        {
            g_VNormals[v1] += g_Normals[f];
            g_VNormals[v1] /= 2;
        }
        if (!vlist[v2])
        {
            g_VNormals[v2] = g_Normals[f];
            vlist[v2] = true;
        }
        else
        {
            g_VNormals[v2] += g_Normals[f];
            g_VNormals[v2] /= 2;
        }
        if (!vlist[v3])
        {
            g_VNormals[v3] = g_Normals[f];
            vlist[v3] = true;
        }
        else
        {
            g_VNormals[v3] += g_Normals[f];
            g_VNormals[v3] /= 2;
        }
    }

    fclose(fp);

    return;
}


void Load3DNData(char *fname)
{
    newScene();

    FILE *fp = fopen(fname,"rb");

    if ( !fp )
    {
        MessageBox( g_hMain, "The selected file does not exist or is corrupt!", "File Error", MB_ICONEXCLAMATION );
        return;
    }

    //--Header
    fread(&Model.mVertCount,1,4,fp);
    fread(&Model.mFaceCount,1,4,fp);
    fread(&Model.mBoneCount,1,4,fp);
	fread(Model.mName,1,32,fp);
	fseek( fp, 4, SEEK_CUR );
    //fread(&Model.mTextureSize,1,4,fp);
	Model.mTextureSize = 256 * 512;

    if ( Model.mVertCount == 0 )
    {
        MessageBox( g_hMain, "There are no vertices in this file!", "File Error", MB_ICONEXCLAMATION );
        return;
    }
    if ( Model.mFaceCount == 0 )
    {
        MessageBox( g_hMain, "There are no triangles in this file!", "File Error", MB_ICONEXCLAMATION );
        return;
    }


	//Vertices
    for (int v=0; v<Model.mVertCount; v++)
    {
        fread(&g_Verticies[v],16,1,fp);

		char str[256];
        sprintf(str,"Vertex #%d",v+1);
        g_TVItems.push_back(TreeView_AddResource(g_FileView, rootNodes[TV_VERTICES], str, LPARAM( &g_Verticies[v] ) ));
    }
	
    //Triangles
    for (int t=0; t<Model.mFaceCount; t++)
    {
		fread(&g_Triangles[t].v1, 4, 1, fp );
		fread(&g_Triangles[t].v2, 4, 1, fp );
		fread(&g_Triangles[t].v3, 4, 1, fp );

		fread(&g_Triangles[t].tx1, 2, 1, fp );
		fread(&g_Triangles[t].ty1, 2, 1, fp );
		fread(&g_Triangles[t].tx2, 2, 1, fp );
		fread(&g_Triangles[t].ty2, 2, 1, fp );
		fread(&g_Triangles[t].tx3, 2, 1, fp );
		fread(&g_Triangles[t].ty3, 2, 1, fp );

		fread(&g_Triangles[t].u1, 4, 1, fp );
		fread(&g_Triangles[t].flags, 4, 1, fp );
		fread(&g_Triangles[t].parent, 4, 1, fp );
		fread(&g_Triangles[t].u2, 4, 1, fp );
		fread(&g_Triangles[t].u3, 4, 1, fp );
		fread(&g_Triangles[t].u4, 4, 1, fp );
		fread(&g_Triangles[t].u5, 4, 1, fp );

		g_Triangles[t].ty1 = -g_Triangles[t].ty1;
		g_Triangles[t].ty2 = -g_Triangles[t].ty2;
		g_Triangles[t].ty3 = -g_Triangles[t].ty3;

		char str[256];
        sprintf(str,"Triangles #%d",t+1);
        g_TVItems.push_back(TreeView_AddResource(g_FileView, rootNodes[TV_TRIANGLES], str, LPARAM( &g_Triangles[t] ) ));
    }

    //Normals
    bool vlist[ MAX_VERTICES ];
    ZeroMemory(vlist,sizeof(bool));
    for (int f=0; f<Model.mFaceCount; f++)
    {
        int v1 = g_Triangles[f].v1;
        int v2 = g_Triangles[f].v2;
        int v3 = g_Triangles[f].v3;
        g_Normals[f] = ComputeNormals(g_Verticies[v1],g_Verticies[v2],g_Verticies[v3]);
        if (!vlist[v1])
        {
            g_VNormals[v1] = g_Normals[f];
            vlist[v1] = true;
        }
        else
        {
            g_VNormals[v1] += g_Normals[f];
            g_VNormals[v1] /= 2;
        }
        if (!vlist[v2])
        {
            g_VNormals[v2] = g_Normals[f];
            vlist[v2] = true;
        }
        else
        {
            g_VNormals[v2] += g_Normals[f];
            g_VNormals[v2] /= 2;
        }
        if (!vlist[v3])
        {
            g_VNormals[v3] = g_Normals[f];
            vlist[v3] = true;
        }
        else
        {
            g_VNormals[v3] += g_Normals[f];
            g_VNormals[v3] /= 2;
        }
    }

    //Bones
    for (int b=0; b<Model.mBoneCount; b++)
    {
        fread(&g_Bones[b],sizeof(BONE),1,fp);

		g_TVItems.push_back(TreeView_AddResource(g_FileView, rootNodes[TV_BONES], g_Bones[b].name, LPARAM( &g_Bones[b] ) ));
    }

    //Texture
	delete [] Model.mTexture;
	Model.mTexture = 0;
	Model.mTexture = new char [ Model.mTextureSize ];
	memset( Model.mTexture, 255, Model.mTextureSize );
    //fread( Model.mTexture, Model.mTextureSize,1, fp);

	Model.FindTextureDimensions();
	Model.ApplyTextureOpacity();

    glBindTexture(GL_TEXTURE_2D, g_TextureID[0]);
    glTexEnvi(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	
	if ( Model.mTextureColor == 16 )
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB5_A1, Model.mTextureWidth, Model.mTextureHeight, 0, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, Model.mTexture);
	if ( Model.mTextureColor == 32 )
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Model.mTextureWidth, Model.mTextureHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, Model.mTexture);

    // -- Close the file
    fclose(fp);
}


void Load3DFData(char *fname)
{
    newScene();

    FILE *fp = fopen(fname,"rb");

    if ( !fp )
    {
        MessageBox( g_hMain, "The selected file does not exist or is corrupt!", "File Error", MB_ICONEXCLAMATION );
        return;
    }

    //--Header
    fread(&Model.mVertCount,1,4,fp);
    fread(&Model.mFaceCount,1,4,fp);
    fread(&Model.mBoneCount,1,4,fp);
    fread(&Model.mTextureSize,1,4,fp);

    if ( Model.mVertCount == 0 )
    {
        MessageBox( g_hMain, "There are no vertices in this file!", "File Error", MB_ICONEXCLAMATION );
        return;
    }
    if ( Model.mFaceCount == 0 )
    {
        MessageBox( g_hMain, "There are no triangles in this file!", "File Error", MB_ICONEXCLAMATION );
        return;
    }


    //Triangles
    for (int t=0; t<Model.mFaceCount; t++)
    {
        fread(&g_Triangles[t],64,1,fp);

		char str[256];
        sprintf(str,"Triangles #%d",t+1);
        g_TVItems.push_back(TreeView_AddResource(g_FileView, rootNodes[TV_TRIANGLES], str, LPARAM( &g_Triangles[t] ) ));
    }

    //Vertices
    for (int v=0; v<Model.mVertCount; v++)
    {
        fread(&g_Verticies[v],16,1,fp);

		char str[256];
        sprintf(str,"Vertex #%d",v+1);
        g_TVItems.push_back(TreeView_AddResource(g_FileView, rootNodes[TV_VERTICES], str, LPARAM( &g_Verticies[v] ) ));
    }

    //Normals
    bool vlist[ MAX_VERTICES ];
    ZeroMemory(vlist,sizeof(bool));
    for (int f=0; f<Model.mFaceCount; f++)
    {
        int v1 = g_Triangles[f].v1;
        int v2 = g_Triangles[f].v2;
        int v3 = g_Triangles[f].v3;
        g_Normals[f] = ComputeNormals(g_Verticies[v1],g_Verticies[v2],g_Verticies[v3]);
        if (!vlist[v1])
        {
            g_VNormals[v1] = g_Normals[f];
            vlist[v1] = true;
        }
        else
        {
            g_VNormals[v1] += g_Normals[f];
            g_VNormals[v1] /= 2;
        }
        if (!vlist[v2])
        {
            g_VNormals[v2] = g_Normals[f];
            vlist[v2] = true;
        }
        else
        {
            g_VNormals[v2] += g_Normals[f];
            g_VNormals[v2] /= 2;
        }
        if (!vlist[v3])
        {
            g_VNormals[v3] = g_Normals[f];
            vlist[v3] = true;
        }
        else
        {
            g_VNormals[v3] += g_Normals[f];
            g_VNormals[v3] /= 2;
        }
    }

    //Bones
    for (int b=0; b<Model.mBoneCount; b++)
    {
        fread(&g_Bones[b],sizeof(BONE),1,fp);

		g_TVItems.push_back(TreeView_AddResource(g_FileView, rootNodes[TV_BONES], g_Bones[b].name, LPARAM( &g_Bones[b] ) ));
    }

    //Texture
	delete [] Model.mTexture;
	Model.mTexture = 0;
	Model.mTexture = new char [ Model.mTextureSize ];
    fread( Model.mTexture, Model.mTextureSize,1, fp);

	Model.FindTextureDimensions();
	Model.ApplyTextureOpacity();

    glBindTexture(GL_TEXTURE_2D, g_TextureID[0]);
    glTexEnvi(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	
	if ( Model.mTextureColor == 16 )
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB5_A1, Model.mTextureWidth, Model.mTextureHeight, 0, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, Model.mTexture);
	if ( Model.mTextureColor == 32 )
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Model.mTextureWidth, Model.mTextureHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, Model.mTexture);

    // -- Close the file
    fclose(fp);
}

void Save3DFData(char *fname)
{
    //Pre-processing:
    if (Model.mBoneCount == 0)
    {
        bool bone[32];
        ZeroMemory(&bone, sizeof(bool) * 32);
        for (int v=0; v<Model.mVertCount; v++)
        {
            if (bone[g_Verticies[v].bone]==false)
            {
                bone[g_Verticies[v].bone]=true;
                sprintf(g_Bones[Model.mBoneCount].name,"Bone-%u", (UINT)Model.mBoneCount);
                g_Bones[Model.mBoneCount].x=0;
                g_Bones[Model.mBoneCount].z=0;
                g_Bones[Model.mBoneCount].y = Model.mBoneCount * 2.0f;
                g_Bones[Model.mBoneCount].parent = -1;
                g_Bones[Model.mBoneCount].hide = 0;
                Model.mBoneCount++;
            }
        }
    }
    //End Pre-processing

    FILE *fp = fopen(fname,"wb");

    //--Header
    fwrite(&Model.mVertCount,1,4,fp);
    fwrite(&Model.mFaceCount,1,4,fp);
    fwrite(&Model.mBoneCount,1,4,fp);
    fwrite(&Model.mTextureSize,1,4,fp);

    //Triangles
    for (int t=0; t<Model.mFaceCount; t++)
    {
        fwrite(&g_Triangles[t],64,1,fp);
    }

    //Verticies
    for (int v=0; v<Model.mVertCount; v++)
    {
        fwrite(&g_Verticies[v],16,1,fp);
    }

    //Bones
    for (int b=0; b<Model.mBoneCount; b++)
    {
        fwrite(g_Bones[b].name,1,32,fp);
        fwrite(&g_Bones[b].x,1,4,fp);
        fwrite(&g_Bones[b].y,1,4,fp);
        fwrite(&g_Bones[b].z,1,4,fp);
        fwrite(&g_Bones[b].parent,1,2,fp);
        fwrite(&g_Bones[b].hide,1,2,fp);
    }

    //Texture
    fwrite( Model.mTexture, Model.mTextureSize,1, fp );

    //Close the file
    fclose(fp);
}

void SaveC2OData(char *fname)
{
    FILE *fp = fopen(fname,"wb");

	if ( g_Animations[0].FrameCount ) Model.mObjInfo.flags |= ofANIMATED;

    //--Header
    fwrite(&Model.mObjInfo,1,48,fp);
    fwrite(&Model.mObjInfo.res,1,16,fp);
    fwrite(&Model.mVertCount,1,4,fp);
    fwrite(&Model.mFaceCount ,1,4,fp);
    fwrite(&Model.mBoneCount,1,4,fp);
    fwrite(&Model.mTextureSize,1,4,fp);

    //Triangles
    for (int t=0; t<Model.mFaceCount; t++)
    {
        fwrite(&g_Triangles[t],64,1,fp);
    }

    //Verticies
    for (int v=0; v<Model.mVertCount; v++)
    {
        fwrite(&g_Verticies[v],sizeof(VERTEX),1,fp);
    }

    //Bones
    for (int b=0; b<Model.mBoneCount; b++)
    {
        fwrite(&g_Bones[b],sizeof(BONE),1,fp);
    }

    //Texture
    fwrite( Model.mTexture, Model.mTextureSize, 1, fp );

    //Sprite
	if ( !(Model.mObjInfo.flags & ofNOBMP) )
    fwrite(&g_Sprite,128*128,2,fp);

    //Animation
	if ( g_Animations[0].FrameCount )
	{
		fwrite( &g_Animations[0].Name, 32, 1, fp );
		fwrite( &g_Animations[0].KPS, 4, 1, fp );
		fwrite( &g_Animations[0].FrameCount, 4, 1, fp );
		fwrite( g_Animations[0].Data, g_Animations[0].FrameCount * Model.mVertCount * 6, 1, fp );
	}

    //Close the file
    fclose(fp);
}

void LoadCMFData(char *fname)
{

    FILE *fp = fopen( fname, "r" );

    if ( !fp )
    {
        MessageBox( g_hMain, "The selected file does not exist or is corrupt!", "File Error", MB_ICONEXCLAMATION );
        return;
    }

	char line[512];

	while ( !feof( fp ) )
	{
		fgets( line, 512, fp );
		
		if ( line[0] == 0 ) break;
		if ( line[0] == 4 ) break;

		char *tok = strtok( line, " \n" );

		// -- File Types
		if ( !strcmp( tok, "Type" ) )
		{
			newScene();
		}
		else if ( !strcmp( tok, "Version" ) )
		{
			// -- Version check
			// Version == 1.0
		}
		else if ( !strcmp( tok, "Name" ) )
		{
			// -- Version check
			// Version == 1.0
		}
	}

    //Normals
    bool vlist[ MAX_VERTICES ];
    ZeroMemory(vlist,sizeof(bool));
    for (int f=0; f<Model.mFaceCount; f++)
    {
        int v1 = g_Triangles[f].v1;
        int v2 = g_Triangles[f].v2;
        int v3 = g_Triangles[f].v3;
        g_Normals[f] = ComputeNormals(g_Verticies[v1],g_Verticies[v2],g_Verticies[v3]);
        if (!vlist[v1])
        {
            g_VNormals[v1] = g_Normals[f];
            vlist[v1] = true;
        }
        else
        {
            g_VNormals[v1] += g_Normals[f];
            g_VNormals[v1] /= 2;
        }
        if (!vlist[v2])
        {
            g_VNormals[v2] = g_Normals[f];
            vlist[v2] = true;
        }
        else
        {
            g_VNormals[v2] += g_Normals[f];
            g_VNormals[v2] /= 2;
        }
        if (!vlist[v3])
        {
            g_VNormals[v3] = g_Normals[f];
            vlist[v3] = true;
        }
        else
        {
            g_VNormals[v3] += g_Normals[f];
            g_VNormals[v3] /= 2;
        }
    }

	fclose( fp );

    //Texture
	Model.mTextureSize = 256*256*2;
	delete [] Model.mTexture;
	Model.mTexture = 0;
	Model.mTexture = new char [ Model.mTextureSize ];
    for ( int i=0; i<Model.mTextureSize/2; i++)
	{
		*( (WORD*) Model.mTexture + i ) = 0xFFFF;
	}

	Model.FindTextureDimensions();
	Model.ApplyTextureOpacity();

    glBindTexture(GL_TEXTURE_2D, g_TextureID[0]);
    glTexEnvi(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	
	if ( Model.mTextureColor == 16 )
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB5_A1, Model.mTextureWidth, Model.mTextureHeight, 0, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, Model.mTexture);
	if ( Model.mTextureColor == 32 )
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Model.mTextureWidth, Model.mTextureHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, Model.mTexture);
}

void SaveCMFData(char *fname)
{
    //Pre-processing:
    if (Model.mBoneCount == 0)
    {
        bool bone[32];
        ZeroMemory(&bone, sizeof(bool) * 32);
        for (int v=0; v<Model.mVertCount; v++)
        {
            if (bone[g_Verticies[v].bone]==false)
            {
                bone[g_Verticies[v].bone]=true;
                sprintf(g_Bones[Model.mBoneCount].name,"Bone-%u", (UINT)Model.mBoneCount);
                g_Bones[Model.mBoneCount].x=0;
                g_Bones[Model.mBoneCount].z=0;
                g_Bones[Model.mBoneCount].y = Model.mBoneCount * 4.0f;
                g_Bones[Model.mBoneCount].parent = -1;
                g_Bones[Model.mBoneCount].hide = 0;
                Model.mBoneCount++;
            }
        }
    }
    //End Pre-processing

    FILE *fp = fopen(fname,"w");

	if ( fp == NULL )
	{
		MessageBox( g_hMain, "Failed to open the desired file for writing!\r\nDo you have administrator access?", "C3Dit", MB_ICONWARNING );
		return;
	}

    //--Header ( ID, VersionMajor, VersionMinor, Name, VertexCount, FaceCount, BoneCount )
	fprintf( fp, "H,%u,%u,%s,%u,%u,%u\n", 1, 0, Model.mName, Model.mVertCount, Model.mFaceCount, Model.mBoneCount );

	// --Verts
	for ( int v=0; v<Model.mVertCount; v++ )
	{
		fprintf( fp, "V,%f,%f,%f,%u\n",
									g_Verticies[v].x,
									g_Verticies[v].y,
									g_Verticies[v].z,
									g_Verticies[v].bone );
	}

	// --Faces
	for ( int f=0; f<Model.mFaceCount; f++ )
	{
		fprintf( fp, "F,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u\n",
									g_Triangles[f].v1,
									g_Triangles[f].v2,
									g_Triangles[f].v3,
									g_Triangles[f].tx1,
									g_Triangles[f].tx2,
									g_Triangles[f].tx3,
									g_Triangles[f].ty1,
									g_Triangles[f].ty2,
									g_Triangles[f].ty3,
									g_Triangles[f].flags );
	}

	//-- Bones
	for ( int b=0; b<Model.mBoneCount; b++ )
	{
		fprintf( fp, "B,%s,%f,%f,%f\n",
									g_Bones[b].name,
									g_Bones[b].x,
									g_Bones[b].y,
									g_Bones[b].z );
	}

    //Close the file
    fclose(fp);
}

/*
	void SaveProject()
	This function saves the current scene as the desired model format.
	This deprecated the import/export model feature.
*/
void SaveProject()
{
    if (!SaveFileDlg("All Supported (*.car;*.c2o;*.3df;*.obj;*.cmf;)\0*.car;*.c2o;*.3df;*.obj*.cmf\0"
                     "Character files (*.car)\0*.car\0"
                     "Carnivores 2 Object (*.c2o)\0*.c2o\0"
                     "3D File (*.3df)\0*.3df\0"
					 "Text Format (.cmf)\0*.cmf\0"
                     "Wavefront OBJ File (.obj)\0*.obj\0", "car"))
    return;

	char tempFile[260];
	memcpy( tempFile, fileName, 260 );
	_strlwr( tempFile );

    //--Find out what kind of file this is
    if (strstr( tempFile, ".car")!=NULL)
    {
        SaveCARData(fileName);
    }
    if (strstr( tempFile, ".c2o")!=NULL)
    {
        SaveC2OData(fileName);
    }
    if (strstr( tempFile, ".3df")!=NULL)
    {
        Save3DFData(fileName);
    }
    if (strstr( tempFile, ".obj")!=NULL)
    {
        SaveOBJData(fileName);
    }
	if (strstr( tempFile, ".cmf")!=NULL)
    {
        SaveCMFData(fileName);
    }
	if (strstr( tempFile, ".3ds")!=NULL)
    {
        //Save3DSData(fileName);
    }
}


void tga16_to_bgr24(WORD *A, byte *bgr, int L)
{
    //A is the pointer to the 16-bit texture data.
    //bgr is the pointer to the BGR image.
    //L is the width*height of the texture.
    int pos=0;

    for (int c=0; c<L; c++)
    {
        WORD w = *(A + c);
        int B = ((w>> 0) & 31)*8;
        int G = ((w>> 5) & 31)*8;
        int R = ((w>>10) & 31)*8;

        *(bgr + pos) = B;
        pos++;
        *(bgr + pos) = G;
        pos++;
        *(bgr + pos) = R;
        pos++;
    }
}


void keyboardE()
{
    if ( key[VK_SUBTRACT] )
        cam.dist += 0.5f;

    if ( key[VK_ADD] )
        cam.dist -= 0.5f;

    if ( key[VK_LEFT] )
        cam.yaw += 1.0f;

    if ( key[VK_RIGHT] )
        cam.yaw -= 1.0f;

    if ( key[VK_UP] )
	{
        if ( key['Z'] )
			cam.dist -= 0.25f;
		else
			cam.pitch += 1.0f;
	}

    if ( key[VK_DOWN] )
	{
		if ( key['Z'] )
			cam.dist += 0.25f;
		else
			cam.pitch -= 1.0f;
	}

    cam.x = cam.xt+lengthdir_x(-cam.dist, cam.yaw, cam.pitch);
    cam.y = cam.yt+lengthdir_y(-cam.dist, cam.yaw, cam.pitch);
    cam.z = cam.zt+lengthdir_z(-cam.dist, cam.pitch);
}

int LastMidMouseX = 0;
int LastMidMouseY = 0;

void mouseE()
{
    if ( mouse[0] )
    {
        POINT cms = { CurX, CurY };
        ScreenToClient(g_hMain,&cms);

        RECT rc;
        GetWindowRect(g_DrawArea,&rc);

        if ( cms.x > rc.left && cms.x < rc.right &&
             cms.y > rc.top && cms.y < rc.bottom )
        {
            SetFocus( g_hMain );
        }

        GetClientRect(g_hMain,&rc);
        int CH = rc.bottom;
    }
    if ( mouse[1] )     // Middle mouse
    {
		if ( LastMidMouseX == 0 && LastMidMouseY == 0 )
		{
			POINT pn;
			GetCursorPos(&pn);
			LastMidMouseX = pn.x;
			LastMidMouseY = pn.y;
		}
		else
        {
			POINT pn;
			GetCursorPos(&pn);

			if ( CameraView > VIEW_PERSPECTIVE+1 )
			{
				float distx = (float)(pn.x - LastMidMouseX);
				float disty = (float)(pn.y - LastMidMouseY);
				ortho.xt += distx;
				ortho.yt += disty;
			}
			else
			{
				float disth = (float)(pn.x - LastMidMouseX) / 100.0f;
				float distz = (float)(pn.y - LastMidMouseY) / 100.0f;
				cam.xt += lengthdir_x( disth, -cam.yaw-90, 0 );
				cam.yt += lengthdir_y( disth, -cam.yaw+90, 0 );
				cam.zt += lengthdir_z( distz, -cam.pitch-90 );
			}

			LastMidMouseX = pn.x;
			LastMidMouseY = pn.y;
		}
    }
	else
	{
		LastMidMouseX = 0;
		LastMidMouseY = 0;
	}
    if ( mouse[2] )     // Right mouse
    {
        if ( mx!=CurX || my!=CurY )
        {
            cam.yaw += (CurX-mx);
            cam.pitch += (CurY-my);

            if (cam.pitch>=89) cam.pitch=89;
            if (cam.pitch<=-89) cam.pitch=-89;

            mx = CurX;
            my = CurY;
        }
    }
}

NORM ComputeNormals(VERTEX vt1, VERTEX vt2, VERTEX vt3)
{
    float nx,ny,nz;
    float m;
    NORM norm, a, b;

	a.x = vt1.x - vt2.x;
	a.y = vt1.y - vt2.y;
	a.z = vt1.z - vt2.z;

	b.x = vt3.x - vt2.x;
	b.y = vt3.y - vt2.y;
	b.z = vt3.z - vt2.z;
	
    // Cross Product
    nx = (a.y)*(b.z)-(b.y)*(a.z);
    ny = (a.z)*(b.x)-(b.z)*(a.x);
    nz = (a.x)*(b.y)-(b.x)*(a.y);

    // Magnitude
    m = sqrtf(nx*nx + ny*ny + nz*nz);

    // Normalize
    nx /= m;
    ny /= m;
    nz /= m;

    // Store
    norm.n[0] = nx;
    norm.n[1] = nz;
    norm.n[2] = -ny;

    return norm;
}

HTREEITEM TreeView_AddNode(HWND tree, HTREEITEM parent, const char *name)
{
    TV_INSERTSTRUCT tvis;
    TVITEM tvi;

    tvis.hParent = ((parent)?parent:TVI_ROOT);
    tvis.hInsertAfter = TVI_LAST;
    tvi.mask = TVIF_TEXT;
    tvi.pszText = (char*)name;
    tvi.cchTextMax = strlen(name);
    tvis.item = tvi;
    return TreeView_InsertItem(tree,&tvis);
}

HTREEITEM TreeView_AddResource(HWND tree, HTREEITEM parent, const char *name, LPARAM lParam )
{
    TV_INSERTSTRUCT tvis;
    TVITEM tvi;

    //printf("lParam: %d\n", lParam);

    tvis.hParent = parent;
    tvis.hInsertAfter = TVI_LAST;
    tvi.mask = TVIF_TEXT | TVIF_PARAM;
    tvi.pszText = (char*)name;
    tvi.cchTextMax = strlen(name);
    tvi.lParam = lParam;
    tvis.item = tvi;

    return TreeView_InsertItem(tree,&tvis);
}

BOOL OpenFileDlg(const char *filter, const char *ext)
{
    OPENFILENAME ofn;
    char szFileName[MAX_PATH] = "";

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn); // SEE NOTE BELOW
    ofn.hwndOwner = g_hMain;
    ofn.lpstrFilter = filter;
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = ext;

    if (GetOpenFileName(&ofn))
    {
        for (int i=0; i<260; i++)
            fileName[i] = szFileName[i];

        return TRUE;
    }
    else
        return FALSE;
}

BOOL SaveFileDlg(const char *filter, const char *ext)
{
    OPENFILENAME ofn;
    char szFileName[MAX_PATH] = "";

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn); // SEE NOTE BELOW
    ofn.hwndOwner = g_hMain;
    ofn.lpstrFilter = filter;
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = ext;

    if (GetSaveFileName(&ofn))
    {
        for (int i=0; i<260; i++)
            fileName[i] = szFileName[i];

        return TRUE;
    }
    return FALSE;
}

BOOL SaveFileDlg(char *filter, char *ext, char *filename)
{
    OPENFILENAME ofn;
    char szFileName[MAX_PATH] = "";

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn); // SEE NOTE BELOW
    ofn.hwndOwner = g_hMain;
    ofn.lpstrFilter = filter;
    ofn.lpstrFileTitle = filename;
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = ext;

    if (GetSaveFileName(&ofn))
    {
        for (int i=0; i<260; i++)
            fileName[i] = szFileName[i];

        return TRUE;
    }
    return FALSE;
}

void importSound()
{
    if (!OpenFileDlg("WAVE sound file (.wav)\0*.wav\0", "wav"))
        return;

    FILE *sfp;
    sfp = fopen(fileName,"rb");
    if (sfp==NULL) return;
    long freq,bpas,length;
    short pcm,channels,blocks,bp;

    //--Header
    fseek(sfp,20,SEEK_CUR);
    fread(&pcm,1,2,sfp);        //Type of Compression
    fread(&channels,1,2,sfp);   //Number of channels
    fread(&freq,1,4,sfp);       //Sound Frequency
    fread(&bpas,1,4,sfp);       //Bytes per Average Second
    fread(&blocks,1,2,sfp);     //Block Alignment
    fread(&bp,1,2,sfp);         //Bit Precision (Bits per Block)
    fseek(sfp,4,SEEK_CUR);
    fread(&length,1,4,sfp);     //Length of data

    //--Header check
    if (pcm!=1)
    {
        MessageBox( g_hMain,
                    "The sound file is not an uncompressed PCM waveform sound!",
                    "Warning",
                    MB_OK | MB_ICONWARNING);
        return;
    }
    if (channels!=1)
    {
        MessageBox( g_hMain,
                    "The sound file is not a Mono sound!",
                    "Warning",
                    MB_OK | MB_ICONWARNING);
        return;
    }
    if (freq!=22050)
    {
        MessageBox( g_hMain,
                    "The sound file does not have a frequency of 22050 kHz!",
                    "Warning",
                    MB_OK | MB_ICONWARNING);
        return;
    }
    if (blocks!=2)
    {
        MessageBox( g_hMain,
                    "The sound file does not have a block size of 2 bytes!",
                    "Warning",
                    MB_OK | MB_ICONWARNING);
        return;
    }
    if (bp!=16)
    {
        MessageBox( g_hMain,
                    "The sound file is not a 16-bit sound!",
                    "Warning",
                    MB_OK | MB_ICONWARNING);
        return;
    }

    //--Read waveform data
    g_Sounds[Snd].len = length;
    delete [] g_Sounds[Snd].data;
    g_Sounds[Snd].data = new BYTE[g_Sounds[Snd].len];
    fread(g_Sounds[Snd].data,g_Sounds[Snd].len,1,sfp);

    //--Close the file
    fclose(sfp);

    //--Update the window information
    sprintf(str,"%d Kbs", (int)g_Sounds[Snd].len/1000);
    SetDlgItemText(g_hSndDlg, ID_NAME, g_Sounds[Snd].name);
    SetDlgItemText(g_hSndDlg, ID_SIZE, str);
}

void importTexture()
{
    //if (!OpenFileDlg("TGA image (.tga)\0*.tga\0Windows Bitmap (.bmp)\0*.bmp\0", "tga"))
    if (!OpenFileDlg("All Supported Files\0*.bmp;*.tga\0TGA image (.tga)\0*.tga\0Windows Bitmap (.bmp)\0*.bmp\0", "tga"))
        return;

    FILE *ifp;
    WORD w=0,h=0;
    BYTE bpp=0;

    ifp = fopen(fileName,"rb");
    if (ifp==NULL) return;

    if (strstr(_strlwr(fileName),".tga"))
    {
		TARGAINFOHEADER tih;
		fread( &tih, sizeof(TARGAINFOHEADER), 1, ifp );

		if ( tih.tgaColorMapType != 0 || tih.tgaImageType != 2 )
		{
			fclose(ifp);
			return;
		}

		w = tih.tgaWidth;
		h = tih.tgaHeight;
		bpp = tih.tgaBits;
    }
    if (strstr(_strlwr(fileName),".bmp"))
    {
        BITMAPFILEHEADER bfh;
        BITMAPINFOHEADER bih;

        fread(&bfh, sizeof(BITMAPFILEHEADER), 1, ifp);
        fread(&bih, sizeof(BITMAPINFOHEADER), 1, ifp);

        w = bih.biWidth;
        h = bih.biHeight;
        bpp = bih.biBitCount;
    }

    printf("W: %u, H: %u, BPP: %u\n", w, h, bpp);

    //--Error catch
    if ( bpp!=16 && bpp!=24 && bpp!=32 )
    {
        MessageBox(g_hMain, "The image file is not a 16-bit, 24-bit or 32-bit image!", "Error", MB_OK | MB_ICONERROR);
        fclose(ifp);
        return;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, g_TextureID[0]);

	Model.mTextureSize = w * h * 2;
	delete [] Model.mTexture;
	Model.mTexture = new uint8_t [ Model.mTextureSize ];
	Model.mTextureColor = 16;

    if (bpp==16)
	{
		if ( (size_t)Model.mTexture == 0x80428042 ) MessageBox( 0, "Failed to allocate texture!", "Fatal Error", 0 );
		
        fread( Model.mTexture,w*h*2,1,ifp);

		Model.FindTextureDimensions();
		Model.ApplyTextureOpacity();

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB5_A1, w,h,0, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, Model.mTexture );
    }
    if (bpp==24)
    {
		for ( int i=0; i<w*h; i++ )
		{
			uint32_t pix;
			fread( &pix, 3, 1, ifp );
			int B = ( (pix>>0 ) & 0xFF ) / 8;
			int G = ( (pix>>8 ) & 0xFF ) / 8;
			int R = ( (pix>>16) & 0xFF ) / 8;
			if ( !R && !G && !B )
				*( (uint16_t*)Model.mTexture + i ) = 0;
			else
				*( (uint16_t*)Model.mTexture + i ) = (B) | (G<<5) | (R<<10) | (1<<15);
		}

		//Model.FindTextureDimensions();
		//Model.ApplyTextureOpacity();

		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB5_A1, w,h, 0, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, Model.mTexture );
    }
    if (bpp==32)
    {
        for ( int i=0; i<w*h; i++ )
		{
			uint32_t pix;
			fread( &pix, 4, 1, ifp );
			int B = ( (pix>>0 ) & 0xFF ) / 8;
			int G = ( (pix>>8 ) & 0xFF ) / 8;
			int R = ( (pix>>16) & 0xFF ) / 8;
			int A = ( (pix>>24) & 0xFF );
			if ( A < 128 )
				*( (uint16_t*)Model.mTexture + i ) = 0;
			else
				*( (uint16_t*)Model.mTexture + i ) = (B) | (G<<5) | (R<<10) | (1<<15);
		}

		//Model.FindTextureDimensions();
		//Model.ApplyTextureOpacity();

		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB5_A1, w,h, 0, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, Model.mTexture );
    }

    fclose(ifp);
}

void exportSound()
{
    if (!SaveFileDlg("WAVE sound file (.wav)\0*.wav\0", "wav"))
        return;

    FILE *sfp;
    sfp = fopen(fileName,"wb");
    if (sfp==NULL)
        return;
    long lval;
    short sval;

    fputc('R',sfp);
    fputc('I',sfp);
    fputc('F',sfp);
    fputc('F',sfp);
    lval = g_Sounds[Snd].len+(44-8);
    fwrite(&lval,1,4,sfp);
    fputc('W',sfp);
    fputc('A',sfp);
    fputc('V',sfp);
    fputc('E',sfp);
    fputc('f',sfp);
    fputc('m',sfp);
    fputc('t',sfp);
    fputc(' ',sfp);
    lval = 16;
    fwrite(&lval,1,4,sfp);
    sval = 1;
    fwrite(&sval,1,2,sfp);
    sval = 1;
    fwrite(&sval,1,2,sfp);
    lval = 22050;
    fwrite(&lval,1,4,sfp);
    lval = 22050 * (1 * 16 / 8);
    fwrite(&lval,1,4,sfp);
    sval = (1 * 16 / 8);
    fwrite(&sval,1,2,sfp);
    sval = 16;
    fwrite(&sval,1,2,sfp);
    fputc('d',sfp);
    fputc('a',sfp);
    fputc('t',sfp);
    fputc('a',sfp);
    lval = g_Sounds[Snd].len;
    fwrite(&lval,1,4,sfp);
    fwrite( g_Sounds[Snd].data,g_Sounds[Snd].len,1,sfp);
    fclose(sfp);
}

void exportTexture()
{
    if (!SaveFileDlg("TGA image (.tga)\0*.tga\0Windows Bitmap Image (.bmp)\0*.bmp\0", "tga"))
        return;

    FILE *ifp;

    ifp = fopen(fileName,"wb");
    if (ifp==NULL) return;

    if (strstr(_strupr(fileName),".TGA")!=NULL)
    {
        TARGAINFOHEADER tih; /* targa info-header */
        // = {0,0,2, 0,0,0, 0,0,w,h,16,0};

        ZeroMemory(&tih, sizeof(TARGAINFOHEADER));
        tih.tgaIdentSize = 0;
        tih.tgaColorMapType = 0;
        tih.tgaImageType = 2;//TGA_UNCOMPRESSED
        tih.tgaWidth = Model.mTextureWidth;
        tih.tgaHeight = Model.mTextureHeight;
        tih.tgaBits = Model.mTextureColor;

        fwrite(&tih, 18, 1, ifp);
    }
    if (strstr(_strupr(fileName),".BMP")!=NULL)
    {
        BITMAPFILEHEADER hdr;       /* bitmap file-header */
        BITMAPINFOHEADER bmi;       /* bitmap info-header */

		ZeroMemory( &bmi, sizeof(bmi) );
        bmi.biSize = sizeof(BITMAPINFOHEADER);
        bmi.biWidth = Model.mTextureWidth;
        bmi.biHeight = Model.mTextureHeight;
        bmi.biPlanes = 1;
        bmi.biBitCount = Model.mTextureColor;
        bmi.biCompression = BI_RGB;
        bmi.biSizeImage = Model.mTextureSize;
        bmi.biClrImportant = 0;
        bmi.biClrUsed = 0;

		ZeroMemory( &hdr, sizeof(hdr) );
        hdr.bfType = 0x4d42;
        hdr.bfSize = (DWORD) (sizeof(BITMAPFILEHEADER) + bmi.biSize + bmi.biSizeImage);
        hdr.bfReserved1 = 0;
        hdr.bfReserved2 = 0;
        hdr.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) + bmi.biSize;

        fwrite(&hdr,sizeof(BITMAPFILEHEADER),1,ifp);
        fwrite(&bmi,sizeof(BITMAPINFOHEADER),1,ifp);
    }

    fwrite( Model.mTexture, Model.mTextureSize, 1, ifp );
    fclose(ifp);
}

void exportModel()
{
    if (!SaveFileDlg("3D Model File (.3DF)\0*.3df\0Wavefront .OBJ (.obj)\0*.obj\0", "3df"))
        return;

	char uprFileName[260];
	memcpy( uprFileName, fileName, 260 );
	strupr(uprFileName);

    if (strstr(uprFileName,".3DF")!=NULL)
    {
        Save3DFData(fileName);
        return;
    }
	if (strstr(uprFileName,".OBJ")!=NULL)
    {
        SaveOBJData(fileName);
        return;
    }

    return;
}

/*
	void importModel()
	DEPRECATED: Use void LoadProject() instead!
*/
void importModel()
{
    if (!OpenFileDlg("3D File (*.3DF)\0*.3DF\0Wavefront OBJ File (.obj)\0*.obj\0", "3DF"))
        return;

    if (strstr(strlwr(fileName),".3df")!=NULL)
    {
        Load3DFData(fileName);
    }

    if (strstr(strlwr(fileName),".obj")!=NULL)
    {
        LoadOBJData(fileName);
    }

    // Set Status bar
    char sstr[64];
    sprintf(sstr,"Triangles: %ld",Model.mFaceCount);
    SendMessage(gHStatus, SB_SETTEXT, 0, (LPARAM)sstr);
    MessageBox(g_hMain,sstr,"triangles",MB_OK);
}

void PlayWave(SOUND *sptr)
{
    /* We have to save the .WAV file temporarily >.> */
	if ( sptr->len == 0 ) return;
	if ( sptr->data == 0 ) return;
	if ( (int)sptr->data == 0xFFFFFFFF ) return;

	char out[MAX_PATH];
	sprintf( out, "%s%s", lpszExePath, "temp_sample.wav" );

    FILE *fp = fopen( out, "wb" );

	if ( fp == 0 )
	{
		printf( "Failed to create/open the new file!\n" );
		return;
	}
	long lval = 0;
    short sval = 0;

	fwrite( "RIFF", 4, 1, fp );
    lval = sptr->len+(44-8);
    fwrite(&lval,1,4,fp);
	fwrite( "WAVEfmt ", 8, 1, fp );
    lval = 16;
    fwrite(&lval,1,4,fp);
    sval = 1;
    fwrite(&sval,1,2,fp);
    sval = 1;
    fwrite(&sval,1,2,fp);
    lval = 22050;
    fwrite(&lval,1,4,fp);
    lval = 22050 * (1 * 16 / 8);
    fwrite(&lval,1,4,fp);
    sval = (1 * 16 / 8);
    fwrite(&sval,1,2,fp);
    sval = 16;
    fwrite(&sval,1,2,fp);
    fwrite( "data", 4, 1, fp );
    lval = sptr->len;
    fwrite(&lval,1,4,fp);

    fwrite(sptr->data,sptr->len,1,fp);
    fclose(fp);

    PlaySound( out, NULL, SND_ASYNC );
}
