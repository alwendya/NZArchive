#ifndef CONTEXT_MENU_EXT_H
#define CONTEXT_MENU_EXT_H

#include <WinSock2.h>
#include <windows.h>
#include <map>
#include <vector>
#include <memory>
#include <shlobj.h>
#include <string>

namespace nsADDTO
{
    class MenuExt: public IShellExtInit, public IContextMenu
    {
    public:
        #define NZA_Index 0x1981
        // IUnknown
        IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv);
        IFACEMETHODIMP_(ULONG) AddRef();
        IFACEMETHODIMP_(ULONG) Release();

        // IShellExtInit
        IFACEMETHODIMP Initialize(LPCITEMIDLIST pidlFolder, LPDATAOBJECT pDataObj, HKEY hKeyProgID);

        // IContextMenu
        IFACEMETHODIMP QueryContextMenu(HMENU hMenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
        IFACEMETHODIMP InvokeCommand(LPCMINVOKECOMMANDINFO pici);
        IFACEMETHODIMP GetCommandString(UINT_PTR idCommand, UINT uFlags, UINT *pwReserved, LPSTR pszName, UINT cchMax);

        static HRESULT ComponentCreator(const IID& iid, void** ppv);

        MenuExt(void);
        ~MenuExt(void);

    private:
        void PerformActionAdd();
        void PerformActionExtTo();
        void PerformActionExtHere();

    private:
        long m_cRef;
        std::wstring m_menuTextAdd;
        std::string m_verbAdd;
        std::wstring m_wVerbAdd;
        std::string m_verbHelpTextAdd;
        std::wstring m_wVerbHelpTextAdd;
        std::string m_verbCanonicalNameAdd;
        std::wstring m_wVerbCanonicalNameAdd;
        std::wstring m_menuTextExtTo;
        std::string m_verbExtTo;
        std::wstring m_wVerbExtTo;
        std::string m_verbHelpTextExtTo;
        std::wstring m_wVerbHelpTextExtTo;
        std::string m_verbCanonicalNameExtTo;
        std::wstring m_wVerbCanonicalNameExtTo;
        std::wstring m_menuTextExtHere;
        std::string m_verbExtHere;
        std::wstring m_wVerbExtHere;
        std::string m_verbHelpTextExtHere;
        std::wstring m_wVerbHelpTextExtHere;
        std::string m_verbCanonicalNameExtHere;
        std::wstring m_wVerbCanonicalNameExtHere;
        std::vector<std::wstring> m_wFileList;
        HANDLE m_hMenuBmp;
    };

}

#endif CONTEXT_MENU_EXT_H