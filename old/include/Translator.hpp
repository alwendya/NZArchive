#pragma once
#include <string>
#include <vector>
#include <codecvt>

inline std::wstring utf8string_toWstring(std::string _str)
{
	int wchars_num = MultiByteToWideChar(CP_UTF8, 0, _str.c_str(), -1, NULL, 0);
	wchar_t* wstr = new wchar_t[wchars_num];
	MultiByteToWideChar(CP_UTF8, 0, _str.c_str(), -1, wstr, wchars_num);
	// do whatever with wstr
	std::wstring Result(wstr);
	delete[] wstr;
	return Result;
}

struct locItem
{
	wchar_t * pValue = 0;
	std::wstring variable = L"";
	std::wstring value = L"";
};
std::vector<locItem> vLocalizationList;
void unloadTrad()
{
	for (size_t i = 0; i < vLocalizationList.size(); i++)
	{
		free(vLocalizationList[i].pValue);
	}
}

bool LoadLocalisationTRD(std::wstring _file)
{
	vLocalizationList.clear();
	std::ifstream file(_file);
	if (file.fail())
		return false;
	std::string line;
	while (std::getline(file, line))
	{
		size_t EqualPos = line.find_first_of("=");
		locItem _locItem;
		_locItem.variable = utf8string_toWstring(line.substr(0, EqualPos));
		std::transform(_locItem.variable.begin(), _locItem.variable.end(), _locItem.variable.begin(), towupper);
		_locItem.value = utf8string_toWstring(line.substr(EqualPos + 1));
		int pLength = _locItem.value.size() + 1;
		_locItem.pValue = (wchar_t*)malloc(pLength * sizeof(wchar_t));
		memset(_locItem.pValue, 0,pLength);
		wmemcpy_s(_locItem.pValue, pLength,&_locItem.value[0], pLength);
		vLocalizationList.push_back(_locItem);
	}
	return true;
}
/// @brief      Translate Entry using the loaded translation file.
/// @param  _Entry		Variable to search in the localisation table, if empty, simply format _Default with ... arguments
/// @param  _Default		Preformatted string with argument eg. "Listing files %d on %d within the %s folder"
/// @param  ...		Variadic argument (has many as needed)
/// @return Translated and formatted string
std::wstring TRANSLATE(std::wstring _Entry, std::wstring _Default, ...)
{
	const wchar_t* format = L"";
	format = _Default.c_str();
	if (_Entry != L"")//Entry is empty, don't mind loose time, lets format with default
		if (vLocalizationList.size() != 0)
			for (size_t i = 0; i < vLocalizationList.size(); i++)
				if (_Entry == vLocalizationList[i].variable)
				{
					format = vLocalizationList[i].value.c_str();
					break;
				}
	std::vector<wchar_t> buffer;
	buffer.resize(1024);
	va_list args;
	va_start(args, _Default);/*_Default is the last arg before the variadic argument ...*/
	auto writenSize = vswprintf_s(&buffer[0], 1024, format, args);
	va_end(args);
	buffer.resize(writenSize);
	std::wstring Result = std::wstring(buffer.begin(), buffer.end());
	return Result;
}

/// @brief      Translate Entry using the loaded translation file.
/// @param  _Entry		Variable to search in the localisation table, if empty, simply return  _Default
/// @param  _Default		Preformatted string with argument eg. "Listing files %d on %d within the %s folder"
/// @return Translated and formatted string
const wchar_t * TRANSLATEptr(std::wstring _Entry, std::wstring _Default)
{
	if (_Entry != L"")//Entry is empty, don't mind loose time, lets format with default
		if (vLocalizationList.size() != 0)
			for (size_t i = 0; i < vLocalizationList.size(); i++)
				if (_Entry == vLocalizationList[i].variable)
				{
					return vLocalizationList[i].pValue;
				}
	wchar_t* Out = 0;
	return _Default.c_str();
}

