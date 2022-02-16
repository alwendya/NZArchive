#pragma once
#include <Windows.h>
#include <shlobj.h>
#include <atlcomcli.h>  // for COM smart pointers
#include <vector>
#include <system_error>
#include <memory>


/// <summary>
/// A clacc able to retrieve all windows explorer opened as a vector list.
/// The vector list contain the path, the window handle and an unique ID (eg. useable in a menu allocation message retriever)
/// </summary>
class WindowsExplorerHelper
{
public:

    /// <summary>
    /// A Deleter for a PIDL allocated by the shell.
    /// </summary>
    struct CoTaskMemDeleter
    {
        void operator()(ITEMIDLIST* pidl) const { ::CoTaskMemFree(pidl); }
    };
    /// <summary>
    /// A smart pointer for PIDLs.
    /// </summary>
    using UniquePidlPtr = std::unique_ptr< ITEMIDLIST, CoTaskMemDeleter >;
    /// <summary>
    /// Return value of GetCurrentExplorerFolders()
    /// </summary>
    struct ExplorerFolderInfo
    {
        HWND hwnd = nullptr;  // window handle of explorer
        UniquePidlPtr pidl;   // PIDL that points to current folder
        std::wstring Path = L"";
        int PeekID = 0;
    };
    /// <summary>
    /// Vector's Containing Explorer's information
    /// </summary>
    std::vector<ExplorerFolderInfo> vFExplorer;
    /// <summary>
    /// Boolean is True when ExtractSel mean Extract to explorer given path
    /// </summary>
    bool ExtractToExplorer = false;
    /// <summary>
    /// Containing Explorer's windows ID
    /// </summary>
    int ExplorerPathID = 0;


    // Get information about all currently open explorer windows.
    // Throws std::system_error exception to report errors.
    std::vector<ExplorerFolderInfo> GetCurrentExplorerFolders();
    WindowsExplorerHelper();
	~WindowsExplorerHelper();

private:

};
