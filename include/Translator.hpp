#pragma once
#include <string>
#include <vector>
#include <codecvt>
static std::wstring IniFILE;	//Wstring containing the locale file selected for translation
/*	 _____                     _       _   _               _          _
	/__   \_ __ __ _ _ __  ___| | __ _| |_(_) ___  _ __   | |__   ___| |_ __   ___ _ __
	  / /\/ '__/ _` | '_ \/ __| |/ _` | __| |/ _ \| '_ \  | '_ \ / _ \ | '_ \ / _ \ '__|
	 / /  | | | (_| | | | \__ \ | (_| | |_| | (_) | | | | | | | |  __/ | |_) |  __/ |
	 \/   |_|  \__,_|_| |_|___/_|\__,_|\__|_|\___/|_| |_| |_| |_|\___|_| .__/ \___|_|
																	   |_|
*/


/// @brief	Convert UTF8 string to wstring.
/// @param	_str	string to convert
/// @return	wstring containg the utf8 string
inline std::wstring utf8string_toWstring(std::string _str)
{
	int wchars_num = MultiByteToWideChar(CP_UTF8, 0, _str.c_str(), -1, NULL, 0);
	wchar_t* wstr = new wchar_t[wchars_num];
	MultiByteToWideChar(CP_UTF8, 0, _str.c_str(), -1, wstr, wchars_num);
	std::wstring Result(wstr);
	delete[] wstr;
	return Result;
}

/// @brief      Translate Entry using the loaded translation file.
/// @param  _Entry		Variable to search in the localisation table, if empty, simply format _Default with ... arguments
/// @param  _Default		Preformatted string with argument eg. "Listing files %d on %d within the %s folder"
/// @param  ...		Variadic argument (has many as needed)
/// @return Translated and formatted string
std::wstring TRANSLATE(std::wstring _Entry, std::wstring _Default, ...)
{
	//const wchar_t* format = L"";
	//format = _Default.c_str();
	wchar_t  lpReturnedString[1024];
	ZeroMemory(lpReturnedString, 1024);
	GetPrivateProfileString(
		L"Translation",
		_Entry.c_str(),
		_Default.c_str(),
		lpReturnedString,
		sizeof(lpReturnedString),
		IniFILE.c_str());
	std::vector<wchar_t> buffer;
	buffer.resize(1024);
	va_list args;
	va_start(args, _Default);/*_Default is the last arg before the variadic argument ...*/
	auto writenSize = vswprintf_s(&buffer[0], 1024, lpReturnedString, args);
	va_end(args);
	buffer.resize(writenSize);
	std::wstring Result = std::wstring(buffer.begin(), buffer.end());
	return Result;
}


/// @brief      Translate Entry using the loaded translation file.
/// @param  _Entry		Variable to search in the localisation table, if empty, simply return  _Default
/// @param  _Default		Preformatted string with argument eg. "Listing files %d on %d within the %s folder"
/// @return Translated and formatted string
const wchar_t* TRANSLATEptr(std::wstring _Entry, const wchar_t* _Default, ...)
{
	wchar_t* lpReturnedString = new wchar_t[1024];
	ZeroMemory(lpReturnedString, 1024*sizeof(wchar_t));
	GetPrivateProfileString(
		L"Translation",
		_Entry.c_str(),
		_Default,
		lpReturnedString,
		1024,
		IniFILE.c_str());
	wchar_t* buffer = new wchar_t[2048];
	ZeroMemory(buffer, 2048 * sizeof(wchar_t));
	va_list args;
	va_start(args, _Default);/*_Default is the last arg before the variadic argument ...*/
	auto writenSize = vswprintf_s(buffer, 2048, lpReturnedString, args);
	va_end(args);
	return buffer;
}


/// @brief      Return all child window from a main window, extract child text and translate according to the text.
/// @param  hDlg		Window to list all child window
void TranslateAllInThisChild(HWND hDlg)
{
	::EnumChildWindows(hDlg, [](HWND __hwnd, LPARAM __lparam) -> BOOL {

		auto cTxtLen = GetWindowTextLength(__hwnd); auto pszMem = (PWSTR)VirtualAlloc((LPVOID)NULL, (DWORD)(cTxtLen + 1), MEM_COMMIT, PAGE_READWRITE);
		GetWindowText(__hwnd, pszMem, cTxtLen + 1);
		std::wstring wPSZMEM = std::wstring(pszMem);
		if (wPSZMEM == L"LBLCREATE_DEST")
			SetWindowText(__hwnd, TRANSLATE(L"LBLCREATE_DEST", L"NZArchive location for creation :").c_str());
		if (wPSZMEM == L"LBLCREATE_LEVELDESC")
			SetWindowText(__hwnd, TRANSLATE(L"LBLCREATE_LEVELDESC", L"Compression settings :").c_str());
		if (wPSZMEM == L"LBLCREATE_PWDDESC")
			SetWindowText(__hwnd, TRANSLATE(L"LBLCREATE_PWDDESC", L"NZArchive encryption password (Empty mean defaut encryption readable by anyone) :").c_str());
		if (wPSZMEM == L"LBLCREATE_LBL_THREAD1")
			SetWindowText(__hwnd, TRANSLATE(L"LBLCREATE_LBL_THREAD1", L"Number of CPU core:").c_str());
		if (wPSZMEM == L"LBLCREATE_LEVELIS")
			SetWindowText(__hwnd, TRANSLATE(L"LBLCREATE_LEVELIS", L"Current level : ").c_str());
		if (wPSZMEM == L"LBLCREATE_PWDMAX")
			SetWindowText(__hwnd, TRANSLATE(L"LBLCREATE_PWDMAX", L"Max length : 32 characters").c_str());
		if (wPSZMEM == L"LBLCREATE_ENCRYPTION")
			SetWindowText(__hwnd, TRANSLATE(L"LBLCREATE_ENCRYPTION", L"Method of encryption :").c_str());
		if (wPSZMEM == L"LBLCREATE_ENCRYPTIONCHACHA")
			SetWindowText(__hwnd, TRANSLATE(L"LBLCREATE_ENCRYPTIONCHACHA", L"ChaCha20").c_str());
		if (wPSZMEM == L"LBLCREATE_ENCRYPTIONAES")
			SetWindowText(__hwnd, TRANSLATE(L"LBLCREATE_ENCRYPTIONAES", L"AES256").c_str());
		if (wPSZMEM == L"LBLCREATE_ENCRYPTIONNONE")
			SetWindowText(__hwnd, TRANSLATE(L"LBLCREATE_ENCRYPTIONNONE", L"None").c_str());
		if (wPSZMEM == L"LBLCREATE_SOURCE")
			SetWindowText(__hwnd, TRANSLATE(L"LBLCREATE_SOURCE", L"Source folder to add to NZArchive :").c_str());
		if (wPSZMEM == L"LBLCREATE_COMPPROG")
			SetWindowText(__hwnd, TRANSLATE(L"LBLCREATE_COMPPROG", L"File being compressed :").c_str());
		if (wPSZMEM == L"LBLCREATE_PROG")
			SetWindowText(__hwnd, TRANSLATE(L"LBLCREATE_PROG", L"Progression :").c_str());
		if (wPSZMEM == L"LBLCREATE_BROWSE")
			SetWindowText(__hwnd, TRANSLATE(L"LBLCREATE_BROWSE", L"Browse").c_str());
		if (wPSZMEM == L"LBLCREATE_BTNSTART")
			SetWindowText(__hwnd, TRANSLATE(L"LBLCREATE_BTNSTART", L"Start compression").c_str());
		if (wPSZMEM == L"LBLCREATE_BTNCANCEL")
			SetWindowText(__hwnd, TRANSLATE(L"LBLCREATE_BTNCANCEL", L"Cancel").c_str());
		if (wPSZMEM == L"LBLCREATE_BTNCLOSE")
			SetWindowText(__hwnd, TRANSLATE(L"LBLCREATE_BTNCLOSE", L"Close").c_str());
		if (wPSZMEM == L"LBLCREATE_LBL_REDUNDANCY1")
			SetWindowText(__hwnd, TRANSLATE(L"LBLCREATE_LBL_REDUNDANCY1", L"Redundancy data (0%% mean no redundancy files)").c_str());
		if (wPSZMEM == L"LBLCREATE_PROG")
			SetWindowText(__hwnd, TRANSLATE(L"LBLCREATE_PROG", L"Progression status :").c_str());
		if (wPSZMEM == L"LBL_TEST_FILE")
			SetWindowText(__hwnd, TRANSLATE(L"LBL_TEST_FILE", L"File :").c_str());
		if (wPSZMEM == L"LBL_TEST_POSITION")
			SetWindowText(__hwnd, TRANSLATE(L"LBL_TEST_POSITION", L"Position in archive :").c_str());
		if (wPSZMEM == L"LBL_TEST_BTNCLOSE")
			SetWindowText(__hwnd, TRANSLATE(L"LBL_TEST_BTNCLOSE", L"Close").c_str());
		if (wPSZMEM == L"LBL_TEST_BTNCANCEL")
			SetWindowText(__hwnd, TRANSLATE(L"LBL_TEST_BTNCANCEL", L"Cancel").c_str());
		if (wPSZMEM == L"LBLCREATE_BROWSE")
			SetWindowText(__hwnd, TRANSLATE(L"LBLCREATE_BROWSE", L"Browse").c_str());
		if (wPSZMEM == L"LBLCREATE_BTNCLOSE")
			SetWindowText(__hwnd, TRANSLATE(L"LBLCREATE_BTNCLOSE", L"Close").c_str());
		if (wPSZMEM == L"LBLCREATE_BTNCANCEL")
			SetWindowText(__hwnd, TRANSLATE(L"LBLCREATE_BTNCANCEL", L"Cancel").c_str());
		if (wPSZMEM == L"LBL_ADD_BTN_START")
			SetWindowText(__hwnd, TRANSLATE(L"LBL_ADD_BTN_START", L"Add selection to archive").c_str());
		if (wPSZMEM == L"LBL_ADD_RADIO_FILE")
			SetWindowText(__hwnd, TRANSLATE(L"LBL_ADD_RADIO_FILE", L"File").c_str());
		if (wPSZMEM == L"LBL_ADD_RADIO_FOLDER")
			SetWindowText(__hwnd, TRANSLATE(L"LBL_ADD_RADIO_FOLDER", L"Folder").c_str());
		if (wPSZMEM == L"LBL_ADD_STATIC_ADD")
			SetWindowText(__hwnd, TRANSLATE(L"LBL_ADD_STATIC_ADD", L"Adding :").c_str());
		if (wPSZMEM == L"LBL_ADD_STATIC_INSERTAT")
			SetWindowText(__hwnd, TRANSLATE(L"LBL_ADD_STATIC_INSERTAT", L"Entry point in archive :").c_str());
		if (wPSZMEM == L"LBLCREATE_LEVELDESC")
			SetWindowText(__hwnd, TRANSLATE(L"LBLCREATE_LEVELDESC", L"Compression level :").c_str());
		if (wPSZMEM == L"LBLCREATE_PROG")
			SetWindowText(__hwnd, TRANSLATE(L"LBLCREATE_PROG", L"Progression status :").c_str());
		if (wPSZMEM == L"LBLCREATE_LBL_COMP2")
			SetWindowText(__hwnd, TRANSLATE(L"LBLCREATE_LBL_COMP2", L"Compression level :").c_str());
		if (wPSZMEM == L"LBLCREATE_LBL_THREAD1")
			SetWindowText(__hwnd, TRANSLATE(L"LBLCREATE_LBL_THREAD1", L"Number of CPU core:").c_str());
		if (wPSZMEM == L"LBL_EXTR_FILE")
			SetWindowText(__hwnd, TRANSLATE(L"LBL_EXTR_FILE", L"File :").c_str());
		if (wPSZMEM == L"LBL_EXTR_TOTAL")
			SetWindowText(__hwnd, TRANSLATE(L"LBL_EXTR_TOTAL", L"Total :").c_str());
		if (wPSZMEM == L"LBL_EXTR_FILEWRITTEN")
			SetWindowText(__hwnd, TRANSLATE(L"LBL_EXTR_FILEWRITTEN", L"File data written :").c_str());
		if (wPSZMEM == L"LBL_EXTR_TOTALWRITTEN")
			SetWindowText(__hwnd, TRANSLATE(L"LBL_EXTR_TOTALWRITTEN", L"Total data written :").c_str());
		if (wPSZMEM == L"LBL_EXTR_BTNCLOSE")
			SetWindowText(__hwnd, TRANSLATE(L"LBL_EXTR_BTNCLOSE", L"Close").c_str());
		if (wPSZMEM == L"LBL_EXTR_BTNCANCEL")
			SetWindowText(__hwnd, TRANSLATE(L"LBL_EXTR_BTNCANCEL", L"Cancel").c_str());
		if (std::wstring(pszMem) == L"LBL_AI_BTCCLOSE")
			SetWindowText(__hwnd, TRANSLATE(L"LBL_AI_BTCCLOSE", L"Close").c_str());
		if (wPSZMEM == L"LBL_PWD_PWD")
			SetWindowText(__hwnd, TRANSLATE(L"LBL_PWD_PWD", L"Password (max 32 characters) :").c_str());
		if (wPSZMEM == L"LBL_PWD_BTN_CONTINUE")
			SetWindowText(__hwnd, TRANSLATE(L"LBL_PWD_BTN_CONTINUE", L"Continue").c_str());
		if (wPSZMEM == L"LBL_PWD_BTN_CANCEL")
			SetWindowText(__hwnd, TRANSLATE(L"LBL_PWD_BTN_CANCEL", L"Cancel").c_str());
		if (wPSZMEM == L"LBL_EXTR_VALIDCURDIR")
			SetWindowText(__hwnd, TRANSLATE(L"LBL_EXTR_VALIDCURDIR", L"Extract archive in the current directory :").c_str());
		if (wPSZMEM == L"LBL_EXTR_CURDIR")
			SetWindowText(__hwnd, TRANSLATE(L"LBL_EXTR_CURDIR", L"Current directory").c_str());
		if (wPSZMEM == L"LBL_EXTR_PICKDIR")
			SetWindowText(__hwnd, TRANSLATE(L"LBL_EXTR_PICKDIR", L"Change destination").c_str());
		if (wPSZMEM == L"LBL_EXTR_BTNCANCEL")
			SetWindowText(__hwnd, TRANSLATE(L"LBL_EXTR_BTNCANCEL", L"Cancel").c_str());
		return TRUE;
		}, NULL);
}