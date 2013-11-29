
#ifdef _WIN32_IE
#undef _WIN32_IE
#endif

#undef UNICODE

#define _WIN32_IE	0x0700

#ifdef WINVER
#undef WINVER
#endif

#define WINVER 0x0501

#include <windows.h>
#include <commctrl.h>
#include <windowsx.h>



/******************************************************************
* ezwInitComCtl() Function
* Initializes the Common Control Classes
*/
void ezwInitComCtl()
{
    INITCOMMONCONTROLSEX icex;

    icex.dwSize =		sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC =		ICC_TAB_CLASSES |
						ICC_LINK_CLASS |
						ICC_BAR_CLASSES |
						ICC_TREEVIEW_CLASSES |
						ICC_PROGRESS_CLASS |
						ICC_STANDARD_CLASSES;
    
	InitCommonControlsEx( &icex );
}


/******************************************************************
* ezwGetWorkingDirectory() Function
* Retrieves the working directory of the process.
*/
bool ezwGetWorkingDirectory( LPSTR Output, DWORD Length )
{
	GetModuleFileName( NULL, Output, Length );
	return true;
}


/******************************************************************
* ezwGetScreenWidth() Function
* Retrieves the width of the screen in pixels.
*/
int ezwGetScreenWidth( void )
{
	return GetSystemMetrics( SM_CXSCREEN );
}


/******************************************************************
* ezwGetScreenHeight() Function
* Retrieves the width of the screen in pixels.
*/
int ezwGetScreenHeight( void )
{
	return GetSystemMetrics( SM_CYSCREEN );
}


/******************************************************************
* EZWControl Base Class for Controls
* Contains a range of standard functions, also contains
* operators and constructor/destructors to handle the classes.
*/
class EZWControl
{
protected:
	HWND hwnd;
public:

	EZWControl()
	{
		this->hwnd = NULL;
	}
	~EZWControl()
	{
		if ( this->hwnd != NULL )
		{
			DestroyWindow( hwnd );
			this->hwnd = NULL;
		}
	}

	inline operator HWND(void)
    {
        return hwnd;
    }

	inline void operator = (const HWND &other)
    {
        this->hwnd = other;
        return;
    }

	// -- Prototype functions

	inline void Destroy( void )
	{
		if ( this->hwnd != NULL )
		{
			DestroyWindow( hwnd );
			this->hwnd = NULL;
		}
	}

	inline bool SetText( LPSTR Text )
	{
		if ( this->hwnd == NULL ) return (false);
		
		SetWindowText( this->hwnd, Text );

		return (true);
	}

	inline DWORD GetText( LPSTR Text )
	{
		if ( this->hwnd == NULL ) return (false);
		
		if ( Text == NULL )
		{
			return GetWindowTextLength( this->hwnd );
		}

		GetWindowText( this->hwnd, Text, GetWindowTextLength( this->hwnd ) );

		return (true);
	}

	inline bool Show( void )
	{
		if ( this->hwnd == NULL ) return (false);
		
		ShowWindow( this->hwnd, SW_SHOW );

		return (true);
	}

	inline bool Hide( void )
	{
		if ( this->hwnd == NULL ) return (false);
		
		ShowWindow( this->hwnd, SW_HIDE );

		return (true);
	}

	inline bool Enable()
	{
		if ( this->hwnd == NULL ) return (false);
		
		EnableWindow( this->hwnd, TRUE );

		return (true);
	}

	inline bool Disable()
	{
		if ( this->hwnd == NULL ) return (false);
		
		EnableWindow( this->hwnd, FALSE );

		return (true);
	}

	inline void Resize( DWORD Width, DWORD Height )
	{
		if ( this->hwnd == NULL ) return;

		RECT rc;
		GetWindowRect( this->hwnd, &rc );
		MoveWindow( this->hwnd, rc.left,rc.top, Width,Height, TRUE );
	}

	inline void Move( DWORD X, DWORD Y )
	{
		if ( this->hwnd == NULL ) return;

		RECT rc;
		GetWindowRect( this->hwnd, &rc );
		MoveWindow( this->hwnd, X,Y, rc.right,rc.bottom, TRUE );
	}

	inline void Redraw()
	{
		if ( this->hwnd == NULL ) return;

		RedrawWindow( this->hwnd, NULL, NULL, RDW_UPDATENOW );
	}
};


/******************************************************************
* EZWFileDialog Control
* Contains the Open/Save File Dialog and it's properties in a set
* of easily manageable methods.
*/
class EZWDialog : public EZWControl
{
private:
	DLGTEMPLATE			dialogTemplate;
	DLGITEMTEMPLATE		*dialogItemTemplate;
	WORD				*wordList;
	DWORD				size;
public:

	EZWDialog()
	{
		size = 0;
		wordList = 0;
		dialogItemTemplate = 0;
		ZeroMemory( &dialogTemplate, sizeof(DLGTEMPLATE) );
	}
	~EZWDialog()
	{
		if (wordList) delete [] wordList;
		if (dialogItemTemplate) delete [] dialogItemTemplate;
	}

	inline bool Create( LPSTR DialogName, LPSTR Title, DWORD Styles, WORD Width, WORD Height, HWND Parent, DLGPROC ProcFunc )
	{
		dialogTemplate.style = Styles;
		dialogTemplate.dwExtendedStyle = 0;
		dialogTemplate.cdit = 0;
		dialogTemplate.x = 50;
		dialogTemplate.y = 50;
		dialogTemplate.cx = Width;
		dialogTemplate.cy = Height;

		hwnd = CreateDialogIndirect( GetModuleHandle(NULL), &dialogTemplate, Parent, ProcFunc );

		if ( hwnd == NULL )
		{
			MessageBox( Parent, "Failed to create the modeless Dialog...", "EZUI Error", MB_ICONERROR );
			return false;
		}

		return true;
	}
};


/******************************************************************
* EZWFileDialog Control
* Contains the Open/Save File Dialog and it's properties in a set
* of easily manageable methods.
*/
class EZWFileDialog
{
private:
	OPENFILENAME	ofn;
public:

	EZWFileDialog()
	{
		ZeroMemory( &ofn, sizeof(OPENFILENAME) );
		ofn.lStructSize = sizeof(ofn);
		ofn.hInstance = GetModuleHandle( NULL );
		ofn.nMaxFile = MAX_PATH;
		ofn.Flags = OFN_EXPLORER;
	}

	inline void SetOwnerWindow( HWND Window )
	{
		ofn.hwndOwner = Window;
	}

	inline void SetInitialDirectory( LPSTR InitDir )
	{
		ofn.lpstrInitialDir = InitDir;
	}

	inline void SetDefaultExt( LPSTR Extension )
	{
		ofn.lpstrDefExt = Extension;
	}

	inline void SetOutputString( LPSTR Output )
	{
		ofn.lpstrFile = Output;
	}

	inline void SetFilters( LPSTR Filters )
	{
		ofn.lpstrFilter = Filters;
	}

	inline void SetTitle( LPSTR Title )
	{
		ofn.lpstrTitle = Title;
	}

	inline void SetFileNameOutput( LPSTR Output )
	{
		ofn.lpstrFileTitle = Output;
		ofn.nMaxFileTitle = MAX_PATH;
	}

	inline DWORD GetMaxFileNameLength( void )
	{
		return MAX_PATH;
	}

	inline void BasicOpenDialog()
	{
		ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_ENABLESIZING;
	}

	inline void BasicSaveDialog()
	{
		ofn.Flags = OFN_EXPLORER | OFN_ENABLESIZING | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
	}

	inline bool OpenFile( void )
	{
		return GetOpenFileName( &this->ofn );
	}

	inline bool SaveFile( void )
	{
		return GetSaveFileName( &this->ofn );
	}
};


/******************************************************************
* EZWTab Control
* Tab Control encapsulation to simplify use of Tabs
*/
class EZWTab
{
    private:
    HWND hwnd;
    int TabCount;
    public:

    EZWTab() {}
    ~EZWTab()
    {
        DestroyWindow(hwnd);
    }

    inline operator HWND(void) {return hwnd;}

    inline bool Create(HWND hParent,RECT &rc, UINT lFlags, HINSTANCE hInst)
    {
        hwnd = CreateWindow(WC_TABCONTROL, "",
                            WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | lFlags,
                            rc.left,rc.top,rc.right,rc.bottom,
                            hParent,
                            NULL,
                            hInst,
                            NULL);

        EnableWindow(hwnd,TRUE);

        if (hwnd==NULL)
        return false;

        return true;
    }

    inline void Resize(RECT *rc)
    {
        TabCtrl_AdjustRect(hwnd, FALSE, rc);
    }

    inline bool AddTab(char *Text)
    {
        TCITEM tie;

        tie.mask = TCIF_TEXT;
        //tie.mask |= TCIF_IMAGE;
        //tie.iImage = -1;
        tie.pszText = Text;

        if (TabCtrl_InsertItem(hwnd, TabCount, &tie) != -1)
        {
            TabCount++;
            return true;
        }
        return false;
    }
};

class EZWComboBox : public EZWControl
{
public:
    EZWComboBox(){}
	~EZWComboBox(){}

    inline void Create(HWND hParent, char *sCaption, long lFlags, RECT &rc)
    {
        hwnd = CreateWindowEx(  0,
                                WC_COMBOBOX,
                                sCaption,
                                WS_BORDER | WS_CHILD | WS_VISIBLE | lFlags,
                                rc.left,rc.top,
                                rc.right,rc.bottom,
                                hParent,
                                NULL,
                                GetModuleHandle(NULL),
                                NULL);
        this->Enable();
    }

    inline void AddItem(char *sString)
    {
        ComboBox_AddString(hwnd,sString);
    }

    inline void SetCurSel(int iIndex)
    {
        ComboBox_SetCurSel(hwnd, iIndex);
    }

    inline void ClearItems()
    {
        ComboBox_ResetContent(hwnd);
    }
};


/******************************************************************
* EZWListBox Control
* A standard list box control
*/
class EZWListBox : public EZWControl
{
public:
    EZWListBox(){}
	~EZWListBox(){}

    inline void Create(HWND hParent, char *sCaption, long lFlags)
    {
        hwnd = CreateWindowEx(  0,
                                WC_LISTBOX,
                                sCaption,
                                WS_BORDER | WS_CLIPSIBLINGS | WS_CHILD | WS_VISIBLE | lFlags,
                                0,0,0,0,
                                hParent,
                                NULL,
                                GetModuleHandle(NULL),
                                NULL);
        this->Enable();
    }

    inline void AddItem(char *sString)
    {
        ListBox_AddString(hwnd,sString);
    }

    inline void SetSelection(int iIndex)
    {
        ListBox_SetCurSel(hwnd, iIndex);
    }

	inline int GetSelection( void )
    {
        return ListBox_GetCurSel(hwnd);
    }

    inline void ClearItems()
    {
        ListBox_ResetContent(hwnd);
    }
};


/******************************************************************
* EZWListView Control
* A standard list view control
*/
class EZWListView : public EZWControl
{
public:
	HIMAGELIST ImageList;

	~EZWListView()
	{
		ImageList_Destroy( ImageList );
	}

    inline void Create(HWND Parent, LPSTR Caption, DWORD Styles)
    {
        hwnd = CreateWindowEx(  0,
                                WC_LISTVIEW,
                                Caption,
                                WS_BORDER | WS_CLIPSIBLINGS | WS_CHILD | WS_VISIBLE | Styles,
                                0,0,0,0,
                                Parent,
                                NULL,
                                GetModuleHandle(NULL),
                                NULL);

		ImageList = ImageList_Create( 128,128, ILC_COLOR16, 0, 1024 );
    }

	inline bool AddImageToList( HBITMAP Bitmap )
	{
		if (ImageList_Add(ImageList, Bitmap, NULL) == -1)
			return false;
		else
			return true;
	}

    inline int AddItemText( LPSTR Text, int Item )
	{
		LVITEM lvi;
		memset( &lvi, 0, sizeof(LVITEM));
		lvi.mask = LVIF_TEXT;
		lvi.iItem = Item;
		lvi.iSubItem = 0;
		lvi.pszText = Text;
		lvi.cchTextMax = strlen( Text ) + 1;

		return SendMessage( hwnd, LVM_INSERTITEM, (WPARAM)0, (LPARAM)&lvi );
	}

	inline int AddItemTextImage( LPSTR Text, int Image, int Item )
	{
		LVITEM lvi;
		memset( &lvi, 0, sizeof(LVITEM));
		lvi.mask = LVIF_TEXT | LVIF_IMAGE;
		lvi.iItem = Item;
		lvi.iSubItem = 0;
		lvi.pszText = Text;
		lvi.cchTextMax = strlen( Text ) + 1;
		lvi.iImage = Image;

		return SendMessage( hwnd, LVM_INSERTITEM, (WPARAM)0, (LPARAM)&lvi );
	}
};


/******************************************************************
* EZWButton Control
* A standard pushbutton control
*/
class EZWButton : public EZWControl
{
public:

    EZWButton(){}
	~EZWButton(){}

    inline void Create(HWND hParent, char *sCaption, long lFlags, RECT &rc)
    {
        hwnd = CreateWindow(    WC_BUTTON,
                                sCaption,
                                WS_CHILD | WS_VISIBLE | lFlags,
                                rc.left,rc.top,
                                rc.right,rc.bottom,
                                hParent,
                                NULL,
                                GetModuleHandle(NULL),
                                NULL);
        Button_Enable(hwnd,TRUE);
    }

    inline void SetText(char *Text)
    {
        Button_SetText(hwnd,Text);
    }

    inline void SetBitmap(HBITMAP Bitmap)
    {
        PostMessage(hwnd,BM_SETIMAGE,(WPARAM)IMAGE_BITMAP,(LPARAM)Bitmap);
    }

    inline void SetIcon(HICON Icon)
    {
        PostMessage(hwnd,BM_SETIMAGE,(WPARAM)IMAGE_ICON,(LPARAM)Icon);
    }
};



class EZWGroup
{
private:
    HWND hwnd;
public:

    EZWGroup(){}
    ~EZWGroup()
    {
        if (hwnd!=NULL)
        DestroyWindow(hwnd);
    }

    inline operator HWND(void)
    {
        return hwnd;
    }

    inline void Create(HWND hParent, char *sCaption, long lFlags, RECT &rc)
    {
        hwnd = CreateWindow(    WC_BUTTON,
                                sCaption,
                                WS_CHILD | WS_VISIBLE | BS_GROUPBOX | lFlags,
                                rc.left,rc.top,
                                rc.right,rc.bottom,
                                hParent,
                                NULL,
                                GetModuleHandle(NULL),
                                NULL);
        Button_Enable(hwnd,TRUE);
    }

    inline void SetLabel(char *Text)
    {
        Button_SetText(hwnd,Text);
    }
};


/******************************************************************
* EZWCheckBox Control
* A standard checkbox control
*/
class EZWCheckBox : public EZWControl
{
public:

    EZWCheckBox(){}
	~EZWCheckBox(){}

    inline void Create(HWND Parent, LPSTR Caption, long Flags, RECT &rc)
    {
        hwnd = CreateWindow(    WC_BUTTON,
                                Caption,
                                WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_AUTOCHECKBOX | Flags,
                                rc.left,rc.top,
                                rc.right,rc.bottom,
                                Parent,
                                NULL,
                                GetModuleHandle(NULL),
                                NULL );
    }

    inline void SetCheck( bool Check )
	{
		int Value = 0;

		if (Check)	Value = BST_CHECKED;
		else		Value = BST_UNCHECKED;

		SendMessage( hwnd, BM_SETCHECK, (WPARAM)Value, (LPARAM)0 );
	}

	inline bool GetCheck( void )
	{
		if ( SendMessage( hwnd, BM_GETCHECK, (WPARAM)0, (LPARAM)0 ) == BST_CHECKED )
			return true;
		else
			return false;
	}
};




class EZWStatic
{
private:
    HWND hwnd;
public:

    EZWStatic(){}
    ~EZWStatic()
    {
        if (hwnd!=NULL)
        DestroyWindow(hwnd);
    }

    inline operator HWND(void)
    {
        return hwnd;
    }

    inline void Create(HWND hParent, char *sCaption, long lFlags, RECT &rc)
    {
        hwnd = CreateWindow(    WC_STATIC,
                                sCaption,
                                WS_CHILD | WS_VISIBLE | lFlags,
                                rc.left,rc.top,
                                rc.right,rc.bottom,
                                hParent,
                                NULL,
                                GetModuleHandle(NULL),
                                NULL);
        Static_Enable(hwnd,TRUE);
    }

    inline void SetText(char *Text)
    {
        Static_SetText(hwnd,Text);
    }

    inline void SetBitmap(HBITMAP Bitmap)
    {
        SendMessage(hwnd, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)Bitmap);
    }
};


class EZWScrollBar : public EZWControl
{
public:

    EZWScrollBar(){}
	~EZWScrollBar(){}

    inline void Create(HWND hParent, DWORD lFlags, RECT &Rc)
    {
        RECT rc,prc;
        GetClientRect(hParent,&prc);
        if (!(lFlags & SBS_VERT))
        {
            rc.left = prc.left;
            rc.right = prc.right-16;
            rc.top = prc.bottom-16;
            rc.bottom = 16;
        }
        if (lFlags & SBS_VERT)
        {
            rc.left = prc.right-16;
            rc.right = 16;
            rc.top = prc.top;
            rc.bottom = prc.bottom-16;
        }

        //flags = lFlags;

        hwnd = CreateWindow(    WC_SCROLLBAR,
                                NULL,
                                WS_CHILD | WS_VISIBLE | lFlags,
                                rc.left,rc.top,
                                rc.right,rc.bottom,
                                hParent,
                                NULL,
                                GetModuleHandle(NULL),
                                NULL);
        int arrow = 0;
        if (lFlags & SBS_HORZ) arrow = SB_HORZ;
        if (lFlags & SBS_VERT) arrow = SB_VERT;
        ScrollBar_Enable(hwnd,arrow);
    }

    inline void Resize(HWND hParent)
    {
        
    }

    inline void SetPos(unsigned int Pos, bool Redraw)
    {
        ScrollBar_SetPos(hwnd,Pos,Redraw);
    }

    inline void SetRange(unsigned int Min, unsigned int Max, bool Redraw)
    {
        ScrollBar_SetRange(hwnd,Min,Max,Redraw);
    }
};



class EZWMenuBar
{
private:
    HMENU   hMenu;
    HMENU   pSubMenus[64];
    int     nSubMenus;
public:

    EZWMenuBar()
    {
        hMenu = NULL;
        nSubMenus = 0;
    }
    ~EZWMenuBar()
    {
        if (nSubMenus>0)
        {
            for (int i=0; i<nSubMenus; i++)
            DestroyMenu(pSubMenus[i]);
        }
        if (hMenu!=NULL)
        DestroyMenu(hMenu);
    }

    inline operator HMENU(void)
    {
        return hMenu;
    }

    inline void Create()
    {
        hMenu = CreateMenu();
    }

    inline int AddPopup(int iSubMenu, char* sName)
    {
		if (iSubMenu == -1)
        {
			pSubMenus[nSubMenus] = CreatePopupMenu();
			AppendMenu(hMenu,MF_POPUP|MF_STRING,(UINT)pSubMenus[nSubMenus], sName);
			nSubMenus++;
		}
		else
		{
			pSubMenus[nSubMenus] = CreatePopupMenu();
			AppendMenu(pSubMenus[iSubMenu],MF_POPUP|MF_STRING,(UINT)pSubMenus[nSubMenus], sName);
			nSubMenus++;
		}
		return nSubMenus-1;
    }

    inline void AddItem(int iSubMenu, char *sItem, unsigned int idItem)
    {
        AppendMenu(pSubMenus[iSubMenu],MF_STRING,idItem,sItem);
    }

    inline void InsertItem(int iSubMenu, unsigned int iPos, char *sItem, unsigned int idItem)
    {
        InsertMenu(pSubMenus[iSubMenu], iPos, MF_STRING, idItem, sItem);
    }

    inline void RemoveItem(int iSubMenu, unsigned int iPos)
    {
        RemoveMenu(pSubMenus[iSubMenu], iPos, 0);
    }

    inline void AddSeparator(int iSubMenu)
    {
        AppendMenu(pSubMenus[iSubMenu],MF_SEPARATOR,0,NULL);
    }

	inline void AddBreak(int iSubMenu)
    {
        AppendMenu(pSubMenus[iSubMenu], MF_MENUBREAK, 0, NULL);
    }
};

class EZWStatusBar
{
private:
    HWND        hwnd;
public:
    int iHeight;

    EZWStatusBar()
    {
        hwnd = NULL;
    }
    ~EZWStatusBar()
    {
        if (hwnd!=NULL) {
            DestroyWindow(this->hwnd);
            this->hwnd = NULL;
        }
    }

    inline operator HWND(void)
    {
        return this->hwnd;
    }
    inline void operator = (const HWND &other)
    {
        hwnd = other;
        return;
    }

    inline void Create(HWND hParent,DWORD idStatus)
    {
        if (hParent == NULL) MessageBox(0,"No parent","b",MB_OK);
        hwnd = CreateWindowEx(0,
                          STATUSCLASSNAME,
                          NULL,
                          WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
                          0,0,0,0,
                          hParent,
                          (HMENU)idStatus,
                          GetModuleHandle(NULL),
                          NULL);
        if (hwnd==NULL) MessageBox(0,"No statusbar","b",MB_OK);
    }

    inline void SetupBar(int nSections, int *pWidths)
    {
        SendMessage(this->hwnd, SB_SETPARTS, nSections,(LPARAM)pWidths);
    }

    inline bool SetBarSection(int iSection, char *sText)
    {
        SendMessage(this->hwnd, SB_SETTEXT, iSection, (LPARAM)sText);
        return true;
    }

    inline int Size()
    {
        RECT rc;
        SendMessage(this->hwnd, WM_SIZE, 0, 0);
        GetWindowRect(this->hwnd, &rc);
        iHeight = rc.bottom - rc.top;
        return rc.bottom - rc.top;
    }
};


enum ENUM_EZW_PROGRESSBAR
{
	EZW_PB_NORMAL		= 0,
	EZW_PB_SCROLLING	= 1,
	EZW_PB_VERTICAL		= 2,
};


class EZWProgressBar : public EZWControl
{
public:

	EZWProgressBar(){}
	~EZWProgressBar(){}

    inline bool Create(HWND hParent, DWORD lFlags, DWORD idProgress)
    {
        if (hParent == NULL)
		{
			MessageBox(hParent, "HANDLE Error\r\nThe parent handle is NULL!","EpicZen Windows Error", MB_ICONERROR);
			return false;
		}

        this->hwnd = CreateWindowEx(0,
								PROGRESS_CLASS,
								NULL,
								WS_CHILD | WS_VISIBLE | PBS_SMOOTH | lFlags,
								0,0,0,0,
								hParent,
								(HMENU)idProgress,
								GetModuleHandle(NULL),
								NULL );
        if (hwnd==NULL)
		{
			MessageBox(hParent, "Win32 & COM Error\r\nFailed to create the control!","EpicZenWindows Error", MB_ICONERROR);
			return false;
		}

		if (lFlags & PBS_MARQUEE)
		{
			SendMessage( hwnd, PBM_SETMARQUEE, (WPARAM)FALSE, (LPARAM)0);
		}

		return true;
    }

    inline void SetRange( DWORD Min, DWORD Max )
    {
		SendMessage(this->hwnd, PBM_SETRANGE, (WPARAM)0, MAKELPARAM( Min, Max ) );
    }

    inline void SetPos( int Pos )
    {
        SendMessage(this->hwnd, PBM_SETPOS, (WPARAM)Pos, (LPARAM)0);
    }

	inline void SetState( int State )
    {
        SendMessage(this->hwnd, PBM_SETSTATE, (WPARAM)State, (LPARAM)0);
    }

	inline void SetStep( int StepSize )
    {
        SendMessage(this->hwnd, PBM_SETSTEP, (WPARAM)StepSize, (LPARAM)0);
    }

	inline void Step( void )
    {
        SendMessage(this->hwnd, PBM_STEPIT, (WPARAM)0, (LPARAM)0);
    }

	inline void SetMarquee( bool Enable, int Ticks )
	{
		SendMessage( this->hwnd, PBM_SETMARQUEE, (WPARAM)Enable, (LPARAM)Ticks );
	}

	inline void SetBKColor( DWORD Color )
    {
        SendMessage(this->hwnd, PBM_SETBKCOLOR, (WPARAM)0, (LPARAM)Color);
    }

	inline void SetBarColor( DWORD Color )
    {
        SendMessage(this->hwnd, PBM_SETBARCOLOR, (WPARAM)0, (LPARAM)Color);
    }
};

class EZWToolBar
{
private:
    HWND        hwnd;
    //TBBUTTON    *tbb;
    //TBADDBITMAP tbab;
    //DWORD       nButtons;
public:
    int iHeight;

    EZWToolBar()
    {
        this->hwnd = NULL;
        //this->nButtons = 0;
    }
    ~EZWToolBar()
    {
        if (this->hwnd!=NULL)
        DestroyWindow(this->hwnd);
    }

    inline operator HWND(void)
    {
        return this->hwnd;
    }

    inline void Create(HWND hParent,DWORD idTool)
    {
        this->hwnd = CreateWindowEx(0,
                          TOOLBARCLASSNAME,
                          NULL,
                          WS_CHILD | WS_VISIBLE,
                          0,0,0,0,
                          hParent,
                          (HMENU)idTool,
                          GetModuleHandle(NULL),
                          NULL);

        // Compatability fix
        SendMessage(this->hwnd,TB_BUTTONSTRUCTSIZE,(WPARAM)sizeof(TBBUTTON),0);
    }

    inline void CreateButtons(unsigned int idBitmap)
    {
        TBADDBITMAP tbab;

        tbab.hInst = GetModuleHandle(NULL);
        tbab.nID = idBitmap;

        SendMessage(hwnd, TB_ADDBITMAP, 3,(LONG)&tbab);

        TBBUTTON tbb[5];

        /* Make and define the toolbar buttons */
        ZeroMemory(tbb, sizeof(tbb));
        tbb[0].iBitmap = 0;
        tbb[0].fsState = TBSTATE_ENABLED;
        tbb[0].fsStyle = TBSTYLE_BUTTON;
        tbb[0].idCommand = 5;
        tbb[1].iBitmap = 1;
        tbb[1].fsState = TBSTATE_ENABLED;
        tbb[1].fsStyle = TBSTYLE_BUTTON;
        tbb[1].idCommand = 6;
        tbb[2].iBitmap = 2;
        tbb[2].fsState = TBSTATE_ENABLED;
        tbb[2].fsStyle = TBSTYLE_BUTTON;
        tbb[2].idCommand = 8;
        tbb[3].iBitmap = 0;
        tbb[3].fsState = TBSTATE_ENABLED;
        tbb[3].fsStyle = TBSTYLE_SEP;
        tbb[3].idCommand = 0;
        tbb[4].iBitmap = 3;
        tbb[4].fsState = TBSTATE_ENABLED;
        tbb[4].fsStyle = TBSTYLE_BUTTON;
        tbb[4].idCommand = 0;

        SendMessage(hwnd, TB_ADDBUTTONS, sizeof(tbb)/sizeof(TBBUTTON), (LPARAM)&tbb);
    }

    inline int Size()
    {
        RECT rc;
        SendMessage(hwnd, TB_AUTOSIZE, 0, 0);
        GetWindowRect(hwnd, &rc);
        iHeight = rc.bottom - rc.top;
        return rc.bottom - rc.top;
    }
};

class EZWToolTip
{
private:
	HWND hwnd;
	HWND control;
public:

	EZWToolTip()
	{
		hwnd = NULL;
		control = NULL;
	}
	~EZWToolTip()
	{
		DestroyWindow( this->hwnd );
	}

	inline operator HWND(void)
    {
        return this->hwnd;
    }

	inline void Create(HWND Parent, HWND Control, LPSTR Text)
	{
		this->control = Control;
		this->hwnd = CreateWindowEx(0,
									TOOLTIPS_CLASS,
									NULL, 
									WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP, 
									CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
									NULL,
									NULL,
									GetModuleHandle( NULL ),
									NULL);
		SetWindowPos(this->hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

		TOOLINFO ti;
		memset(&ti, 0, sizeof(TOOLINFO));
		ti.cbSize = sizeof(TOOLINFO);
		ti.hwnd = Parent;
		ti.uId = (UINT) Control;
		ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
		ti.lpszText = Text;
		SendMessage(this->hwnd, TTM_ADDTOOL, 0, (LPARAM) &ti);
	}
};
