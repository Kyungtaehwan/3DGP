#include "pch.h"
#include "Level_Manager.h"



CLevel_Manager* CLevel_Manager::m_pInstance = nullptr;

void CLevel_Manager::Level_Change(LEVEL_ID eID,
                                   ID3D12Device* pd3dDevice,
                                   ID3D12GraphicsCommandList* pd3dCommandList,
                                   ID3D12RootSignature* pd3dRootSignature)
{

    if (m_pLevel)
    {
        m_pLevel->Release();
        delete m_pLevel;
        m_pLevel = NULL;
    }

    m_ePreLevel = m_eCurLevel;
    m_eCurLevel = eID;

    switch (eID)
    {
    case LEVEL_MENU:
        m_pLevel = new CLevel_Menu();
        break;
    case LEVEL_GAMEPLAY:
        m_pLevel = new CLevel_GamePlay();
        break;
    default:
        m_pLevel = new CLevel_Menu();
        break;
    }

    if (m_pLevel)
        m_pLevel->Initialize(pd3dDevice, pd3dCommandList, pd3dRootSignature);
}

int CLevel_Manager::Update(float dt)
{
    if (!m_pLevel) return 0;
    return m_pLevel->Update(dt);
}

void CLevel_Manager::Late_Update(float dt)
{
    if (m_pLevel) m_pLevel->Late_Update(dt);
}

void CLevel_Manager::Render(ID3D12GraphicsCommandList* pd3dCommandList)
{
    if (m_pLevel)
        m_pLevel->Render(pd3dCommandList);
}

void CLevel_Manager::Release()
{
    if (m_pLevel)
    {
        m_pLevel->Release();
        delete m_pLevel;
        m_pLevel = NULL;
    }
}

void CLevel_Manager::ReleaseUploadBuffers()
{
    if (m_pLevel) m_pLevel->ReleaseUploadBuffers();
}
