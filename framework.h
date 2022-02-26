#pragma once
/*		___      __ _
	   /   \___ / _(_)_ __   ___
	  / /\ / _ \ |_| | '_ \ / _ \
	 / /_//  __/  _| | | | |  __/
	/___,' \___|_| |_|_| |_|\___|	*/
#define FastPERCd(Actual,Total) (((double)Actual*100.0)/(double)Total) 
#define FastPERCi(Actual,Total) (uint64_t)(((double)Actual*100.0)/(double)Total) 
#define idListView 0x100
#define MAX_PATH_EX 32767

	/*	  _____            _           _
		  \_   \_ __   ___| |_   _  __| | ___
		   / /\/ '_ \ / __| | | | |/ _` |/ _ \
		/\/ /_ | | | | (__| | |_| | (_| |  __/
		\____/ |_| |_|\___|_|\__,_|\__,_|\___|	*/
#include "targetver.h"					//NZArchive target version
#include <windows.h>					//Windows base function
#include "resource.h"					//Resource of NZArchive
#include <CommCtrl.h>					//Windows modern control
#pragma comment(lib, "comctl32.lib")
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#include <Uxtheme.h>					//Progressbar color change
#pragma comment(lib,"UxTheme.lib")
#include <string>						//Base string class
#include <iomanip>						//Help with time manipulation
#include <atlbase.h>					//Progress bar and open/save dialog
#include <thread>						//Detaching operation in background
#include <random>						//Session generation
#include <fstream>						//Language parser
#include <sstream>						//Split function
#include <ShlObj.h>						//Shell operation
#include "pathHelper.h"					//Path helper
#include <NZArchive/NZArchive.hpp>		//NZArchive header
#include "Translator.hpp"				//Translation helper
#include "plf_nanotimer.h"				//Timer class
#include "WindowsExplorerHELPER.hpp"	//Windows Explorer helper
#include <propkey.h>					//Add to recent Prop_KEY (TITLE)
#include <VersionHelpers.h>				//Check If Windows Vista or more
using namespace std::chrono_literals;	//Using time litterals like 100ms

/*				  _                  _           _                     _           _                 _   _
	  /\/\   __ _(_)_ __   __      _(_)_ __   __| | _____      __   __| | ___  ___| | __ _ _ __ __ _| |_(_) ___  _ __
	 /    \ / _` | | '_ \  \ \ /\ / / | '_ \ / _` |/ _ \ \ /\ / /  / _` |/ _ \/ __| |/ _` | '__/ _` | __| |/ _ \| '_ \
	/ /\/\ \ (_| | | | | |  \ V  V /| | | | | (_| | (_) \ V  V /  | (_| |  __/ (__| | (_| | | | (_| | |_| | (_) | | | |
	\/    \/\__,_|_|_| |_|   \_/\_/ |_|_| |_|\__,_|\___/ \_/\_/    \__,_|\___|\___|_|\__,_|_|  \__,_|\__|_|\___/|_| |_|
																													*/
HINSTANCE hInst;//Main dialog windows HINSTANCE
WCHAR szTitle[MAX_PATH];//Main dialog windows title
WCHAR szWindowClass[MAX_PATH];//Main dialog windows class
static HWND g_hWnd;//Main dialog windows HWND

/*	 __ _                   _
	/ _\ |_ _ __ _   _  ___| |_ _   _ _ __ ___
	\ \| __| '__| | | |/ __| __| | | | '__/ _ \
	_\ \ |_| |  | |_| | (__| |_| |_| | | |  __/
	\__/\__|_|   \__,_|\___|\__|\__,_|_|  \___|	 */

	/// @brief	Icon structure 
	/// @param	index	Index of the icon
	/// @param	pIcon	HICON handle for the icon
	/// @param	wstring	Extension associated to this icon
struct iconStruct
{
	int index = 0;
	HICON pIcon;
	std::wstring pExtension;
};

/// @brief	Icon structure 
/// @param	FileIndex		Fileindex in the list
/// @param	fullPath		Fullpath of the item
struct _ListItemRetrieve
{
	size_t FileIndex = 0;
	std::wstring fullPath = L"";
};

/// @brief	Icon structure 
/// @param	mPAth		Local NZArchivePath of the file
/// @param	mtPAthInArchive	Path in the archive (must start with \ )
/// @param	mFileSize	File size
struct _CreateListAdd
{
	std::wstring mPAth = L"";
	std::wstring mtPAthInArchive = L"";
	uint64_t mFileSize = 0;
};

/*					  _       _     _
	 /\   /\__ _ _ __(_) __ _| |__ | | ___
	 \ \ / / _` | '__| |/ _` | '_ \| |/ _ \
	  \ V / (_| | |  | | (_| | |_) | |  __/
	   \_/ \__,_|_|  |_|\__,_|_.__/|_|\___|

*/
//FONT
static HFONT hFont14 = NULL;		//Font with size 14
static HFONT hFont14bold = NULL;	//Font with size 14 Bold
static HFONT hFont20 = NULL;		//Font with size 20
static HFONT hFont20bold = NULL;	//Font with size 20 Bold
//HWND
static HWND ghListView;		//Handle to file list listview
static HWND ghToolBar;		//Handle to the toolbar
static HWND ghTEXTNAV;		//Handle to the text navigation
static HWND ghStatusbar;	//Handle to the status bar
//STRING
static std::string mPassword = ""; //String containing the password
//WSTRING
static std::wstring SessionPath = L"";		//Wstring containing the session NZArchivePath
static std::wstring mExtractPAth = L"";		//Wstring containing the extraction NZArchivePath
std::wstring mArchiveName = L"";			//Wstring containing the archive IconPath
std::wstring ListTREEpth = L"\\";			//Wstring containing the NZArchivePath used for explorer like navigation in main file list view
//BOOL
static bool ExtractAll = false;					//bool meaning if extract all or selected file
static bool ExtractToExplorerWindows = false;	//bool meaning if extraction process is redirected to save as dialog or to a Windows Explorer window
static bool NPARAnalysisConform = false;		//bool returning if NPAR file analysis is conform
static bool OperationInProgress = false;		//bool telling if an operation is in progress
static bool NeedAnalysis = false;				//bool returning the analysis need
bool ListViewSortOrder[9];						//bool memorizing sort status of the listview
//bool OpenFromcommandLine = false;				//bool to tell if the archive to open is y user choice (Open) or from the command line
//bool isLocalizationLoaded = false;				//bool telling if Localization is successfully loaded
bool OpenWith = false;							//bool telling if file must be open using defaut association or user selected one
bool AddingFromDrop = false;					//bool telling if the files to add come from user choice (file / folders) or from an external drop
//NZArchive
NZArchive::Archive::ArchiveEntry OpenListing;	//NZArchive used for Opened archive file listing
NZArchive::Archive::ArchiveEntry ListTREE;		//NZArchive used specifically for listview navigation
//WindowsExplorerHelper
auto WFHelper = new WindowsExplorerHelper();	//Windows explorer helper for retrieving all opened Explorer windows
//VECTOR
static std::vector<_ListItemRetrieve> FileToTreat;	//Vector with all files to treat
std::vector<iconStruct> vIconStruct;				//Vector with all loaded icon for file association
static std::vector<std::wstring> vDropFILE;			//Vector with files to add via dropping
static 	std::vector<std::wstring> vCMDFilesToAdd;

//INT64
static int64_t FastExtractIndex = -1;			//Index of the file to fast extract
//IMAGELIST
HIMAGELIST gLVIimgList = NULL;	//Image list used in the listiew (dynamically generated according to files in the archive)
HIMAGELIST gTBImgList = NULL;	//Image list for the main toolbar
//StatusBar ID's
constexpr auto ID_OPEN = 0x2340;		//Id's for open button button in toolbar
constexpr auto ID_CREATE = 0x2341;		//Id's for create button button in toolbar
constexpr auto ID_ARCINFO = 0x2342;		//Id's for archive information button button in toolbar
constexpr auto ID_EXTRACTALL = 0x2343;	//Id's for extract all button button in toolbar
constexpr auto ID_EXTRACTSEL = 0x2344;	//Id's for extract selection button button in toolbar
constexpr auto ID_ADDTO = 0x2345;		//Id's for add to button button in toolbar
constexpr auto ID_DELETE = 0x2346;		//Id's for delete button button in toolbar
constexpr auto ID_TEST = 0x2347;		//Id's for test button button in toolbar
constexpr auto ID_TEXTNAV = 0x2350;		//Id's for navigation bar (read only) in main window
//Command line
enum class CmdMode : uint8_t { Mode_Open, Mode_Create, Mode_Extract, Mode_None, Mode_DropFile };
static CmdMode sCMDMode = CmdMode::Mode_None;	//Keep trace of command line arguments

/*	   ___          _                     __                  _   _
	  / __\   _ ___| |_ ___  _ __ ___    / _|_   _ _ __   ___| |_(_) ___  _ __
	 / / | | | / __| __/ _ \| '_ ` _ \  | |_| | | | '_ \ / __| __| |/ _ \| '_ \
	/ /__| |_| \__ \ || (_) | | | | | | |  _| |_| | | | | (__| |_| | (_) | | | |
	\____/\__,_|___/\__\___/|_| |_| |_| |_|  \__,_|_| |_|\___|\__|_|\___/|_| |_|
*/

/// @brief	Generate a random wstring sequence of a given length.
/// @param	length	Length of the random wstring sequence
/// @return	WSTRING with the random wstring
std::wstring random_string(std::size_t length)
{
	const std::wstring CHARACTERS = L"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	std::random_device random_device;
	std::mt19937 generator(random_device());
	std::uniform_int_distribution<> distribution(0, (int)(CHARACTERS.size() - 1));
	std::wstring random_string;
	for (std::size_t i = 0; i < length; ++i)
		random_string += CHARACTERS[distribution(generator)];
	return random_string;
}

/// @brief	Center the windows inside parent HWND Rect.
/// @param	hwndWindow	HWND of parent windows
/// @return	BOOL CenterWindow failed or successed ?
BOOL CenterWindowFromParent(HWND hwndWindow)
{
	HWND hwndParent;
	RECT rectWindow, rectParent;

	// make the window relative to its parent
	if ((hwndParent = GetParent(hwndWindow)) != NULL)
	{
		GetWindowRect(hwndWindow, &rectWindow);
		GetWindowRect(hwndParent, &rectParent);

		int nWidth = rectWindow.right - rectWindow.left;
		int nHeight = rectWindow.bottom - rectWindow.top;

		int nX = ((rectParent.right - rectParent.left) - nWidth) / 2 + rectParent.left;
		int nY = ((rectParent.bottom - rectParent.top) - nHeight) / 2 + rectParent.top;

		int nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
		int nScreenHeight = GetSystemMetrics(SM_CYSCREEN);
		MoveWindow(hwndWindow, nX, nY, nWidth, nHeight, FALSE);

		return TRUE;
	}

	return FALSE;
}

/// @brief	Center the windows inside active screen.
/// @param	hwndWindow	HWND of parent windows
/// @return	BOOL CenterWindow failed or successed ?
BOOL CenterWindowFromActiveScreen(HWND hwndWindow)
{
	POINT p;
	if (GetCursorPos(&p))
	{
		RECT rcWindow;
		GetWindowRect(hwndWindow, &rcWindow);
		RECT rcMouse = { p.x,p.y,p.x + 1,p.y + 1 };
		MONITORINFOEX lpmi2;
		lpmi2.cbSize = sizeof(MONITORINFOEX);
		if (GetMonitorInfo(MonitorFromRect(&rcMouse, MONITOR_DEFAULTTONEAREST), &lpmi2))
		{
			int xPos = ((lpmi2.rcMonitor.right - lpmi2.rcMonitor.left) - (rcWindow.right - rcWindow.left)) / 2;
			int yPos = ((lpmi2.rcMonitor.bottom - lpmi2.rcMonitor.top) - (rcWindow.bottom - rcWindow.top)) / 2;
			MoveWindow(hwndWindow, lpmi2.rcMonitor.left + xPos, lpmi2.rcMonitor.top + yPos, rcWindow.right- rcWindow.left, rcWindow.bottom - rcWindow.top, TRUE);
		}
	}
	return 0;
}

/// @brief	Format a wstring with a space for thousand millions ....
/// @param	n	INT64 number to add thousand space
/// @return	WSTRING with the formatted wstring
std::wstring thousandSeparator(INT64 n)
{
	std::wstring ans(L"");
	std::wstring num = std::to_wstring(n);
	int count = 0;
	for (int i = (int)num.size() - 1; i >= 0; i--) {
		count++;
		ans.push_back(num[i]);
		if (count == 3) {
			ans.push_back(L' ');
			count = 0;
		}
	}
	reverse(ans.begin(), ans.end());
	if (ans.size() % 4 == 0) {

		ans.erase(ans.begin());
	}

	return ans;
}

/// @brief	Create a listview inside Parents HWND
/// @param	hwndParent	HWND of parent windows
/// @return	HWND containing the ListView handle
HWND CreateListView(HWND hwndParent)
{
	HWND hWndListView;
	RECT rcClient;                       // The parent window's client area.
	GetClientRect(hwndParent, &rcClient);
	hWndListView = CreateWindowEx(0, (LPCWSTR)WC_LISTVIEW, NULL, WS_VISIBLE | WS_CHILD | /*WS_BORDER |*/ LVS_SHOWSELALWAYS | LVS_REPORT | LVS_EX_DOUBLEBUFFER | LVS_OWNERDATA, 0, 0,
		rcClient.right - rcClient.left,
		rcClient.bottom - rcClient.top,
		hwndParent, (HMENU)idListView, NULL, NULL);
	HINSTANCE dllhinst;
	HICON hIcon;
	InitCommonControls();
	gLVIimgList = ImageList_Create(16, 16, ILC_COLOR32, 3, 3);
	ImageList_SetBkColor(gLVIimgList, GetSysColor(COLOR_WINDOW));
	dllhinst = LoadLibrary(L"shell32.dll");
	if (dllhinst != 0)
	{
		hIcon = LoadIcon(dllhinst, MAKEINTRESOURCE(4));
		ImageList_AddIcon(gLVIimgList, hIcon);
		hIcon = LoadIcon(dllhinst, MAKEINTRESOURCE(1));
		ImageList_AddIcon(gLVIimgList, hIcon);
		FreeLibrary(dllhinst);
		ListView_SetImageList(hWndListView, gLVIimgList, LVSIL_SMALL);
	}
	ListView_SetExtendedListViewStyle(hWndListView, LVS_EX_INFOTIP | LVS_EX_LABELTIP);

	SendMessage(hWndListView, WM_SETFONT, (WPARAM)hFont20, FALSE);

	return (hWndListView);
}

/// @brief	Initialize all columns of a Listview
/// @param	hWndListView	HWND of the Listview to initialize
/// @return	BOOL True if init succeeded, False if init failed
BOOL InitListViewColumns(HWND hWndListView)
{
	LVCOLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.iSubItem = 0;
	lvc.pszText = (LPWSTR)L"Name";
	lvc.cx = 400;
	lvc.fmt = LVCFMT_LEFT;
	if (ListView_InsertColumn(hWndListView, 0, &lvc) == -1)
		return FALSE;

	lvc.iSubItem = 1;
	lvc.pszText = (LPWSTR)L"Size";
	lvc.cx = 100;
	lvc.fmt = LVCFMT_RIGHT;
	if (ListView_InsertColumn(hWndListView, 1, &lvc) == -1)
		return FALSE;

	lvc.iSubItem = 2;
	lvc.pszText = (LPWSTR)L"Packed size";
	lvc.cx = 100;
	lvc.fmt = LVCFMT_RIGHT;
	if (ListView_InsertColumn(hWndListView, 2, &lvc) == -1)
		return FALSE;

	lvc.iSubItem = 3;
	lvc.pszText = (LPWSTR)L"Modified";
	lvc.cx = 140;
	lvc.fmt = LVCFMT_LEFT;
	if (ListView_InsertColumn(hWndListView, 3, &lvc) == -1)
		return FALSE;

	lvc.iSubItem = 4;
	lvc.pszText = (LPWSTR)L"Created";
	lvc.cx = 140;
	lvc.fmt = LVCFMT_LEFT;
	if (ListView_InsertColumn(hWndListView, 4, &lvc) == -1)
		return FALSE;

	lvc.iSubItem = 5;
	lvc.pszText = (LPWSTR)L"Accessed";
	lvc.cx = 140;
	lvc.fmt = LVCFMT_LEFT;
	if (ListView_InsertColumn(hWndListView, 5, &lvc) == -1)
		return FALSE;

	lvc.iSubItem = 6;
	lvc.pszText = (LPWSTR)L"Attribute";
	lvc.cx = 80;
	lvc.fmt = LVCFMT_LEFT;
	if (ListView_InsertColumn(hWndListView, 6, &lvc) == -1)
		return FALSE;

	lvc.iSubItem = 7;
	lvc.pszText = (LPWSTR)L"Offset";
	lvc.cx = 100;
	lvc.fmt = LVCFMT_LEFT;
	if (ListView_InsertColumn(hWndListView, 7, &lvc) == -1)
		return FALSE;

	lvc.iSubItem = 8;
	lvc.pszText = (LPWSTR)L"Folder";
	lvc.cx = 60;
	lvc.fmt = LVCFMT_LEFT;
	if (ListView_InsertColumn(hWndListView, 9, &lvc) == -1)
		return FALSE;

	return TRUE;
}

/// @brief	Convert a givent float number to a formated wstring (eg. 312000 => 300KiB)
/// @param	Bytes	Value to be formatted
/// @return	WSTRING with formatted size
std::wstring BytesToSize(float Bytes) {

	float tb = 1099511627776;
	float gb = 1073741824;
	float mb = 1048576;
	float kb = 1024;
	wchar_t returnSize[256];
	if (Bytes >= tb)
		swprintf_s(returnSize, L"%.2f TiB", (float)Bytes / tb);
	else if (Bytes >= gb && Bytes < tb)
		swprintf_s(returnSize, L"%.2f GiB", (float)Bytes / gb);
	else if (Bytes >= mb && Bytes < gb)
		swprintf_s(returnSize, L"%.2f MiB", (float)Bytes / mb);
	else if (Bytes >= kb && Bytes < mb)
		swprintf_s(returnSize, L"%.2f KiB", (float)Bytes / kb);
	else if (Bytes < kb)
		swprintf_s(returnSize, L"%.2f B", Bytes);
	else
		swprintf_s(returnSize, L"%.2f B", Bytes);
	return std::wstring(returnSize);
}

/// @brief	std::sort sorter for file string analysis of a FileEntryRead
/// @param	left	First FileEntryRead to compare
/// @param	right	Second FileEntryRead to compare
/// @return	BOOL std::sort result
bool sorter(const NZArchive::Archive::FileEntryRead& left, const NZArchive::Archive::FileEntryRead& right)
{
	std::wstring cpLeft = left.FullPath;
	std::wstring cpRight = right.FullPath;
	std::transform(cpLeft.begin(), cpLeft.end(), cpLeft.begin(), towlower);
	std::transform(cpRight.begin(), cpRight.end(), cpRight.begin(), towlower);
	//go through each column
	for (int i = 0; i < (int)cpLeft.size() && i < (int)right.FullPath.size(); i++) {
		// if cpLeft is "more" return that we go higher
		if (cpLeft[i] > cpRight[i])
			return true;
		// if cpLeft is "less" return that we go lower
		else if (cpLeft[i] < cpRight[i])
			return false;
	}
	// if cpLeft is longer, it goes higher
	if (cpLeft.size() > cpRight.size())
		return true;
	else //otherwise, cpLeft go lower
		return false;
}

/// @brief	Insert Message in a Listview at given position with date shown
/// @param	_Hwnd		Listview HWND
/// @param	_Message	Message to insert
/// @param	Position		(Defaut 0) Insert item at given position (0=At the beginning ; -1=At the end)
/// @param	showDate	(Defaut TRUE) Begin message with date/time
void InsertItemListbox(HWND _Hwnd, std::wstring _Message, int Position = 0 /*First item in the list*/, bool showDate = true)
{
	if (showDate)
	{
		time_t     now = time(0);
		struct tm  tstruct;
		std::vector<wchar_t> Buffer(1024);
		localtime_s(&tstruct, &now);
		wcsftime(&Buffer[0], Buffer.size(), L"%Y-%m-%d %X", &tstruct);
		std::wstring ConstMsg(Buffer.data());
		ConstMsg += L" : " + _Message;
		SendMessage(_Hwnd, LB_INSERTSTRING, Position, (LPARAM)ConstMsg.c_str());
	}
	else
	{
		SendMessage(_Hwnd, LB_INSERTSTRING, Position, (LPARAM)_Message.c_str());
	}
}

/// @brief	Create a Statusbar to a Windows divided in specified parts
/// @param	hwndParent	Windows HWND
/// @param	hinst		Windows HINSTANCE
/// @param	cParts		Number of parts
/// @return	HWND containing the StatusBar handle
HWND CreateStatusBar(HWND hwndParent, HINSTANCE	hinst, int cParts)
{
	HWND hwndStatus;
	RECT rcClient;
	HLOCAL hloc;
	PINT paParts;
	int i, nWidth;
#define idStatus 0x1723
	// Ensure that the common control DLL is loaded.
	InitCommonControls();

	// Create the status bar.
	hwndStatus = CreateWindowEx(
		0,                       // no extended styles
		STATUSCLASSNAME,         // IconPath of status bar class
		(PCTSTR)NULL,           // no text when first created
		SBARS_SIZEGRIP |         // includes a sizing grip
		WS_CHILD | WS_VISIBLE,   // creates a visible child window
		0, 0, 0, 0,              // ignores size and position
		hwndParent,              // handle to parent window
		(HMENU)idStatus,       // child window identifier
		hinst,                   // handle to application instance
		NULL);                   // no window creation data

	// Get the coordinates of the parent window's client area.
	GetClientRect(hwndParent, &rcClient);

	// Allocate an array for holding the right edge coordinates.
	hloc = LocalAlloc(LHND, sizeof(int) * cParts);
	paParts = (PINT)LocalLock(hloc);

	// Calculate the right edge coordinate for each part, and
	// copy the coordinates to the array.
	nWidth = rcClient.right / cParts;
	int rightEdge = nWidth;
	for (i = 0; i < cParts; i++) {
		paParts[i] = rightEdge;
		rightEdge += nWidth;
	}

	// Tell the status bar to create the window parts.
	SendMessage(hwndStatus, SB_SETPARTS, (WPARAM)cParts, (LPARAM)
		paParts);

	// Free the array, and return.
	LocalUnlock(hloc);
	LocalFree(hloc);
	return hwndStatus;
}

/// @brief	Update Statusbar parts with given text
/// @param	_Sbar		Statusbar HWND
/// @param	_Message	Message to write to HWND
/// @param	_PartId		Parts of the Statusbar to update (Defaut=0)
void UpdateStatusBar(HWND _Sbar, std::wstring _Message, int _PartId = 0)
{
	SendMessage(_Sbar, SB_SETTEXTW, (WPARAM)_PartId | SBT_NOBORDERS/*SBT_POPOUT*/, (LPARAM)_Message.c_str());
}

/// @brief	Create a Toolbar in a windows
/// @param	hWndParent	Handle of the parents Window
/// @param	hwndToolBar	Return the Toolbar HWND (Must be NULL initialized)
/// @param	hInst		Hinstance of the parent Window
void CreateToolbar(HWND hWndParent, HWND& hwndToolBar, HINSTANCE hInst)
{
	// Declare and initialize local constants.
	const int ImageListID = 0;
	const int numButtons = 11;
	const DWORD buttonStyles = BTNS_AUTOSIZE;
	// Create the toolbar.
	hwndToolBar = CreateWindowEx(0, TOOLBARCLASSNAME, NULL,
		WS_CHILD | /*TBSTYLE_WRAPABLE |*/ TBSTYLE_FLAT, 0, 0, 0, 0,
		hWndParent, NULL, hInst, NULL);
	SendMessage(hwndToolBar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
	ImageList_Destroy(gTBImgList);
	gTBImgList = NULL;
	gTBImgList = ImageList_Create(32, 32, ILC_COLOR32 | ILC_MASK, numButtons, 0);
	enum class ILidx : int { idxADD, idxCreate, idxArcInfo, idxDelete, idxExtractAll, idxExtractSel, idxOpen, idxTest };
	//Adding in same order as ILidx to have image 0 = index 0 etc...
	ImageList_AddIcon(gTBImgList, LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ADD)));
	ImageList_AddIcon(gTBImgList, LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_CREATE)));
	ImageList_AddIcon(gTBImgList, LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ARCINFO)));
	ImageList_AddIcon(gTBImgList, LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_DELETE)));
	ImageList_AddIcon(gTBImgList, LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_EXTRACTALL)));
	ImageList_AddIcon(gTBImgList, LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_EXTRACTSEL)));
	ImageList_AddIcon(gTBImgList, LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_OPEN)));
	ImageList_AddIcon(gTBImgList, LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_TEST)));
	SendMessage(hwndToolBar, TB_SETIMAGELIST, (WPARAM)0, (LPARAM)gTBImgList);
	TBBUTTON tbButtons[numButtons] =
	{
		{ (int)ILidx::idxOpen		, ID_OPEN		, TBSTATE_ENABLED,	 BTNS_AUTOSIZE, {0}, 0, (INT_PTR)TRANSLATE(L"TOOLBOX_OPEN", L"Open archive\0").c_str()},
		{ (int)ILidx::idxCreate		, ID_CREATE		, TBSTATE_ENABLED, BTNS_AUTOSIZE, {0}, 0, (INT_PTR)TRANSLATE(L"TOOLBOX_CREATE", L"Create archive\0").c_str() },
		{ (int)ILidx::idxArcInfo	, ID_ARCINFO	, 0, BTNS_AUTOSIZE, {0}, 0, (INT_PTR)TRANSLATE(L"TOOLBOX_INFORMATION", L"Archive information\0").c_str() },
		{ 0, NULL, 0, BTNS_SEP, {0}, 0, 0 },
		{ (int)ILidx::idxExtractAll	, ID_EXTRACTALL	, 0, BTNS_AUTOSIZE, {0}, 0, (INT_PTR)TRANSLATE(L"TOOLBOX_EALL", L"Extract all\0").c_str() },
		{ (int)ILidx::idxExtractSel	, ID_EXTRACTSEL	, 0, BTNS_AUTOSIZE, {0}, 0, (INT_PTR)TRANSLATE(L"TOOLBOX_ESEL", L"Extract selection\0").c_str() },
		{ 0, NULL, 0, BTNS_SEP, {0}, 0, 0 },
		{ (int)ILidx::idxADD		, ID_ADDTO		, 0, BTNS_AUTOSIZE, {0}, 0, (INT_PTR)TRANSLATE(L"TOOLBOX_ADD", L"Add to archive\0").c_str() },
		{ (int)ILidx::idxDelete		, ID_DELETE		, 0, BTNS_AUTOSIZE | BS_MULTILINE, {0}, 0, (INT_PTR)TRANSLATE(L"TOOLBOX_DELETE", L"Delete from archive\0").c_str() },
		{ 0, NULL, 0, BTNS_SEP	, {0}, 0, 0 },
		{ (int)ILidx::idxTest		, ID_TEST		, 0, BTNS_AUTOSIZE, {0}, 0, (INT_PTR)TRANSLATE(L"TOOLBOX_TEST", L"Test archive\0").c_str() },
	};
	SendMessage(hwndToolBar, TB_ADDBUTTONS, numButtons, (LPARAM)tbButtons);
	SendMessage(hwndToolBar, WM_SETFONT, (WPARAM)hFont14, FALSE);
	SendMessage(hwndToolBar, TB_AUTOSIZE, 0, 0);
	ShowWindow(hwndToolBar, TRUE);
	return;
}

/// @brief	Update text of a Toolbar button
/// @param	hTBAR		Handle of the ToolBar
/// @param	buttonID		Number ID's of the button to update text (From 0 to xxx)
/// @param	Message		Message to write
/// @param	Enabled		Button is enabled or disabled after update ?
void UI_UpdateTOOLBARtext(HWND hTBAR, DWORD buttonID, const wchar_t* Message, bool Enabled = true)
{
	TBBUTTONINFO tbInfo;
	tbInfo.cbSize = sizeof(TBBUTTONINFO);
	tbInfo.dwMask = TBIF_TEXT /*| (Enabled? TBSTATE_ENABLED:0)*/;
	tbInfo.pszText = (LPWSTR)Message;
	SendMessage(hTBAR, TB_SETBUTTONINFO, (WPARAM)buttonID, (LPARAM)&tbInfo);
	SendMessage(hTBAR, TB_ENABLEBUTTON, (WPARAM)buttonID, MAKELONG(Enabled ? 1 : 0, 0));
}

/// @brief	Enable/disable a Toolbar button
/// @param	hTBAR		Handle of the ToolBar
/// @param	buttonID		Number ID's of the button to update text (From 0 to xxx)
/// @param	Enabled		Button is enabled or disabled ?
void UI_TOOLBAREnable(HWND hTBAR, DWORD buttonID, bool Enabled = true)
{
	SendMessage(hTBAR, TB_ENABLEBUTTON, (WPARAM)buttonID, MAKELONG(Enabled ? 1 : 0, 0));
}

/// @brief	Split a wstring with a given separator
/// @param	sentences		wstring to split
/// @param	separator		wchar_t used to split
/// @return	WSTRING vector with splitted word using separator
std::vector<std::wstring> SplitWstring(std::wstring sentences, wchar_t separator)
{
	std::wstring temp;
	std::vector<std::wstring> parts;
	std::wstringstream wss(sentences);
	while (std::getline(wss, temp, separator))
		if (temp != L"")
			parts.push_back(temp);
	return parts;
}

/// @brief	Search for multiple word in a sentence
/// @param	_where		Sentences to be searched
/// @param	_what		Sequences of words to be found (Separated with a given separator)
/// @param	_separator	Separator for words sequence
/// @return	BOOL True if all words are found, false if none or not all words
bool MultiSearch(std::wstring _where, std::wstring _what, std::wstring  _separator)
{
	std::transform(_where.begin(), _where.end(), _where.begin(), towlower);
	std::transform(_what.begin(), _what.end(), _what.begin(), towlower);
	std::transform(_separator.begin(), _separator.end(), _separator.begin(), towlower);
	wchar_t separator = _separator[0];

	auto ListeMot = SplitWstring(_what, separator);
	int nbOK = 0;
	for (size_t i = 0; i < ListeMot.size(); i++)
	{
		if (_where.find(ListeMot[i].c_str(), 0) != std::wstring::npos)
			nbOK++;
	}
	if (nbOK == ListeMot.size())
		return true;
	return false;

}


/// @brief	Replaces all occurrences of the 'oldValue' wstring with the 'newValue' wstring
/// @param	source		Source Wstring
/// @param	oldValue	Wstring to be replaced
/// @param	newValue	Wstring to replace all occurences of oldValue
/// @return	WSTRING with all occurrences replaced
std::wstring replaceAll(const std::wstring& source
	, const std::wstring& oldValue, const std::wstring& newValue) {
	if (oldValue.empty()) {
		return source;
	}
	std::wstring newString;
	newString.reserve(source.length());
	std::size_t lastPos = 0;
	std::size_t findPos;
	while (std::wstring::npos != (findPos = source.find(oldValue, lastPos))) {
		newString.append(source, lastPos, findPos - lastPos);
		newString += newValue;
		lastPos = findPos + oldValue.length();
	}
	newString += source.substr(lastPos);
	return newString;
}

/// @brief	Replaces all occurrences of the 'oldValue' wstring with the 'newValue' wstring
/// @param	source		Source Wstring
/// @param	oldValue	wstring to be replaced
/// @param	newValue	const wchar_t to replace all occurences of oldValue
/// @return	WSTRING with all occurrences replaced
std::wstring replaceAll(const std::wstring& source
	, const std::wstring& oldValue, const wchar_t& newValue) {
	if (oldValue.empty()) {
		return source;
	}
	std::wstring newString;
	newString.reserve(source.length());
	std::size_t lastPos = 0;
	std::size_t findPos;
	while (std::wstring::npos != (findPos = source.find(oldValue, lastPos))) {
		newString.append(source, lastPos, findPos - lastPos);
		newString += newValue;
		lastPos = findPos + oldValue.length();
	}
	newString += source.substr(lastPos);
	return newString;
}

/// @brief	Convert FileAttribute DWORD to a readable wstring 
/// @param	FileAttributeID	DWORD FileAttribute
/// @return	wchar* readable wstring
wchar_t* FileAttributeFromID(DWORD FileAttributeID)
{
	std::wstring vOUT;
	if (FileAttributeID & FILE_ATTRIBUTE_READONLY) vOUT += L"A_ReadOnly;";
	if (FileAttributeID & FILE_ATTRIBUTE_HIDDEN) vOUT += L"A_Hidden;";
	if (FileAttributeID & FILE_ATTRIBUTE_SYSTEM) vOUT += L"A_System;";
	if (FileAttributeID & FILE_ATTRIBUTE_DIRECTORY) vOUT += L"A_Directory;";
	if (FileAttributeID & FILE_ATTRIBUTE_ARCHIVE) vOUT += L"A_Archive;";
	if (FileAttributeID & FILE_ATTRIBUTE_DEVICE) vOUT += L"A_Device;";
	if (FileAttributeID & FILE_ATTRIBUTE_NORMAL) vOUT += L"A_Normal;";
	if (FileAttributeID & FILE_ATTRIBUTE_TEMPORARY) vOUT += L"A_Temoprary;";
	if (FileAttributeID & FILE_ATTRIBUTE_SPARSE_FILE) vOUT += L"A_SparseFile;";
	if (FileAttributeID & FILE_ATTRIBUTE_REPARSE_POINT) vOUT += L"A_ReparsePoint;";
	if (FileAttributeID & FILE_ATTRIBUTE_COMPRESSED) vOUT += L"A_Compressed;";
	if (FileAttributeID & FILE_ATTRIBUTE_OFFLINE) vOUT += L"A_Offline;";
	if (FileAttributeID & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED) vOUT += L"ANotContentIndexed;";
	if (FileAttributeID & FILE_ATTRIBUTE_ENCRYPTED) vOUT += L"A_Encrypted;";
	if (FileAttributeID & FILE_ATTRIBUTE_INTEGRITY_STREAM) vOUT += L"A_IintegritySystem;";
	if (FileAttributeID & FILE_ATTRIBUTE_VIRTUAL) vOUT += L"A_Virtual;";
	if (FileAttributeID & FILE_ATTRIBUTE_NO_SCRUB_DATA) vOUT += L"A_NoScrubData;";
	if (FileAttributeID & FILE_ATTRIBUTE_EA) vOUT += L"A_EA;";
	if (FileAttributeID & FILE_ATTRIBUTE_PINNED) vOUT += L"A_Pinned;";
	if (FileAttributeID & FILE_ATTRIBUTE_UNPINNED) vOUT += L"A_Unpinned;";
	if (FileAttributeID & FILE_ATTRIBUTE_RECALL_ON_OPEN) vOUT += L"A_RecallOnOpen;";
	if (FileAttributeID & FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS) vOUT += L"A_RecallOnDataAccess;";
	if (vOUT.size() == 0)
		vOUT = L"A_NONE";
	else
		if (vOUT[vOUT.size() - 1] == L';') vOUT.pop_back();//Removing last ;
	vOUT.resize(vOUT.size() + 1);//\0
	wchar_t* sumWC = new wchar_t[vOUT.size()];
	swprintf_s(sumWC, vOUT.size(), vOUT.c_str());
	return sumWC;
}

/// @brief	Modify the text of a specific Menu
/// @param	_hwnd	Handle of the window containing the menu to modify
/// @param	Position	Position of the item in the menu
/// @param	_wTEXT	Text to set
void UI_SetText_ToMenu(HWND _hwnd, int Position, std::wstring _wTEXT)
{
	HMENU hMenu = GetMenu(_hwnd);
	ModifyMenu(hMenu, Position, MF_BYPOSITION | MF_STRING, 0, _wTEXT.c_str());
}

/// @brief	Modify the text of a specific column of a listview
/// @param	_hwnd	Handle of the listview to modify
/// @param	Position	Position of the column in the menu
/// @param	_wTEXT	Text to set
void UI_SetText_ToLVColumn(HWND _hwnd, int Position, std::wstring _wTEXT)
{
	int nSize = 255;
	LVCOLUMN lvc = { 0 };
	lvc.mask = LVCF_TEXT;
	lvc.pszText = NULL;
	free(lvc.pszText);
	lvc.pszText = (LPTSTR)calloc(nSize, sizeof(TCHAR));
	lvc.cchTextMax = nSize;
	ListView_GetColumn(_hwnd, Position, &lvc);
	lvc.pszText = (LPTSTR)_wTEXT.c_str();
	ListView_SetColumn(_hwnd, Position, &lvc);
}

/// @brief	Modify the text of a specific sub-menu
/// @param	_hwnd	Handle of the window containing the sub-menu to modify
/// @param	MsgID	Define ID of the sub-menu item
/// @param	_wTEXT	Text to set
void UI_SetText_ToSubMenu(HWND _hwnd, int MsgID, std::wstring _wTEXT)
{
	HMENU hMenu = GetMenu(_hwnd);
	if (hMenu) {
		MENUITEMINFO menuitem = { sizeof(MENUITEMINFO) };
		GetMenuItemInfo(hMenu, MsgID, false, &menuitem);
		menuitem.dwTypeData = (LPWSTR)_wTEXT.c_str();
		menuitem.fMask = MIIM_TYPE | MIIM_DATA;
		SetMenuItemInfo(hMenu, MsgID, false, &menuitem);
	}
}

/// @brief	Modify the text of a specific sub-menu from a right click
/// @param	hMenu	Handle of the menu to modify
/// @param	MsgID	Define ID of the sub-menu item
/// @param	_wTEXT	Text to set
void UI_SetText_ToSubMenuRClick(HMENU hMenu, int MsgID, std::wstring _wTEXT)
{
	if (hMenu) {
		MENUITEMINFO menuitem = { sizeof(MENUITEMINFO) };
		GetMenuItemInfo(hMenu, MsgID, false, &menuitem);
		menuitem.dwTypeData = (LPWSTR)_wTEXT.c_str();
		menuitem.fMask = MIIM_TYPE | MIIM_DATA;
		SetMenuItemInfo(hMenu, MsgID, false, &menuitem);
	}
}

/// @brief	Modify the text of a specific sub-menu from a right click
/// @param	hMenu	Handle of the menu to modify
/// @param	MsgID	Position of the sub-menu item
/// @param	_wTEXT	Text to set
void UI_SetTextFromPos_ToSubMenuRClick(HMENU hMenu, int Position, std::wstring _wTEXT)
{
	if (hMenu) {
		MENUITEMINFO menuitem = { sizeof(MENUITEMINFO) };
		GetMenuItemInfo(hMenu, Position, true, &menuitem);
		menuitem.dwTypeData = (LPWSTR)_wTEXT.c_str();
		menuitem.fMask = MIIM_TYPE | MIIM_DATA;
		SetMenuItemInfo(hMenu, Position, true, &menuitem);
	}
}

/// @brief	Refresh all the variable in NZArchive
/// @param	hWnd	Handle of the main window
void UI_Refresh_Variable(HWND hWnd)
{
	UI_SetText_ToMenu(hWnd, 0, TRANSLATE(L"MENU_FILE", L"&File"));
	UI_SetText_ToMenu(hWnd, 1, TRANSLATE(L"MENU_EDIT", L"&Edit"));
	UI_SetText_ToMenu(hWnd, 2, TRANSLATE(L"MENU_SETTING", L"&Settings"));
	UI_SetText_ToMenu(hWnd, 3, TRANSLATE(L"MENU_HELP", L"&Help"));
	UI_SetText_ToSubMenu(hWnd, IDM_EXIT, TRANSLATE(L"MENU_FILE_EXIT", L"E&xit"));
	UI_SetText_ToSubMenu(hWnd, ID_EDIT_SELECTALL, TRANSLATE(L"MENU_EDIT_SELECT_ALL", L"Select &all  (Ctrl+A)"));
	UI_SetText_ToSubMenu(hWnd, ID_EDIT_SELECTNONE, TRANSLATE(L"MENU_EDIT_SELECT_NONE", L"Select &none  (Ctrl+N)"));
	UI_SetText_ToSubMenu(hWnd, ID_EDIT_INVERTSELECTION, TRANSLATE(L"MENU_EDIT_INVERT_SELECTION", L"Invert selection  (Ctrl+I)"));
	UI_SetText_ToSubMenu(hWnd, ID_LANG, TRANSLATE(L"MENU_LANG", L"&Language  (Ctrl+L)"));
	UI_SetText_ToSubMenu(hWnd, ID_ABOUT, TRANSLATE(L"MENU_ABOUT", L"&About  (Ctrl+B)"));
	UI_SetText_ToSubMenu(hWnd, ID_SETTINGS_ASSOCIATENZARCHIVEWITHNZAFILES, TRANSLATE(L"MENU_ASSONZA", L"&Associate NZArchive with NZA files"));
	UI_SetText_ToSubMenu(hWnd, ID_SETTINGS_REMOVENZAASSOCIATION, TRANSLATE(L"MENU_ASSOREMOVE", L"&Remove NZA association"));
	UI_SetText_ToLVColumn(ghListView, 0, TRANSLATE(L"LIST_COL0", L"Name"));
	UI_SetText_ToLVColumn(ghListView, 1, TRANSLATE(L"LIST_COL1", L"Size"));
	UI_SetText_ToLVColumn(ghListView, 2, TRANSLATE(L"LIST_COL2", L"Packed"));
	UI_SetText_ToLVColumn(ghListView, 3, TRANSLATE(L"LIST_COL3", L"Modified"));
	UI_SetText_ToLVColumn(ghListView, 4, TRANSLATE(L"LIST_COL4", L"Created"));
	UI_SetText_ToLVColumn(ghListView, 5, TRANSLATE(L"LIST_COL5", L"Accessed"));
	UI_SetText_ToLVColumn(ghListView, 6, TRANSLATE(L"LIST_COL6", L"Attribute"));
	UI_SetText_ToLVColumn(ghListView, 7, TRANSLATE(L"LIST_COL7", L"Offset"));
	UI_SetText_ToLVColumn(ghListView, 8, TRANSLATE(L"LIST_COL8", L"Folder"));
	UI_UpdateTOOLBARtext(ghToolBar, ID_OPEN, TRANSLATE(L"TOOLBOX_OPEN", L"Open archive\0").c_str());
	UI_UpdateTOOLBARtext(ghToolBar, ID_CREATE, TRANSLATE(L"TOOLBOX_CREATE", L"Create archive\0").c_str());
	UI_UpdateTOOLBARtext(ghToolBar, ID_ARCINFO, TRANSLATE(L"TOOLBOX_INFORMATION", L"Archive information\0").c_str());
	UI_UpdateTOOLBARtext(ghToolBar, ID_EXTRACTALL, TRANSLATE(L"TOOLBOX_EALL", L"Extract all\0").c_str());
	UI_UpdateTOOLBARtext(ghToolBar, ID_EXTRACTSEL, TRANSLATE(L"TOOLBOX_ESEL", L"Extract selection\0").c_str());
	UI_UpdateTOOLBARtext(ghToolBar, ID_ADDTO, TRANSLATE(L"TOOLBOX_ADD", L"Add to archive\0").c_str());
	UI_UpdateTOOLBARtext(ghToolBar, ID_DELETE, TRANSLATE(L"TOOLBOX_DELETE", L"Delete from archive\0").c_str());
	UI_UpdateTOOLBARtext(ghToolBar, ID_TEST, TRANSLATE(L"TOOLBOX_TEST", L"Test archive\0").c_str());
}

/// @brief	Switch state of main menu between enabled/disabled
/// @param	hWnd	Handle of the main window
/// @param	_state	State of main menu (enabled/disabled)
void UI_Enable_Menu(HWND hWnd, bool _state)
{
	HMENU hMenu = GetMenu(hWnd);
	if (hMenu) {
		UI_TOOLBAREnable(ghToolBar, ID_ARCINFO, _state ? true : false);
		EnableMenuItem(hMenu, ID_EDIT_SELECTALL, _state ? MF_ENABLED : MF_GRAYED);
		EnableMenuItem(hMenu, ID_EDIT_SELECTNONE, _state ? MF_ENABLED : MF_GRAYED);
		EnableMenuItem(hMenu, ID_EDIT_INVERTSELECTION, _state ? MF_ENABLED : MF_GRAYED);
		UI_TOOLBAREnable(ghToolBar, ID_EXTRACTALL, _state ? true : false);
		UI_TOOLBAREnable(ghToolBar, ID_EXTRACTSEL, _state ? true : false);
		UI_TOOLBAREnable(ghToolBar, ID_ADDTO, _state ? true : false);
		UI_TOOLBAREnable(ghToolBar, ID_DELETE, _state ? true : false);
		UI_TOOLBAREnable(ghToolBar, ID_TEST, _state ? true : false);
	}
}

/// @brief	Create a link to a file
HRESULT CreateLinkNew(LPCWSTR pszTargetfile, LPCWSTR pszTargetargs,
	LPCWSTR pszLinkfile, LPCWSTR pszDescription,
	int iShowmode, LPCWSTR pszCurdir,
	LPCWSTR pszIconfile, int iIconindex)
{
	HRESULT       hRes;                  /* Returned COM result code */
	IShellLink* pShellLink;            /* IShellLink object pointer */
	IPersistFile* pPersistFile;          /* IPersistFile object pointer */
	int           iWideCharsWritten;     /* Number of wide characters
											written */
	bool DoUnInit = true;
	//CoInitialize(NULL);
	hRes = E_INVALIDARG;
	if (
		(pszTargetfile != NULL) && (wcslen(pszTargetfile) > 0) &&
		(pszTargetargs != NULL) &&
		(pszLinkfile != NULL) && (wcslen(pszLinkfile) > 0) &&
		(pszDescription != NULL) &&
		(iShowmode >= 0) &&
		(pszCurdir != NULL) &&
		(pszIconfile != NULL) &&
		(iIconindex >= 0)
		)
	{
		hRes = CoCreateInstance(
			CLSID_ShellLink,     /* pre-defined CLSID of the IShellLink
									 object */
			NULL,                 /* pointer to parent interface if part of
									 aggregate */
			CLSCTX_INPROC_SERVER, /* caller and called code are in same
									 process */
			IID_IShellLink,      /* pre-defined interface of the
									 IShellLink object */
			(LPVOID*)&pShellLink);         /* Returns a pointer to the IShellLink
									 object */
		if (SUCCEEDED(hRes))
		{
			/* Set the fields in the IShellLink object */
			hRes = pShellLink->SetPath(pszTargetfile);
			hRes = pShellLink->SetArguments(pszTargetargs);
			if (wcslen(pszDescription) > 0)
			{
				hRes = pShellLink->SetDescription(pszDescription);
			}
			if (iShowmode > 0)
			{
				hRes = pShellLink->SetShowCmd(iShowmode);
			}
			if (wcslen(pszCurdir) > 0)
			{
				hRes = pShellLink->SetWorkingDirectory(pszCurdir);
			}
			if (wcslen(pszIconfile) > 0 && iIconindex >= 0)
			{
				hRes = pShellLink->SetIconLocation(pszIconfile, iIconindex);
			}

			/* Use the IPersistFile object to save the shell lLink */
			hRes = pShellLink->QueryInterface(
				IID_IPersistFile,         /* pre-defined interface of the
											  IPersistFile object */
				(LPVOID*)&pPersistFile);            /* returns a pointer to the
											  IPersistFile object */
			if (SUCCEEDED(hRes))
			{
				hRes = pPersistFile->Save(pszLinkfile, TRUE);
				pPersistFile->Release();
			}
			pShellLink->Release();
		}

	}
	//CoUninitialize();
	return (hRes);
}

/// @brief	Taskbar helper, if Windows version is at least Vista, run CoInitialize
///			Allow progression change, overlay icon...
///			Add, for further use, function to create link
///			Also load font 'Roboto Condensed' from resource into memory and discharge it at program exit
class TaskBarHelper
{
public:
	TaskBarHelper() : pTaskbar(NULL), mComInit(false), hICON(NULL), m_fonthandle(NULL)
	{
		if (IsWindowsVistaOrGreater())
		{
			mComInit = true;
			const HRESULT hrComInit = CoInitialize(NULL);
			HRESULT hr = CoCreateInstance(CLSID_TaskbarList, nullptr,
				CLSCTX_INPROC_SERVER, __uuidof(pTaskbar),
				reinterpret_cast<void**>(&pTaskbar));
			pTaskbar->HrInit();
		}
		else
			mComInit = false;

		HRSRC myResource = ::FindResource(NULL, MAKEINTRESOURCE(IDR_ZSTDRobotoCondensedRegularTTF), RT_RCDATA);
		if (myResource != NULL)
		{
			unsigned int myResourceSize = ::SizeofResource(NULL, myResource);
			HGLOBAL myResourceData = ::LoadResource(NULL, myResource);
			if (myResourceData != NULL)
			{
				std::vector<std::byte> vTTFin(myResourceSize);
				void* pMyBinaryData = ::LockResource(myResourceData);
				memcpy_s(&vTTFin[0], vTTFin.size(), pMyBinaryData, vTTFin.size());
				std::vector<std::byte> vTTFout;
				NZArchive::Archive lARC;
				lARC.ZSTD_stream_uncompress(vTTFin, vTTFout);
				DWORD fontInstalled = 0;
				m_fonthandle = AddFontMemResourceEx(&vTTFout[0], vTTFout.size(), NULL, &fontInstalled);
			}
		}

	}
	~TaskBarHelper()
	{
		if (pTaskbar != NULL)
			pTaskbar->Release();
		pTaskbar = NULL;
		CoUninitialize();
		if (m_fonthandle != NULL)
			RemoveFontMemResourceEx(m_fonthandle);
	}

	bool isComInit()
	{
		return mComInit;
	}

	void setOverlayIcon(const std::wstring& IconPath, const std::wstring& description, HWND ProgramHWND) {
		if (pTaskbar == NULL || ProgramHWND == NULL)
			return;
		if (IconPath.empty()) {
			pTaskbar->SetOverlayIcon(ProgramHWND, NULL, (LPCWSTR)L"");
			return;
		}
		HICON pIcon = (HICON)LoadImage(NULL, IconPath.c_str(), IMAGE_ICON, 16, 16, LR_LOADFROMFILE);
		if (!pIcon) {
			return;
		}
		pTaskbar->SetOverlayIcon(ProgramHWND, pIcon, description.c_str());
		DestroyIcon(pIcon);
	}

	void setOverlayIconFromResource(const int RessID, const std::wstring& description, HWND ProgramHWND, HINSTANCE hInst) {
		if (pTaskbar == NULL || ProgramHWND == NULL || hInst == NULL)
			return;
		if (RessID == 0) {
			pTaskbar->SetOverlayIcon(ProgramHWND, NULL, (LPCWSTR)L"");
			return;
		}
		HICON pIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(RessID), IMAGE_ICON, 16, 16, LR_SHARED);
		if (!pIcon) {
			return;
		}
		pTaskbar->SetOverlayIcon(ProgramHWND, pIcon, description.c_str());
		DestroyIcon(pIcon);
	}

	void addArchiveToRecent(const std::wstring& ArchivePATH, const std::wstring& description)
	{
		if (pTaskbar == NULL)	return;
		WCHAR NZArchivePath[MAX_PATH];
		GetModuleFileNameW(NULL, NZArchivePath, MAX_PATH);
		IShellLinkW* lLink;
		if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC, IID_IShellLinkW, reinterpret_cast<void**> (&lLink)))) {
			lLink->SetIconLocation(NZArchivePath, 0);
			lLink->SetPath(NZArchivePath);
			lLink->SetArguments(std::wstring(L"\"" + ArchivePATH + L"\"").c_str());
			IPropertyStore* propStore;
			HRESULT hr = lLink->QueryInterface(IID_IPropertyStore, reinterpret_cast<void**> (&(propStore)));
			if (SUCCEEDED(hr)) {
				PROPVARIANT pv;
				pv.vt = VT_LPWSTR;
				pv.pwszVal = (LPWSTR)description.c_str();
				//	System.Title property key, values taken from http://msdn.microsoft.com/en-us/library/bb787584.aspx
				//	const PROPERTYKEY PKEY_Title = { { 0xF29F85E0, 0x4FF9, 0x1068, { 0xAB, 0x91, 0x08, 0x00, 0x2B, 0x27, 0xB3, 0xD9 } }, 2 };
				//	Using value from propkey.h
				hr = propStore->SetValue(PKEY_Title, pv);	//PropStore value from propkey.h
				propStore->Commit();
				propStore->Release();
			}
			SHAddToRecentDocs(SHARD_LINK, lLink);
			lLink->Release();
		}
	}

	void setProgressValue(HWND Window, ULONGLONG completed, ULONGLONG total) {
		if (pTaskbar == NULL)
			return;
		pTaskbar->SetProgressValue(Window, completed, total);
	}

	void setProgressState(HWND Window, TBPFLAG state) {
		if (pTaskbar == NULL)
			return;
		pTaskbar->SetProgressState(Window, (TBPFLAG)state);
	}

	void setCount(HWND Window, int count, const std::wstring& description) {
		if (pTaskbar == NULL)
			return;
		if (hICON)
			DestroyIcon(hICON);
		if (count == -1)
		{
			pTaskbar->SetOverlayIcon(Window, NULL, L"");
			return;
		}
		std::wstring countString = std::to_wstring(count);
		// Create transparent background
		BITMAPV5HEADER bmHeader;
		ZeroMemory(&bmHeader, sizeof(BITMAPV5HEADER));
		bmHeader.bV5Size = sizeof(BITMAPV5HEADER);
		bmHeader.bV5Width = 16;
		bmHeader.bV5Height = 16;
		bmHeader.bV5Planes = 1;
		bmHeader.bV5BitCount = 32;
		bmHeader.bV5Compression = BI_RGB;
		bmHeader.bV5RedMask = 0x00FF0000;
		bmHeader.bV5GreenMask = 0x0000FF00;
		bmHeader.bV5BlueMask = 0x000000FF;
		bmHeader.bV5AlphaMask = 0xFF000000;
		// Get DC
		HDC hdc;
		hdc = GetDC(NULL);
		HDC hMemDC = CreateCompatibleDC(hdc);
		ReleaseDC(NULL, hdc);
		HBITMAP hBitmapMask = CreateBitmap(16, 16, 1, 1, NULL);
		void* lpBits;
		HBITMAP hBitmap = CreateDIBSection(hdc, (BITMAPINFO*)&bmHeader, DIB_RGB_COLORS, (void**)&lpBits, NULL, 0);
		HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);
		HICON hBackground = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_NBOX));
		DrawIconEx(hMemDC, 0, 0, hBackground, 16, 16, 0, 0, DI_NORMAL);
		LOGFONT lFont;
		memset(&lFont, 0, sizeof(LOGFONT));
		lFont.lfHeight = 12;
		lFont.lfWeight = FW_MEDIUM;
		wcscpy_s(lFont.lfFaceName, L"Roboto Condensed");
		HFONT hFont = CreateFontIndirect(&lFont);
		SelectObject(hMemDC, hFont);
		RECT rect;
		SetTextColor(hMemDC, RGB(255, 255, 255));
		SetBkMode(hMemDC, TRANSPARENT);
		SetRect(&rect, 0, 2, 16, 12);
		DrawText(hMemDC, countString.c_str(), -1, &rect, DT_NOCLIP | DT_CENTER);
		//Transparency of Color R255 G0 B255
		RGBQUAD* pPtr = (RGBQUAD*)lpBits;
		for (int y = 0; y < 16; ++y)
			for (int x = 0; x < 16; ++x, ++pPtr)
			{
				if (pPtr->rgbRed == 255 && pPtr->rgbGreen == 0 && pPtr->rgbBlue == 255)
					pPtr->rgbReserved = 0;
				else
					pPtr->rgbReserved = 255;
			}
		// Cleanup DC
		DeleteObject(hBackground);
		DeleteObject(hFont);
		SelectObject(hMemDC, hOldBitmap);
		DeleteDC(hMemDC);
		ICONINFO ii;
		ii.fIcon = FALSE;
		ii.xHotspot = 0;
		ii.yHotspot = 0;
		ii.hbmMask = hBitmapMask;
		ii.hbmColor = hBitmap;
		hICON = CreateIconIndirect(&ii);
		DeleteObject(hBitmap);
		DeleteObject(hBitmapMask);
		if (!hICON)
			return;
		LPWSTR desc = (LPWSTR)description.c_str();
		pTaskbar->SetOverlayIcon(Window, hICON, desc);
	}

private:
	ITaskbarList3* pTaskbar;
	bool mComInit;
	HICON hICON;
	HANDLE m_fonthandle;			//FontHandle containig custom font handle
};
static TaskBarHelper cTASKBar;

/// @brief	Detect if environnement is 64 or 32 bits
/// @param	isWindows64bit	(OUT) set this bool to true if 64 bits, false for 32 bits
/// @return	bool to tell if function successed or not
bool getWindowsBit(bool& isWindows64bit)
{
#if _WIN64
	isWindows64bit = true;
	return true;
#elif _WIN32

	BOOL isWow64 = FALSE;
	//IsWow64Process is not available on all supported versions of Windows.
	//Use GetModuleHandle to get a handle to the DLL that contains the function
	//and GetProcAddress to get a pointer to the function if available.
	LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS)
		GetProcAddress(GetModuleHandle(TEXT("kernel32")), "IsWow64Process");
	if (fnIsWow64Process)
	{
		if (!fnIsWow64Process(GetCurrentProcess(), &isWow64))
			return false;
		if (isWow64)
			isWindows64bit = true;
		else
			isWindows64bit = false;
		return true;
	}
	else
		return false;
#else
	assert(0);
	return false;

#endif
}


