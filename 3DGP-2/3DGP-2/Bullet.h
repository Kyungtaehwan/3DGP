#pragma once
#include "GameObject.h"
#include "Map_Manager.h"

class CBullet : public CGameObject
{
public:
    CBullet()  = default;
    ~CBullet() { Release(); }

    virtual int  Update(float dt) override;
    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera) override;
    virtual void Release() override;
    virtual void ReleaseUploadBuffers() override;

    // vPos/vDir in world space; pShader must outlive the bullet
    void Fire(ID3D12Device* pd3dDevice,
              XMFLOAT3 vPos, XMFLOAT3 vDir,
              CShader* pShader, float fSpeed = 35.f);

    void SetBlowingUp();

    // Called once at level init / release
    static void PrepareShared(ID3D12Device* pd3dDevice,
                               ID3D12GraphicsCommandList* pd3dCommandList,
                               CShader* pShader);
    static void ReleaseShared();
    static void    ReleaseSharedUploadBuffers();
    static CShader* GetSharedShader() { return s_pSharedShader; }

private:
    XMFLOAT3 m_vPos       = {};
    XMFLOAT3 m_vDir       = {};
    float    m_fSpeed     = 35.f;
    float    m_fTravelled = 0.f;

    bool  m_bBlowingUp = false;
    float m_fBlowTimer = 0.f;

    static constexpr float MAX_RANGE  = 150.f;
    static constexpr float BLOW_DUR   = 0.45f;
    static constexpr float DEBRIS_SPD = 8.f;
    static constexpr int   DEBRIS_CNT = 12;

    XMFLOAT4X4 m_DebrisTx[DEBRIS_CNT] = {};

    // Per-bullet debris CB: DEBRIS_CNT × 256-byte slots
    ID3D12Resource* m_pDebrisCB     = nullptr;
    UINT8*          m_pMappedDebris = nullptr;

    static XMFLOAT3 s_SphereVecs[DEBRIS_CNT];
    static CMesh*   s_pBulletMesh;
    static CMesh*   s_pDebrisMesh;
    static CShader* s_pSharedShader;
};
