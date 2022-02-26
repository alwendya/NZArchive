#include "Global.h"

#include "OverlayIconExt.h"
#include "MenuExt.h"


// {C0552A55-A874-4ACC-9C64-F43A86D73F4E}
const CLSID CLSID_MenuExt = 
{ 0xc0552a55, 0xa874, 0x4acc, { 0x9c, 0x64, 0xf4, 0x3a, 0x86, 0xd7, 0x3f, 0x4e } };

// {E4FCBC8E-C70D-4B28-AF68-B98C99015730}
const CLSID CLSID_OverlayIconExt = 
{ 0xe4fcbc8e, 0xc70d, 0x4b28, { 0xaf, 0x68, 0xb9, 0x8c, 0x99, 0x1, 0x57, 0x30 } };


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
        &nsADDTO::OverlayIconExt::ComponentCreator,
        L"NZAEXTRHTOVERLAY"
        )
    );

    m_FactoryData.push_back(
        FactoryInfo(CLSID_MenuExt, 
        &nsADDTO::MenuExt::ComponentCreator,
        L"NZAEXTRTOMENU"
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