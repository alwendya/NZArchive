#include "MenuExt.h"
#include <stdio.h>
#include <strsafe.h>
#include <Shlwapi.h>
#include <string>
#include <assert.h>
#include <memory>
#include <algorithm>
#include <Shtypes.h>
#include <ShlObj.h>

#include "Global.h"
#include "resource.h"

const UINT kMaxQueryFileIndex = 0xFFFFFFFF;
extern std::unique_ptr<Global> g_Dll;


namespace nsADDTO
{
#define IDM_DISPLAY             0 

	MenuExt::MenuExt(void) :
		m_cRef(1),
		m_menuText(L"NZArchive - Add to new archive"),
		m_verb("nzaaddto"),
		m_wVerb(L"nzaaddto"),
		m_verbHelpText("NZArchive - Add to new archive"),
		m_wVerbHelpText(L"NZArchive - Add to new archive"),
		m_verbCanonicalName("NZArcAddTo"),
		m_wVerbCanonicalName(L"NZArcAddTo")
	{

		//https://www.robvanderwoude.com/languagecodes.php

		//LANGUAGE ID'S
		LANGID CurLiD = GetUserDefaultUILanguage();
		switch (CurLiD)
		{
		case 1036://French
		case 2060:
		case 3084:
		case 5132:
		case 4108:
			m_wVerbHelpText = L"NZArchive - Ajouter à une nouvelle archive";
			break;
		case 3076://Chinese
		case 2052:
		case 4100:
		case 1028:
			m_wVerbHelpText = L"NZArchive - 添加到新存档";
			break;
		case 3081://English
		case 10249:
		case 4105:
		case 6153:
		case 8201:
		case 5129:
		case 7177:
		case 11273:
		case 2057:
		case 1033:
			m_wVerbHelpText = L"NZArchive - Add to new archive";
			break;
		case 1084://German
		case 1031:
		case 3079:
		case 5127:
		case 4103:
		case 2055:
			m_wVerbHelpText = L"NZArchive - Zu neuem Archiv hinzufügen";
			break;
		case 1041://Japanese
			m_wVerbHelpText = L"NZArchive - 新しいアーカイブに追加";
			break;
		case 1042://Korean
			m_wVerbHelpText = L"NZArchive - 새 아카이브에 추가";
			break;
		case 1040://Italian
		case 2064:
			m_wVerbHelpText = L"NZArchive - Aggiungi al nuovo archivio";
			break;
		case 1046://Portuguese
		case 2070:
			m_wVerbHelpText = L"NZArchive - Adicionar ao novo arquivo";
			break;
		case 1049://Russian
		case 2073:
			m_wVerbHelpText = L"NZArchive - Добавить в новый архив";
			break;
		case 11274://Spanish
		case 16394:
		case 13322:
		case 9226:
		case 5130:
		case 7178:
		case 12298:
		case 17418:
		case 4106:
		case 18442:
		case 2058:
		case 19466:
		case 6154:
		case 15370:
		case 10250:
		case 20490:
		case 1034:
		case 14346:
		case 8202:
			m_wVerbHelpText = L"NZArchive - Agregar a un nuevo archivo";
			break;
		case 1066://Vietnamese
			m_wVerbHelpText = L"NZArchive - Thêm vào kho lưu trữ mới";
			break;
		default:
			m_wVerbHelpText = L"NZArchive - Add to new archive";
			break;
		}
		m_menuText = m_wVerbHelpText;
		g_Dll->IncrementDllRef();
		m_hMenuBmp = LoadImage(g_Dll->DllInst(), MAKEINTRESOURCE(IDB_OK),
			IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADTRANSPARENT);
	}

	MenuExt::~MenuExt(void)
	{
		g_Dll->DecrementDllRef();
	}

	HRESULT MenuExt::ComponentCreator(const IID& iid, void** ppv)
	{
		HRESULT hr = E_OUTOFMEMORY;

		MenuExt* pExt = new (std::nothrow) MenuExt();
		if (pExt)
		{
			hr = pExt->QueryInterface(iid, ppv);
			pExt->Release();
		}
		return hr;
	}

	void MenuExt::PerformAction()
	{
		std::wstring CMDLine = L":NEW ";
		for (size_t i = 0; i < m_wFileList.size(); i++)
			CMDLine += L"\"" + m_wFileList[i] + L"\" ";

		HKEY rCmdKey;
		if (RegOpenKey(HKEY_CURRENT_USER, L"Software\\Classes\\NZArchive\\", &rCmdKey) == ERROR_SUCCESS)
		{
			DWORD BufferSize = MAX_PATH;
			TCHAR wKeyValue[MAX_PATH];
			memset(wKeyValue, 0, MAX_PATH);
			auto Res = RegGetValue(rCmdKey, L"DefaultIcon", NULL, RRF_RT_ANY, NULL, &wKeyValue, &BufferSize);
			RegCloseKey(rCmdKey);
			SHELLEXECUTEINFO ExecuteInfo;
			memset(&ExecuteInfo, 0, sizeof(ExecuteInfo));
			ExecuteInfo.cbSize = sizeof(ExecuteInfo);
			ExecuteInfo.fMask = 0;
			ExecuteInfo.hwnd = NULL;
			ExecuteInfo.lpVerb = L"open";
			ExecuteInfo.lpFile = wKeyValue;
			ExecuteInfo.lpParameters = CMDLine.c_str();
			ExecuteInfo.lpDirectory = L"";
			ExecuteInfo.nShow = SW_SHOW;
			ExecuteInfo.hInstApp = NULL;
			ShellExecuteEx(&ExecuteInfo);
		}
	}

#pragma region IUnknown

	IFACEMETHODIMP MenuExt::QueryInterface(REFIID riid, void** ppv)
	{
		static const QITAB qit[] =
		{
			QITABENT(MenuExt, IContextMenu),
			QITABENT(MenuExt, IShellExtInit),
			{ 0 },
		};
		return QISearch(this, qit, riid, ppv);
	}

	IFACEMETHODIMP_(ULONG) MenuExt::AddRef()
	{
		return InterlockedIncrement(&m_cRef);
	}

	IFACEMETHODIMP_(ULONG) MenuExt::Release()
	{
		ULONG cRef = InterlockedDecrement(&m_cRef);
		if (cRef == 0)
		{
			delete this;
		}

		return cRef;
	}

#pragma endregion


#pragma region IShellExtInit
	int m_shellIdList = RegisterClipboardFormat(CFSTR_SHELLIDLIST);

	IFACEMETHODIMP MenuExt::Initialize(
		LPCITEMIDLIST pidlFolder, LPDATAOBJECT pDataObj, HKEY hKeyProgID)
	{
		m_wFileList.clear();
		if (pDataObj == NULL)
		{
			return E_INVALIDARG;
		}

		HRESULT hr = E_FAIL;

		FORMATETC fe = { (CLIPFORMAT)g_Dll->ShellIdList(), NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
		STGMEDIUM stm;

		if (SUCCEEDED(pDataObj->GetData(&fe, &stm)))
		{
			FORMATETC etc = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
			STGMEDIUM stg = { TYMED_HGLOBAL };

			if (SUCCEEDED(pDataObj->GetData(&etc, &stg)))
			{
				HDROP hDrop = static_cast<HDROP>(GlobalLock(stg.hGlobal));
				if (hDrop != NULL)
				{
					UINT nFiles = DragQueryFileW(hDrop, kMaxQueryFileIndex, NULL, 0);
					// some logic to process selected items
					for (size_t i = 0; i < nFiles; i++)
					{
						TCHAR FullFileName[32567];
						auto filesize = DragQueryFileW(hDrop, i, NULL, 0);
						DragQueryFileW(hDrop, i, FullFileName, sizeof(FullFileName));
						std::wstring conv(FullFileName);
						conv.resize(filesize);
						m_wFileList.push_back(conv);
					}
					hr = S_OK;

					GlobalUnlock(stg.hGlobal);

					ReleaseStgMedium(&stg);
				}
			}

			ReleaseStgMedium(&stm);
		}

		return hr;
	}

#pragma endregion

#pragma region IContextMenu

	IFACEMETHODIMP MenuExt::QueryContextMenu(
		HMENU hMenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
	{
		if (CMF_DEFAULTONLY & uFlags || CMF_NOVERBS & uFlags || CMF_VERBSONLY & uFlags)
		{
			return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(0));
		}

		MENUITEMINFO item = { sizeof(MENUITEMINFO) };

		item.fMask = MIIM_BITMAP | MIIM_STRING | MIIM_FTYPE | MIIM_ID | MIIM_STATE;
		item.wID = idCmdFirst + IDM_DISPLAY;
		item.fType = MFT_STRING;
		item.fState = MFS_ENABLED;
		item.dwTypeData = &m_menuText[0];
		item.hbmpItem = static_cast<HBITMAP>(m_hMenuBmp);

		if (!InsertMenuItem(hMenu, indexMenu, TRUE, &item))
		{
			return HRESULT_FROM_WIN32(GetLastError());
		}

		MENUITEMINFO sep = { sizeof(sep) };
		sep.fMask = MIIM_TYPE;
		sep.fType = MFT_SEPARATOR;
		if (!InsertMenuItem(hMenu, indexMenu + 1, TRUE, &sep))
		{
			return HRESULT_FROM_WIN32(GetLastError());
		}

		return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(IDM_DISPLAY + 1));
	}

	IFACEMETHODIMP MenuExt::InvokeCommand(LPCMINVOKECOMMANDINFO pici)
	{
		BOOL fUnicode = FALSE;
		int commandID = -1;

		if (pici->cbSize == sizeof(CMINVOKECOMMANDINFOEX))
		{
			if (pici->fMask & CMIC_MASK_UNICODE)
			{
				fUnicode = TRUE;
			}
		}

		if (!fUnicode && HIWORD(pici->lpVerb))
		{
			if (StrCmpIA(pici->lpVerb, &m_verb[0]) == 0)
			{
				PerformAction();
			}
			else
			{
				return E_FAIL;
			}
		}
		else if (fUnicode && HIWORD(((CMINVOKECOMMANDINFOEX*)pici)->lpVerbW))
		{
			if (StrCmpIW(((CMINVOKECOMMANDINFOEX*)pici)->lpVerbW, &m_wVerb[0]) == 0)
			{
				PerformAction();
			}
			else
			{
				return E_FAIL;
			}
		}
		else
		{
			if (LOWORD(pici->lpVerb) == IDM_DISPLAY)
			{
				PerformAction();
			}
			else
			{
				return E_FAIL;
			}
		}

		return S_OK;

	}

	IFACEMETHODIMP MenuExt::GetCommandString(UINT_PTR idCommand,
		UINT uFlags, UINT* pwReserved, LPSTR pszName, UINT cchMax)
	{
		switch (uFlags)
		{
		case GCS_HELPTEXTW:
			StringCchCopy(reinterpret_cast<PWSTR>(pszName), cchMax, m_wVerbHelpText.c_str());
			break;

		case GCS_VERBW:
			StringCchCopy(reinterpret_cast<PWSTR>(pszName), cchMax, m_wVerbCanonicalName.c_str());
			break;

		default:
			return S_OK;
		}

		return S_OK;
	}

#pragma endregion
}
