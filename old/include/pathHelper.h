#pragma once
#include <PathCch.h>
#pragma comment(lib, "Pathcch.lib")
#include <vector>
#include <string>
#include <filesystem>
namespace pHelper
{
	const int MaxPath = 32768;
	/// @brief  Simplifies a path by removing navigation elements such as "." and ".." to produce a direct, well-formed path.
	/// @details    This function does not convert forward slashes (/) into back slashes (\).
	/// @param  _Path		Path as std::wstring
	/// @return Well formed path with long path format if necessary
	std::wstring NormalizePath(std::wstring _Path)
	{
		std::vector<wchar_t> Out(MaxPath);
		PathCchCanonicalizeEx(&Out[0], MaxPath, _Path.c_str(), PATHCCH_ALLOW_LONG_PATHS);
		return std::wstring(Out.data());
	}

	/// @brief  Combines two path fragments into a single path. This function also canonicalizes any relative path elements, removing "." and ".." elements to simplify the final path.
	/// @details	If both _Path1 and _Path2 are NULL or empty, a single backslash is returned.
	///				If _Path2 begins with a single backslash, it is combined with only the root of _Path1.
	///				If _Path2 is fully qualified, it is copied directly to the output return without being combined with _Path1.
	/// @param  _Path1		Base Path as std::wstring
	/// @param  _Path2		Path to combine as std::wstring
	/// @return Combined path
	std::wstring CombinePath(std::wstring _Path1, std::wstring _Path2)
	{
		std::vector<wchar_t> Out(MaxPath);
		PathCchCombineEx(&Out[0], MaxPath, _Path1.c_str(), _Path2.c_str(), PATHCCH_ALLOW_LONG_PATHS);
		return std::wstring(Out.data());
	}

	/// @brief  Adds a backslash to the end of a string to create the correct syntax for a path. If the source path already has a trailing backslash, no backslash will be added.
	/// @param  _Path		Patsh as std::wstring
	/// @return Path ending with backslash
	std::wstring AddBackslash(std::wstring _Path)
	{
		if (_Path.size() > MaxPath)
			return L"";
		std::vector<wchar_t> Out(MaxPath);
		wmemcpy_s(&Out[0], _Path.size(), &_Path[0], _Path.size());
		PathCchAddBackslashEx(&Out[0], MaxPath, 0, 0);
		return std::wstring(Out.data());
	}

	/// @brief  Removes the trailing backslash from the end of a path string. If no trailing backslash was found, the string is unchanged.
	/// @param  _Path		Path as std::wstring
	/// @return Path ending with no backslash
	std::wstring RemoveBackslash(std::wstring _Path)
	{
		if (_Path.size() > MaxPath)
			return L"";
		std::vector<wchar_t> Out(MaxPath);
		wmemcpy_s(&Out[0], _Path.size(), &_Path[0], _Path.size());
		PathCchRemoveBackslashEx(&Out[0], MaxPath, 0, 0);
		return std::wstring(Out.data());
	}

	/// @brief  Adds a file name extension to a path string. NOTE: If the original string already has a file name extension present, no new extension will be addedand the original string will be unchanged.
	/// @param  _Path		Path as std::wstring
	/// @param  _Extension	Extension, can be ".ext" or "ext"
	/// @return Path with added extension
	std::wstring AddExtension(std::wstring _Path, std::wstring _Extension)
	{
		if (_Path.size() > MaxPath)
			return L"";
		std::vector<wchar_t> Out(MaxPath);
		wmemcpy_s(&Out[0], _Path.size(), &_Path[0], _Path.size());
		PathCchAddExtension(&Out[0], MaxPath, _Extension.c_str());
		return std::wstring(Out.data());
	}

	/// @brief  Removes the file name extension from a path, if one is present.
	/// @param  _Path		Path as std::wstring
	/// @return Path with any extension removed
	std::wstring RemoveExtension(std::wstring _Path)
	{
		if (_Path.size() > MaxPath)
			return L"";
		std::vector<wchar_t> Out(MaxPath);
		wmemcpy_s(&Out[0], _Path.size(), &_Path[0], _Path.size());
		PathCchRemoveExtension(&Out[0], MaxPath);
		return std::wstring(Out.data());
	}

	/// @brief  Appends one path to the end of another.
	/// @details	UNC paths and paths that begin with the sequence \?\ are accepted and recognized as fully-qualified paths.
	///				These paths replace _Path1 instead of being appended to it.
	/// @param  _Path1		Base Path as std::wstring
	/// @param  _Path2		Path to append as std::wstring
	/// @return original path plus the appended path
	std::wstring Append(std::wstring _Path1, std::wstring _Path2)
	{
		if (_Path1.size() > MaxPath || _Path2.size() > MaxPath)
			return L"";
		std::vector<wchar_t> Out(MaxPath);
		wmemcpy_s(&Out[0], _Path1.size(), &_Path1[0], _Path1.size());
		PathCchAppendEx(&Out[0], MaxPath, _Path2.c_str(), PATHCCH_ALLOW_LONG_PATHS);
		return std::wstring(Out.data());
	}

	/// @brief  Appends one path to the end of another.	
	/// @param  _Path		Base Path as std::wstring
	/// @return Extension as std::wstring, if no extension is found, return L""
	std::wstring GetExtension(std::wstring _Path)
	{
		if (_Path.size() > MaxPath)
			return L"";
		std::vector<wchar_t> In(MaxPath);
		//std::vector<wchar_t*> Out(1);
		wchar_t* Out;
		wmemcpy_s(&In[0], _Path.size(), &_Path[0], _Path.size());
		PathCchFindExtension((PWSTR)&In[0], MaxPath, &Out);
		std::wstring wsResult((PWSTR)Out);
		return wsResult;
	}

	/// @brief  Replaces a file name's extension at the end of a path string with a new extension. If the path string does not end with an extension, the new extension is added.
	/// @param  _Path			Base Path as std::wstring
	/// @param  _Extension		The leading '.' character is optional. In the case of an empty string (""), any existing extension in the path string is removed.
	/// @return _Path with the renamed or added extension
	std::wstring RenameExtension(std::wstring _Path, std::wstring _Extension)
	{
		if (_Path.size() > MaxPath)
			return L"";
		std::vector<wchar_t> Out(MaxPath);
		wmemcpy_s(&Out[0], _Path.size(), &_Path[0], _Path.size());
		PathCchRenameExtension(&Out[0], MaxPath, _Extension.c_str());
		return std::wstring(Out.data());
	}

	/// @brief  Remove root from a path
	/// @param  _Path			Base Path as std::wstring
	/// @return _Path with the root removed
	std::wstring RemoveRoot(std::wstring _Path)
	{
		if (_Path.size() > MaxPath)
			return L"";
		wchar_t* Out;
		PathCchSkipRoot((PWSTR)_Path.c_str(), &Out);
		std::wstring wsResult((PWSTR)Out);
		return wsResult;
	}

	/// @brief  Remove root from a path
	/// @param  _Path			Base Path as std::wstring
	/// @return _Path with the root removed
	std::wstring GetRoot(std::wstring _Path)
	{
		if (_Path.size() > MaxPath)
			return L"";
		std::vector<wchar_t> Out(MaxPath);
		wmemcpy_s(&Out[0], _Path.size(), &_Path[0], _Path.size());
		PathCchStripToRoot((PWSTR)&Out[0], MaxPath);
		std::wstring wsResult((PWSTR)Out.data());
		return wsResult;
	}

	/// @brief  Determines if a path string is a valid Universal Naming Convention (UNC) path, as opposed to a path based on a drive letter. 
	/// @param  _Path			Base Path as std::wstring
	/// @return TRUE if the string is a valid UNC path; otherwise, FALSE.
	bool isPathNetwork(std::wstring _Path)
	{
		return PathIsUNCEx((PWSTR)_Path.c_str(), 0);
	}

	/// @brief  Get the filename from the path. 
	/// @param  _Path			Base Path as std::wstring
	/// @return Filename as std::wstring.
	std::wstring getFileFromPath(std::wstring _Path)
	{
		return std::filesystem::path(_Path).filename().wstring();
	}
	/// @brief  Get the directory from the path. 
	/// @param  _Path			Base Path as std::wstring
	/// @param  KeepBackslash			TRUE keep the '\' at the end, FALSE remove the '\'
	/// @return Directory without Filename as std::wstring.
	std::wstring getDirectoryFromPath(std::wstring _Path, bool KeepBackslash = false)
	{
		return std::filesystem::path(_Path).parent_path().wstring();
	}


}