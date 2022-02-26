#include "Global.h"

#include "OverlayIconExt.h"
#include "MenuExt.h"


// {EA7F9DBC-29D8-49E2-88DD-E48953EAFC17}
const CLSID CLSID_MenuExt =
{ 0xea7f9dbc, 0x29d8, 0x49e2, { 0x88, 0xdd, 0xe4, 0x89, 0x53, 0xea, 0xfc, 0x17 } };

// {D6EC575B-FF7A-4C04-B092-35BDC0C2FC75}
const CLSID CLSID_OverlayIconExt =
{ 0xd6ec575b, 0xff7a, 0x4c04, { 0xb0, 0x92, 0x35, 0xbd, 0xc0, 0xc2, 0xfc, 0x75 } };


Global::Global(): m_hInst(nullptr), m_cDllRef(0)
{
    InitializeExtensionsData();
    m_shellIdList = RegisterClipboardFormat(CFSTR_SHELLIDLIST);
}

Global::~Global()
{
}

void Global::InitializeExtensionsData()
{
    m_FactoryData.push_back(
        FactoryInfo(CLSID_OverlayIconExt, 
        &nsEXTRACT::OverlayIconExt::ComponentCreator,
        L"NZAEXTRACTOVERLAY"
        )
    );

    m_FactoryData.push_back(
        FactoryInfo(CLSID_MenuExt, 
        &nsEXTRACT::MenuExt::ComponentCreator,
        L"NZAEXTRACTMENU"
        )
    );
}


long Global::DllRef() const
{
    return m_cDllRef;
}

void Global::DllInst(HINSTANCE val)
{
    m_hInst = val;
}

HINSTANCE Global::DllInst() const
{
    return m_hInst;
}

int Global::ShellIdList() const
{
    return m_shellIdList;
}

std::vector<FactoryInfo>& Global::FactoryData()
{
    return m_FactoryData;
}

void Global::IncrementDllRef()
{
    InterlockedIncrement(&m_cDllRef);
}

void Global::DecrementDllRef()
{
    InterlockedDecrement(&m_cDllRef);
}