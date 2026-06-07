#pragma once
#include "Level.h"

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

    void Request_Level_Change(LEVEL_ID eID)
    {
        m_bHasPending = true;
        m_ePendingLevel = eID;
    }
    bool HasPendingChange() const { return m_bHasPending; }
    void Apply_Pending_Change(ID3D12Device* pd3dDevice,
                              ID3D12GraphicsCommandList* pd3dCommandList,
                              ID3D12RootSignature* pd3dRootSignature);

    int Update(float dt);
    void Render(ID3D12GraphicsCommandList* pd3dCommandList);
    void Release();
    void ReleaseUploadBuffers();

    static CLevel_Manager* Get_Instance()
    {
        if(!m_pInstance)
            m_pInstance = new CLevel_Manager;
        return m_pInstance;
    }
    static void Destroy_Instance()
    {
        if(m_pInstance)
        {
            delete m_pInstance;
            m_pInstance = nullptr;
        }
    }

private:
    static CLevel_Manager* m_pInstance;
    CLevel* m_pLevel = nullptr;
    LEVEL_ID m_eCurLevel = LEVEL_LOGO;

    bool m_bHasPending = false;
    LEVEL_ID m_ePendingLevel = LEVEL_LOGO;
};
