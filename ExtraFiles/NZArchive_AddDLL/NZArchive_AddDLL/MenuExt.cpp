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
		m_menuTextAdd(L"NZArchive - Add to new archive"),
		m_verbAdd("nzaaddto"),
		m_wVerbAdd(L"nzaaddto"),
		m_verbHelpTextAdd("Add to new archive"),
		m_wVerbHelpTextAdd(L"Add to new archive"),
		m_verbCanonicalNameAdd("NZArcAddTo"),
		m_wVerbCanonicalNameAdd(L"NZArcAddTo"),
		m_menuTextExtTo(L"NZArchive - Extract to"),
		m_verbExtTo("nzaextrto"),
		m_wVerbExtTo(L"nzaextrto"),
		m_verbHelpTextExtTo("Extract to"),
		m_wVerbHelpTextExtTo(L"Extract to"),
		m_verbCanonicalNameExtTo("NZArcExtrTo"),
		m_wVerbCanonicalNameExtTo(L"NZArcExtrTo"),
		m_menuTextExtHere(L"NZArchive - Extract here"),
		m_verbExtHere("nzaextrhere"),
		m_wVerbExtHere(L"nzaextrhere"),
		m_verbHelpTextExtHere("Extract here"),
		m_wVerbHelpTextExtHere(L"Extract here"),
		m_verbCanonicalNameExtHere("NZArcExtrHere"),
		m_wVerbCanonicalNameExtHere(L"NZArcExtrHere")
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
			m_wVerbHelpTextAdd = L"NZArchive - Ajouter à une nouvelle archive";
			break;
		case 3076://Chinese
		case 2052:
		case 4100:
		case 1028:
			m_wVerbHelpTextAdd = L"NZArchive - 添加到新存档";
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
			m_wVerbHelpTextAdd = L"NZArchive - Add to new archive";
			break;
		case 1084://German
		case 1031:
		case 3079:
		case 5127:
		case 4103:
		case 2055:
			m_wVerbHelpTextAdd = L"NZArchive - Zu neuem Archiv hinzufügen";
			break;
		case 1041://Japanese
			m_wVerbHelpTextAdd = L"NZArchive - 新しいアーカイブに追加";
			break;
		case 1042://Korean
			m_wVerbHelpTextAdd = L"NZArchive - 새 아카이브에 추가";
			break;
		case 1040://Italian
		case 2064:
			m_wVerbHelpTextAdd = L"NZArchive - Aggiungi al nuovo archivio";
			break;
		case 1046://Portuguese
		case 2070:
			m_wVerbHelpTextAdd = L"NZArchive - Adicionar ao novo arquivo";
			break;
		case 1049://Russian
		case 2073:
			m_wVerbHelpTextAdd = L"NZArchive - Добавить в новый архив";
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
			m_wVerbHelpTextAdd = L"NZArchive - Agregar a un nuevo archivo";
			break;
		case 1066://Vietnamese
			m_wVerbHelpTextAdd = L"NZArchive - Thêm vào kho lưu trữ mới";
			break;
		default:
			m_wVerbHelpTextAdd = L"NZArchive - Add to new archive";
			break;
		}
		m_menuTextAdd = m_wVerbHelpTextAdd;

		switch (CurLiD)
		{
		case 1036://French
		case 2060:
		case 3084:
		case 5132:
		case 4108:
			m_wVerbHelpTextExtTo = L"NZArchive - Extraire vers";
			break;
		case 3076://Chinese
		case 2052:
		case 4100:
		case 1028:
			m_wVerbHelpTextExtTo = L"NZArchive - 解压到";
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
			m_wVerbHelpTextExtTo = L"NZArchive - Extract to";
			break;
		case 1084://German
		case 1031:
		case 3079:
		case 5127:
		case 4103:
		case 2055:
			m_wVerbHelpTextExtTo = L"NZArchive - Entpacken nach";
			break;
		case 1041://Japanese
			m_wVerbHelpTextExtTo = L"NZArchive - に抽出";
			break;
		case 1042://Korean
			m_wVerbHelpTextExtTo = L"NZArchive - 추출";
			break;
		case 1040://Italian
		case 2064:
			m_wVerbHelpTextExtTo = L"NZArchive - Estrai in";
			break;
		case 1046://Portuguese
		case 2070:
			m_wVerbHelpTextExtTo = L"NZArchive - Extrair para";
			break;
		case 1049://Russian
		case 2073:
			m_wVerbHelpTextExtTo = L"NZArchive - Извлечь в";
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
			m_wVerbHelpTextExtTo = L"NZArchive - Extraer a";
			break;
		case 1066://Vietnamese
			m_wVerbHelpTextExtTo = L"NZArchive - Giải nén ra";
			break;
		default:
			m_wVerbHelpTextExtTo = L"NZArchive - Extract to";
			break;
		}
		m_menuTextExtTo = m_wVerbHelpTextExtTo;

		switch (CurLiD)
		{
		case 1036://French
		case 2060:
		case 3084:
		case 5132:
		case 4108:
			m_wVerbHelpTextExtHere = L"NZArchive - Extraire ici";
			break;
		case 3076://Chinese
		case 2052:
		case 4100:
		case 1028:
			m_wVerbHelpTextExtHere = L"NZArchive - 在这里提取 ";
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
			m_wVerbHelpTextExtHere = L"NZArchive - Extract here";
			break;
		case 1084://German
		case 1031:
		case 3079:
		case 5127:
		case 4103:
		case 2055:
			m_wVerbHelpTextExtHere = L"NZArchive - Hier entpacken";
			break;
		case 1041://Japanese
			m_wVerbHelpTextExtHere = L"NZArchive - ここで抽出";
			break;
		case 1042://Korean
			m_wVerbHelpTextExtHere = L"NZArchive - 추출여기에 추출";
			break;
		case 1040://Italian
		case 2064:
			m_wVerbHelpTextExtHere = L"NZArchive - Estrarre qui ";
			break;
		case 1046://Portuguese
		case 2070:
			m_wVerbHelpTextExtHere = L"NZArchive - Extrair aqui ";
			break;
		case 1049://Russian
		case 2073:
			m_wVerbHelpTextExtHere = L"NZArchive - Извлечь сюда ";
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
			m_wVerbHelpTextExtHere = L"NZArchive - Extraer aqui ";
			break;
		case 1066://Vietnamese
			m_wVerbHelpTextExtHere = L"NZArchive - Trích xuất tại đây ";
			break;
		default:
			m_wVerbHelpTextExtHere = L"NZArchive - Extract here";
			break;
		}
		m_menuTextExtHere = m_wVerbHelpTextExtHere;

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

	void MenuExt::PerformActionAdd()
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
	void MenuExt::PerformActionExtTo()
	{
		std::wstring CMDLine = L":EXTRACT ";
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
	void MenuExt::PerformActionExtHere()
	{
		std::wstring CMDLine = L":EXTRACTHERE ";
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

		int pos = indexMenu;
		int nbADD = 0;
		{
			MENUITEMINFO item = { sizeof(MENUITEMINFO) };
			item.fMask = MIIM_BITMAP | MIIM_STRING | MIIM_FTYPE | MIIM_ID | MIIM_STATE;
			item.wID = idCmdFirst + IDM_DISPLAY + 1;
			item.fType = MFT_STRING;
			item.fState = MFS_ENABLED;
			item.dwTypeData = &m_menuTextAdd[0];
			item.hbmpItem = static_cast<HBITMAP>(m_hMenuBmp);
			if (!InsertMenuItem(hMenu, pos++, TRUE, &item))
			{
				return HRESULT_FROM_WIN32(GetLastError());
			}
			nbADD++;
		}
		if (m_wFileList.size() == 1)//Single file
			if (m_wFileList[0].size() >= 4)
			{
				std::wstring lFile = m_wFileList[0];
				std::transform(lFile.begin(), lFile.end(), lFile.begin(), std::tolower);
				if (lFile.substr(lFile.size() - 4, 4) == L".nza")
				{
					{
						MENUITEMINFO item = { sizeof(MENUITEMINFO) };
						item.fMask = MIIM_BITMAP | MIIM_STRING | MIIM_FTYPE | MIIM_ID | MIIM_STATE;
						item.wID = idCmdFirst + IDM_DISPLAY + 2;
						item.fType = MFT_STRING;
						item.fState = MFS_ENABLED;
						item.dwTypeData = &m_menuTextExtTo[0];
						item.hbmpItem = static_cast<HBITMAP>(m_hMenuBmp);
						if (!InsertMenuItem(hMenu, pos++, TRUE, &item))
						{
							return HRESULT_FROM_WIN32(GetLastError());
						}
						nbADD++;
					}
					{
						MENUITEMINFO item = { sizeof(MENUITEMINFO) };
						item.fMask = MIIM_BITMAP | MIIM_STRING | MIIM_FTYPE | MIIM_ID | MIIM_STATE;
						item.wID = idCmdFirst + IDM_DISPLAY + 3;
						item.fType = MFT_STRING;
						item.fState = MFS_ENABLED;
						item.dwTypeData = &m_menuTextExtHere[0];
						item.hbmpItem = static_cast<HBITMAP>(m_hMenuBmp);
						if (!InsertMenuItem(hMenu, pos++, TRUE, &item))
						{
							return HRESULT_FROM_WIN32(GetLastError());
						}
						nbADD++;
					}
				}
			}
		nbADD++;

		MENUITEMINFO sep = { sizeof(sep) };
		sep.fMask = MIIM_TYPE;
		sep.fType = MFT_SEPARATOR;
		if (!InsertMenuItem(hMenu, pos++, TRUE, &sep))
		{
			return HRESULT_FROM_WIN32(GetLastError());
		}

		return MAKE_HRESULT(SEVERITY_SUCCESS, 0, nbADD);
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
			if (StrCmpIA(pici->lpVerb, &m_verbAdd[0]) == 0)
			{
				PerformActionAdd();
			}
			else if (StrCmpIA(pici->lpVerb, &m_verbExtTo[0]) == 0)
			{
				PerformActionExtTo();
			}
			else if (StrCmpIA(pici->lpVerb, &m_verbExtHere[0]) == 0)
			{
				PerformActionExtHere();
			}
			else
			{
				return E_FAIL;
			}
		}
		else if (fUnicode && HIWORD(((CMINVOKECOMMANDINFOEX*)pici)->lpVerbW))
		{
			if (StrCmpIW(((CMINVOKECOMMANDINFOEX*)pici)->lpVerbW, &m_wVerbAdd[0]) == 0)
			{
				PerformActionAdd();
			}
			else if (StrCmpIW(((CMINVOKECOMMANDINFOEX*)pici)->lpVerbW, &m_wVerbExtTo[0]) == 0)
			{
				PerformActionExtTo();
			}
			else if (StrCmpIW(((CMINVOKECOMMANDINFOEX*)pici)->lpVerbW, &m_wVerbExtHere[0]) == 0)
			{
				PerformActionExtHere();
			}
			else
			{
				return E_FAIL;
			}
		}
		else
		{
			if (LOWORD(pici->lpVerb) == IDM_DISPLAY + 1)
			{
				PerformActionAdd();
			}
			else if (LOWORD(pici->lpVerb) == (IDM_DISPLAY + 2))
			{
				PerformActionExtTo();
			}
			else if (LOWORD(pici->lpVerb) == (IDM_DISPLAY + 3))
			{
				PerformActionExtHere();
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
		{
			if (idCommand == 1)
				StringCchCopy(reinterpret_cast<PWSTR>(pszName), cchMax, m_wVerbHelpTextAdd.c_str());
			if (idCommand == 2)
				StringCchCopy(reinterpret_cast<PWSTR>(pszName), cchMax, m_wVerbHelpTextExtTo.c_str());
			if (idCommand == 3)
				StringCchCopy(reinterpret_cast<PWSTR>(pszName), cchMax, m_wVerbHelpTextExtHere.c_str());
		}
		break;
		case GCS_VERBW:
		{
			if (idCommand == 1)
				StringCchCopy(reinterpret_cast<PWSTR>(pszName), cchMax, m_wVerbCanonicalNameAdd.c_str());
			if (idCommand == 2)
				StringCchCopy(reinterpret_cast<PWSTR>(pszName), cchMax, m_wVerbCanonicalNameExtTo.c_str());
			if (idCommand == 3)
				StringCchCopy(reinterpret_cast<PWSTR>(pszName), cchMax, m_wVerbCanonicalNameExtHere.c_str());
		}
		break;

		default:
			return S_OK;
		}

		return S_OK;
	}

#pragma endregion
}
