/*
*
* 𝐍𝐙𝐀-𝐆𝐔𝐈
*
* ɴᴢᴀ-ɢᴜɪ is using version 1.0 of ɴᴢᴀʀᴄʜɪᴠᴇ compression format
* Version :
*	1.0	:	Stable version
*	0.8 :	Adding file association
*	0.7 :	Adding Archive opening via command line
*
*
* For personnal information :
* >Size naming
*  Terms Kib, Mib and more come from IEC 80000-13 standard for 1024 bytes.
*  The IEC further specifiy that the kilobyte should only be used to refer to 1000 bytes.
*	1024	KiB	kibibyte
*	1024^2	MiB	mebibyte
*	1024^3	GiB	gibibyte
*	1024^4	TiB	tebibyte
*	1024^5	PiB	pebibyte
*	1024^6	EiB	exbibyte
*	1024^7	ZiB	zebibyte
*	1024^8	YiB	yobibyte
*
*/
#include "framework.h"

#define MAX_LOADSTRING 100
// Variables globales :
HINSTANCE hInst;                                // instance actuelle
WCHAR szTitle[MAX_LOADSTRING];                  // Texte de la barre de titre
WCHAR szWindowClass[MAX_LOADSTRING];            // nom de la classe de fenêtre principale
HINSTANCE g_hInst;
HWND g_hWnd;
static HWND g_ListView;
static HWND g_ToolBar;
static HWND g_TEXTNAV;
static HWND  hwndStatusbar;
static std::string mPassword = "";
std::wstring mArchiveName = L"";
NZArchive::Archive::ArchiveEntry OpenListing;
std::wstring ListTREEpth = L"\\";
NZArchive::Archive::ArchiveEntry ListTREE;
bool ListViewSortOrder[8];
static bool ExtractAll = false;
static bool ExtractToExplorerWindows = false;
static bool NPARAnalysisConform = false;
auto WFHelper = new WindowsExplorerHelper();

struct _ListItemRetrieve
{
	size_t FileIndex = 0;
	std::wstring fullPath = L"";
};
struct _CreateListAdd
{
	std::wstring mPAth = L"";
	std::wstring mtPAthInArchive = L"";
	uint64_t mFileSize = 0;
};
static std::vector<_ListItemRetrieve> FileToTreat;
static std::wstring mExtractPAth = L"";
static int64_t FastExtractIndex = -1;
bool OpenFromcommandLine = false;
std::wstring OpenFromcommandLine_Args = L"";
bool isLocalizationLoaded = false;
std::wstring LocaleName = L"";
bool OpenWith = false;
static bool OperationInProgress = false;
static std::vector<std::wstring> vDropFILE;
bool AddingFromDrop = false;
static bool NeedAnalysis = false;
// Déclarations anticipées des fonctions incluses dans ce module de code :
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    MaindWndProcDialog(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    AboutDialog(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    InputPasswordDialog(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    ArchiveInformationDialog(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    TestArchiveDialog(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	ExtractionDialog(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	FastExtractDialog(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	ArcCreateDialog(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	DeleteDialog(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	ChangeDialog(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	AddFileDialog(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	IntegrityChecking(HWND, UINT, WPARAM, LPARAM);

static LRESULT CALLBACK lvWndProc(const HWND, const UINT, const WPARAM, const LPARAM);
BOOL ListView_handler(HWND, UINT, WPARAM, LPARAM);
ITaskbarList3* ptbl;
//NODE TESTING
void UI_SetText_ToMenu(HWND _hwnd, int Position, std::wstring _wTEXT)
{
	HMENU hMenu = GetMenu(_hwnd);
	ModifyMenu(hMenu, Position, MF_BYPOSITION | MF_STRING, 0, _wTEXT.c_str());
}
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
void UI_Refresh_Variable(HWND hWnd)
{
	////Language Mods
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
	UI_SetText_ToLVColumn(g_ListView, 0, TRANSLATE(L"LIST_COL0", L"Name"));
	UI_SetText_ToLVColumn(g_ListView, 1, TRANSLATE(L"LIST_COL1", L"Size"));
	UI_SetText_ToLVColumn(g_ListView, 2, TRANSLATE(L"LIST_COL2", L"Packed"));
	UI_SetText_ToLVColumn(g_ListView, 3, TRANSLATE(L"LIST_COL3", L"Modified"));
	UI_SetText_ToLVColumn(g_ListView, 4, TRANSLATE(L"LIST_COL4", L"Created"));
	UI_SetText_ToLVColumn(g_ListView, 5, TRANSLATE(L"LIST_COL5", L"Accessed"));
	UI_SetText_ToLVColumn(g_ListView, 6, TRANSLATE(L"LIST_COL6", L"Attribute"));
	UI_SetText_ToLVColumn(g_ListView, 7, TRANSLATE(L"LIST_COL7", L"Offset"));
	UI_SetText_ToLVColumn(g_ListView, 8, TRANSLATE(L"LIST_COL8", L"Folder"));
	UI_UpdateTOOLBARtext(g_ToolBar, ID_OPEN, TRANSLATE(L"TOOLBOX_OPEN", L"Open archive\0").c_str());
	UI_UpdateTOOLBARtext(g_ToolBar, ID_CREATE, TRANSLATE(L"TOOLBOX_CREATE", L"Create archive\0").c_str());
	UI_UpdateTOOLBARtext(g_ToolBar, ID_ARCINFO, TRANSLATE(L"TOOLBOX_INFORMATION", L"Archive information\0").c_str());
	UI_UpdateTOOLBARtext(g_ToolBar, ID_EXTRACTALL, TRANSLATE(L"TOOLBOX_EALL", L"Extract all\0").c_str());
	UI_UpdateTOOLBARtext(g_ToolBar, ID_EXTRACTSEL, TRANSLATE(L"TOOLBOX_ESEL", L"Extract selection\0").c_str());
	UI_UpdateTOOLBARtext(g_ToolBar, ID_ADDTO, TRANSLATE(L"TOOLBOX_ADD", L"Add to archive\0").c_str());
	UI_UpdateTOOLBARtext(g_ToolBar, ID_DELETE, TRANSLATE(L"TOOLBOX_DELETE", L"Delete from archive\0").c_str());
	UI_UpdateTOOLBARtext(g_ToolBar, ID_TEST, TRANSLATE(L"TOOLBOX_TEST", L"Test archive\0").c_str());
}
void UI_Enable_Menu(HWND hWnd, bool _state)
{
	HMENU hMenu = GetMenu(hWnd);
	if (hMenu) {
		UI_TOOLBAREnable(g_ToolBar, ID_ARCINFO, _state ? true : false);
		EnableMenuItem(hMenu, ID_EDIT_SELECTALL, _state ? MF_ENABLED : MF_GRAYED);
		EnableMenuItem(hMenu, ID_EDIT_SELECTNONE, _state ? MF_ENABLED : MF_GRAYED);
		EnableMenuItem(hMenu, ID_EDIT_INVERTSELECTION, _state ? MF_ENABLED : MF_GRAYED);
		UI_TOOLBAREnable(g_ToolBar, ID_EXTRACTALL, _state ? true : false);
		UI_TOOLBAREnable(g_ToolBar, ID_EXTRACTSEL, _state ? true : false);
		UI_TOOLBAREnable(g_ToolBar, ID_ADDTO, _state ? true : false);
		UI_TOOLBAREnable(g_ToolBar, ID_DELETE, _state ? true : false);
		UI_TOOLBAREnable(g_ToolBar, ID_TEST, _state ? true : false);
	}
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	NeedAnalysis = false;
	if (std::wstring(lpCmdLine).size() > 0)
	{
		OpenFromcommandLine = true;
		OpenFromcommandLine_Args = std::wstring(lpCmdLine);
		//Remove start and end quote from command line
		OpenFromcommandLine_Args.erase(OpenFromcommandLine_Args.begin(), OpenFromcommandLine_Args.begin() + 1);
		OpenFromcommandLine_Args.pop_back();
	}
	//Localization
	{
		HKEY lLocKey;
		if (RegOpenKey(HKEY_CURRENT_USER, L"Software\\Classes\\NZArchive\\", &lLocKey) == ERROR_SUCCESS)//Classes doesn't exist, need to rewrite
		{
			DWORD BufferSize = 256;
			//TCHAR wKeyValue[MAX_PATH];
			std::vector<TCHAR> wKeyValue;
			wKeyValue.resize(BufferSize);
			if (RegGetValue(lLocKey, L"Lang", NULL, RRF_RT_ANY, NULL, &wKeyValue[0], &BufferSize) == ERROR_SUCCESS)
				LocaleName = std::wstring(wKeyValue.begin(), wKeyValue.end());
			else
				LocaleName = L"";
			RegCloseKey(lLocKey);
		}
		TCHAR szPath[MAX_PATH];
		DWORD pathLen = 0;
		pathLen = GetModuleFileName(NULL, szPath, MAX_PATH);
		std::wstring PathLocale(szPath);
		PathLocale = PathLocale.substr(0, PathLocale.find_last_of(L"\\") + 1);
		PathLocale += L"Language\\" + LocaleName;
		isLocalizationLoaded = LoadLocalisationTRD(PathLocale);
	}
	g_hInst = hInstance;
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_NZAGUI, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}
	CreateToolbar(g_hWnd, g_ToolBar, g_hInst);
	UI_Refresh_Variable(g_hWnd);
	UI_Enable_Menu(g_hWnd, false);

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_NZAGUI));
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return (int)msg.wParam;
}
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = MaindWndProcDialog;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_NZAGUI));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_NZAGUI);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	return RegisterClassExW(&wcex);
}
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance;
	g_hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);
	if (!g_hWnd)
		return FALSE;
	DragAcceptFiles(g_hWnd, TRUE);
	g_TEXTNAV = CreateWindowEx(
		0, L"EDIT",   // predefined class 
		NULL,         // no window title 
		WS_CHILD | WS_VISIBLE | ES_LEFT | ES_READONLY | WS_BORDER,
		4, 0, 100, 18,   // set size in WM_SIZE message 
		g_hWnd,         // parent window 
		(HMENU)ID_TEXTNAV,   // edit control ID 
		(HINSTANCE)GetWindowLongPtr(g_hWnd, GWLP_HINSTANCE),
		NULL);        // pointer not needed
	auto hFont = CreateFont(14, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH, /*TEXT("Microsoft Sans Serif")*/L"");
	SendMessage(g_TEXTNAV, WM_SETFONT, (WPARAM)hFont, FALSE);
	ShowWindow(g_hWnd, nCmdShow);
	UpdateWindow(g_hWnd);

	return TRUE;
}
LRESULT CALLBACK MaindWndProcDialog(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE://WM_CREATE return g_hwnd at end of this bracket, so don(t use it here
	{
		const HRESULT hrComInit = CoInitialize(NULL);
		HRESULT hr = CoCreateInstance(CLSID_TaskbarList, nullptr,
			CLSCTX_INPROC_SERVER, __uuidof(ptbl),
			reinterpret_cast<void**>(&ptbl));
		ptbl->HrInit();
		g_ListView = CreateListView(hWnd);
		hwndStatusbar = CreateStatusBar(hWnd, hInst, 1);
		UpdateStatusBar(hwndStatusbar, L"Welcome to NZArchive");//Translation
		SetWindowLongPtr(g_ListView, GWLP_USERDATA, (LONG_PTR)SetWindowLongPtr(g_ListView, GWLP_WNDPROC, (LONG_PTR)lvWndProc));
		InitListViewColumns(g_ListView);
		CenterWindow(hWnd);
		SessionPath = pHelper::CombinePath(std::filesystem::temp_directory_path().wstring(), L"NZArchive." + random_string(8));
		if (OpenFromcommandLine)
		{
			NeedAnalysis = true;
			PostMessage(hWnd, WM_COMMAND, ID_OPEN, 0);
		}
		UI_Refresh_Variable(hWnd);
		UI_Enable_Menu(hWnd, false);
	}
	break;
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Analyse les sélections de menu :
		if (wmId == ID_CONTEXT_OPEN)
		{
			int index;
			index = (int)SendMessage(g_ListView, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
			if (index == -1)
				break;
			if (OpenListing.FileList[index].isDirectory)
				break;
			OpenWith = false;
			FastExtractIndex = OpenListing.FileList[index].IndexInArchive;
			DialogBox(hInst, MAKEINTRESOURCE(IDD_FastOpen1), g_hWnd, FastExtractDialog);
		}
		else if (wmId == ID_CONTEXT_OPENWITH)
		{
			int index;
			index = (int)SendMessage(g_ListView, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
			if (index == -1)
				break;
			if (OpenListing.FileList[index].isDirectory)
				break;
			OpenWith = true;
			FastExtractIndex = OpenListing.FileList[index].IndexInArchive;
			DialogBox(hInst, MAKEINTRESOURCE(IDD_FastOpen1), g_hWnd, FastExtractDialog);
		}
		else if (wmId == ID_CONTEXT_DELETE)
		{
			PostMessage(g_hWnd, WM_COMMAND, ID_DELETE, 0);
		}
		else if (wmId == ID_CONTEXT_ADD)
		{
			PostMessage(g_hWnd, WM_COMMAND, ID_ADDTO, 0);
		}
		else if (wmId == ID_ADDTO)
		{
			if (mArchiveName != L"")
				DialogBox(hInst, MAKEINTRESOURCE(IDD_ADD_FILE), g_hWnd, AddFileDialog);
		}
		else if (wmId == ID_OPEN)
		{
			HRESULT hr;
			CComPtr<IFileOpenDialog> pDlg;
			std::wstring NameNZAFiles = TRANSLATE(L"LBL_CREATE_NZAFILES", L"NZArchive files");
			std::wstring NameAllFiles = TRANSLATE(L"LBL_ADD_ALLFILES", L"All files");
			COMDLG_FILTERSPEC aFileTypes[] = {
				{NameNZAFiles.c_str(), L"*.nza"},
				{ NameAllFiles.c_str(), L"*.*" }
			};
			hr = pDlg.CoCreateInstance(__uuidof(FileOpenDialog));
			pDlg->SetFileTypes(_countof(aFileTypes), aFileTypes);
			pDlg->SetTitle(TRANSLATE(L"LBL_OPENARCHIVE", L"Open NZArchive file").c_str());
			if (!OpenFromcommandLine)
			{
				hr = pDlg->Show(hWnd);
				if (SUCCEEDED(hr))
					NeedAnalysis = true;
			}
			if (SUCCEEDED(hr) || OpenFromcommandLine)
			{
				CComPtr<IShellItem> pItem;
				if (!OpenFromcommandLine)
					hr = pDlg->GetResult(&pItem);
				if (SUCCEEDED(hr) || OpenFromcommandLine)
				{
					LPOLESTR pwsz = NULL;
					if (!OpenFromcommandLine)
						hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pwsz);
					if (SUCCEEDED(hr) || OpenFromcommandLine)
					{
						std::wstring mArchivePath = pHelper::NormalizePath(OpenFromcommandLine ? OpenFromcommandLine_Args : pwsz);
						mArchiveName = mArchivePath;
						if (NeedAnalysis)
						{
							NPARAnalysisConform = false;
							if (std::filesystem::exists(mArchiveName + L".npar"))
								DialogBox(hInst, MAKEINTRESOURCE(IDD_CheckTable), hWnd, IntegrityChecking);
							if (NPARAnalysisConform == false)
							{
								MessageBox(hWnd, TRANSLATE(L"LBL_NPARDAMAGED_EXIT", L"Reparation failed, archive content is still damaged, exiting...").c_str()
									, L"NZArchive", MB_ICONERROR);
								PostMessage(hWnd, WM_COMMAND, IDM_EXIT, 0);
								return DefWindowProc(hWnd, message, wParam, lParam);
							}
						}
						CoTaskMemFree(pwsz);
						{
							if (OpenFromcommandLine)
								OpenFromcommandLine = false;
							NZArchive::Archive _lArchive;
							_lArchive.archiveNZ_set_key(mPassword);
							FILE* fIN;
							_wfopen_s(&fIN, mArchivePath.c_str(), L"rb");
							if (fIN != NULL)
							{
								OpenListing = _lArchive.archiveNZ_read_extract_file_list(fIN);
								if (!OpenListing.isNZArchiveFile)
								{
									fclose(fIN);
									MessageBox(hWnd, TRANSLATE(L"LBL_OPEN_NOTANARCHIVE", L"Not an NZArchive file, cancelling...").c_str()
										, L"NZArchive", MB_ICONERROR);
									return DefWindowProc(hWnd, message, wParam, lParam);
								}
								if (OpenListing.passwordError)
								{
									//Ask for passwords
									auto Result = DialogBox(hInst, MAKEINTRESOURCE(IDD_PWDBOX), hWnd, InputPasswordDialog);
									if (Result == ID_PWD_OK)
									{
										_lArchive.archiveNZ_set_key(mPassword);
										OpenListing = _lArchive.archiveNZ_read_extract_file_list(fIN);
										if (OpenListing.passwordError)
										{
											fclose(fIN);
											MessageBox(hWnd, TRANSLATE(L"LBL_OPEN_PASSWORDERROR", L"Password error, cancelling...").c_str()
												, L"NZArchive", MB_ICONERROR);
											return DefWindowProc(hWnd, message, wParam, lParam);
										}
									}
									else
									{
										fclose(fIN);
										return DefWindowProc(hWnd, message, wParam, lParam);
									}
								}
								fclose(fIN);
								SendMessage(g_ListView, LVM_DELETEALLITEMS, NULL, NULL);
								/*
								* EXPLORER LIKE
								*/
								ListTREE.FileList.clear();
								ListTREEpth = L"\\";
								//GetRoot
								for (size_t i = 0; i < OpenListing.FileList.size(); i++)
								{
									std::wstring lITEM = OpenListing.FileList[i].FullPath;
									if (lITEM.substr(ListTREEpth.length()).find_first_of(L"\\") == std::wstring::npos)
									{//No backslash, it's root item
										auto GoodItem = OpenListing.FileList[i];
										GoodItem.FullPath = GoodItem.FullPath.substr(ListTREEpth.length());
										ListTREE.FileList.push_back(GoodItem);
									}
								}
								std::sort(ListTREE.FileList.begin(), ListTREE.FileList.end(), sorter);//We put all in good or
								std::reverse(ListTREE.FileList.begin(), ListTREE.FileList.end());
								std::sort(ListTREE.FileList.begin(), ListTREE.FileList.end(),
									[](NZArchive::Archive::FileEntryRead t1, NZArchive::Archive::FileEntryRead t2) {return (uint8_t)t1.isDirectory > (uint8_t)t2.isDirectory; });
								SetWindowText(g_TEXTNAV, ListTREEpth.c_str());
								UpdateStatusBar(hwndStatusbar, TRANSLATE(L"LBL_OPEN_OPENINGARCHIVE", L"Opening archive ..."));
								ListView_SetItemCount(g_ListView, ListTREE.FileList.size());
								RedrawWindow(g_ListView, NULL, NULL, RDW_INVALIDATE);
								uint64_t FileNbr = 0;
								uint64_t DireNbr = 0;
								for (size_t i = 0; i < OpenListing.FileList.size(); i++)
									if (OpenListing.FileList[i].isDirectory)
										DireNbr++;
									else
										FileNbr++;
								std::wstring wsMETHOD = L"";
								switch (OpenListing.EncryptionMethod)
								{
								case NZArchive::EncryptionMethod::encNone:
									wsMETHOD = TRANSLATE(L"LBLCREATE_ENCRYPTIONNONE", L"None");
									break;
								case NZArchive::EncryptionMethod::encAES256:
									wsMETHOD = L"AES256";
									break;
								case NZArchive::EncryptionMethod::encChaCha20:
									wsMETHOD = L"ChaCha20";
									break;
								default:
									break;
								}
								UpdateStatusBar(hwndStatusbar, TRANSLATE(L"LBL_STATUS_TOTALOF", L"  Total of %s files and %s folders, compressed size %s, uncompressed size %s, encryption with %s"
									, thousandSeparator(FileNbr).c_str(), thousandSeparator(DireNbr).c_str()
									, BytesToSize((float)OpenListing.FullArchiveIncludingHeader).c_str(), BytesToSize((float)OpenListing.ArchiveSizeUncompressed).c_str(), wsMETHOD.c_str()));
								mArchivePath = pHelper::getFileFromPath(mArchivePath);
								std::vector<wchar_t> __Path;
								__Path.resize(32000);
								wmemcpy_s(&__Path[0], mArchivePath.size(), &mArchivePath[0], mArchivePath.size());
								PathCompactPath(NULL, &__Path[0], 1200);
								SetWindowText(g_hWnd, std::wstring(L"NZArchive - " + std::wstring(__Path.begin(), __Path.end())).c_str());
								UI_Enable_Menu(hWnd, true);
							}
							else
							{
								MessageBox(hWnd, TRANSLATE(L"LBL_OPEN_CANTOPENFILE", L"Can't open file, cancelling...").c_str(), L"NZArchive", MB_ICONERROR);
							}
						}
					}
				}
			}
		}
		else if (wmId == ID_LANG)
		{
			DialogBox(hInst, MAKEINTRESOURCE(IDD_LANGUAGE), hWnd, ChangeDialog);
		}
		else if (wmId == ID_ARCINFO)
		{
			if (mArchiveName == L"")
				break;
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ARCHIVE_INFORMATION), hWnd, ArchiveInformationDialog);
		}
		else if (wmId == ID_CREATE)
		{
			DialogBox(hInst, MAKEINTRESOURCE(IDD_CreateArchive), g_hWnd, ArcCreateDialog);
			break;
		}
		else if (wmId == ID_EXTRACTSEL)
		{
			if (mArchiveName == L"")
				break;
			FileToTreat.clear();
			int iPos = ListView_GetNextItem(g_ListView, -1, LVNI_SELECTED);
			while (iPos != -1) {
				_ListItemRetrieve lItRetrieve;
				lItRetrieve.FileIndex = ListTREE.FileList[iPos].IndexInArchive;
				lItRetrieve.fullPath = ListTREEpth + ListTREE.FileList[iPos].FullPath;
				FileToTreat.push_back(lItRetrieve);
				std::wstring lowerPathBASE = ListTREEpth + ListTREE.FileList[iPos].FullPath;
				std::transform(lowerPathBASE.begin(), lowerPathBASE.end(), lowerPathBASE.begin(), towlower);
				if (ListTREE.FileList[iPos].isDirectory)//Adding all subdirectories items (Folders and files)
					for (size_t i = 0; i < OpenListing.FileList.size(); i++)
					{
						std::wstring lowerPathToCompare = OpenListing.FileList[i].FullPath;
						std::transform(lowerPathToCompare.begin(), lowerPathToCompare.end(), lowerPathToCompare.begin(), towlower);
						if (lowerPathToCompare == lowerPathBASE)
							continue;//It's the same item, no need to add it twice
						else if (lowerPathToCompare.size() <= lowerPathBASE.size())
							continue;//LowerPathToCompare length is inferior or equal so it can't a sub-item/folders
						else if (lowerPathToCompare.size() > lowerPathBASE.size())
							if (lowerPathToCompare.substr(0, lowerPathBASE.size()) == lowerPathBASE)
							{
								lItRetrieve.FileIndex = OpenListing.FileList[i].IndexInArchive;
								lItRetrieve.fullPath = OpenListing.FileList[i].FullPath;
								FileToTreat.push_back(lItRetrieve);
							}

					}
				iPos = ListView_GetNextItem(g_ListView, iPos, LVNI_SELECTED);
			}
			if (FileToTreat.size() > 0)
			{
				ExtractAll = false;
				ExtractToExplorerWindows = false;
				if (WFHelper->ExtractToExplorer)
				{
					//We keep ExtractToExplorer to true, it will lead to show windows of explorer
					mExtractPAth = pHelper::NormalizePath(WFHelper->vFExplorer[WFHelper->ExplorerPathID].Path);//Setting Windows Explorer Path
					ExtractToExplorerWindows = true;
					DialogBox(hInst, MAKEINTRESOURCE(IDD_ExtractArchive), hWnd, ExtractionDialog);//Extracting !!!
				}
				else
				{
					HRESULT hr;
					CComPtr<IFileOpenDialog> pDlg;
					hr = pDlg.CoCreateInstance(__uuidof(FileOpenDialog));
					pDlg->SetTitle(TRANSLATE(L"LBL_EXTRACT_EXTRACTTO", L"Extract selection to ...").c_str());
					DWORD options;
					pDlg->GetOptions(&options);
					options &= ~FOS_FILEMUSTEXIST;
					options &= ~FOS_PATHMUSTEXIST;
					pDlg->SetOptions(options | FOS_PICKFOLDERS);
					hr = pDlg->Show(hWnd);
					if (SUCCEEDED(hr))
					{
						CComPtr<IShellItem> pItem;
						hr = pDlg->GetResult(&pItem);
						if (SUCCEEDED(hr))
						{
							LPOLESTR pwsz = NULL;
							hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pwsz);
							if (SUCCEEDED(hr))
							{
								mExtractPAth = pHelper::NormalizePath(pwsz);
								CoTaskMemFree(pwsz);
								DialogBox(hInst, MAKEINTRESOURCE(IDD_ExtractArchive), hWnd, ExtractionDialog);
							}
						}
					}
				}
			}
		}
		else if (wmId == ID_EXTRACTALL)
		{
			if (mArchiveName == L"")
				break;
			FileToTreat.clear();
			auto iCount = OpenListing.FileList.size();
			if (iCount == 0)
				break;
			//It could be easy to just process the whole uncompressio thing with the whole list and archiveNZ_read_extract_all_files_to
			//But cancelling operation is more easy this way, treating list one file at a time...
			{
				ExtractAll = true;
				HRESULT hr;
				CComPtr<IFileOpenDialog> pDlg;
				hr = pDlg.CoCreateInstance(__uuidof(FileOpenDialog));
				pDlg->SetTitle(TRANSLATE(L"LBL_EXTRACT_EXTRACTALLTO", L"Extract all files to ...").c_str());
				DWORD options;
				pDlg->GetOptions(&options);
				options &= ~FOS_FILEMUSTEXIST;
				options &= ~FOS_PATHMUSTEXIST;
				pDlg->SetOptions(options | FOS_PICKFOLDERS);
				hr = pDlg->Show(hWnd);
				if (SUCCEEDED(hr))
				{
					CComPtr<IShellItem> pItem;
					hr = pDlg->GetResult(&pItem);
					if (SUCCEEDED(hr))
					{
						LPOLESTR pwsz = NULL;
						hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pwsz);
						if (SUCCEEDED(hr))
						{
							mExtractPAth = pHelper::NormalizePath(pwsz);
							CoTaskMemFree(pwsz);
							DialogBox(hInst, MAKEINTRESOURCE(IDD_ExtractArchive), hWnd, ExtractionDialog);
						}
					}
				}
			}
		}
		else if (wmId == ID_TEST)
		{
			if (mArchiveName == L"")
				break;
			DialogBox(hInst, MAKEINTRESOURCE(IDD_TestArchive), hWnd, TestArchiveDialog);
		}
		else if (wmId == ID_EDIT_SELECTALL)
		{
			if (ListView_GetItemCount(g_ListView) == 0)
				break;
			ListView_SetItemState(g_ListView, -1, LVIS_SELECTED, LVIS_SELECTED);
			SetFocus(g_ListView);
		}
		else if (wmId == ID_EDIT_SELECTNONE)
		{
			if (ListView_GetItemCount(g_ListView) == 0)
				break;
			ListView_SetItemState(g_ListView, -1, ~LVIS_SELECTED, LVIS_SELECTED);
			SetFocus(g_ListView);
		}
		else if (wmId == ID_EDIT_INVERTSELECTION)
		{
			if (ListView_GetItemCount(g_ListView) == 0)
				break;
			for (size_t i = 0; i < ListView_GetItemCount(g_ListView); i++)
				if (ListView_GetItemState(g_ListView, i, LVIS_SELECTED))
				{
					ListView_SetItemState(g_ListView, i, ~LVIS_SELECTED, LVIS_SELECTED);
				}
				else
				{
					ListView_SetItemState(g_ListView, i, LVIS_SELECTED, LVIS_SELECTED);
				}
			SetFocus(g_ListView);
		}
		else if (wmId == ID_DELETE)
		{
			if (mArchiveName == L"")
				break;
			FileToTreat.clear();
			std::vector<_ListItemRetrieve> selectedFile;
			int iPos = ListView_GetNextItem(g_ListView, -1, LVNI_SELECTED);
			while (iPos != -1) {
				_ListItemRetrieve lItRetrieve;
				lItRetrieve.FileIndex = ListTREE.FileList[iPos].IndexInArchive;
				lItRetrieve.fullPath = ListTREEpth + ListTREE.FileList[iPos].FullPath + (ListTREE.FileList[iPos].isDirectory ? L"\\" : L"");
				std::transform(lItRetrieve.fullPath.begin(), lItRetrieve.fullPath.end(), lItRetrieve.fullPath.begin(), towlower);//To lower
				selectedFile.push_back(lItRetrieve);
				iPos = ListView_GetNextItem(g_ListView, iPos, LVNI_SELECTED);
			}
			if (selectedFile.size() > 0)
				if (MessageBox(g_hWnd, TRANSLATE(L"LBL_DELETE_YOUSURE", L"Do you really want to delete selection ?").c_str(),
					TRANSLATE(L"LBL_DELETE_TITLE", L"Delete").c_str(),
					MB_YESNO | MB_ICONQUESTION) == IDYES)
				{
					for (size_t i = 0; i < selectedFile.size(); i++)//For each selected entry
					{
						if (OpenListing.FileList[selectedFile[i].FileIndex].isDirectory)
						{//FOLDER, checking for subitems, 
							//Adding BaseItems
							_ListItemRetrieve lItRetrieve;
							lItRetrieve.FileIndex = selectedFile[i].FileIndex;
							lItRetrieve.fullPath = selectedFile[i].fullPath;
							FileToTreat.push_back(lItRetrieve);
							//SUB
							for (size_t j = 0; j < OpenListing.FileList.size(); j++)
							{
								std::wstring oFile = OpenListing.FileList[j].FullPath;
								std::transform(oFile.begin(), oFile.end(), oFile.begin(), towlower);//To lower
								if (wcsncmp(oFile.c_str(), selectedFile[i].fullPath.c_str(), selectedFile[i].fullPath.length()) == 0)
								{
									lItRetrieve.FileIndex = OpenListing.FileList[j].IndexInArchive;
									lItRetrieve.fullPath = oFile;
									FileToTreat.push_back(lItRetrieve);
								}
							}
						}
						else
						{//FILE
							_ListItemRetrieve lItRetrieve;
							lItRetrieve.FileIndex = OpenListing.FileList[selectedFile[i].FileIndex].IndexInArchive;
							std::wstring oFile = OpenListing.FileList[selectedFile[i].FileIndex].FullPath;
							std::transform(oFile.begin(), oFile.end(), oFile.begin(), towlower);
							lItRetrieve.fullPath = oFile;
							FileToTreat.push_back(lItRetrieve);
						}
					}
					//Clear duplicate entries
					auto comp = [](const _ListItemRetrieve& lhs, const _ListItemRetrieve& rhs) {return lhs.fullPath == rhs.fullPath; };
					auto pred = [](const _ListItemRetrieve& lhs, const _ListItemRetrieve& rhs) {return lhs.fullPath < rhs.fullPath; };
					std::sort(FileToTreat.begin(), FileToTreat.end(), pred);//Sort by name
					auto last = std::unique(FileToTreat.begin(), FileToTreat.end(), comp);
					FileToTreat.erase(last, FileToTreat.end());
					auto pred2 = [](const _ListItemRetrieve& lhs, const _ListItemRetrieve& rhs) {return lhs.FileIndex < rhs.FileIndex; };//Then sort by index for faster file access
					std::sort(FileToTreat.begin(), FileToTreat.end(), pred2);
					DialogBox(hInst, MAKEINTRESOURCE(IDD_DeleteFromArchive), hWnd, DeleteDialog);
				}
		}
		else if (wmId == ID_SETTINGS_ASSOCIATENZARCHIVEWITHNZAFILES)
		{
			TCHAR szPath[MAX_PATH];//Application Path
			DWORD pathLen = 0;
			pathLen = GetModuleFileName(NULL, szPath, MAX_PATH);
			std::wstring ArgPAth = L"\"" + std::wstring(szPath) + L"\" \"%1\"";
			bool WriteToRegistry = false;
			while (true)
			{
				HKEY rTestKey;
				if (RegOpenKey(HKEY_CURRENT_USER, L"Software\\Classes\\NZArchive\\", &rTestKey) != ERROR_SUCCESS)//Classes doesn't exist, need to rewrite
				{
					WriteToRegistry = true;
					RegCloseKey(rTestKey);
					break;
				}
				DWORD BufferSize = MAX_PATH;
				TCHAR wKeyValue[MAX_PATH];
				auto Res = RegGetValue(rTestKey, L"DefaultIcon", NULL, RRF_RT_ANY, NULL, &wKeyValue, &BufferSize);
				if (std::wstring(wKeyValue) != std::wstring(szPath))//Classes does exist but filepath aren't the same, need to rewrite
					WriteToRegistry = true;
				RegCloseKey(rTestKey);
				break;
			}
			if (WriteToRegistry)
			{
				//Need to write 'cause the path has changed before or entry never existed
				HKEY rKeySoftwareClasses;
				RegOpenKey(HKEY_CURRENT_USER, L"Software\\Classes", &rKeySoftwareClasses);
				HKEY rKeySoftwareClassesTypes;
				RegCreateKey(rKeySoftwareClasses, L"NZArchive\\DefaultIcon", &rKeySoftwareClassesTypes);
				RegSetValue(rKeySoftwareClassesTypes, NULL, REG_SZ, szPath, NULL);
				RegCreateKey(rKeySoftwareClasses, L"NZArchive\\shell\\open\\command", &rKeySoftwareClassesTypes);
				RegSetValue(rKeySoftwareClassesTypes, NULL, REG_SZ, ArgPAth.c_str(), NULL);
				RegCreateKey(rKeySoftwareClasses, L".nza\\DefaultIcon", &rKeySoftwareClassesTypes);
				RegSetValue(rKeySoftwareClassesTypes, NULL, REG_SZ, szPath, NULL);
				RegCreateKey(rKeySoftwareClasses, L".nza", &rKeySoftwareClassesTypes);
				RegSetValue(rKeySoftwareClassesTypes, NULL, REG_SZ, L"NZArchive", NULL);
				RegCloseKey(rKeySoftwareClassesTypes);
				RegCloseKey(rKeySoftwareClasses);
				//Update Shell
				SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
			}
		}
		else if (wmId == ID_SETTINGS_REMOVENZAASSOCIATION)
		{
			/*
			* Key to remove :
			* Software\\Classes\\NZArchive\\DefaultIcon
			* Software\\Classes\\NZArchive\\shell
			* Software\\Classes\\.nza
			* We keep the Lang information
			*/
			HKEY rKeySoftwareClasses;
			RegOpenKey(HKEY_CURRENT_USER, L"Software\\Classes", &rKeySoftwareClasses);
			RegDeleteKey(HKEY_CURRENT_USER, L"Software\\Classes\\NZArchive\\DefaultIcon");
			RegDeleteKey(HKEY_CURRENT_USER, L"Software\\Classes\\NZArchive\\shell\\open\\command");
			RegDeleteKey(HKEY_CURRENT_USER, L"Software\\Classes\\NZArchive\\shell\\open");
			RegDeleteKey(HKEY_CURRENT_USER, L"Software\\Classes\\NZArchive\\shell");
			RegDeleteKey(HKEY_CURRENT_USER, L"Software\\Classes\\.nza");

		}
		else if (wmId == ID_ABOUT)
		{
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, AboutDialog);
		}
		else if (wmId == IDM_EXIT)
		{
			DestroyWindow(hWnd);
		}
		else if (WFHelper->vFExplorer.size() > 0)//At least one Explorer widows is in queue, checking...
		{
			WFHelper->ExtractToExplorer = false;
			for (size_t i = 0; i < WFHelper->vFExplorer.size(); i++)
			{
				if (wmId == WFHelper->vFExplorer[i].PeekID)
				{
					WFHelper->ExplorerPathID = i;
					WFHelper->ExtractToExplorer = true;//Bypassing extraction dialog folder
					PostMessage(g_hWnd, WM_COMMAND, ID_EXTRACTSEL, 0);
					return DefWindowProc(hWnd, message, wParam, lParam);
				}

			}
		}
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	break;
	case WM_DROPFILES:
	{
		UINT  uNumFiles;
		TCHAR szNextFile[32767];
		// Get the # of files being dropped.
		uNumFiles = DragQueryFile((HDROP)wParam, -1, NULL, 0);
		vDropFILE.clear();
		for (UINT uFile = 0; uFile < uNumFiles; uFile++)
		{
			if (DragQueryFile((HDROP)wParam, uFile, szNextFile, MAX_PATH) > 0)
				vDropFILE.push_back(szNextFile);
		}
		// Free up memory.
		DragFinish((HDROP)wParam);

		if (vDropFILE.size() > 0 && mArchiveName != L"")
		{
			AddingFromDrop = true;
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ADD_FILE), g_hWnd, AddFileDialog);
		}
	}
	break;
	case WM_NOTIFY:
		switch (LOWORD(wParam))
		{
		case idListView:
			if (!OperationInProgress)
				return ListView_handler(hWnd, message, wParam, lParam);
			break;
		default:
			return false;
		}
		break;
	case WM_SIZE:
	{
		int _Width = LOWORD(lParam);
		int _Height = HIWORD(lParam);
		SetWindowPos(g_TEXTNAV, NULL, 0, 55, _Width, 18, /*SWP_NOMOVE | */SWP_NOZORDER);
		SetWindowPos(g_ListView, NULL, 0, 55 + 18, _Width, _Height - 18 - 18 - 55, /*SWP_NOMOVE | */SWP_NOZORDER);
		SetWindowPos(hwndStatusbar, NULL, 0, _Height - 18, _Width, 18, SWP_NOZORDER);
		// Resize the toolbar, and then show it.
		SendMessage(g_ToolBar, TB_AUTOSIZE, 0, 0);
	}
	break;
	case WM_GETMINMAXINFO:
	{
		MINMAXINFO* minmax = (MINMAXINFO*)lParam;
		minmax->ptMinTrackSize.x = 800;
		minmax->ptMinTrackSize.y = 600;
		// No max size limitation
		//minmax->ptMaxTrackSize.x = 600;	minmax->ptMaxTrackSize.y = 600;
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
	{
		ptbl->Release();
		try
		{
			std::filesystem::remove_all(SessionPath);
		}
		catch (const std::exception&)
		{
		}
		try
		{
			unloadTrad();
		}
		catch (const std::exception&)
		{
		}
		try
		{
			for (size_t i = 0; i < vIconStruct.size(); i++)
				DestroyIcon(vIconStruct[i].pIcon);
		}
		catch (const std::exception&)
		{

		}
		PostQuitMessage(0);
	}
	break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
INT_PTR CALLBACK InputPasswordDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_CREATE:
	{
	}
	break;
	case WM_INITDIALOG:
	{
		SetWindowText(hDlg, TRANSLATE(L"LBL_PWD_TITLE", L"Password input").c_str());
		SetWindowText(GetDlgItem(hDlg, IDC_LBL_PWD), TRANSLATE(L"LBL_PWD_PWD", L"Password (max 32 characters) :").c_str());
		SetWindowText(GetDlgItem(hDlg, ID_PWD_OK), TRANSLATE(L"LBL_PWD_BTN_CONTINUE", L"Continue").c_str());
		SetWindowText(GetDlgItem(hDlg, ID_PWD_CANCEL), TRANSLATE(L"LBL_PWD_BTN_CANCEL", L"Cancel").c_str());
		ptbl->SetProgressValue(g_hWnd, 0, 100);
		ptbl->SetProgressState(g_hWnd, TBPF_INDETERMINATE);
		CenterWindow(hDlg);
		SetDlgItemTextA(hDlg, ID_PWD_TEXTBOX, (LPCSTR)mPassword.c_str());
		SetFocus(GetDlgItem(hDlg, ID_PWD_TEXTBOX));
		SendDlgItemMessage(hDlg, ID_PWD_TEXTBOX, EM_SETSEL, 0, -1);
		SendMessage(GetDlgItem(hDlg, ID_PWD_TEXTBOX), EM_SETLIMITTEXT, 32, 0);
		return false;
	}
	return (INT_PTR)TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_PWD_OK:
		{
			std::vector<char> txt;
			txt.resize(255);
			GetDlgItemTextA(hDlg, ID_PWD_TEXTBOX, &txt[0], (int)txt.size());
			mPassword = std::string(txt.data());
			ptbl->SetProgressState(g_hWnd, TBPF_NOPROGRESS);
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
		case ID_PWD_CANCEL:
		{
			ptbl->SetProgressState(g_hWnd, TBPF_NOPROGRESS);
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)FALSE;
		}
		break;
		default:
			break;
		}
	}
	return (INT_PTR)FALSE;
}
INT_PTR CALLBACK ArchiveInformationDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_CREATE:
	{
	}
	break;
	case WM_INITDIALOG:
	{
		HWND AI_RESUME = GetDlgItem(hDlg, IDC_AI_RESUME);
		SetWindowText(hDlg, TRANSLATE(L"LBL_AI_TITLE", L"Archive information").c_str());
		SetWindowText(GetDlgItem(hDlg, ID_ARC_INF_CLOSE), TRANSLATE(L"LBL_AI_BTCCLOSE", L"Close").c_str());
		NZArchive::Archive _LArchive;
		_LArchive.archiveNZ_set_key(mPassword);
		std::wstring ResumeConstruct = L"";
		ResumeConstruct.append(TRANSLATE(L"LBL_AI_ARCHIVEFILES", L"* Archive path :\n'%s'\n\n", mArchiveName.c_str()));
		FILE* fIN;
		_wfopen_s(&fIN, mArchiveName.c_str(), L"rb");
		if (fIN != NULL)
		{
			NZArchive::Archive::ArchiveEntry Listing = _LArchive.archiveNZ_read_extract_file_list(fIN);
			ResumeConstruct += TRANSLATE(L"LBL_AI_VERSION", L"* Using NZArchive version \n\t%s\n\n", Listing.NZArchiveVersion.c_str());
			std::wstring wsMETHOD = L"";
			switch (Listing.EncryptionMethod)
			{
			case NZArchive::EncryptionMethod::encNone:
				wsMETHOD = TRANSLATE(L"LBLCREATE_ENCRYPTIONNONE", L"None").c_str();
				break;
			case NZArchive::EncryptionMethod::encAES256:
				wsMETHOD = L"AES256";
				break;
			case NZArchive::EncryptionMethod::encChaCha20:
				wsMETHOD = L"ChaCha20";
				break;
			default:
				break;
			}

			ResumeConstruct += TRANSLATE(L"LBL_AI_ENCRYPTION", L"* Encryption method :\n\t%s\n\n", wsMETHOD.c_str());

			ResumeConstruct += TRANSLATE(L"LBL_AI_COMPRESS", L"* Archive compressed size :\n\t%s bytes\n\n", thousandSeparator(Listing.FullArchiveIncludingHeader).c_str());
			ResumeConstruct += TRANSLATE(L"LBL_AI_UNCOMPRESS", L"* Archive uncompressed size :\n\t%s bytes\n\n", thousandSeparator(Listing.ArchiveSizeUncompressed).c_str());
			int lRatio = (int)((double)Listing.FullArchiveIncludingHeader * 100.0 / (double)Listing.ArchiveSizeUncompressed);
			ResumeConstruct += TRANSLATE(L"LBL_AI_RATIO", L"* Archive compression ratio :\n\t%d%%\n\n", lRatio);

			uint64_t FileNbr = 0;
			uint64_t DireNbr = 0;
			for (size_t i = 0; i < Listing.FileList.size(); i++)
				if (Listing.FileList[i].isDirectory)
					DireNbr++;
				else
					FileNbr++;
			ResumeConstruct += TRANSLATE(L"LBL_AI_NBFILES", L"* Number of file(s) :\n\t%s file(s)\n\n", thousandSeparator(FileNbr).c_str());
			ResumeConstruct += TRANSLATE(L"LBL_AI_NBDIR", L"* Number of folder(s) :\n\t%s folder(s)", thousandSeparator(DireNbr).c_str());
			fclose(fIN);
		}
		ResumeConstruct = replaceAll(ResumeConstruct, L"\\n", L"\r\n");
		ResumeConstruct = replaceAll(ResumeConstruct, L"\\t", L"\t");
		SetWindowText(AI_RESUME, ResumeConstruct.c_str());
		CenterWindow(hDlg);
	}
	return (INT_PTR)FALSE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_ARC_INF_CLOSE:
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
		default:
			break;
		}
	}
	return (INT_PTR)FALSE;
}
INT_PTR CALLBACK ArcCreateDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND CREATE_IDC_CREATE_EDIT_Dest = NULL;
	static HWND CREATE_IDC_CREATE_BTN_BrowseDest = NULL;
	static HWND CREATE_IDC_CREATE_Slider = NULL;
	static HWND CREATE_IDC_CREATE_PASSWORD = NULL;
	static HWND CREATE_IDC_CREATE_Edit_SourceFolder = NULL;
	static HWND CREATE_IDC_CREATE_BTN_BrowseSource = NULL;
	static HWND CREATE_IDC_CREATE_BTN_Start = NULL;
	static HWND CREATE_IDC_CREATE_File_being_compressed = NULL;
	static HWND CREATE_IDC_CREATE_Progression_byte = NULL;
	static HWND CREATE_IDC_CREATE_Progress = NULL;
	static HWND CREATE_IDC_CREATE_BTN_CANCELOP = NULL;
	static HWND CREATE_IDC_CREATE_BTN_CLOSE = NULL;
	static HWND CREATE_IDC_CREATE_LBL_LEVEL = NULL;
	static HWND CREATE_IDC_CREATE_RADIO_AES = NULL;
	static HWND CREATE_IDC_CREATE_RADIO_CHACHA = NULL;
	static HWND CREATE_IDC_CREATE_RADIO_NONE = NULL;
	static HWND CREATE_IDC_CREATE_LBL_REDUNDANCY1 = NULL;
	static HWND CREATE_IDC_CREATE_LBL_REDUNDANCY2 = NULL;
	static HWND CREATE_IDC_CREATE_SliderREDUNDANCY = NULL;

	static HWND CREATE_IDC_CREATE_LBL_THREAD1 = NULL;
	static HWND CREATE_IDC_CREATE_LBL_THREAD2 = NULL;
	static HWND CREATE_IDC_CREATE_SliderTHREAD = NULL;

	static HWND CREATE_IDC_CREATE_ListInfo = NULL;
	static NZArchive::Archive _LArchiveCREATE;
	static WireHairHelper WireHelper;

	static std::wstring CREATE_PATHARCHIVE = L"";
	static std::wstring CREATE_PATHSOURCE = L"";
	static bool CancelOperation = false;
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_CREATE:
	{
	}
	break;
	case WM_INITDIALOG:
	{
		CREATE_IDC_CREATE_Slider = GetDlgItem(hDlg, IDC_CREATE_Slider);
		CREATE_IDC_CREATE_PASSWORD = GetDlgItem(hDlg, IDC_CREATE_PASSWORD);
		CREATE_IDC_CREATE_EDIT_Dest = GetDlgItem(hDlg, IDC_CREATE_EDIT_Dest);
		CREATE_IDC_CREATE_Edit_SourceFolder = GetDlgItem(hDlg, IDC_CREATE_Edit_SourceFolder);
		CREATE_IDC_CREATE_LBL_LEVEL = GetDlgItem(hDlg, CREATE_LBL_COMP2);
		CREATE_IDC_CREATE_File_being_compressed = GetDlgItem(hDlg, IDC_CREATE_File_being_compressed);
		CREATE_IDC_CREATE_Progression_byte = GetDlgItem(hDlg, IDC_CREATE_Progression_byte);
		CREATE_IDC_CREATE_Progress = GetDlgItem(hDlg, IDC_CREATE_Progress);
		CREATE_IDC_CREATE_BTN_BrowseDest = GetDlgItem(hDlg, IDC_CREATE_BTN_BrowseDest);
		CREATE_IDC_CREATE_BTN_BrowseSource = GetDlgItem(hDlg, IDC_CREATE_BTN_BrowseSource);
		CREATE_IDC_CREATE_BTN_Start = GetDlgItem(hDlg, IDC_CREATE_BTN_Start);
		CREATE_IDC_CREATE_BTN_CLOSE = GetDlgItem(hDlg, IDC_CREATE_BTN_CLOSE);
		CREATE_IDC_CREATE_BTN_CANCELOP = GetDlgItem(hDlg, IDC_CREATE_BTN_CANCELOP);
		CREATE_IDC_CREATE_RADIO_AES = GetDlgItem(hDlg, IDC_CREATE_RADIOAES256);
		CREATE_IDC_CREATE_RADIO_CHACHA = GetDlgItem(hDlg, IDC_CREATE_RADIOCHACHA);
		CREATE_IDC_CREATE_RADIO_NONE = GetDlgItem(hDlg, IDC_CREATE_RADIONONE);
		CREATE_IDC_CREATE_LBL_REDUNDANCY1 = GetDlgItem(hDlg, CREATE_LBL_REDUNDANCY1);
		CREATE_IDC_CREATE_LBL_REDUNDANCY2 = GetDlgItem(hDlg, CREATE_LBL_REDUNDANCY2);
		CREATE_IDC_CREATE_SliderREDUNDANCY = GetDlgItem(hDlg, IDC_CREATE_SliderREDUNDANCY);
		CREATE_IDC_CREATE_ListInfo = GetDlgItem(hDlg, IDC_CREATE_ListInfo);

		CREATE_IDC_CREATE_LBL_THREAD1 = GetDlgItem(hDlg, CREATE_LBL_NBTHREAD1);
		CREATE_IDC_CREATE_LBL_THREAD2 = GetDlgItem(hDlg, CREATE_LBL_NBTHREAD2);
		CREATE_IDC_CREATE_SliderTHREAD = GetDlgItem(hDlg, IDC_CREATE_SliderThread);


		SendMessageW(CREATE_IDC_CREATE_ListInfo, LB_INITSTORAGE, 2000/*Pre allocate list box with 2000 items*/, 2048/*With 2048 bytes*/);
		SendMessageW(CREATE_IDC_CREATE_ListInfo, LB_SETHORIZONTALEXTENT, (WPARAM)4000, 0);
		SetWindowTheme(CREATE_IDC_CREATE_Progress, L"", L"");
		SetWindowLongPtr(hDlg, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(CREATE_IDC_CREATE_Progress));
		SendMessage(CREATE_IDC_CREATE_Progress, (UINT)PBM_SETBKCOLOR, 0, RGB(200, 200, 200));
		SendMessage(CREATE_IDC_CREATE_Progress, (UINT)PBM_SETBARCOLOR, 0, (LPARAM)RGB(100, 100, 100));
		SendMessage(CREATE_IDC_CREATE_Progress, (UINT)PBM_SETMARQUEE, (WPARAM)TRUE, (LPARAM)50);
		CheckRadioButton(hDlg, IDC_CREATE_RADIOCHACHA, IDC_CREATE_RADIONONE, IDC_CREATE_RADIOCHACHA);
		SendMessage(CREATE_IDC_CREATE_Slider, TBM_SETRANGE, (WPARAM)1, (LPARAM)MAKELONG(1, 22));
		SendMessage(CREATE_IDC_CREATE_Slider, TBM_SETPOS, (WPARAM)1, 5);
		SendMessage(CREATE_IDC_CREATE_SliderREDUNDANCY, TBM_SETRANGE, (WPARAM)1, (LPARAM)MAKELONG(0, 100));
		SendMessage(CREATE_IDC_CREATE_SliderREDUNDANCY, TBM_SETPOS, (WPARAM)1, 5);

		SYSTEM_INFO sysinfo;
		GetSystemInfo(&sysinfo);
		int numCPU = sysinfo.dwNumberOfProcessors;
		SendMessage(CREATE_IDC_CREATE_SliderTHREAD, TBM_SETRANGE, (WPARAM)1, (LPARAM)MAKELONG(1, numCPU));
		SendMessage(CREATE_IDC_CREATE_SliderTHREAD, TBM_SETPOS, (WPARAM)1, 1);

		SendMessage(CREATE_IDC_CREATE_PASSWORD, EM_SETLIMITTEXT, 32, 0);
		SetWindowText(hDlg, TRANSLATE(L"LBLCREATE_TITLE", L"Create NZArchive").c_str());
		SetWindowText(GetDlgItem(hDlg, CREATE_LBL_NZALOC), TRANSLATE(L"LBLCREATE_DEST", L"NZArchive location for creation :").c_str());
		SetWindowText(GetDlgItem(hDlg, CREATE_LBL_COMP1), TRANSLATE(L"LBLCREATE_LEVELDESC", L"Compression level (1 is the fastest, 22 is the strongest) :").c_str());
		SetWindowText(GetDlgItem(hDlg, CREATE_LBL_ENC1), TRANSLATE(L"LBLCREATE_PWDDESC", L"NZArchive encryption password (Empty mean defaut encryption readable by anyone) :").c_str());
		SetWindowText(GetDlgItem(hDlg, CREATE_LBL_ENC2), TRANSLATE(L"LBLCREATE_PWDMAX", L"Max length : 32 characters").c_str());
		SetWindowText(GetDlgItem(hDlg, CREATE_LBL_ENC3), TRANSLATE(L"LBLCREATE_ENCRYPTION", L"Method of encryption :").c_str());
		SetWindowText(GetDlgItem(hDlg, CREATE_LBL_NBTHREAD1), TRANSLATE(L"LBLCREATE_LBL_THREAD1", L"Number of CPU core:").c_str());
		SetWindowText(GetDlgItem(hDlg, IDC_CREATE_RADIOCHACHA), TRANSLATE(L"LBLCREATE_ENCRYPTIONCHACHA", L"ChaCha20").c_str());
		SetWindowText(GetDlgItem(hDlg, IDC_CREATE_RADIOAES256), TRANSLATE(L"LBLCREATE_ENCRYPTIONAES", L"AES256").c_str());
		SetWindowText(GetDlgItem(hDlg, IDC_CREATE_RADIONONE), TRANSLATE(L"LBLCREATE_ENCRYPTIONNONE", L"None").c_str());
		SetWindowText(GetDlgItem(hDlg, CREATE_LBL_SOURCE), TRANSLATE(L"LBLCREATE_SOURCE", L"Source folder to add to NZArchive :").c_str());
		SetWindowText(GetDlgItem(hDlg, CREATE_LBL_COMPPROG), TRANSLATE(L"LBLCREATE_COMPPROG", L"COMPRESSION PROGRESS").c_str());
		SetWindowText(GetDlgItem(hDlg, CREATE_LBL_FILE), TRANSLATE(L"LBLCREATE_FILECOMP", L"File being compressed :").c_str());
		SetWindowText(GetDlgItem(hDlg, CREATE_LBL_PROGRESS), TRANSLATE(L"LBLCREATE_PROG", L"Progression :").c_str());
		SetWindowText(GetDlgItem(hDlg, IDC_CREATE_BTN_BrowseDest), TRANSLATE(L"LBLCREATE_BROWSE", L"Browse").c_str());
		SetWindowText(GetDlgItem(hDlg, IDC_CREATE_BTN_BrowseSource), TRANSLATE(L"LBLCREATE_BROWSE", L"Browse").c_str());
		SetWindowText(GetDlgItem(hDlg, IDC_CREATE_BTN_Start), TRANSLATE(L"LBLCREATE_BTNSTART", L"Start compression").c_str());
		SetWindowText(GetDlgItem(hDlg, IDC_CREATE_BTN_CANCELOP), TRANSLATE(L"LBLCREATE_BTNCANCEL", L"Cancel").c_str());
		SetWindowText(GetDlgItem(hDlg, IDC_CREATE_BTN_CLOSE), TRANSLATE(L"LBLCREATE_BTNCLOSE", L"Close").c_str());
		SetWindowText(GetDlgItem(hDlg, CREATE_LBL_REDUNDANCY1), TRANSLATE(L"LBLCREATE_LBL_REDUNDANCY1", L"Create redundancy data with parity files (PAR2) : (0%% mean no redundancy files)").c_str());
		SetWindowText(GetDlgItem(hDlg, CREATE_LBL_REDUNDANCY2), TRANSLATE(L"LBLCREATE_LBL_REDUNDANCY2", L"%s%% of the archive", L"5").c_str());
		std::wstring Conv = TRANSLATE(L"LBLCREATE_LEVELIS", L"Current level : ") + std::to_wstring(5);
		SendMessage(CREATE_IDC_CREATE_LBL_LEVEL, WM_SETTEXT, 0, (LPARAM)Conv.c_str());
		Conv = TRANSLATE(L"LBLCREATE_LBL_REDUNDANCY2", L"%s%% of the archive", std::to_wstring(5).c_str());
		SendMessage(CREATE_IDC_CREATE_LBL_REDUNDANCY2, WM_SETTEXT, 0, (LPARAM)Conv.c_str());
		{
			std::wstring MemCost = BytesToSize(((1 > 1) ? 1 * ZSTD_estimateCCtxSize(5) : ZSTD_estimateCCtxSize(5)) + 32 * 1024 * 1024 + ZSTD_compressBound(32 * 1024 * 1024) + 4 * 1024 * 1024 + 1 * 1024 * 1024/*ZSTD_CStreamInSize() + ZSTD_CStreamOutSize()*/);
			Conv = TRANSLATE(L"LBLCREATE_LBL_THREAD2", L"%s core [Memory cost : %s]", std::to_wstring(1).c_str(), MemCost.c_str());
			SendMessage(CREATE_IDC_CREATE_LBL_THREAD2, WM_SETTEXT, 0, (LPARAM)Conv.c_str());
		}


		CenterWindow(hDlg);
	}
	return (INT_PTR)FALSE;
	case WM_HSCROLL:
	{
		auto dwPosLVC = SendMessage(CREATE_IDC_CREATE_Slider, TBM_GETPOS, 0, 0);
		std::wstring Conv = TRANSLATE(L"LBLCREATE_LEVELIS", L"Current level : ") + std::to_wstring(dwPosLVC);
		SendMessage(CREATE_IDC_CREATE_LBL_LEVEL, WM_SETTEXT, 0, (LPARAM)Conv.c_str());
		auto dwPosRED = SendMessage(CREATE_IDC_CREATE_SliderREDUNDANCY, TBM_GETPOS, 0, 0);
		Conv = TRANSLATE(L"LBLCREATE_LBL_REDUNDANCY2", L"%s%% of the archive", std::to_wstring(dwPosRED).c_str());
		SendMessage(CREATE_IDC_CREATE_LBL_REDUNDANCY2, WM_SETTEXT, 0, (LPARAM)Conv.c_str());
		auto dwPosTRD = SendMessage(CREATE_IDC_CREATE_SliderTHREAD, TBM_GETPOS, 0, 0);
		std::wstring MemCost = BytesToSize(((dwPosTRD > 1) ? dwPosTRD * ZSTD_estimateCCtxSize(dwPosLVC) : ZSTD_estimateCCtxSize(dwPosLVC)) + 32 * 1024 * 1024 + ZSTD_compressBound(32 * 1024 * 1024) + 4 * 1024 * 1024 + 1 * 1024 * 1024/*ZSTD_CStreamInSize() + ZSTD_CStreamOutSize()*/);
		Conv = TRANSLATE(L"LBLCREATE_LBL_THREAD2", L"%s core [Memory cost : %s]", std::to_wstring(dwPosTRD).c_str(), MemCost.c_str());
		SendMessage(CREATE_IDC_CREATE_LBL_THREAD2, WM_SETTEXT, 0, (LPARAM)Conv.c_str());
	}
	break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_CREATE_BTN_BrowseDest:
		{
			HRESULT hr;
			CComPtr<IFileSaveDialog> pDlg;
			std::wstring TranslationALLFile = TRANSLATE(L"LBL_CREATE_NZAFILES", L"NZArchive files");
			COMDLG_FILTERSPEC aFileTypes[] = {
				{TranslationALLFile.c_str(), L"*.nza" }
			};
			hr = pDlg.CoCreateInstance(__uuidof(FileSaveDialog));
			pDlg->SetFileTypes(_countof(aFileTypes), aFileTypes);
			pDlg->SetTitle(TRANSLATE(L"LBL_CREATE_NZAFILESTITLE", L"Pick a new archive name (Extension will be automatically added):").c_str());
			hr = pDlg->Show(hDlg);
			if (SUCCEEDED(hr))
			{
				CComPtr<IShellItem> pItem;
				hr = pDlg->GetResult(&pItem);
				if (SUCCEEDED(hr))
				{
					LPOLESTR pwsz = NULL;
					hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pwsz);
					if (SUCCEEDED(hr))
					{
						std::wstring fullPathArchive(pwsz);
						std::transform(fullPathArchive.begin(), fullPathArchive.end(), fullPathArchive.begin(), towupper);
						std::wstring fullPathArchiveWithExt(pwsz);
						if (pHelper::GetExtension(fullPathArchive) != L".NZA")
							fullPathArchiveWithExt += L".nza";
						CREATE_PATHARCHIVE = pHelper::NormalizePath(fullPathArchiveWithExt);
						SetWindowText(CREATE_IDC_CREATE_EDIT_Dest, fullPathArchiveWithExt.c_str());
						CoTaskMemFree(pwsz);
					}
				}
			}
		}
		break;
		case IDC_CREATE_BTN_BrowseSource:
		{
			HRESULT hr;
			CComPtr<IFileOpenDialog> pDlg;
			hr = pDlg.CoCreateInstance(__uuidof(FileOpenDialog));
			pDlg->SetTitle(TRANSLATE(L"LBL_CREATE_NZASOURCETITLE", L"Pick a folder to compress (sub-folder will be automatically added):").c_str());
			DWORD options;
			pDlg->GetOptions(&options);
			options &= ~FOS_FILEMUSTEXIST;
			options &= ~FOS_PATHMUSTEXIST;
			pDlg->SetOptions(options | FOS_PICKFOLDERS);
			hr = pDlg->Show(hDlg);
			if (SUCCEEDED(hr))
			{
				CComPtr<IShellItem> pItem;
				hr = pDlg->GetResult(&pItem);
				if (SUCCEEDED(hr))
				{
					LPOLESTR pwsz = NULL;
					hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pwsz);
					if (SUCCEEDED(hr))
					{
						SetWindowText(CREATE_IDC_CREATE_Edit_SourceFolder, pwsz);
						CREATE_PATHSOURCE = pHelper::NormalizePath(std::wstring(pwsz));
						CoTaskMemFree(pwsz);
					}
				}
			}
		}
		break;
		case IDC_CREATE_BTN_Start:
		{
			static NZArchive::EncryptionMethod EncryptInChacha = NZArchive::EncryptionMethod::encNone;
			if (IsDlgButtonChecked(hDlg, IDC_CREATE_RADIOCHACHA) == BST_CHECKED)
				EncryptInChacha = NZArchive::EncryptionMethod::encChaCha20;
			if (IsDlgButtonChecked(hDlg, IDC_CREATE_RADIOAES256) == BST_CHECKED)
				EncryptInChacha = NZArchive::EncryptionMethod::encAES256;
			if (IsDlgButtonChecked(hDlg, IDC_CREATE_RADIONONE) == BST_CHECKED)
				EncryptInChacha = NZArchive::EncryptionMethod::encNone;
			if (CREATE_PATHARCHIVE == L"" || CREATE_PATHSOURCE == L"")
				break;
			static bool KillFileProgressThread;
			KillFileProgressThread = false;//Because staic
			CancelOperation = false;
			EnableWindow(CREATE_IDC_CREATE_BTN_Start, false);
			EnableWindow(CREATE_IDC_CREATE_BTN_CLOSE, false);
			EnableWindow(CREATE_IDC_CREATE_BTN_CANCELOP, true);
			EnableWindow(CREATE_IDC_CREATE_BTN_BrowseDest, false);
			EnableWindow(CREATE_IDC_CREATE_BTN_BrowseSource, false);
			EnableWindow(CREATE_IDC_CREATE_PASSWORD, false);
			EnableWindow(CREATE_IDC_CREATE_Slider, false);
			std::thread CreateThread([&]() {
				ptbl->SetProgressValue(g_hWnd, 0, 100);
				ptbl->SetProgressState(g_hWnd, TBPF_INDETERMINATE);
				std::vector< _CreateListAdd> vecCreateFileList;
				uint64_t FullSize = 0;
				for (const auto& file : std::filesystem::recursive_directory_iterator(CREATE_PATHSOURCE))
				{
					_CreateListAdd _CLAdd;
					//TODO File can be readen ?
					_CLAdd.mPAth = file.path().wstring();
					_CLAdd.mtPAthInArchive = file.path().wstring().substr(CREATE_PATHSOURCE.length());
					_CLAdd.mFileSize = file.file_size();
					vecCreateFileList.push_back(_CLAdd);
					FullSize += file.file_size();
				}
				char txt[32] = { 0 };
				GetWindowTextA(CREATE_IDC_CREATE_PASSWORD, txt, 32);
				std::string mCreatePassword(txt);//RECUP hwnd text
				_LArchiveCREATE.archiveNZ_set_key(mCreatePassword);
				_LArchiveCREATE.archiveNZ_write_reset_progress();
				auto dwPosTRD = SendMessage(CREATE_IDC_CREATE_SliderTHREAD, TBM_GETPOS, 0, 0);
				_LArchiveCREATE.archiveNZ_set_number_of_thread(dwPosTRD);

				FILE* fOUT;
				_wfopen_s(&fOUT, CREATE_PATHARCHIVE.c_str(), L"wb");
				ptbl->SetProgressState(g_hWnd, TBPF_NOPROGRESS);
				if (fOUT != NULL)
				{
					WireHelper.SetPATH(CREATE_PATHARCHIVE, CREATE_PATHARCHIVE + L".npar");
					ptbl->SetProgressValue(g_hWnd, 0, 100);
					ptbl->SetProgressState(g_hWnd, TBPF_NORMAL);
					_LArchiveCREATE.archiveNZ_write_header(fOUT);
					_LArchiveCREATE.archiveNZ_write_set_max_value_for_progress(FullSize);
					InsertItemListbox(CREATE_IDC_CREATE_ListInfo, TRANSLATE(L"LBL_START_ARCHIVE_CREATION", L"Starting creation of archive '%s'", pHelper::getFileFromPath(CREATE_PATHARCHIVE).c_str()));
					std::thread WatchThread([&]() {
						while (!KillFileProgressThread)
						{
							std::this_thread::sleep_for(100ms);
							WireHairHelper::Progression lPGType = WireHairHelper::Progression::P_None;
							auto PercentPAR = WireHelper.GenerateProgress(lPGType);
							std::wstring wsAvancement(L"");
							auto PercentInformation = _LArchiveCREATE.archiveNZ_write_get_progress_reporting();
							if (PercentInformation.Total > 0 && lPGType == WireHairHelper::Progression::P_None)
							{
								std::vector<wchar_t> __Path;
								__Path.resize(32000);
								wmemcpy_s(&__Path[0], PercentInformation.CurrentFile.size(), &PercentInformation.CurrentFile[0], PercentInformation.CurrentFile.size());
								PathCompactPath(NULL, &__Path[0], 500);
								SendMessage(CREATE_IDC_CREATE_File_being_compressed, WM_SETTEXT, 0, (LPARAM)&__Path[0]/*PercentInformation.CurrentFile.c_str()*/);
								SendMessage(CREATE_IDC_CREATE_Progress, PBM_SETPOS, (int)PercentInformation.Progress, 0);
								wsAvancement = thousandSeparator(PercentInformation.Current / 1024) + L" / " + thousandSeparator(PercentInformation.Total / 1024) + L" Kib";
								SendMessage(CREATE_IDC_CREATE_Progression_byte, WM_SETTEXT, 0, (LPARAM)wsAvancement.c_str());
								ptbl->SetProgressValue(g_hWnd, (ULONGLONG)PercentInformation.Progress, 100);
							}
							else if (lPGType != WireHairHelper::Progression::P_None)
							{
								std::wstring nparPATH = CREATE_PATHARCHIVE + L".npar" + L"\0";
								PathCompactPath(NULL, &nparPATH[0], 500);
								SendMessage(CREATE_IDC_CREATE_File_being_compressed, WM_SETTEXT, 0, (LPARAM)&nparPATH[0]);
								SendMessage(CREATE_IDC_CREATE_Progress, PBM_SETPOS, (int)PercentPAR, 0);
								if (lPGType == WireHairHelper::Progression::P_CreateTable)
									wsAvancement = TRANSLATE(L"LBL_NPAR_CREATETABLE", L"Creating integrity table");
								if (lPGType == WireHairHelper::Progression::P_Error)
									wsAvancement = TRANSLATE(L"LBL_NPAR_PERROR", L"Error during generation of parity table");
								if (lPGType == WireHairHelper::Progression::P_Success)
									wsAvancement = TRANSLATE(L"LBL_NPAR_PSUCCESS", L"Parity table successfully created");

								SendMessage(CREATE_IDC_CREATE_Progression_byte, WM_SETTEXT, 0, (LPARAM)wsAvancement.c_str());
								ptbl->SetProgressValue(g_hWnd, (ULONGLONG)PercentInformation.Progress, 100);
							}
						}
						auto PercentInformation = _LArchiveCREATE.archiveNZ_write_get_progress_reporting();
						std::wstring wsAvancement = thousandSeparator(PercentInformation.Total / 1024) + L" / " + thousandSeparator(PercentInformation.Total / 1024) + L" Kib";
						SendMessage(CREATE_IDC_CREATE_Progress, PBM_SETPOS, (int)100, 0);
						SendMessage(CREATE_IDC_CREATE_Progression_byte, WM_SETTEXT, 0, (LPARAM)wsAvancement.c_str());
						ptbl->SetProgressValue(g_hWnd, 0, 100);
						ptbl->SetProgressState(g_hWnd, TBPF_NOPROGRESS);
						OutputDebugString(L"I quit !!\n");
						});
					WatchThread.detach();
					auto dwCompValue = (int)SendMessage(CREATE_IDC_CREATE_Slider, TBM_GETPOS, 0, 0);
					for (size_t i = 0; i < vecCreateFileList.size(); i++)
					{
						if (CancelOperation)
						{
							InsertItemListbox(CREATE_IDC_CREATE_ListInfo, TRANSLATE(L"LBL_CANCEL", L"Cancelled..."));
							break;
						}
						auto Result = _LArchiveCREATE.archiveNZ_write_add_file(fOUT, vecCreateFileList[i].mPAth, vecCreateFileList[i].mtPAthInArchive, dwCompValue, EncryptInChacha);
						if (Result != NZArchive::ADDFILE_SUCCESS)
						{
							InsertItemListbox(CREATE_IDC_CREATE_ListInfo, TRANSLATE(L"LBL_ERROR_ADDING", L"Error adding file '%s'", vecCreateFileList[i].mPAth.c_str()));
							FullSize -= vecCreateFileList[i].mFileSize;
							_LArchiveCREATE.archiveNZ_write_set_max_value_for_progress(FullSize);
						}
					}
					_LArchiveCREATE.archiveNZ_write_footer(fOUT);
					fclose(fOUT);
					//PARITY PARTS
					auto dwPos = (int)SendMessage(CREATE_IDC_CREATE_SliderREDUNDANCY, TBM_GETPOS, 0, 0);
					if (dwPos > 0)
					{
						std::this_thread::sleep_for(200ms);//Ensure thread is finished
						std::wstring wsAvancement = TRANSLATE(L"LBLCREATE_REDUNDANCYINPROGRESS", L"Creating Parity files. Please wait...");
						InsertItemListbox(CREATE_IDC_CREATE_ListInfo, TRANSLATE(L"LBLCREATE_REDUNDANCYINPROGRESS", L"Creating Parity files. Please wait..."));
						ptbl->SetProgressState(g_hWnd, TBPF_PAUSED);
						auto result = WireHelper.GenerateNPARfile(dwPos);
						ptbl->SetProgressState(g_hWnd, TBPF_NOPROGRESS);
						wsAvancement = TRANSLATE(L"LBLCREATE_REDUNDANCYDONE", L"Creation of parity files done, job finished...");
						InsertItemListbox(CREATE_IDC_CREATE_ListInfo, TRANSLATE(L"LBLCREATE_REDUNDANCYDONE", L"Creation of parity files done, job finished..."));
					}
					InsertItemListbox(CREATE_IDC_CREATE_ListInfo, TRANSLATE(L"LBL_ARC_CREATED", L"Archive created"));
					OpenFromcommandLine = true;
					OpenFromcommandLine_Args = std::wstring(CREATE_PATHARCHIVE);
					char txt[32] = { 0 };
					GetWindowTextA(CREATE_IDC_CREATE_PASSWORD, txt, 32);
					mPassword = std::string(txt);
					NeedAnalysis = false;
					PostMessage(g_hWnd, WM_COMMAND, ID_OPEN, 0);
				}
				KillFileProgressThread = true;
				EnableWindow(CREATE_IDC_CREATE_BTN_CANCELOP, false);
				EnableWindow(CREATE_IDC_CREATE_BTN_CLOSE, true);
				//	}
				});
			CreateThread.detach();
		}
		break;
		case IDC_CREATE_BTN_CLOSE:
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
		case IDC_CREATE_BTN_CANCELOP:
		{
			CancelOperation = true;
			EnableWindow(CREATE_IDC_CREATE_BTN_CANCELOP, false);
		}
		break;
		case IDC_CREATE_RADIONONE:
			CheckRadioButton(hDlg, IDC_CREATE_RADIOCHACHA, IDC_CREATE_RADIONONE, IDC_CREATE_RADIONONE);
			break;
		case IDC_CREATE_RADIOAES256:
			CheckRadioButton(hDlg, IDC_CREATE_RADIOCHACHA, IDC_CREATE_RADIONONE, IDC_CREATE_RADIOAES256);
			break;
		case IDC_CREATE_RADIOCHACHA:
			CheckRadioButton(hDlg, IDC_CREATE_RADIOCHACHA, IDC_CREATE_RADIONONE, IDC_CREATE_RADIOCHACHA);
			break;
		default:
			break;
		}
	}
	return (INT_PTR)FALSE;
}
INT_PTR CALLBACK ExtractionDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND EXTR_lblAvancement = NULL;
	static HWND EXTR_lblAvancementFile = NULL;
	static HWND EXTR_FileName = NULL;
	static HWND EXTR_PROGRESS = NULL;
	static HWND EXTR_PROGRESS_FILE = NULL;
	static HWND EXTR_LISTBOX = NULL;
	static HWND EXTR_BTNCancel = NULL;
	static HWND EXTR_BTNClose = NULL;
	static bool CancelOperation = false;
	static NZArchive::Archive _LArchiveEXTRACT;
	static UINT64 EXTR_TotalBytes = 0;
	static UINT64 EXTR_CurrentByte = 0;
	static HWND cpHDLG = 0;
#define KillThisDialog 0x1234
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_CREATE:
	{
	}
	break;
	case WM_INITDIALOG:
	{
		CenterWindow(hDlg);
		cpHDLG = hDlg;
		EXTR_lblAvancement = GetDlgItem(hDlg, IDC_EXTR_lblAvancement);
		EXTR_lblAvancementFile = GetDlgItem(hDlg, IDC_EXTR_lblAvancementFile);
		EXTR_FileName = GetDlgItem(hDlg, IDC_EXTR_FileName);
		EXTR_PROGRESS = GetDlgItem(hDlg, IDC_EXTR_PROGRESS);
		EXTR_PROGRESS_FILE = GetDlgItem(hDlg, IDC_EXTR_PROGRESS_FILE);
		EXTR_BTNCancel = GetDlgItem(hDlg, ID_EXTR_Cancel);
		EXTR_BTNClose = GetDlgItem(hDlg, ID_EXTR_Close);
		SendMessage(EXTR_PROGRESS, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
		SendMessage(EXTR_PROGRESS_FILE, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
		EXTR_LISTBOX = GetDlgItem(hDlg, EXTRACT_LISTBOX);
		SendMessageW(EXTR_LISTBOX, LB_INITSTORAGE, 2000/*Pre allocate list box with 2000 items*/, 2048/*With 2048 bytes*/);
		SendMessageW(EXTR_LISTBOX, LB_SETHORIZONTALEXTENT, (WPARAM)4000, 0);
		EXTR_TotalBytes = 0;
		EXTR_CurrentByte = 0;
		static bool KillFileProgressThread;
		KillFileProgressThread = false;
		CancelOperation = false;
		_LArchiveEXTRACT.archiveNZ_set_key(mPassword);
		SetWindowTheme(EXTR_PROGRESS, L"", L"");
		SetWindowLongPtr(hDlg, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(EXTR_PROGRESS));
		SendMessage(EXTR_PROGRESS, (UINT)PBM_SETBKCOLOR, 0, RGB(200, 200, 200));
		SendMessage(EXTR_PROGRESS, (UINT)PBM_SETBARCOLOR, 0, (LPARAM)RGB(100, 100, 100));
		SendMessage(EXTR_PROGRESS, (UINT)PBM_SETMARQUEE, (WPARAM)TRUE, (LPARAM)50);
		SetWindowTheme(EXTR_PROGRESS_FILE, L"", L"");
		SetWindowLongPtr(hDlg, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(EXTR_PROGRESS_FILE));
		SendMessage(EXTR_PROGRESS_FILE, (UINT)PBM_SETBKCOLOR, 0, RGB(200, 200, 200));
		SendMessage(EXTR_PROGRESS_FILE, (UINT)PBM_SETBARCOLOR, 0, (LPARAM)RGB(150, 150, 150));
		SendMessage(EXTR_PROGRESS_FILE, (UINT)PBM_SETMARQUEE, (WPARAM)TRUE, (LPARAM)50);
		SetWindowText(hDlg, TRANSLATE(L"LBL_EXTR_TITLE", L"Archive extraction").c_str());
		SetWindowText(GetDlgItem(hDlg, IDC_EXTRACT_FILE1), TRANSLATE(L"LBL_EXTR_FILE", L"File :").c_str());
		SetWindowText(GetDlgItem(hDlg, IDC_EXTRACT_FILE2), TRANSLATE(L"LBL_EXTR_FILE", L"File :").c_str());
		SetWindowText(GetDlgItem(hDlg, IDC_EXTRACT_TOTAL), TRANSLATE(L"LBL_EXTR_TOTAL", L"Total :").c_str());
		SetWindowText(GetDlgItem(hDlg, IDC_EXTRACT_FILEWRITTEN), TRANSLATE(L"LBL_EXTR_FILEWRITTEN", L"File data written :").c_str());
		SetWindowText(GetDlgItem(hDlg, IDC_EXTRACT_DATAWRITTEN), TRANSLATE(L"LBL_EXTR_TOTALWRITTEN", L"Total data written :").c_str());
		SetWindowText(GetDlgItem(hDlg, ID_EXTR_Close), TRANSLATE(L"LBL_EXTR_BTNCLOSE", L"Close").c_str());
		SetWindowText(GetDlgItem(hDlg, ID_EXTR_Cancel), TRANSLATE(L"LBL_EXTR_BTNCANCEL", L"Cancel").c_str());
		std::thread ExtractionThread([&]() {
			FILE* fIN;
			_wfopen_s(&fIN, mArchiveName.c_str(), L"rb");
			if (fIN != NULL)
			{
				ptbl->SetProgressState(g_hWnd, TBPF_NORMAL);
				ptbl->SetProgressValue(g_hWnd, 0, 100);
				std::thread WatchThread([&]() {
					while (!KillFileProgressThread)
					{
						std::this_thread::sleep_for(100ms);
						std::wstring wsAvancement(L"");
						//FILE
						auto PercentInformation = _LArchiveEXTRACT.archiveNZ_read_get_progress_reporting();
						if (PercentInformation.Total > 0)
						{
							std::vector<wchar_t> __Path;
							__Path.resize(32000);
							wmemcpy_s(&__Path[0], PercentInformation.CurrentFile.size(), &PercentInformation.CurrentFile[0], PercentInformation.CurrentFile.size());
							PathCompactPath(NULL, &__Path[0], 500);
							SendMessage(EXTR_FileName, WM_SETTEXT, 0, (LPARAM)&__Path[0]/*PercentInformation.CurrentFile.c_str()*/);
							SendMessage(EXTR_PROGRESS_FILE, PBM_SETPOS, (int)PercentInformation.Progress, 0);
							wsAvancement = thousandSeparator(PercentInformation.Current / 1024) + L" / " + thousandSeparator(PercentInformation.Total / 1024) + L" Kib";
							SendMessage(EXTR_lblAvancementFile, WM_SETTEXT, 0, (LPARAM)wsAvancement.c_str());
						}
						//TOTAL
						double Percent = (double)(EXTR_CurrentByte) * 100.0 / (double)EXTR_TotalBytes;
						ptbl->SetProgressState(g_hWnd, TBPF_NORMAL);
						ptbl->SetProgressValue(g_hWnd, (int)Percent, 100);
						SendMessage(EXTR_PROGRESS, PBM_SETPOS, (int)Percent, 0);
						wsAvancement = thousandSeparator(EXTR_CurrentByte / 1024) + L" / " + thousandSeparator(EXTR_TotalBytes / 1024) + L" Kib";
						SendMessage(EXTR_lblAvancement, WM_SETTEXT, 0, (LPARAM)wsAvancement.c_str());
					}
					ptbl->SetProgressValue(g_hWnd, (int)0, 100);
					ptbl->SetProgressState(g_hWnd, TBPF_NOPROGRESS);
					OutputDebugString(L"I quit !!\n");
					});
				WatchThread.detach();
				NZArchive::Archive::ArchiveEntry Listing = _LArchiveEXTRACT.archiveNZ_read_extract_file_list(fIN);
				if (ExtractAll)
					for (size_t i = 0; i < Listing.FileList.size(); i++)
						EXTR_TotalBytes += Listing.FileList[i].FileSize;
				else
					for (size_t i = 0; i < FileToTreat.size(); i++)
						EXTR_TotalBytes += Listing.FileList[FileToTreat[i].FileIndex].FileSize;
				std::wstring ResultString = L"";
				InsertItemListbox(EXTR_LISTBOX, TRANSLATE(L"LBL_START_ARCHIVE_EXTR", L"Starting extraction of archive '%s'", pHelper::getFileFromPath(mArchiveName).c_str()));
				for (size_t i = 0; i < (ExtractAll ? Listing.FileList.size() : FileToTreat.size()); i++)
				{
					if (CancelOperation)
						break;
					std::wstring outputPath = mExtractPAth;
					if (mExtractPAth[mExtractPAth.length() - 1] == L'\\')
						outputPath.pop_back()/*erase(outputPath.end() - 1, outputPath.end())*/;
					outputPath += Listing.FileList[ExtractAll ? i : FileToTreat[i].FileIndex].FullPath.substr(ListTREEpth.size() - 1);
					auto ResultExtract = _LArchiveEXTRACT.archiveNZ_read_extract_file_to_this_full_path(fIN,
						ExtractAll ? i : FileToTreat[i].FileIndex, outputPath, (i == 0) ? false : true);
					if (ResultExtract != NZArchive::OP_EXTRACT::OP_EXTRACT_SUCCESS)
					{
						ResultString = TRANSLATE(L"LBL_EXTR_ERRORWHILE", L"Error while decompressing file '") + outputPath
							+ L"'.\r\n" + ResultString;
						InsertItemListbox(EXTR_LISTBOX, ResultString);
					}
					EXTR_CurrentByte += Listing.FileList[ExtractAll ? i : FileToTreat[i].FileIndex].FileSize;
				}
				fclose(fIN);
				if (CancelOperation)
					ResultString = TRANSLATE(L"LBL_EXTR_CANCELLED", L"Cancelled...\r\n") + ResultString;
				else
					if (ResultString == L"")
						ResultString = TRANSLATE(L"LBL_EXTR_FINISHEDNOERRORS", L"Finished...No errors during decompression...");
					else
						ResultString = TRANSLATE(L"LBL_EXTR_FINISHED", L"Finished...\r\n") + ResultString;
				InsertItemListbox(EXTR_LISTBOX, ResultString);
				EnableWindow(EXTR_BTNCancel, FALSE);
				EnableWindow(EXTR_BTNClose, TRUE);
				if (WFHelper->ExtractToExplorer)
				{
					WFHelper->ExtractToExplorer = false;//Trigger done, resetting

					FLASHWINFO fInfo;
					fInfo.cbSize = sizeof(fInfo);
					fInfo.dwFlags = FLASHW_ALL;
					fInfo.hwnd = WFHelper->vFExplorer[WFHelper->ExplorerPathID].hwnd;
					fInfo.uCount = 6;
					fInfo.dwTimeout = 60;
					FlashWindowEx(&fInfo);

					SetWindowPos(WFHelper->vFExplorer[WFHelper->ExplorerPathID].hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
					SetWindowPos(WFHelper->vFExplorer[WFHelper->ExplorerPathID].hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
					SetForegroundWindow(WFHelper->vFExplorer[WFHelper->ExplorerPathID].hwnd);
					SetFocus(WFHelper->vFExplorer[WFHelper->ExplorerPathID].hwnd);
					SetActiveWindow(WFHelper->vFExplorer[WFHelper->ExplorerPathID].hwnd);
					PostMessage(cpHDLG, WM_COMMAND, ID_EXTR_Close, 0);
				}
				KillFileProgressThread = true;
			}
			});
		ExtractionThread.detach();
	}
	return (INT_PTR)FALSE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_EXTR_Close:
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		case ID_EXTR_Cancel:
		{
			CancelOperation = true;
			EnableWindow(EXTR_BTNCancel, FALSE);
		}
		break;
		default:
			break;
		}
	}
	return (INT_PTR)FALSE;
}
INT_PTR CALLBACK FastExtractDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND FEXTR_lblAvancementFile = NULL;
	static HWND FEXTR_PROGRESS_FILE = NULL;
	static HWND FEXTR_WinDialog = NULL;
	static NZArchive::Archive _LArchiveFASTEXTRACT;
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
	{
		CenterWindow(hDlg);
		FEXTR_lblAvancementFile = GetDlgItem(hDlg, IDC_FASTEXTR_lblAvancementFile);
		FEXTR_PROGRESS_FILE = GetDlgItem(hDlg, IDC_PROGRESS_FASTEXTRACT);
		SetWindowTheme(FEXTR_PROGRESS_FILE, L"", L"");
		SetWindowLongPtr(hDlg, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(FEXTR_PROGRESS_FILE));
		SendMessage(FEXTR_PROGRESS_FILE, (UINT)PBM_SETBKCOLOR, 0, RGB(200, 200, 200));
		SendMessage(FEXTR_PROGRESS_FILE, (UINT)PBM_SETBARCOLOR, 0, (LPARAM)RGB(100, 100, 100));
		SendMessage(FEXTR_PROGRESS_FILE, (UINT)PBM_SETMARQUEE, (WPARAM)TRUE, (LPARAM)50);
		FEXTR_WinDialog = hDlg;
		SendMessage(FEXTR_PROGRESS_FILE, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
		static bool KillFileProgressThread;
		KillFileProgressThread = false;
		_LArchiveFASTEXTRACT.archiveNZ_set_key(mPassword);
		_LArchiveFASTEXTRACT.archiveNZ_read_reset_progress();
		SendMessage(FEXTR_PROGRESS_FILE, PBM_SETPOS, (int)0, 0);
		std::thread FastExtractThread([&]() {
			FILE* fIN;
			_wfopen_s(&fIN, mArchiveName.c_str(), L"rb");
			if (fIN != NULL)
			{
				ptbl->SetProgressState(g_hWnd, TBPF_NORMAL);
				ptbl->SetProgressValue(g_hWnd, 0, 100);
				std::thread WatchThread([&]() {
					while (!KillFileProgressThread)
					{
						std::this_thread::sleep_for(100ms);
						std::wstring wsAvancement(L"");
						auto PercentInformation = _LArchiveFASTEXTRACT.archiveNZ_read_get_progress_reporting();
						if (PercentInformation.Total > 0)
						{
							SendMessage(FEXTR_PROGRESS_FILE, PBM_SETPOS, (int)PercentInformation.Progress, 0);
							wsAvancement = TRANSLATE(L"NULL", L"%s / %s KiB", thousandSeparator(PercentInformation.Current / 1024).c_str(), thousandSeparator(PercentInformation.Total / 1024).c_str());
							SendMessage(FEXTR_lblAvancementFile, WM_SETTEXT, 0, (LPARAM)wsAvancement.c_str());
							ptbl->SetProgressValue(g_hWnd, (int)PercentInformation.Progress, 100);
						}
					}
					ptbl->SetProgressValue(g_hWnd, 0, 100);
					ptbl->SetProgressState(g_hWnd, TBPF_NOPROGRESS);
					OutputDebugString(L"I quit !!\n");
					});
				WatchThread.detach();
				NZArchive::Archive::ArchiveEntry Listing = _LArchiveFASTEXTRACT.archiveNZ_read_extract_file_list(fIN);
				std::wstring ResultString = L"";
				{
					std::wstring outputPath = pHelper::CombinePath(SessionPath + L"\\", pHelper::getFileFromPath(Listing.FileList[FastExtractIndex].FullPath));
					std::vector<wchar_t> __Path;
					__Path.resize(32000);
					wmemcpy_s(&__Path[0], outputPath.size(), &outputPath[0], outputPath.size());
					PathCompactPath(NULL, &__Path[0], 500);
					SetWindowText(FEXTR_WinDialog, &__Path[0]);
					outputPath = pHelper::NormalizePath(outputPath);
					auto ResultFastExtract = _LArchiveFASTEXTRACT.archiveNZ_read_extract_file_to_this_full_path(fIN,
						FastExtractIndex, outputPath);
					if (ResultFastExtract != NZArchive::OP_EXTRACT::OP_EXTRACT_SUCCESS)
						MessageBox(g_hWnd, TRANSLATE(L"LBL_FASTEXTRACT_ERROR", L"Error while decompressing file to temporary place...").c_str()
							, L"NZArchive", MB_ICONERROR);
					else
						if (OpenWith)
						{	// Open with
							OPENASINFO oi = { 0 };
							outputPath = pHelper::NormalizePath(outputPath);
							oi.pcszFile = (LPCWSTR)outputPath.c_str();//TODO MAKE WORKING 'OPEN WITH' CONTAINING LONG PATH
							oi.oaifInFlags = OAIF_EXEC;
							SHOpenWithDialog(NULL, &oi);
						}
						else
						{
							SHELLEXECUTEINFO ExecuteInfo;
							memset(&ExecuteInfo, 0, sizeof(ExecuteInfo));
							ExecuteInfo.cbSize = sizeof(ExecuteInfo);
							ExecuteInfo.fMask = 0;
							ExecuteInfo.hwnd = g_hWnd;
							ExecuteInfo.lpVerb = L"open";
							ExecuteInfo.lpFile = outputPath.c_str();
							ExecuteInfo.lpParameters = L"";
							ExecuteInfo.lpDirectory = SessionPath.c_str();
							ExecuteInfo.nShow = SW_SHOW;
							ExecuteInfo.hInstApp = hInst;
							ShellExecuteEx(&ExecuteInfo);
						}
					PostMessage(FEXTR_WinDialog, WM_COMMAND, KillThisDialog, 0);
				}
				fclose(fIN);
				KillFileProgressThread = true;
			}
			});
		FastExtractThread.detach();
	}
	return (INT_PTR)FALSE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case KillThisDialog:
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
		default:
			break;
		}
	}
	return (INT_PTR)FALSE;
}
INT_PTR CALLBACK AddFileDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND ADD_DROPBOX = NULL;
	static HWND ADD_RADIO_FILE = NULL;
	static HWND ADD_RADIO_FOLDER = NULL;
	static HWND ADD_SLIDER = NULL;
	static HWND ADD_TEXT_SOURCE = NULL;
	static HWND ADD_BTN_ADD = NULL;
	static HWND ADD_BTN_CANCEL = NULL;
	static HWND ADD_BTN_CLOSE = NULL;
	static HWND ADD_BTN_BROWSE = NULL;
	static HWND ADD_LBL_AVANCEMENT = NULL;
	static HWND ADD_LBL_AVANCEMENT2 = NULL;
	static HWND ADD_LBL_LEVEL = NULL;
	static HWND ADD_PROGRESS = NULL;
	static HWND ADD_LISTBOX = NULL;
	static HWND ADD_IDC_ADD_LBL_THREAD1 = NULL;
	static HWND ADD_IDC_ADD_LBL_THREAD2 = NULL;
	static HWND ADD_IDC_ADD_SliderTHREAD = NULL;
	static bool CancelOperation = false;
	static std::wstring SourcePath = L"";
	static std::wstring CBOX_WString = L"";
	static bool FileChoice = true;
	static NZArchive::Archive _LArchiveADD;
	static WireHairHelper WireHelper;

	static std::vector<std::wstring> vTreePath;
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
	{
		ADD_IDC_ADD_LBL_THREAD1 = GetDlgItem(hDlg, ADD_LBL_NBTHREAD1);
		ADD_IDC_ADD_LBL_THREAD2 = GetDlgItem(hDlg, ADD_LBL_NBTHREAD2);
		ADD_IDC_ADD_SliderTHREAD = GetDlgItem(hDlg, IDC_ADD_SLIDERTHR);
		ADD_DROPBOX = GetDlgItem(hDlg, IDC_ADD_DROPLIST);
		ADD_RADIO_FILE = GetDlgItem(hDlg, IDC_RADIOFILE);
		ADD_RADIO_FOLDER = GetDlgItem(hDlg, IDC_RADIOFOLDER);
		ADD_TEXT_SOURCE = GetDlgItem(hDlg, IDC_ADD_SOURCE);
		ADD_BTN_ADD = GetDlgItem(hDlg, ID_ADDSTART);
		ADD_BTN_CANCEL = GetDlgItem(hDlg, ID_ADD_CANCEL2);
		ADD_BTN_CLOSE = GetDlgItem(hDlg, ID_ADD_CLOSE);
		ADD_SLIDER = GetDlgItem(hDlg, IDC_ADD_SLIDER);
		ADD_LBL_AVANCEMENT = GetDlgItem(hDlg, IDC_STATIC_AVANCEMENT);
		ADD_LBL_AVANCEMENT2 = GetDlgItem(hDlg, IDC_STATIC_AVANCEMENT2);
		ADD_PROGRESS = GetDlgItem(hDlg, IDC_ADD_PROGRESS);
		ADD_LBL_LEVEL = GetDlgItem(hDlg, IDC_ADD_LBL_LEVEL2);
		ADD_BTN_BROWSE = GetDlgItem(hDlg, IDC_ADD_BROWSE);
		SendMessage(ADD_SLIDER, TBM_SETRANGE, (WPARAM)1, (LPARAM)MAKELONG(1, 22));
		SendMessage(ADD_SLIDER, TBM_SETPOS, (WPARAM)1, 5);
		std::wstring Conv = TRANSLATE(L"LBLCREATE_LEVELIS", L"Current level : ") + std::to_wstring(5);
		SendMessage(ADD_LBL_LEVEL, WM_SETTEXT, 0, (LPARAM)Conv.c_str());
		SetWindowTheme(ADD_PROGRESS, L"", L"");
		SetWindowLongPtr(hDlg, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(ADD_PROGRESS));
		SendMessage(ADD_PROGRESS, (UINT)PBM_SETBKCOLOR, 0, RGB(200, 200, 200));
		SendMessage(ADD_PROGRESS, (UINT)PBM_SETBARCOLOR, 0, (LPARAM)RGB(100, 100, 100));
		SendMessage(ADD_PROGRESS, (UINT)PBM_SETMARQUEE, (WPARAM)TRUE, (LPARAM)50);
		ADD_LISTBOX = GetDlgItem(hDlg, IDC_ADD_LISTBOX);
		SendMessageW(ADD_LISTBOX, LB_INITSTORAGE, 2000/*Pre allocate list box with 2000 items*/, 2048/*With 2048 bytes*/);
		SendMessageW(ADD_LISTBOX, LB_SETHORIZONTALEXTENT, (WPARAM)4000, 0);
		CenterWindow(hDlg);
		SetWindowText(hDlg, TRANSLATE(L"LBL_ADD_TITLE", L"Add to existing archive").c_str());
		SetWindowText(GetDlgItem(hDlg, IDC_ADD_BROWSE), TRANSLATE(L"LBLCREATE_BROWSE", L"Browse").c_str());
		SetWindowText(GetDlgItem(hDlg, ID_ADD_CLOSE), TRANSLATE(L"LBLCREATE_BTNCLOSE", L"Close").c_str());
		SetWindowText(GetDlgItem(hDlg, ID_ADD_CANCEL2), TRANSLATE(L"LBLCREATE_BTNCANCEL", L"Cancel").c_str());
		SetWindowText(GetDlgItem(hDlg, ID_ADDSTART), TRANSLATE(L"LBL_ADD_BTN_START", L"Add selection to archive").c_str());
		SetWindowText(GetDlgItem(hDlg, IDC_RADIOFILE), TRANSLATE(L"LBL_ADD_RADIO_FILE", L"File").c_str());
		SetWindowText(GetDlgItem(hDlg, IDC_RADIOFOLDER), TRANSLATE(L"LBL_ADD_RADIO_FOLDER", L"Folder").c_str());
		SetWindowText(GetDlgItem(hDlg, IDC_STATIC_ADD), TRANSLATE(L"LBL_ADD_STATIC_ADD", L"Add :").c_str());
		SetWindowText(GetDlgItem(hDlg, IDC_STATIC_INSERTPOINT), TRANSLATE(L"LBL_ADD_STATIC_INSERTAT", L"Entry point in archive :").c_str());
		SetWindowText(GetDlgItem(hDlg, IDC_ADD_LBL_LEVEL1), TRANSLATE(L"LBLCREATE_LEVELDESC", L"Compression level (1 is the fastest, 22 is the strongest) :").c_str());
		SetWindowText(GetDlgItem(hDlg, IDC_STATIC_PROGRESSION), TRANSLATE(L"LBLCREATE_PROG", L"Progression :").c_str());

		{
			SYSTEM_INFO sysinfo;
			GetSystemInfo(&sysinfo);
			int numCPU = sysinfo.dwNumberOfProcessors;
			SendMessage(ADD_IDC_ADD_SliderTHREAD, TBM_SETRANGE, (WPARAM)1, (LPARAM)MAKELONG(1, numCPU));
			SendMessage(ADD_IDC_ADD_SliderTHREAD, TBM_SETPOS, (WPARAM)1, 1);
			std::wstring MemCost = BytesToSize(((1 > 1) ? 1 * ZSTD_estimateCCtxSize(5) : ZSTD_estimateCCtxSize(5)) + 32 * 1024 * 1024 + ZSTD_compressBound(32 * 1024 * 1024) + 4 * 1024 * 1024 + 1 * 1024 * 1024/*ZSTD_CStreamInSize() + ZSTD_CStreamOutSize()*/);
			Conv = TRANSLATE(L"LBLCREATE_LBL_THREAD2", L"%s core [Memory cost : %s]", std::to_wstring(1).c_str(), MemCost.c_str());
			SendMessage(ADD_IDC_ADD_LBL_THREAD2, WM_SETTEXT, 0, (LPARAM)Conv.c_str());
			SetWindowText(GetDlgItem(hDlg, ADD_LBL_NBTHREAD1), TRANSLATE(L"LBLCREATE_LBL_THREAD1", L"Number of CPU core:").c_str());
		}


		NZArchive::Archive _lArchive;
		NZArchive::Archive::ArchiveEntry _Listing;
		_lArchive.archiveNZ_set_key(mPassword);
		FILE* fIN;
		_wfopen_s(&fIN, mArchiveName.c_str(), L"rb");
		if (fIN != NULL)
		{
			_Listing = _lArchive.archiveNZ_read_extract_file_list(fIN);
			fclose(fIN);
		}
		int _inc = 0;
		{
			int pos = (int)SendMessage(ADD_DROPBOX, CB_ADDSTRING, 0, (LPARAM)ListTREEpth.c_str());
			SendMessage(ADD_DROPBOX, CB_SETITEMDATA, pos, (LPARAM)_inc);
		}
		vTreePath.clear();
		for (size_t i = 0; i < _Listing.FileList.size(); i++)
		{
			if (_Listing.FileList[i].isDirectory)
			{
				std::wstring _lPTH = _Listing.FileList[i].FullPath;
				if (_lPTH.size() > 0)
					_lPTH += (_lPTH.substr(_lPTH.size() - 1, 1) == L"\\") ? L"" : L"\\";
				vTreePath.push_back(_lPTH);
				std::transform(vTreePath[vTreePath.size() - 1].begin(), vTreePath[vTreePath.size() - 1].end(), vTreePath[vTreePath.size() - 1].begin(), towlower);
				int pos = (int)SendMessage(ADD_DROPBOX, CB_ADDSTRING, 0,
					(LPARAM)_lPTH.c_str());
				SendMessage(ADD_DROPBOX, CB_SETITEMDATA, pos, (LPARAM)_inc);
				_inc++;
			}
		}
		SendMessage(ADD_DROPBOX, CB_SETCURSEL, 0, 0);
		CheckRadioButton(hDlg, IDC_RADIOFILE, IDC_RADIOFOLDER, IDC_RADIOFILE);
		//File was dropped ?
		if (AddingFromDrop)
		{
			EnableWindow(ADD_RADIO_FILE, false);
			EnableWindow(ADD_RADIO_FOLDER, false);
			EnableWindow(ADD_BTN_BROWSE, false);
			EnableWindow(ADD_BTN_ADD, true);
			std::wstring SumFile = L"";
			for (size_t i = 0; i < vDropFILE.size(); i++)
				SumFile.append(vDropFILE[i] + L";");
			if (SumFile.size() > 0)
				if (SumFile.substr(SumFile.size() - 1, 1) == L";")
					SumFile.pop_back();//Remove last ;
			if (SumFile.size() > 32000)
			{
				SumFile.resize(32000);
				SumFile.append(L"…");
			}
			SetWindowText(ADD_TEXT_SOURCE, SumFile.c_str());
		}

		SetFocus(ADD_DROPBOX);
		return TRUE;
	}
	return (INT_PTR)FALSE;
	case WM_HSCROLL:
	{
		auto dwPosLVL = SendMessage(ADD_SLIDER, TBM_GETPOS, 0, 0);
		std::wstring Conv = TRANSLATE(L"LBLCREATE_LEVELIS", L"Current level : ") + std::to_wstring(dwPosLVL);
		SendMessage(ADD_LBL_LEVEL, WM_SETTEXT, 0, (LPARAM)Conv.c_str());
		auto dwPosTRD = SendMessage(ADD_IDC_ADD_SliderTHREAD, TBM_GETPOS, 0, 0);
		std::wstring MemCost = BytesToSize(((dwPosTRD > 1) ? dwPosTRD * ZSTD_estimateCCtxSize(dwPosLVL) : ZSTD_estimateCCtxSize(dwPosLVL)) + 32 * 1024 * 1024 + ZSTD_compressBound(32 * 1024 * 1024) + 4 * 1024 * 1024 + 1 * 1024 * 1024/*ZSTD_CStreamInSize() + ZSTD_CStreamOutSize()*/);
		Conv = TRANSLATE(L"LBLCREATE_LBL_THREAD2", L"%s core [Memory cost : %s]", std::to_wstring(dwPosTRD).c_str(), MemCost.c_str());
		SendMessage(ADD_IDC_ADD_LBL_THREAD2, WM_SETTEXT, 0, (LPARAM)Conv.c_str());
	}
	break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_ADD_BROWSE:
		{
			if (IsDlgButtonChecked(hDlg, IDC_RADIOFILE) == BST_CHECKED)
			{//Adding a file
				HRESULT hr;
				CComPtr<IFileOpenDialog> pDlg;
				COMDLG_FILTERSPEC aFileTypes[] = {
					{ TRANSLATE(L"LBL_ADD_ALLFILES", L"All files").c_str() , L"*.*" }
				};
				hr = pDlg.CoCreateInstance(__uuidof(FileOpenDialog));
				pDlg->SetFileTypes(_countof(aFileTypes), aFileTypes);
				pDlg->SetTitle(TRANSLATE(L"LBL_ADD_TITLE", L"Add to existing archive").c_str());
				hr = pDlg->Show(hDlg);
				if (SUCCEEDED(hr))
				{
					CComPtr<IShellItem> pItem;
					hr = pDlg->GetResult(&pItem);
					if (SUCCEEDED(hr))
					{
						LPOLESTR pwsz = NULL;
						if (!OpenFromcommandLine)
							hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pwsz);
						if (SUCCEEDED(hr) || OpenFromcommandLine)
						{
							SetWindowText(ADD_TEXT_SOURCE, pHelper::NormalizePath(pwsz).c_str());
							SourcePath = pHelper::NormalizePath(pwsz);
							FileChoice = (IsDlgButtonChecked(hDlg, IDC_RADIOFILE) == BST_CHECKED);
							EnableWindow(ADD_BTN_ADD, TRUE);
							CoTaskMemFree(pwsz);
						}
					}
				}
			}
			else
			{//Adding a folder
				HRESULT hr;
				CComPtr<IFileOpenDialog> pDlg;
				COMDLG_FILTERSPEC aFileTypes[] = {
					{ TRANSLATE(L"LBL_ADD_ALLFILES", L"All files").c_str() , L"*.*" }
				};
				hr = pDlg.CoCreateInstance(__uuidof(FileOpenDialog));
				DWORD options;
				pDlg->GetOptions(&options);
				options &= ~FOS_FILEMUSTEXIST;
				options &= ~FOS_PATHMUSTEXIST;
				pDlg->SetOptions(options | FOS_PICKFOLDERS);
				pDlg->SetTitle(TRANSLATE(L"LBL_ADD_TITLE", L"Add to existing archive").c_str());
				hr = pDlg->Show(hDlg);
				if (SUCCEEDED(hr))
				{
					CComPtr<IShellItem> pItem;
					hr = pDlg->GetResult(&pItem);
					if (SUCCEEDED(hr))
					{
						LPOLESTR pwsz = NULL;
						if (!OpenFromcommandLine)
							hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pwsz);
						if (SUCCEEDED(hr) || OpenFromcommandLine)
						{
							SourcePath = (pwsz == NULL) ? L"" : pHelper::NormalizePath(pwsz);
							SetWindowText(ADD_TEXT_SOURCE, SourcePath.c_str());
							EnableWindow(ADD_BTN_ADD, TRUE);
							FileChoice = (IsDlgButtonChecked(hDlg, IDC_RADIOFILE) == BST_CHECKED);
							CoTaskMemFree(pwsz);
						}
					}
				}
			}
		}
		break;
		case ID_ADDSTART:
		{
			EnableWindow(ADD_BTN_CLOSE, false);
			EnableWindow(ADD_BTN_ADD, false);
			EnableWindow(ADD_BTN_CANCEL, true);
			EnableWindow(ADD_RADIO_FILE, false);
			EnableWindow(ADD_RADIO_FOLDER, false);
			EnableWindow(ADD_BTN_BROWSE, false);
			EnableWindow(ADD_DROPBOX, false);
			EnableWindow(ADD_SLIDER, false);
			EnableWindow(ADD_IDC_ADD_SliderTHREAD, false);
			CancelOperation = false;
			static bool KillFileProgressThread;
			KillFileProgressThread = false;//Because staic
			CancelOperation = false;
			std::vector<wchar_t> vBuffOut;
			vBuffOut.resize(32500);
			GetWindowText(ADD_DROPBOX, (LPWSTR)&vBuffOut[0], (int)vBuffOut.size());
			CBOX_WString = std::wstring(vBuffOut.data());
			vBuffOut.clear();
			if (CBOX_WString == L"") CBOX_WString = L"\\";
			if (CBOX_WString.substr(0, 1) != L"\\")  CBOX_WString = L"\\" + CBOX_WString;
			CBOX_WString += (CBOX_WString.substr(CBOX_WString.size() - 1, 1) == L"\\") ? L"" : L"\\";
			std::thread AddThread([&]() {
				ptbl->SetProgressValue(g_hWnd, 0, 100);
				ptbl->SetProgressState(g_hWnd, TBPF_INDETERMINATE);
				std::vector< _CreateListAdd> vecCreateFileList;
				uint64_t FullSize = 0;
				WireHelper.SetPATH(mArchiveName, mArchiveName + L".npar");
				//Creating intermediate directories if needed
				auto OutSplit = SplitWstring(CBOX_WString.substr(1), L'\\');
				std::wstring DummyPath = L"";
				if (OutSplit.size() > 0)
				{
					for (size_t i = 0; i < OutSplit.size(); i++)
					{
						DummyPath += L"\\" + OutSplit[i];
						std::wstring lDummyPTH = DummyPath + L"\\";
						std::transform(lDummyPTH.begin(), lDummyPTH.end(), lDummyPTH.begin(), towlower);
						bool DoAdd = true;
						for (size_t j = 0; j < vTreePath.size(); j++)
						{
							std::wstring lTreePTH = vTreePath[j];
							std::transform(lTreePTH.begin(), lTreePTH.end(), lTreePTH.begin(), towlower);
							OutputDebugString(TRANSLATE(L"", L"%s/%s\n", lTreePTH.c_str(), lDummyPTH.c_str()).c_str());
							if (lTreePTH == lDummyPTH)
							{
								DoAdd = false;
								break;
							}
						}
						if (DoAdd)
						{
							_CreateListAdd _CLAdd;
							_CLAdd.mPAth = L"";//Empty mean dummies folders
							_CLAdd.mtPAthInArchive = DummyPath;
							_CLAdd.mFileSize = 0;
							vecCreateFileList.push_back(_CLAdd);
						}
					}

				}


				if (AddingFromDrop)//File originated from a File Drop
				{
					for (size_t i = 0; i < vDropFILE.size(); i++)
					{
						if (std::filesystem::exists(vDropFILE[i]))
							if (std::filesystem::is_directory(vDropFILE[i]))
							{
								std::wstring DirectoryName = vDropFILE[i].substr(vDropFILE[i].find_last_of(L"\\") + 1) + L"\\";
								{//Adding Base folder
									//TODO TEST IF ALREADY EXIST
									_CreateListAdd _CLAdd;
									_CLAdd.mPAth = L"";//Empty mean dummies folders
									_CLAdd.mtPAthInArchive = CBOX_WString + DirectoryName.substr(0, DirectoryName.size() - 1);
									_CLAdd.mFileSize = 0;
									vecCreateFileList.push_back(_CLAdd);
								}
								for (const auto& file : std::filesystem::recursive_directory_iterator(vDropFILE[i]))
								{
									//TODO Folder can be readen ?
									_CreateListAdd _CLAdd;
									_CLAdd.mPAth = file.path().wstring();
									_CLAdd.mtPAthInArchive = CBOX_WString + DirectoryName + file.path().wstring().substr(vDropFILE[i].size() + 1);
									_CLAdd.mFileSize = file.file_size();
									vecCreateFileList.push_back(_CLAdd);
									FullSize += file.file_size();
								}
							}
							else
							{
								//TODO File can be readen ?
								_CreateListAdd _CLAdd;
								_CLAdd.mPAth = vDropFILE[i];
								std::wstring FileOnlyWithNoSlash = vDropFILE[i].substr(vDropFILE[i].find_last_of(L"\\") + 1);
								_CLAdd.mtPAthInArchive = CBOX_WString + FileOnlyWithNoSlash;
								vecCreateFileList.push_back(_CLAdd);
								FullSize += std::filesystem::file_size(vDropFILE[i]);
							}
					}
				}
				else
				{//File originated from user interface
					if (FileChoice)
					{
						//TODO File can be readen ?
						_CreateListAdd _CLAdd;
						_CLAdd.mPAth = SourcePath;
						std::wstring FileOnlyWithNoSlash = SourcePath.substr(SourcePath.find_last_of(L"\\") + 1);
						_CLAdd.mtPAthInArchive = CBOX_WString + FileOnlyWithNoSlash;
						vecCreateFileList.push_back(_CLAdd);
						FullSize = std::filesystem::file_size(SourcePath);
					}
					else
					{
						std::wstring DirectoryName = SourcePath.substr(SourcePath.find_last_of(L"\\") + 1) + L"\\";
						{//Adding Base folder
							//TODO TEST IF ALREADY EXIST
							_CreateListAdd _CLAdd;
							_CLAdd.mPAth = L"";//Empty mean dummies folders
							_CLAdd.mtPAthInArchive = CBOX_WString + DirectoryName.substr(0, DirectoryName.size() - 1);
							_CLAdd.mFileSize = 0;
							vecCreateFileList.push_back(_CLAdd);
						}
						for (const auto& file : std::filesystem::recursive_directory_iterator(SourcePath))
						{
							//TODO Folder can be readen ?
							_CreateListAdd _CLAdd;
							_CLAdd.mPAth = file.path().wstring();
							_CLAdd.mtPAthInArchive = CBOX_WString + DirectoryName + file.path().wstring().substr(SourcePath.size() + 1);
							_CLAdd.mFileSize = file.file_size();
							vecCreateFileList.push_back(_CLAdd);
							FullSize += file.file_size();
						}
					}
				}
				_LArchiveADD.archiveNZ_set_key(mPassword);
				auto dwPosTRD = SendMessage(ADD_IDC_ADD_SliderTHREAD, TBM_GETPOS, 0, 0);
				_LArchiveADD.archiveNZ_set_number_of_thread(dwPosTRD);
				FILE* fOUT;
				_wfopen_s(&fOUT, mArchiveName.c_str(), L"ab+");//Append (+ :> allow file truncating) and binary for no extra characters...
				ptbl->SetProgressState(g_hWnd, TBPF_NOPROGRESS);
				if (fOUT != NULL)
				{
					InsertItemListbox(ADD_LISTBOX, TRANSLATE(L"LBL_START_ADD", L"Starting addition to archive '%s'", pHelper::getFileFromPath(mArchiveName).c_str()));
					uint64_t aSize = 0;
					uint64_t pSize = 0;
					uint64_t RedundancyPercent = UINT64_MAX;
					if (std::filesystem::exists(mArchiveName + L".npar"))
						RedundancyPercent = WIREHAIR_RetrievePercentFromFile(mArchiveName + L".npar");
					auto _ArcInfo = _LArchiveADD.archiveNZ_read_extract_file_list(fOUT);
					/* TO ADD FILE, we need to remove the old header*/
					{
						uint64_t PosHeaderCompSize = 8/*magic byte*/ + sizeof(uint16_t)/*EOA*/ + (4 * sizeof(uint64_t))/*Header size uncomp / Comp + hashIn + hashOut*/;
						_fseeki64(fOUT, std::filesystem::file_size(mArchiveName) - PosHeaderCompSize, SEEK_SET); //Position of uint64_t containg header comp/uncomp size
						auto szHeaderUncomp = _LArchiveADD.FILE_read64(fOUT);// We get decompressed size
						auto szHeaderComp = _LArchiveADD.FILE_read64(fOUT);//We get the compressed size
						uint64_t PosHeaderCompBegin = PosHeaderCompSize + szHeaderComp;
						_LArchiveADD.archiveNZ_truncate_file_by(fOUT, PosHeaderCompBegin);	//We remove the header from file
					}
					ptbl->SetProgressValue(g_hWnd, 0, 100);
					ptbl->SetProgressState(g_hWnd, TBPF_NORMAL);
					_LArchiveADD.archiveNZ_write_reset_progress();
					_LArchiveADD.archiveNZ_write_set_max_value_for_progress(FullSize);
					std::thread WatchThread([&]() {
						while (!KillFileProgressThread)
						{
							std::this_thread::sleep_for(100ms);
							WireHairHelper::Progression lPGType = WireHairHelper::Progression::P_None;
							double Percent = WireHelper.GenerateProgress(lPGType);
							std::wstring wsAvancement(L"");
							auto PercentInformation = _LArchiveADD.archiveNZ_write_get_progress_reporting();
							if (PercentInformation.Total > 0 && lPGType == WireHairHelper::Progression::P_None)
							{
								std::vector<wchar_t> __Path;
								__Path.resize(32000);
								wmemcpy_s(&__Path[0], PercentInformation.CurrentFile.size(), &PercentInformation.CurrentFile[0], PercentInformation.CurrentFile.size());
								PathCompactPath(NULL, &__Path[0], 500);
								SendMessage(ADD_LBL_AVANCEMENT, WM_SETTEXT, 0, (LPARAM)&__Path[0]/*PercentInformation.CurrentFile.c_str()*/);
								SendMessage(ADD_PROGRESS, PBM_SETPOS, (int)PercentInformation.Progress, 0);
								wsAvancement = TRANSLATE(L"EMPTY", L"%s / %s KiB", thousandSeparator(PercentInformation.Current / 1024).c_str(), thousandSeparator(PercentInformation.Total / 1024).c_str());
								SendMessage(ADD_LBL_AVANCEMENT2, WM_SETTEXT, 0, (LPARAM)wsAvancement.c_str());
								ptbl->SetProgressValue(g_hWnd, (ULONGLONG)PercentInformation.Progress, 100);
							}
							else if (lPGType != WireHairHelper::Progression::P_None)
							{
								std::wstring nparPATH = mArchiveName + L".npar" + L"\0";
								PathCompactPath(NULL, &nparPATH[0], 500);
								SendMessage(ADD_LBL_AVANCEMENT, WM_SETTEXT, 0, (LPARAM)&nparPATH[0]);
								SendMessage(ADD_PROGRESS, PBM_SETPOS, (int)Percent, 0);
								wsAvancement = L"";
								if (lPGType == WireHairHelper::Progression::P_CreateTable)
									wsAvancement = TRANSLATE(L"LBL_NPAR_CREATETABLE", L"Creating integrity table");
								if (lPGType == WireHairHelper::Progression::P_Error)
									wsAvancement = TRANSLATE(L"LBL_NPAR_PERROR", L"Error during generation of parity table");
								if (lPGType == WireHairHelper::Progression::P_Success)
									wsAvancement = TRANSLATE(L"LBL_NPAR_PSUCCESS", L"Parity table successfully created");

								SendMessage(ADD_LBL_AVANCEMENT2, WM_SETTEXT, 0, (LPARAM)wsAvancement.c_str());
								ptbl->SetProgressValue(g_hWnd, (ULONGLONG)Percent, 100);
							}
						}
						auto PercentInformation = _LArchiveADD.archiveNZ_write_get_progress_reporting();
						std::wstring wsAvancement = TRANSLATE(L"", L"%s / %s KiB", thousandSeparator(PercentInformation.Total / 1024).c_str(), thousandSeparator(PercentInformation.Total / 1024).c_str());
						SendMessage(ADD_PROGRESS, PBM_SETPOS, (int)100, 0);
						SendMessage(ADD_LBL_AVANCEMENT2, WM_SETTEXT, 0, (LPARAM)wsAvancement.c_str());
						ptbl->SetProgressValue(g_hWnd, 0, 100);
						ptbl->SetProgressState(g_hWnd, TBPF_NOPROGRESS);
						OutputDebugString(L"I quit !!\n");
						});
					WatchThread.detach();
					auto dwCompValue = SendMessage(ADD_SLIDER, TBM_GETPOS, 0, 0);
					for (size_t i = 0; i < vecCreateFileList.size(); i++)
					{
						if (CancelOperation)
							break;
						auto Result = _LArchiveADD.archiveNZ_write_add_file(fOUT, vecCreateFileList[i].mPAth, vecCreateFileList[i].mtPAthInArchive, (int)dwCompValue, _ArcInfo.EncryptionMethod);
						if (Result != NZArchive::ADDFILE_SUCCESS)
						{
							FullSize -= vecCreateFileList[i].mFileSize;
							_LArchiveADD.archiveNZ_write_set_max_value_for_progress(FullSize);
							InsertItemListbox(ADD_LISTBOX, TRANSLATE(L"LBL_ERROR_ADDING", L"Error adding file '%s'", vecCreateFileList[i].mPAth.c_str()));
						}
					}
					_LArchiveADD.archiveNZ_write_footer(fOUT);
					fclose(fOUT);
					if (RedundancyPercent != UINT64_MAX)//Previous PAR2 file presents
					{
						EnableWindow(ADD_BTN_CANCEL, false);
						ptbl->SetProgressState(g_hWnd, TBPF_PAUSED);
						if (RedundancyPercent == -1.0) RedundancyPercent = 5.0;
						std::wstring wsAvancement = TRANSLATE(L"LBLCREATE_REDUNDANCYINPROGRESS", L"Creating Parity files. Please wait...");
						InsertItemListbox(ADD_LISTBOX, wsAvancement);
						WireHelper.GenerateNPARfile(RedundancyPercent);
						wsAvancement = TRANSLATE(L"LBLCREATE_REDUNDANCYDONE", L"Creation of parity files done, job finished...");
						InsertItemListbox(ADD_LISTBOX, wsAvancement);
						ptbl->SetProgressState(g_hWnd, TBPF_NOPROGRESS);
					}
					InsertItemListbox(ADD_LISTBOX, TRANSLATE(L"LBL_END_ADD", L"Adding to archive done..."));
				}
				std::this_thread::sleep_for(200ms);
				KillFileProgressThread = true;
				EnableWindow(ADD_BTN_CLOSE, true);
				EnableWindow(ADD_BTN_ADD, false);
				EnableWindow(ADD_BTN_CANCEL, false);
				});
			AddThread.detach();
		}
		break;
		case ID_ADD_CLOSE:
		{
			OpenFromcommandLine = true;
			OpenFromcommandLine_Args = std::wstring(mArchiveName);
			AddingFromDrop = false;
			NeedAnalysis = false;
			PostMessage(g_hWnd, WM_COMMAND, ID_OPEN, 0);
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
		case ID_ADD_CANCEL2:
		{
			CancelOperation = true;
			EnableWindow(ADD_BTN_CLOSE, TRUE);
			EnableWindow(ADD_BTN_ADD, FALSE);
			EnableWindow(ADD_BTN_CANCEL, FALSE);
		}
		break;
		default:
			break;
		}
	}
	return (INT_PTR)FALSE;
}
INT_PTR CALLBACK DeleteDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND DELETE_lblAvancementFile = NULL;
	static HWND DELETE_PROGRESS_FILE = NULL;
	static HWND DELETE_WinDialog = NULL;
	static NZArchive::Archive _LArchiveDELETE;
	static NZArchive::Archive _LArchiveSRCHEADER;
	static WireHairHelper WireHelper;

	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_CREATE:
	{
	}
	break;
	case WM_INITDIALOG:
	{
		OperationInProgress = true;
		CenterWindow(hDlg);
		DELETE_lblAvancementFile = GetDlgItem(hDlg, IDC_DELETE_lblAvancementFile);
		DELETE_PROGRESS_FILE = GetDlgItem(hDlg, IDC_DELETE_PROGRESS);
		DELETE_WinDialog = hDlg;
		SetWindowTheme(DELETE_PROGRESS_FILE, L"", L"");
		SetWindowLongPtr(hDlg, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(DELETE_PROGRESS_FILE));
		SendMessage(DELETE_PROGRESS_FILE, (UINT)PBM_SETBKCOLOR, 0, RGB(200, 200, 200));
		SendMessage(DELETE_PROGRESS_FILE, (UINT)PBM_SETBARCOLOR, 0, (LPARAM)RGB(100, 100, 100));
		SendMessage(DELETE_PROGRESS_FILE, (UINT)PBM_SETMARQUEE, (WPARAM)TRUE, (LPARAM)50);
		SendMessage(DELETE_PROGRESS_FILE, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
		static bool KillFileProgressThread;
		KillFileProgressThread = false;
		_LArchiveDELETE.archiveNZ_set_key(mPassword);
		double _Progress = 0.0;
		std::thread DeleteThread([&]() {
			std::wstring  wsAvancement = L"";
			FILE* fIN;
			uint64_t aSize = 0;
			uint64_t pSize = 0;
			uint64_t NPAR_Redundancy = UINT64_MAX;
			if (std::filesystem::exists(mArchiveName + L".npar"))
				NPAR_Redundancy = WIREHAIR_RetrievePercentFromFile(mArchiveName + L".npar");
			try
			{
				std::filesystem::rename(mArchiveName, std::wstring(mArchiveName + L".backup").c_str());
			}
			catch (const std::exception&)
			{
				MessageBox(hDlg, L"ERROR", L"ERROR", MB_ICONERROR);
				PostMessage(DELETE_WinDialog, WM_COMMAND, KillThisDialog, 0);
				return;
			}
			_wfopen_s(&fIN, std::wstring(mArchiveName + L".backup").c_str(), L"rb");
			if (fIN != NULL)
			{
				FILE* fOUT;
				_wfopen_s(&fOUT, std::wstring(mArchiveName + L".backup").c_str(), L"rb");
				NZArchive::Archive::HeaderStream srcHeader;
				if (fOUT != NULL)
				{
					_LArchiveSRCHEADER.archiveNZ_set_key(mPassword);
					auto _SrcHeaderList = _LArchiveSRCHEADER.archiveNZ_read_extract_file_list(fOUT);
					srcHeader = _LArchiveSRCHEADER.mHeader;
					srcHeader.Position = 6; //We keep the first six bytes for Major/Minus/Revision indice
					fclose(fOUT);
				}
				_wfopen_s(&fOUT, mArchiveName.c_str(), L"wb");
				if (fOUT != NULL)
				{
					plf::nanotimer mTMR, mTMR2;
					mTMR.start();
					mTMR2.start();
					ptbl->SetProgressState(g_hWnd, TBPF_NORMAL);
					ptbl->SetProgressValue(g_hWnd, 0, 100);
					NZArchive::Archive::ArchiveEntry Listing = _LArchiveDELETE.archiveNZ_read_extract_file_list(fIN);
					_fseeki64(fIN, 0, SEEK_SET);//Resetting position
					{//Copying Header //Size : 1x8 (8) for MagicByte 
						std::vector<std::byte> _buffer;
						_buffer.resize(sizeof(NZArchive::magicByte));
						fread_s(&_buffer[0], _buffer.size(), sizeof(std::byte), _buffer.size(), fIN);
						fwrite(&_buffer[0], _buffer.size(), sizeof(std::byte), fOUT);
					}
					for (size_t i = 0; i < Listing.FileList.size(); i++)
					{
						if (mTMR.get_elapsed_ms() > 100.0)//Refresh every 1/10 seconds
						{
							_Progress = (double)_ftelli64(fIN) * 100.0 / (double)Listing.FullArchiveIncludingHeader;
							SendMessage(DELETE_PROGRESS_FILE, PBM_SETPOS, (int)_Progress, 0);
							wsAvancement = thousandSeparator(_ftelli64(fIN) / 1024) + L" / " + thousandSeparator(Listing.FullArchiveIncludingHeader / 1024) + L" Kib";
							SendMessage(DELETE_lblAvancementFile, WM_SETTEXT, 0, (LPARAM)wsAvancement.c_str());
							ptbl->SetProgressValue(g_hWnd, (int)_Progress, 100);
							mTMR.start();
						}
						bool skipThisEntry = false;
						for (size_t j = 0; j < FileToTreat.size(); j++)
							if (FileToTreat[j].FileIndex == i)
							{
								skipThisEntry = true;
								break;
							}
						uint32_t nbCharRemove = 0;
						{
							/*
							* Remove Header information
							* *************************
							* FOLDER		: 2*uint16 + (1*uint32_t (Path.len) + Path.Len * 2 (wchar=>char)) + 7*uint64
							* EMPTYFILE		: 2*uint16 + (1*uint32_t (Path.len) + Path.Len * 2 (wchar=>char)) + 8*uint64
							* NONEMPTYFILE	: 2*uint16 + (1*uint32_t (Path.len) + Path.Len * 2 (wchar=>char)) + 8*uint64 + 2*uint64 * NbBlock(szComp/hash) + 1*uint64 (end block)
							*/
							if (Listing.FileList[i].isDirectory)
								nbCharRemove = (2 * sizeof(uint16_t)) + sizeof(uint32_t) + (Listing.FileList[i].FullPath.size() * 2) + (7 * sizeof(uint64_t));
							else
								if (Listing.FileList[i].FileSize == 0)
									nbCharRemove = (2 * sizeof(uint16_t)) + sizeof(uint32_t) + (Listing.FileList[i].FullPath.size() * 2) + (8 * sizeof(uint64_t));
								else
									nbCharRemove = (2 * sizeof(uint16_t)) + sizeof(uint32_t) + (Listing.FileList[i].FullPath.size() * 2) + (8 * sizeof(uint64_t)) + ((2 * sizeof(uint64_t) * Listing.FileList[i].FileChunks.size())) + (1 * sizeof(uint64_t));
						}
						if (skipThisEntry)//Not writing this one
						{// Erase from header
							srcHeader.Header.erase(srcHeader.Header.begin() + srcHeader.Position, srcHeader.Header.begin() + srcHeader.Position + nbCharRemove);
							_fseeki64(fIN, Listing.FileList[i].CopySize, SEEK_CUR);//Skipping this entry in file						
							continue;
						}
						else//We are copying this one
						{
							srcHeader.Position += nbCharRemove;//Advance header position (Mean we keep this entry)
							const UINT64 _BUFFER = 8 * 1024 * 1024;
							UINT64 RemainingByte = Listing.FileList[i].CopySize;
							while (RemainingByte != 0)
							{
								uint64_t _TempBuffer = _BUFFER;
								if (RemainingByte <= _BUFFER)
									_TempBuffer = RemainingByte;
								std::vector<std::byte> Buffer;
								Buffer.clear();
								Buffer.resize(_TempBuffer);
								fread_s(&Buffer[0], Buffer.size(), sizeof(unsigned char), Buffer.size(), fIN);
								fwrite(&Buffer[0], sizeof(std::byte), Buffer.size(), fOUT);
								RemainingByte -= _TempBuffer;
								if (mTMR2.get_elapsed_ms() > 100.0)//Refresh every 1/10s
								{
									//Refresh UI
									_Progress = (double)_ftelli64(fIN) * 100.0 / (double)Listing.FullArchiveIncludingHeader;
									SendMessage(DELETE_PROGRESS_FILE, PBM_SETPOS, (int)_Progress, 0);
									wsAvancement = thousandSeparator(_ftelli64(fIN) / 1024) + L" / " + thousandSeparator(Listing.FullArchiveIncludingHeader / 1024) + L" Kib";
									SendMessage(DELETE_lblAvancementFile, WM_SETTEXT, 0, (LPARAM)wsAvancement.c_str());
									ptbl->SetProgressValue(g_hWnd, (int)_Progress, 100);
									mTMR2.start();
								}
							}
						}
					}
					//Copying footer with the new header
					{
						uint64_t headerHashIn = 0;
						uint64_t headerHashOut = 0;
						std::vector<std::byte> compressedHeader;
						_LArchiveDELETE.CompAndCrypt(srcHeader.Header, compressedHeader, headerHashIn, headerHashOut, 22, NZArchive::EncryptionMethod::encChaCha20);
						fwrite(&compressedHeader[0], sizeof(std::byte), compressedHeader.size(), fOUT);
						_LArchiveDELETE.FILE_write64(fOUT, srcHeader.Header.size());
						_LArchiveDELETE.FILE_write64(fOUT, compressedHeader.size());
						_LArchiveDELETE.FILE_write64(fOUT, headerHashIn);
						_LArchiveDELETE.FILE_write64(fOUT, headerHashOut);
						_LArchiveDELETE.FILE_write16(fOUT, UINT16_MAX);//End archive
						fwrite(NZArchive::magicByte, sizeof(NZArchive::magicByte), 1, fOUT);
					}
					_Progress = (double)_ftelli64(fIN) * 100.0 / (double)Listing.FullArchiveIncludingHeader;
					SendMessage(DELETE_PROGRESS_FILE, PBM_SETPOS, (int)_Progress, 0);
					std::wstring  wsAvancement = thousandSeparator(_ftelli64(fIN) / 1024) + L" / " + thousandSeparator(Listing.FullArchiveIncludingHeader / 1024) + L" Kib";
					SendMessage(DELETE_lblAvancementFile, WM_SETTEXT, 0, (LPARAM)wsAvancement.c_str());
					ptbl->SetProgressValue(g_hWnd, 0, 100);
					ptbl->SetProgressState(g_hWnd, TBPF_NOPROGRESS);
					fclose(fOUT);
				}
				fclose(fIN);
				try
				{
					if (std::filesystem::exists(mArchiveName))//Exists
						if (std::filesystem::file_size(mArchiveName) > 0)//Size superior to 0 bytes
							std::filesystem::remove(std::wstring(mArchiveName + L".backup"));
				}
				catch (const std::exception&)
				{
				}
				if (std::filesystem::exists(mArchiveName))//Exists
				{
					//Guess we can check if parity files exist
					if (NPAR_Redundancy != UINT64_MAX)
					{
						ptbl->SetProgressState(g_hWnd, TBPF_PAUSED);
						std::wstring  wsAvancement = TRANSLATE(L"LBLCREATE_REDUNDANCYINPROGRESS", L"Creating Parity files. Please wait...");
						SendMessage(DELETE_lblAvancementFile, WM_SETTEXT, 0, (LPARAM)wsAvancement.c_str());
						WireHelper.SetPATH(mArchiveName, mArchiveName + L".npar");
						bool killNPARThread = false;
						std::thread WatchThread([&]() {
							while (!killNPARThread)
							{
								std::this_thread::sleep_for(100ms);
								WireHairHelper::Progression lPGType = WireHairHelper::Progression::P_None;
								auto PercentPAR = WireHelper.GenerateProgress(lPGType);
								wsAvancement = L"";
								if (lPGType == WireHairHelper::Progression::P_CreateTable)
									wsAvancement = TRANSLATE(L"LBL_NPAR_CREATETABLE", L"Creating integrity table");
								if (lPGType == WireHairHelper::Progression::P_Error)
									wsAvancement = TRANSLATE(L"LBL_NPAR_PERROR", L"Error during generation of parity table");
								if (lPGType == WireHairHelper::Progression::P_Success)
									wsAvancement = TRANSLATE(L"LBL_NPAR_PSUCCESS", L"Parity table successfully created");

								SendMessage(DELETE_lblAvancementFile, WM_SETTEXT, 0, (LPARAM)wsAvancement.c_str());
								SendMessage(DELETE_PROGRESS_FILE, PBM_SETPOS, (int)PercentPAR, 0);
								ptbl->SetProgressValue(g_hWnd, PercentPAR, 100);
							}

							OutputDebugString(L"I quit !!\n");
							});
						WatchThread.detach();

						auto result = WireHelper.GenerateNPARfile(NPAR_Redundancy);
						killNPARThread = true;
						std::this_thread::sleep_for(300ms);
						wsAvancement = TRANSLATE(L"LBLCREATE_REDUNDANCYDONE", L"Creation of parity files done, job finished...");
						SendMessage(DELETE_lblAvancementFile, WM_SETTEXT, 0, (LPARAM)wsAvancement.c_str());
						ptbl->SetProgressState(g_hWnd, TBPF_NOPROGRESS);
					}
				}
				PostMessage(DELETE_WinDialog, WM_COMMAND, KillThisDialog, 0);
			}
			});
		DeleteThread.detach();
	}
	return (INT_PTR)FALSE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case KillThisDialog:
		{
			OperationInProgress = false;
			OpenFromcommandLine = true;
			OpenFromcommandLine_Args = std::wstring(mArchiveName);
			NeedAnalysis = false;
			PostMessage(g_hWnd, WM_COMMAND, ID_OPEN, 0);
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
		default:
			break;
		}
	}
	return (INT_PTR)FALSE;
}
INT_PTR CALLBACK TestArchiveDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND TEST_lblAvancement = NULL;
	static HWND TEST_FileName = NULL;
	static HWND TEST_PROGRESS = NULL;
	static HWND TEST_LISTBOX = NULL;
	static HWND TEST_BTNCancel = NULL;
	static bool CancelOperation = false;
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_CREATE:
	{
	}
	break;
	case WM_INITDIALOG:
	{
		CenterWindow(hDlg);
		CancelOperation = false;
		TEST_lblAvancement = GetDlgItem(hDlg, IDC_TEST_lblAvancement);
		TEST_FileName = GetDlgItem(hDlg, IDC_TEST_FileName);
		TEST_PROGRESS = GetDlgItem(hDlg, IDC_TEST_PROGRESS);
		TEST_LISTBOX = GetDlgItem(hDlg, TEST_LBOX);
		SendMessageW(TEST_LISTBOX, LB_INITSTORAGE, 2000/*Pre allocate list box with 2000 items*/, 2048/*With 2048 bytes*/);
		SendMessageW(TEST_LISTBOX, LB_SETHORIZONTALEXTENT, (WPARAM)4000, 0);
		TEST_BTNCancel = GetDlgItem(hDlg, ID_TEST_Cancel);
		SetWindowText(GetDlgItem(hDlg, IDC_TEST_LBLFILE), TRANSLATE(L"LBL_TEST_FILE", L"File :").c_str());
		SetWindowText(GetDlgItem(hDlg, IDC_TEST_LBLPOSITION), TRANSLATE(L"LBL_TEST_POSITION", L"Position in archive :").c_str());
		SetWindowText(hDlg, TRANSLATE(L"LBL_TEST_TITLE", L"Test progression").c_str());
		SetWindowText(GetDlgItem(hDlg, ID_TEST_Close), TRANSLATE(L"LBL_TEST_BTNCLOSE", L"Close").c_str());
		SetWindowText(GetDlgItem(hDlg, ID_TEST_Cancel), TRANSLATE(L"LBL_TEST_BTNCANCEL", L"Cancel").c_str());
		SetWindowTheme(TEST_PROGRESS, L"", L"");
		SetWindowLongPtr(hDlg, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(TEST_PROGRESS));
		SendMessage(TEST_PROGRESS, (UINT)PBM_SETBKCOLOR, 0, RGB(200, 200, 200));
		SendMessage(TEST_PROGRESS, (UINT)PBM_SETBARCOLOR, 0, (LPARAM)RGB(100, 100, 100));
		SendMessage(TEST_PROGRESS, (UINT)PBM_SETMARQUEE, (WPARAM)TRUE, (LPARAM)50);
		SendMessage(TEST_PROGRESS, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
		ptbl->SetProgressState(g_hWnd, TBPF_NORMAL);
		ptbl->SetProgressValue(g_hWnd, 0, 100);
		std::thread TestThread([&]() {
			plf::nanotimer mTMER;
			plf::nanotimer mTMERF;
			mTMER.start();
			mTMERF.start();
			NZArchive::Archive _LArchive;
			_LArchive.archiveNZ_set_key(mPassword);
			FILE* fIN;
			_wfopen_s(&fIN, mArchiveName.c_str(), L"rb");
			if (fIN != NULL)
			{
				InsertItemListbox(TEST_LISTBOX, TRANSLATE(L"LBL_START_ARCHIVE_TEST", L"Starting test of archive '%s'", pHelper::getFileFromPath(mArchiveName).c_str()));
				std::wstring ResultString = L"";
				NZArchive::Archive::ArchiveEntry Listing = _LArchive.archiveNZ_read_extract_file_list(fIN);
				for (size_t i = 0; i < Listing.FileList.size(); i++)
				{
					if (CancelOperation)
						break;
					std::vector<wchar_t> __Path;
					__Path.resize(32000);
					wmemcpy_s(&__Path[0], Listing.FileList[i].FullPath.size(), &Listing.FileList[i].FullPath[0], Listing.FileList[i].FullPath.size());
					PathCompactPath(NULL, &__Path[0], 500);
					if (mTMERF.get_elapsed_ms() > 50.0)
					{
						SendMessage(TEST_FileName, WM_SETTEXT, 0, (LPARAM)&__Path[0]/*OpenListing.FileList[i].FullPath.c_str()*/);
						mTMERF.start();
					}
					if (Listing.FileList[i].isDirectory)
						continue;
					_fseeki64(fIN, Listing.FileList[i].PositionInArchive, SEEK_SET);
					for (size_t j = 0; j < Listing.FileList[i].FileChunks.size(); j++)
					{
						//Update GUI
						if (mTMER.get_elapsed_ms() > 50.0)
						{
							double Percent = (double)_ftelli64(fIN) * 100.0 / (double)Listing.FullArchiveIncludingHeader;
							SendMessage(TEST_PROGRESS, PBM_SETPOS, (int)Percent, 0);
							ptbl->SetProgressValue(g_hWnd, (int)Percent, 100);
							std::wstring wsAvancement = thousandSeparator(_ftelli64(fIN) / 1024) + L" / " + thousandSeparator(Listing.ArchiveSizeCompressed / 1024) + L" Kib";
							SendMessage(TEST_lblAvancement, WM_SETTEXT, 0, (LPARAM)wsAvancement.c_str());
							mTMER.start();
						}
						//Reading chunk
						std::vector< std::byte> vReadContent;
						vReadContent.resize(Listing.FileList[i].FileChunks[j].ChunkCompressed);
						fread_s(&vReadContent[0], vReadContent.size(), sizeof(std::byte), vReadContent.size(), fIN);
						//Computing XXHASH
						uint64_t _HashRC = XXH3_64bits(&vReadContent[0], vReadContent.size());
						//Comparing
						if (_HashRC != Listing.FileList[i].FileChunks[j].hashCompressed)
						{
							ResultString = TRANSLATE(L"LBL_TEST_HASHERRORS", L"Hash error at offset %s, file'%s' is corrupted.\r\n",
								thousandSeparator(_ftelli64(fIN)).c_str(), Listing.FileList[i].FullPath.c_str()) + ResultString;
							InsertItemListbox(TEST_LISTBOX, ResultString);
							break;//Advance to next file
						}
					}
				}
				double Percent = 100.0;
				SendMessage(TEST_PROGRESS, PBM_SETPOS, (int)Percent, 0);
				ptbl->SetProgressValue(g_hWnd, (int)Percent, 100);
				std::wstring wsAvancement = thousandSeparator(Listing.FullArchiveIncludingHeader / 1024) + L" / " + thousandSeparator(Listing.FullArchiveIncludingHeader / 1024) + L" KiB";
				SendMessage(TEST_lblAvancement, WM_SETTEXT, 0, (LPARAM)wsAvancement.c_str());
				fclose(fIN);
				if (CancelOperation)
					ResultString = TRANSLATE(L"LBL_TEST_CANCELED", L"Cancelled...\r\n") + ResultString;
				else
					if (ResultString == L"")
						ResultString = TRANSLATE(L"LBL_TEST_FINISHEDNOERRORS", L"Finished...No errors during test...");
					else
						ResultString = TRANSLATE(L"LBL_TEST_FINISHED", L"Finished...") + L"\r\n" + ResultString;
				InsertItemListbox(TEST_LISTBOX, ResultString.c_str());
				EnableWindow(TEST_BTNCancel, FALSE);
				ptbl->SetProgressValue(g_hWnd, 0, 100);
				ptbl->SetProgressState(g_hWnd, TBPF_NOPROGRESS);
			}
			});
		TestThread.detach();
	}
	return (INT_PTR)FALSE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_TEST_Close:
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		case ID_TEST_Cancel:
		{
			CancelOperation = true;
			EnableWindow(TEST_BTNCancel, FALSE);
		}
		break;
		default:
			break;
		}
	}
	return (INT_PTR)FALSE;
}
INT_PTR CALLBACK ChangeDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND CHANGE_LISTBOX = NULL;
	static std::vector<std::wstring> vLANG;
	static std::wstring wsLANGPATH = L"";
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
	{
		CHANGE_LISTBOX = GetDlgItem(hDlg, IDC_LANG_LIST);
		vLANG.clear();
		//
		std::vector<wchar_t> myPath;
		myPath.resize(_MAX_PATH + 1);
		GetModuleFileName(NULL, &myPath[0], _MAX_PATH);
		wsLANGPATH = pHelper::CombinePath(pHelper::getDirectoryFromPath(myPath.data(), true), L"Language\\");
		myPath.clear();
		//Fill with language file
		if (std::filesystem::exists(wsLANGPATH))
		{
			for (const auto& file : std::filesystem::directory_iterator(wsLANGPATH))
			{
				auto Extension = file.path().extension().wstring();
				std::transform(Extension.begin(), Extension.end(), Extension.begin(), towlower);
				if (Extension == L".trd")
				{
					vLANG.push_back(file.path().filename().wstring());
				}
			}
			if (vLANG.size() > 0)
			{
				EnableWindow(GetDlgItem(hDlg, IDC_LANG_CHANGE), true);
				for (int i = 0; i < vLANG.size(); i++)
				{
					int pos = (int)SendMessage(CHANGE_LISTBOX, LB_ADDSTRING, 0,
						(LPARAM)vLANG[i].c_str());
					SendMessage(CHANGE_LISTBOX, LB_SETITEMDATA, pos, (LPARAM)i);
				}
			}
		}
		CenterWindow(hDlg);
		SetWindowText(hDlg, TRANSLATE(L"LBL_LANG_TITLE", L"Change NZArchive language").c_str());
		SetWindowText(GetDlgItem(hDlg, IDC_LANG_CHANGE), TRANSLATE(L"LBL_LANG_BTNCHANGE", L"Change").c_str());
		SetWindowText(GetDlgItem(hDlg, IDC_LANG_CANCEL), TRANSLATE(L"LBL_LANG_BTNCANCEL", L"Cancel").c_str());
		SetFocus(CHANGE_LISTBOX);
		return TRUE;
	}
	return (INT_PTR)FALSE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_LANG_CHANGE:
		{
			int lbItem = (int)SendMessage(CHANGE_LISTBOX, LB_GETCURSEL, 0, 0);
			if (lbItem != -1)
			{
				HKEY rKeySoftwareClasses;
				RegOpenKey(HKEY_CURRENT_USER, L"Software\\Classes", &rKeySoftwareClasses);
				HKEY rKeySoftwareClassesTypes;
				RegCreateKey(rKeySoftwareClasses, L"NZArchive\\Lang", &rKeySoftwareClassesTypes);
				RegSetValue(rKeySoftwareClassesTypes, NULL, REG_SZ, vLANG[lbItem].c_str(), NULL);
				isLocalizationLoaded = LoadLocalisationTRD(wsLANGPATH + vLANG[lbItem]);
				UI_Refresh_Variable(g_hWnd);
			}
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
		case IDC_LANG_CANCEL:
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
		case IDC_LANG_LIST:
		{
			switch (HIWORD(wParam))
			{
			case LBN_SELCHANGE:
			{
				return TRUE;
			}
			}
		}
		return TRUE;
		default:
			break;
		}
	}
	return (INT_PTR)FALSE;
}
INT_PTR CALLBACK AboutDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
	{
		CenterWindow(hDlg);
	}
	return (INT_PTR)TRUE;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
INT_PTR CALLBACK IntegrityChecking(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND INT_PROGRESS = NULL;
	static HWND INT_LISTBOX = NULL;
	static HWND INT_DIALOG = NULL;
	static WireHairHelper wireHelper;

	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
	{
		INT_DIALOG = hDlg;

		INT_LISTBOX = GetDlgItem(hDlg, IDC_INTEGRITY_LISTBOX);
		SendMessageW(INT_LISTBOX, LB_INITSTORAGE, 2000/*Pre allocate list box with 2000 items*/, 2048/*With 2048 bytes*/);
		SendMessageW(INT_LISTBOX, LB_SETHORIZONTALEXTENT, (WPARAM)4000, 0);

		INT_PROGRESS = GetDlgItem(hDlg, IDC_INTEGRITY_PROGRESS);
		SetWindowTheme(INT_PROGRESS, L"", L"");
		SetWindowLongPtr(hDlg, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(INT_PROGRESS));
		SendMessage(INT_PROGRESS, (UINT)PBM_SETBKCOLOR, 0, RGB(200, 200, 200));
		SendMessage(INT_PROGRESS, (UINT)PBM_SETBARCOLOR, 0, (LPARAM)RGB(100, 100, 100));
		SendMessage(INT_PROGRESS, (UINT)PBM_SETMARQUEE, (WPARAM)TRUE, (LPARAM)50);
		SendMessage(INT_PROGRESS, PBM_SETRANGE, 0, MAKELPARAM(0, 100));

		SetWindowText(hDlg, TRANSLATE(L"LBL_INT_TITLE", L"Integrity checking").c_str());

		ptbl->SetProgressValue(g_hWnd, 0, 100);
		ptbl->SetProgressState(g_hWnd, TBPF_INDETERMINATE);
		CenterWindow(hDlg);

		static bool KillFileProgressThread;
		KillFileProgressThread = false;

		std::thread CheckingThread([&]() {
			ptbl->SetProgressState(g_hWnd, TBPF_NORMAL);
			ptbl->SetProgressValue(g_hWnd, 0, 100);
			std::thread WatchThread([&]() {
				WireHairHelper::Progression oldProgression = WireHairHelper::Progression::P_None;
				while (!KillFileProgressThread)
				{
					std::this_thread::sleep_for(100ms);
					std::wstring wsAvancement(L"");
					WireHairHelper::Progression Progression;
					double Percentage = wireHelper.ControlAndRepairProgress(Progression);
					SendMessage(INT_PROGRESS, PBM_SETPOS, (int)Percentage, 0);
					ptbl->SetProgressValue(g_hWnd, (int)Percentage, 100);
					if (Progression != oldProgression)//Changing state
						if (Progression == WireHairHelper::Progression::P_AnalysingFile)
							InsertItemListbox(INT_LISTBOX, TRANSLATE(L"LBL_INT_ANA", L"Hashing archive file "));
						else if (Progression == WireHairHelper::Progression::P_CreateTable)
							InsertItemListbox(INT_LISTBOX, TRANSLATE(L"LBL_INT_CRE", L"Populating block table from parity file "));
						else if (Progression == WireHairHelper::Progression::P_Repairing)
							InsertItemListbox(INT_LISTBOX, TRANSLATE(L"LBL_INT_REP", L"Repairing archive... "));
					oldProgression = Progression;
				}
				ptbl->SetProgressValue(g_hWnd, 0, 100);
				ptbl->SetProgressState(g_hWnd, TBPF_NOPROGRESS);
				OutputDebugString(L"I quit !!\n");
				});
			WatchThread.detach();
			wireHelper.SetPATH(mArchiveName, mArchiveName + L".npar");
			auto Result = wireHelper.ControlAndRepair();
			switch (Result)
			{
			case WireHairHelper::CandRError::C_CantOpenNPARFile:
				InsertItemListbox(INT_LISTBOX, TRANSLATE(L"LBL_INT_C_CANTOPENNPARFILE", L"Can't open parity file "));
				break;
			case WireHairHelper::CandRError::C_CantOpenArchiveFile:
				InsertItemListbox(INT_LISTBOX, TRANSLATE(L"LBL_INT_C_CANTOPENARCHIVEFILE", L"Can't open archive file "));
				break;
			case WireHairHelper::CandRError::C_ArchiveIsNotSameSize:
				InsertItemListbox(INT_LISTBOX, TRANSLATE(L"LBL_INT_C_ARCHIVEISNOTSAMESIZE", L"Archive size inside parity file isn't the same size then actual archive "));
				break;
			case WireHairHelper::CandRError::C_NeedNoRepair:
				InsertItemListbox(INT_LISTBOX, TRANSLATE(L"LBL_INT_C_NEEDNOREPAIR", L"Archive need no reparation "));
				break;
			case WireHairHelper::CandRError::C_CantCreateRepairFile:
				InsertItemListbox(INT_LISTBOX, TRANSLATE(L"LBL_INT_C_CANTCREATEREPAIRFILE", L"Can't create repaired archive file "));
				break;
			case WireHairHelper::CandRError::C_UnknownError:
				InsertItemListbox(INT_LISTBOX, TRANSLATE(L"LBL_INT_C_UNKNOWNERROR", L"An unknown error happened "));
				break;
			case WireHairHelper::CandRError::C_RepairSuccess:
				InsertItemListbox(INT_LISTBOX, TRANSLATE(L"LBL_INT_C_REPAIRSUCCESS", L"Reparation of the archive is successful "));
				break;
			case WireHairHelper::CandRError::C_RepairFailedNotEnoughBlockToRepair:
				InsertItemListbox(INT_LISTBOX, TRANSLATE(L"LBL_INT_C_NOTENOUGHBLOCK", L"Reparation failed, not enough recovery parts available... "));
				break;
			default:
				break;
			}

			if (Result == WireHairHelper::CandRError::C_NeedNoRepair || Result == WireHairHelper::CandRError::C_RepairSuccess)
				NPARAnalysisConform = true;
			else
				NPARAnalysisConform = false;

			KillFileProgressThread = true;
			std::this_thread::sleep_for(1000ms);
			PostMessage(INT_DIALOG, WM_COMMAND, KillThisDialog, 0);
			});
		CheckingThread.detach();
		return false;
	}
	return (INT_PTR)TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case KillThisDialog:
		{
			ptbl->SetProgressState(g_hWnd, TBPF_NOPROGRESS);
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
		default:
			break;
		}
	}
	return (INT_PTR)FALSE;
}

BOOL ListView_handler(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LPNMLISTVIEW  pnm = (LPNMLISTVIEW)lParam;
	switch (pnm->hdr.code)
	{
	case NM_DBLCLK:
	{
		if (pnm->iItem != -1)//Empty space click
		{
			int index;
			index = (int)SendMessage(g_ListView, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
			if (index == -1)
				break;
			if (ListTREE.FileList[index].isDirectory)
			{   //Folder => We explore
				ListTREEpth = ListTREEpth + ListTREE.FileList[index].FullPath + L"\\";
				ListTREE.FileList.clear();
				for (size_t i = 0; i < OpenListing.FileList.size(); i++)
				{
					std::wstring lITEM = OpenListing.FileList[i].FullPath;
					if (lITEM.size() > ListTREEpth.length())//First, is the path long enough to contain folder ? //Superior mean the base path can't be in
						if (lITEM.substr(0, ListTREEpth.length()) == ListTREEpth) //Is the root the same ?
							if (lITEM.substr(ListTREEpth.length() + 1).find_first_of(L"\\") == std::wstring::npos)
							{//No backslash, it's root item
								auto GoodItem = OpenListing.FileList[i];
								GoodItem.FullPath = GoodItem.FullPath.substr(ListTREEpth.length());
								ListTREE.FileList.push_back(GoodItem);
							}
				}
				std::sort(ListTREE.FileList.begin(), ListTREE.FileList.end(), sorter);//We put all in good or
				std::reverse(ListTREE.FileList.begin(), ListTREE.FileList.end());
				std::sort(ListTREE.FileList.begin(), ListTREE.FileList.end(),
					[](NZArchive::Archive::FileEntryRead t1, NZArchive::Archive::FileEntryRead t2) {return (uint8_t)t1.isDirectory > (uint8_t)t2.isDirectory; });
				ListView_SetItemCount(g_ListView, ListTREE.FileList.size());
				SetWindowText(g_TEXTNAV, ListTREEpth.c_str());
				RedrawWindow(g_ListView, NULL, NULL, RDW_INVALIDATE);
			}
			else
			{	//File => We open
				OpenWith = false;
				FastExtractIndex = ListTREE.FileList[index].IndexInArchive;
				DialogBox(hInst, MAKEINTRESOURCE(IDD_FastOpen1), g_hWnd, FastExtractDialog);
			}
		}
		//Extract to temporary place
	}
	break;
	case NM_RCLICK:
	{
		if (ListView_GetItemCount(g_ListView) == 0)
			break;

		::CoInitialize(nullptr);
		WFHelper = new WindowsExplorerHelper();
		WFHelper->vFExplorer.clear();
		try
		{
			WFHelper->vFExplorer = WFHelper->GetCurrentExplorerFolders();
		}
		catch (const std::exception&)
		{


		}
		::CoUninitialize();

		POINT cursor; // Getting the cursor position
		GetCursorPos(&cursor);
		// Creating the po-up menu list
		HMENU subMenu = GetSubMenu(LoadMenu(hInst, MAKEINTRESOURCE(IDR_MENU1)), 0);
		UI_SetText_ToSubMenuRClick(subMenu, ID_CONTEXT_OPEN, TRANSLATE(L"MENU_CONTEXT_OPEN", L"&Open").c_str());
		UI_SetText_ToSubMenuRClick(subMenu, ID_CONTEXT_OPENWITH, TRANSLATE(L"MENU_CONTEXT_OPENWITH", L"O&pen with ...").c_str());
		UI_SetText_ToSubMenuRClick(subMenu, ID_CONTEXT_ADD, TRANSLATE(L"MENU_CONTEXT_ADD", L"&Add to archive").c_str());
		UI_SetText_ToSubMenuRClick(subMenu, ID_CONTEXT_DELETE, TRANSLATE(L"MENU_CONTEXT_DELETE", L"&Remove selection from archive").c_str());
		UI_SetTextFromPos_ToSubMenuRClick(subMenu, 3, TRANSLATE(L"MENU_CONTEXT_EXPLORER", L"&Extract selection to :").c_str());
		HMENU hFolderSubMenu = ::GetSubMenu(subMenu, /*Fexplorer = 3*/ 3);
		UI_SetTextFromPos_ToSubMenuRClick(hFolderSubMenu, 0, TRANSLATE(L"MENU_CONTEXT_WINDOWS", L"Active Windows Explorer list :").c_str());
		for (size_t i = 0; i < WFHelper->vFExplorer.size(); i++)
		{
			std::vector<wchar_t> __Path;
			__Path.resize(32000);
			wmemcpy_s(&__Path[0], WFHelper->vFExplorer[i].Path.size(), &WFHelper->vFExplorer[i].Path[0], WFHelper->vFExplorer[i].Path.size());
			PathCompactPath(NULL, &__Path[0], 400);
			::AppendMenu(hFolderSubMenu, MF_STRING, WFHelper->vFExplorer[i].PeekID, (LPCWSTR)__Path.data());
		}

		TrackPopupMenu(subMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, cursor.x, cursor.y, 0, g_hWnd, NULL);



	}
	break;
	case LVN_GETDISPINFO:
	{//https://www.codeproject.com/Articles/7891/Using-virtual-lists		style | LVS_OWNERDATA
		LV_DISPINFO* pDispInfo = (LV_DISPINFO*)lParam;
		//Create a pointer to the item
		LV_ITEM* pItem = &(pDispInfo)->item;
		//Which item number?
		int itemid = pItem->iItem;
		//Do the list need text information?
		if (pItem->mask & LVIF_TEXT)
		{
			std::wstring text;
			//Which column?
			if (pItem->iSubItem == 0)
				text = ListTREE.FileList[itemid].FullPath;
			else if (pItem->iSubItem == 1)
				text = thousandSeparator(ListTREE.FileList[itemid].FileSize);
			else if (pItem->iSubItem == 2)
			{
				int64_t CompSize = 0;
				for (size_t j = 0; j < ListTREE.FileList[itemid].FileChunks.size(); j++)
					CompSize += ListTREE.FileList[itemid].FileChunks[j].ChunkCompressed;
				text = thousandSeparator(CompSize);
			}
			else if (pItem->iSubItem == 3)
			{
				SYSTEMTIME stUTC, stLocal;
				// Convert the last-write time to local time.
				FileTimeToSystemTime(&ListTREE.FileList[itemid].FiletimeM, &stUTC);
				SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);
				LCID Lcid = GetThreadLocale();
				wchar_t pszDate[255];
				size_t cchDate = 255;
				GetDateFormat(Lcid, DATE_SHORTDATE, &stUTC, NULL, pszDate, (int)cchDate);
				wchar_t pszTime[255];
				size_t cchTime = 255;
				GetTimeFormat(Lcid, 0, &stUTC, NULL, pszTime, (int)cchTime);
				std::wstring Valeur(std::wstring(pszDate) + L" " + std::wstring(pszTime));
				text = (LPWSTR)Valeur.c_str();
			}
			else if (pItem->iSubItem == 4)
			{
				SYSTEMTIME stUTC, stLocal;
				// Convert the last-write time to local time.
				FileTimeToSystemTime(&ListTREE.FileList[itemid].FiletimeC, &stUTC);
				SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);
				LCID Lcid = GetThreadLocale();
				wchar_t pszDate[255];
				size_t cchDate = 255;
				GetDateFormat(Lcid, DATE_SHORTDATE, &stUTC, NULL, pszDate, (int)cchDate);
				wchar_t pszTime[255];
				size_t cchTime = 255;
				GetTimeFormat(Lcid, 0, &stUTC, NULL, pszTime, (int)cchTime);
				std::wstring Valeur(std::wstring(pszDate) + L" " + std::wstring(pszTime));
				text = (LPWSTR)Valeur.c_str();
			}
			else if (pItem->iSubItem == 5)
			{
				SYSTEMTIME stUTC, stLocal;
				// Convert the last-write time to local time.
				FileTimeToSystemTime(&ListTREE.FileList[itemid].FiletimeA, &stUTC);
				SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);
				LCID Lcid = GetThreadLocale();
				wchar_t pszDate[255];
				size_t cchDate = 255;
				GetDateFormat(Lcid, DATE_SHORTDATE, &stLocal, NULL, pszDate, (int)cchDate);
				wchar_t pszTime[255];
				size_t cchTime = 255;
				GetTimeFormat(Lcid, 0, &stLocal, NULL, pszTime, (int)cchTime);
				std::wstring Valeur(std::wstring(pszDate) + L" " + std::wstring(pszTime));
				text = (LPWSTR)Valeur.c_str();
			}
			else if (pItem->iSubItem == 6)
			{
				std::wstring Valeur(std::to_wstring(ListTREE.FileList[itemid].FileAttribute));
				text = (LPWSTR)Valeur.c_str();
			}
			else if (pItem->iSubItem == 7)
			{
				std::wstring Valeur(thousandSeparator(ListTREE.FileList[itemid].PositionInArchive));
				text = (LPWSTR)Valeur.c_str();
			}
			else if (pItem->iSubItem == 8)
			{
				std::wstring Valeur(ListTREE.FileList[itemid].isDirectory ? L"Yes" : L"No");
				text = (LPWSTR)Valeur.c_str();
			}
			//Copy the text to the LV_ITEM structure
			//Maximum number of characters is in pItem->cchTextMax
			lstrcpyn(pItem->pszText, text.c_str(), pItem->cchTextMax);
		}
		//Do the list need image information?
		if (pItem->mask & LVIF_IMAGE)
		{
			int IndexIcon = 0;
			bool IconFound = false;
			std::wstring extension = pHelper::GetExtension(ListTREE.FileList[itemid].FullPath);
			std::transform(extension.begin(), extension.end(), extension.begin(), towlower);
			if (ListTREE.FileList[itemid].isDirectory)
				pItem->iImage = 0;//Folder
			else
				if (extension == L"")
				{
					pItem->iImage = 1;//UnknownFile
				}
				else
				{//Check association
					for (size_t i = 0; i < vIconStruct.size(); i++)
						if (extension == vIconStruct[i].pExtension)
						{
							IconFound = true;
							IndexIcon = 2 + i;
							break;
						}

					if (!IconFound)
					{
						iconStruct tIconS;
						tIconS.pExtension = extension;

						SHFILEINFO shfi;
						memset(&shfi, 0, sizeof(shfi));
						SHGetFileInfo(std::wstring(L"foo" + tIconS.pExtension).c_str(),
							FILE_ATTRIBUTE_NORMAL,
							&shfi, sizeof(shfi),
							SHGFI_ICON | SHGFI_USEFILEATTRIBUTES);
						tIconS.pIcon = shfi.hIcon;
						tIconS.index = 2 + vIconStruct.size();
						IndexIcon = tIconS.index;
						ImageList_AddIcon(himgList, tIconS.pIcon);
						ListView_SetImageList(g_ListView, himgList, LVSIL_SMALL);
						vIconStruct.push_back(tIconS);
					}

					pItem->iImage = IndexIcon;
				}
		}
	}
	break;
	case LVN_COLUMNCLICK:
	{	// SORTING 
		int Column = pnm->iSubItem;
		if (Column == 0)
		{
			std::sort(ListTREE.FileList.begin(), ListTREE.FileList.end(), sorter);
			if (!ListViewSortOrder[0])
				std::reverse(ListTREE.FileList.begin(), ListTREE.FileList.end());
			ListViewSortOrder[0] = !ListViewSortOrder[0];
		}
		else if (Column == 1)
		{
			std::sort(ListTREE.FileList.begin(), ListTREE.FileList.end(),
				[](NZArchive::Archive::FileEntryRead t1, NZArchive::Archive::FileEntryRead t2) {if (ListViewSortOrder[1]) { return t1.FileSize < t2.FileSize; } else { return t1.FileSize > t2.FileSize; } });
			ListViewSortOrder[1] = !ListViewSortOrder[1];
		}
		else if (Column == 2)
		{
			std::sort(ListTREE.FileList.begin(), ListTREE.FileList.end(),
				[](NZArchive::Archive::FileEntryRead t1, NZArchive::Archive::FileEntryRead t2) {uint64_t SumT1 = 0; for (size_t i = 0; i < t1.FileChunks.size(); i++) { SumT1 += t1.FileChunks[i].ChunkCompressed; }; uint64_t SumT2 = 0; for (size_t i = 0; i < t2.FileChunks.size(); i++) { SumT2 += t2.FileChunks[i].ChunkCompressed; };   if (ListViewSortOrder[2]) { return SumT1 < SumT2; } else { return SumT1 > SumT2; } });
			ListViewSortOrder[2] = !ListViewSortOrder[2];
		}
		else if (Column == 3)
		{
			std::sort(ListTREE.FileList.begin(), ListTREE.FileList.end(),
				[](NZArchive::Archive::FileEntryRead t1, NZArchive::Archive::FileEntryRead t2) {
					LARGE_INTEGER lT1, lT2;
					lT1.LowPart = t1.FiletimeM.dwHighDateTime;
					lT1.HighPart = t1.FiletimeM.dwHighDateTime;
					lT2.LowPart = t2.FiletimeM.dwHighDateTime;
					lT2.HighPart = t2.FiletimeM.dwHighDateTime;
					if (ListViewSortOrder[3])
						return lT1.QuadPart < lT2.QuadPart;
					else
						return lT1.QuadPart > lT2.QuadPart;
				});
			ListViewSortOrder[3] = !ListViewSortOrder[3];
		}
		else if (Column == 4)
		{
			std::sort(ListTREE.FileList.begin(), ListTREE.FileList.end(),
				[](NZArchive::Archive::FileEntryRead t1, NZArchive::Archive::FileEntryRead t2) {
					LARGE_INTEGER lT1, lT2;
					lT1.LowPart = t1.FiletimeC.dwHighDateTime;
					lT1.HighPart = t1.FiletimeC.dwHighDateTime;
					lT2.LowPart = t2.FiletimeC.dwHighDateTime;
					lT2.HighPart = t2.FiletimeC.dwHighDateTime;
					if (ListViewSortOrder[4])
						return lT1.QuadPart < lT2.QuadPart;
					else
						return lT1.QuadPart > lT2.QuadPart;
				});
			ListViewSortOrder[4] = !ListViewSortOrder[4];
		}
		else if (Column == 5)
		{
			std::sort(ListTREE.FileList.begin(), ListTREE.FileList.end(),
				[](NZArchive::Archive::FileEntryRead t1, NZArchive::Archive::FileEntryRead t2) {
					LARGE_INTEGER lT1, lT2;
					lT1.LowPart = t1.FiletimeA.dwHighDateTime;
					lT1.HighPart = t1.FiletimeA.dwHighDateTime;
					lT2.LowPart = t2.FiletimeA.dwHighDateTime;
					lT2.HighPart = t2.FiletimeA.dwHighDateTime;
					if (ListViewSortOrder[5])
						return lT1.QuadPart < lT2.QuadPart;
					else
						return lT1.QuadPart > lT2.QuadPart;
				});
			ListViewSortOrder[5] = !ListViewSortOrder[5];
		}
		else if (Column == 6)
		{
			std::sort(ListTREE.FileList.begin(), ListTREE.FileList.end(),
				[](NZArchive::Archive::FileEntryRead t1, NZArchive::Archive::FileEntryRead t2) {if (ListViewSortOrder[6]) { return t1.FileAttribute < t2.FileAttribute; } else { return t1.FileAttribute > t2.FileAttribute; } });
			ListViewSortOrder[6] = !ListViewSortOrder[6];
		}
		else if (Column == 7)
		{
			std::sort(ListTREE.FileList.begin(), ListTREE.FileList.end(),
				[](NZArchive::Archive::FileEntryRead t1, NZArchive::Archive::FileEntryRead t2) {if (ListViewSortOrder[7]) { return t1.PositionInArchive < t2.PositionInArchive; } else { return t1.PositionInArchive > t2.PositionInArchive; } });
			ListViewSortOrder[7] = !ListViewSortOrder[7];
		}
		else if (Column == 8)
		{
			std::sort(ListTREE.FileList.begin(), ListTREE.FileList.end(),
				[](NZArchive::Archive::FileEntryRead t1, NZArchive::Archive::FileEntryRead t2) {if (ListViewSortOrder[8]) { return (uint8_t)t1.isDirectory < (uint8_t)t2.isDirectory; } else { return (uint8_t)t1.isDirectory > (uint8_t)t2.isDirectory; } });
			ListViewSortOrder[8] = !ListViewSortOrder[8];
		}
		RedrawWindow(g_ListView, NULL, NULL, RDW_INVALIDATE);
	}
	break;
	case NM_CUSTOMDRAW:
	{
		LPNMLVCUSTOMDRAW  lplvcd;
		lplvcd = (LPNMLVCUSTOMDRAW)lParam;
		switch (lplvcd->nmcd.dwDrawStage)
		{
		case CDDS_PREPAINT:
			return CDRF_NOTIFYITEMDRAW;
			break;
		case CDDS_ITEMPREPAINT:
			int row;
			row = (int)lplvcd->nmcd.dwItemSpec;
			int nSize = 255;
			LVITEM lvi = { 0 };
			lvi.iSubItem = 9;
			lvi.pszText = NULL;
			free(lvi.pszText);
			lvi.pszText = (LPTSTR)calloc(nSize, sizeof(TCHAR));
			lvi.cchTextMax = nSize;
			int nRet = (int)::SendMessage(g_ListView, LVM_GETITEMTEXT, (WPARAM)row, (LPARAM)&lvi);
			if (lvi.pszText != 0)
				if (std::wstring(lvi.pszText) == std::wstring(L"Yes"))
					lplvcd->clrText = RGB(136, 66, 66);
				else
					lplvcd->clrText = RGB(20, 20, 20);
			return CDRF_NEWFONT;
			break;
		}
	}
	break;
	case LVN_KEYDOWN:
	{
		NMLVKEYDOWN* nmlvkd = (NMLVKEYDOWN*)lParam;
		switch (nmlvkd->wVKey)
		{
		case VK_BACK:
		{   //Folder => We explore
			if (ListTREEpth == L"" || ListTREEpth == L"\\")
				break;
			ListTREEpth = ListTREEpth.substr(0, ListTREEpth.substr(0, ListTREEpth.length() - 1).find_last_of(L"\\") + 1);
			ListTREE.FileList.clear();
			for (size_t i = 0; i < OpenListing.FileList.size(); i++)
			{
				std::wstring lITEM = OpenListing.FileList[i].FullPath;
				if (lITEM.size() > ListTREEpth.length())//First, is the path long enough to contain folder ? //Superior mean the base path can't be in
					if (lITEM.substr(0, ListTREEpth.length()) == ListTREEpth) //Is the root the same ?
						if (lITEM.substr(ListTREEpth.length() + 1).find_first_of(L"\\") == std::wstring::npos)
						{//No backslash, it's root item
							auto GoodItem = OpenListing.FileList[i];
							GoodItem.FullPath = GoodItem.FullPath.substr(ListTREEpth.length());
							ListTREE.FileList.push_back(GoodItem);
						}
			}
			std::sort(ListTREE.FileList.begin(), ListTREE.FileList.end(), sorter);//We put all in good or
			std::reverse(ListTREE.FileList.begin(), ListTREE.FileList.end());
			std::sort(ListTREE.FileList.begin(), ListTREE.FileList.end(),
				[](NZArchive::Archive::FileEntryRead t1, NZArchive::Archive::FileEntryRead t2) {return (uint8_t)t1.isDirectory > (uint8_t)t2.isDirectory; });
			ListView_SetItemCount(g_ListView, ListTREE.FileList.size());
			SetWindowText(g_TEXTNAV, ListTREEpth.c_str());
			RedrawWindow(g_ListView, NULL, NULL, RDW_INVALIDATE);
		}			break;
		case VK_DELETE:
			PostMessage(g_hWnd, WM_COMMAND, ID_DELETE, 0);
			break;
		case VK_INSERT:
			PostMessage(g_hWnd, WM_COMMAND, ID_ADDTO, 0);
			break;
		case VK_RETURN:
		{
			NMITEMACTIVATE activate = { 0 };
			activate.hdr.hwndFrom = g_ListView;
			activate.hdr.idFrom = idListView;
			activate.hdr.code = NM_DBLCLK;
			activate.iItem = (int)SendMessage(g_ListView, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
			activate.iSubItem = 0;
			activate.ptAction = { 0,0 };
			::SendMessage(g_hWnd, WM_NOTIFY, idListView, (LPARAM)reinterpret_cast<LPNMITEMACTIVATE>(&activate));
		}
		break;
		default:
			break;
		}
	}
	break;
	}
	return FALSE;
}
static LRESULT CALLBACK lvWndProc(const HWND hWnd, const UINT message, const WPARAM wParam, const LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
		SendMessage(GetParent(hWnd), message, wParam, lParam);
		return 0;
	default:
		//Assume that GWLP_USERDATA has been set to the original window procedure.
		return CallWindowProc((WNDPROC)GetWindowLongPtr(hWnd, GWLP_USERDATA), hWnd, message, wParam, lParam);
	}
}