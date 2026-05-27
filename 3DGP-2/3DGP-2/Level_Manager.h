#pragma once
#include "Level_Menu.h"
#include "Level_GamePlay.h"
#include "Level_LOGO.h"

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

    void Request_Level_Change(LEVEL_ID eID, int nStage = 0);

    void Apply_Pending_Change(ID3D12Device* pd3dDevice,
                              ID3D12GraphicsCommandList* pd3dCommandList,
                              ID3D12RootSignature* pd3dRootSignature);

    LEVEL_ID GetLevelID()    { return m_eCurLevel; }
    int      GetCurrentStage() const { return m_nCurrentStage; }
    bool     HasPendingChange() const { return m_bHasPending; }

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
    CLevel*  m_pLevel        = nullptr;
    LEVEL_ID m_ePreLevel     = LEVEL_MENU;
    LEVEL_ID m_eCurLevel     = LEVEL_MENU;


    bool     m_bHasPending   = false;
    LEVEL_ID m_ePendingLevel = LEVEL_MENU;
    int      m_nPendingStage = 1;

    int      m_nCurrentStage = 1;
};
