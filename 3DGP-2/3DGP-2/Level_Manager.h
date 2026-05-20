#pragma once
#include "Level_Menu.h"
#include "Level_GamePlay.h"

class CCamera;

class CLevel_Manager
{
private:
    CLevel_Manager() {}
    ~CLevel_Manager() {}

public:
    void Level_Change(LEVEL_ID eID,
                      ID3D12Device* pd3dDevice,
                      ID3D12GraphicsCommandList* pd3dCommandList,
                      ID3D12RootSignature* pd3dRootSignature);

    LEVEL_ID GetLevelID() { return m_eCurLevel; }

    int  Update(float dt);
    void Late_Update(float dt);
    void Render(ID3D12GraphicsCommandList* pd3dCommandList);
    void Release();
    void ReleaseUploadBuffers();
    CLevel* Get_CurLevel() { return m_pLevel; }
    static CLevel_Manager* Get_Instance()
    {
        if (!m_pInstance) m_pInstance = new CLevel_Manager;
        return m_pInstance;
    }

    static void Destroy_Instance()
    {
        if (m_pInstance) { delete m_pInstance; m_pInstance = nullptr; }
    }

private:
    static CLevel_Manager* m_pInstance;
    CLevel*  m_pLevel     = nullptr;
    LEVEL_ID m_ePreLevel  = LEVEL_MENU;
    LEVEL_ID m_eCurLevel  = LEVEL_MENU;
};
