#pragma once
#include "GameObject.h"

class CShader;
class CMesh;
class CHeightMapTerrain;

class CHumvee : public CGameObject
{
public:
    CHumvee();
    virtual ~CHumvee();

    bool LoadModel(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
                   CShader* pShader, const char* pstrFileName);

    void SetTerrain(CHeightMapTerrain* pTerrain) { m_pTerrain = pTerrain; }
    void SetPath(XMFLOAT3 start, XMFLOAT3 end, float fSpeed, float fScale);
    void SetBodyColor(XMFLOAT4 xmf4Color);

    virtual int Update(float fTimeElapsed) override;
    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera) override;
    virtual void ReleaseUploadBuffers() override;

    XMFLOAT3 GetPosition() const { return m_xmf3Pos; }

    void SetCombatResources(ID3D12Device* pd3dDevice, CShader* pColorShader)
    {
        m_pd3dDevice = pd3dDevice;
        m_pColorShader = pColorShader;
    }

    void OnHit(int nDamage)
    {
        if((m_nHP -= nDamage) < 0)
            m_nHP = 0;
    }
    bool IsArrived() const { return m_bArrived; }
    bool IsDestroyed() const { return m_nHP <= 0; }
    int GetHP() const { return m_nHP; }
    int GetMaxHP() const { return m_nMaxHP; }

private:
    CGameObject* m_pModel = NULL;
    CHeightMapTerrain* m_pTerrain = NULL;

    ID3D12Device* m_pd3dDevice = NULL;
    CShader* m_pColorShader = NULL;
    bool m_bDead = false;

    XMFLOAT3 m_xmf3Pos = { 0.0f, 0.0f, 0.0f };
    XMFLOAT3 m_xmf3End = { 0.0f, 0.0f, 0.0f };
    float m_fYaw = 0.0f; 
    float m_fSpeed = 60.0f;
    float m_fScale = 1.0f;
    bool m_bArrived = false;
    int m_nMaxHP = 30;
    int m_nHP = 30;
};
