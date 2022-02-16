// header.h : fichier Include pour les fichiers Include système standard,
// ou les fichiers Include spécifiques aux projets
//

#pragma once
#include "targetver.h"
// Fichiers d'en-tête Windows
#include <windows.h>
// Fichiers d'en-tête C RunTime
#include "resource.h"
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <CommCtrl.h>//Recent control forms
#pragma comment(lib, "comctl32.lib")

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <Uxtheme.h>
#pragma comment(lib,"UxTheme.lib")

#include <string>//Base string class
#include <iomanip>//Help with time manipulation
#include <atlbase.h>//Progress bar and open/save dialog
#include <thread>//Detaching operation in background
#include <random>//Session generation
using namespace std::chrono_literals;//Using time litterals like 100ms
#include <fstream>//Language parser
#include <sstream>//Split function
#include <ShlObj.h> //Shell operation
#define FastPERCd(Actual,Total) (((double)Actual*100.0)/(double)Total) 
#define FastPERCi(Actual,Total) (uint64_t)(((double)Actual*100.0)/(double)Total) 
#include "pathHelper.h"
#include <NZArchive/NZArchive.hpp>
#include "Translator.hpp"//Translation helper
#include "plf_nanotimer.h"//Timer class
#include "WindowsExplorerHELPER.hpp"

struct iconStruct
{
	int index = 0;
	HICON pIcon;
	std::wstring pExtension;
};

std::vector<iconStruct> vIconStruct;

#define idListView 0x100
#define idStaticLabel 0x101
static std::wstring SessionPath = L"";
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
BOOL CenterWindow(HWND hwndWindow)
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
HIMAGELIST himgList;
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
	himgList = ImageList_Create(16, 16, ILC_COLOR32, 3, 3);
	ImageList_SetBkColor(himgList, GetSysColor(COLOR_WINDOW));
	dllhinst = LoadLibrary(L"shell32.dll");
	if (dllhinst != 0)
	{
		hIcon = LoadIcon(dllhinst, MAKEINTRESOURCE(4));
		ImageList_AddIcon(himgList, hIcon);
		hIcon = LoadIcon(dllhinst, MAKEINTRESOURCE(1));
		ImageList_AddIcon(himgList, hIcon);
		FreeLibrary(dllhinst);
		ListView_SetImageList(hWndListView, himgList, LVSIL_SMALL);
	}
	ListView_SetExtendedListViewStyle(hWndListView, LVS_EX_INFOTIP | LVS_EX_LABELTIP);
	return (hWndListView);
}
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

/// @brief      std::sort sorter for file string analysis.
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


void InsertItemListbox(HWND _Hwnd, std::wstring _Message, int Position = 0 /*First item in the list*/,bool showDate = true)
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

// Description: 
//   Creates a status bar and divides it into the specified number of parts.
// Parameters:
//   hwndParent - parent window for the status bar.
//   idStatus - child window identifier of the status bar.
//   hinst - handle to the application instance.
//   cParts - number of parts into which to divide the status bar.
// Returns:
//   The handle to the status bar.
HWND CreateStatusBar(HWND hwndParent, HINSTANCE
	hinst, int cParts)
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
		STATUSCLASSNAME,         // name of status bar class
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

void UpdateStatusBar(HWND _Sbar, std::wstring _Message, int _PartId = 0)
{
	SendMessage(_Sbar, SB_SETTEXTW, (WPARAM)_PartId | SBT_NOBORDERS/*SBT_POPOUT*/, (LPARAM)_Message.c_str());
}


#define ID_OPEN 0x2340
#define ID_CREATE 0x2341
#define ID_ARCINFO 0x2342
#define ID_EXTRACTALL 0x2343
#define ID_EXTRACTSEL 0x2344
#define ID_ADDTO 0x2345
#define ID_DELETE 0x2346
#define ID_TEST 0x2347
#define ID_TEXTNAV 0x2350
HIMAGELIST g_hImageList = NULL;
void CreateToolbar(HWND hWndParent, HWND& hwndToolBar, HINSTANCE hInst)
{
	// Declare and initialize local constants.
	const int ImageListID = 0;
	const int numButtons = 11;

	const DWORD buttonStyles = BTNS_AUTOSIZE;

	// Create the toolbar.
	/*HWND*/ hwndToolBar = CreateWindowEx(0, TOOLBARCLASSNAME, NULL,
		WS_CHILD | /*TBSTYLE_WRAPABLE |*/ TBSTYLE_FLAT, 0, 0, 0, 0,
		hWndParent, NULL, hInst, NULL);

	SendMessage(hwndToolBar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);

	ImageList_Destroy(g_hImageList);
	g_hImageList = NULL;
	g_hImageList = ImageList_Create(32, 32, ILC_COLOR32 | ILC_MASK, numButtons, 0);
	ImageList_AddIcon(g_hImageList, LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ADD)));//0
	ImageList_AddIcon(g_hImageList, LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_CREATE)));//1
	ImageList_AddIcon(g_hImageList, LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ARCINFO)));//2
	ImageList_AddIcon(g_hImageList, LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_DELETE)));//3
	ImageList_AddIcon(g_hImageList, LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_EXTRACTALL)));//4
	ImageList_AddIcon(g_hImageList, LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_EXTRACTSEL)));//5
	ImageList_AddIcon(g_hImageList, LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_FILTER)));//6
	ImageList_AddIcon(g_hImageList, LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_OPEN)));//7
	ImageList_AddIcon(g_hImageList, LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_TEST)));//8
	SendMessage(hwndToolBar, TB_SETIMAGELIST, (WPARAM)0, (LPARAM)g_hImageList);


	TBBUTTON tbButtons[numButtons] =
	{
		{ 7, ID_OPEN, TBSTATE_ENABLED,	 BTNS_AUTOSIZE, {0}, 0, (INT_PTR)TRANSLATE(L"TOOLBOX_OPEN", L"Open archive\0").c_str()},
		{ 1, ID_CREATE, TBSTATE_ENABLED, BTNS_AUTOSIZE, {0}, 0, (INT_PTR)TRANSLATE(L"TOOLBOX_CREATE", L"Create archive\0").c_str() },
		{ 2, ID_ARCINFO, 0,				 BTNS_AUTOSIZE, {0}, 0, (INT_PTR)TRANSLATE(L"TOOLBOX_INFORMATION", L"Archive information\0").c_str() },
		{ 0, NULL, 0, BTNS_SEP, {0}, 0, 0 },
		{ 4, ID_EXTRACTALL, 0,			 BTNS_AUTOSIZE, {0}, 0, (INT_PTR)TRANSLATE(L"TOOLBOX_EALL", L"Extract all\0").c_str() },
		{ 5, ID_EXTRACTSEL, 0,			 BTNS_AUTOSIZE, {0}, 0, (INT_PTR)TRANSLATE(L"TOOLBOX_ESEL", L"Extract selection\0").c_str() },
		{ 0, NULL, 0, BTNS_SEP, {0}, 0, 0 },
		{ 0, ID_ADDTO, 0,				 BTNS_AUTOSIZE, {0}, 0, (INT_PTR)TRANSLATE(L"TOOLBOX_ADD", L"Add to archive\0").c_str() },
		{ 3, ID_DELETE, 0,				 BTNS_AUTOSIZE | BS_MULTILINE, {0}, 0, (INT_PTR)TRANSLATE(L"TOOLBOX_DELETE", L"Delete from archive\0").c_str() },
		{ 0, NULL, 0, BTNS_SEP, {0}, 0, 0 },
		{ 8, ID_TEST, 0,				 BTNS_AUTOSIZE, {0}, 0, (INT_PTR)TRANSLATE(L"TOOLBOX_TEST", L"Test archive\0").c_str() },
	};
	SendMessage(hwndToolBar, TB_ADDBUTTONS, numButtons, (LPARAM)tbButtons);
	auto hFont = CreateFont(14, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH, /*TEXT("Microsoft Sans Serif")*/L"");
	SendMessage(hwndToolBar, WM_SETFONT, (WPARAM)hFont, FALSE);


	SendMessage(hwndToolBar, TB_AUTOSIZE, 0, 0);
	ShowWindow(hwndToolBar, TRUE);

	return /*hWndToolbar*/;
}

void UI_UpdateTOOLBARtext(HWND lHwnd, DWORD buttonID, const wchar_t* Message, bool Enabled = true)
{
	TBBUTTONINFO tbInfo;
	tbInfo.cbSize = sizeof(TBBUTTONINFO);
	tbInfo.dwMask = TBIF_TEXT /*| (Enabled? TBSTATE_ENABLED:0)*/;
	tbInfo.pszText = (LPWSTR)Message;
	SendMessage(lHwnd, TB_SETBUTTONINFO, (WPARAM)buttonID, (LPARAM)&tbInfo);
	SendMessage(lHwnd, TB_ENABLEBUTTON, (WPARAM)buttonID, MAKELONG(Enabled ? 1 : 0, 0));
}
void UI_TOOLBAREnable(HWND lHwnd, DWORD buttonID, bool Enabled = true)
{
	SendMessage(lHwnd, TB_ENABLEBUTTON, (WPARAM)buttonID, MAKELONG(Enabled ? 1 : 0, 0));
}

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

/**
* Replaces all occurrences of the 'oldValue' string with the 'newValue' string
* @param source: The source string
* @param oldValue: The string to be replaced
* @param newValue: The string to replace all occurrences of oldValue
* @return: A new string with all occurrences replaced
*/
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


/**
* Replaces all occurrences of the 'oldValue' string with the 'newValue' string
* @param source: The source string
* @param oldValue: The string to be replaced
* @param newValue: The string to replace all occurrences of oldValue
* @return: A new string with all occurrences replaced
*/
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