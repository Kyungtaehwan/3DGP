#pragma once
#include "GameObject.h"

class CShader;
class CMesh;
class CHeightMapTerrain;
class CPlayer;

class CTank : public CGameObject
{
public:
    CTank();
    virtual ~CTank();

    bool LoadModel(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
                   CShader* pShader, const char* pstrFileName);

    void SetTerrain(CHeightMapTerrain* pTerrain) { m_pTerrain = pTerrain; }
    void SetTarget(CPlayer* pTarget) { m_pTarget = pTarget; }
    void SetSpawn(XMFLOAT3 pos, float fBodyYaw, float fScale);
    void SetBodyColor(XMFLOAT4 xmf4Color);

    void SetCombatResources(ID3D12Device* pd3dDevice, CMesh* pBulletMesh, CShader* pColorShader)
    {
        m_pd3dDevice = pd3dDevice;
        m_pBulletMesh = pBulletMesh;
        m_pColorShader = pColorShader;
    }

    void OnHit(int nDamage)
    {
        if((m_nHP -= nDamage) < 0)
            m_nHP = 0;
    }

    virtual int Update(float fTimeElapsed) override;
    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera) override;
    virtual void ReleaseUploadBuffers() override;

private:
    void Fire(XMFLOAT3 aimDir);

    CGameObject* m_pModel = NULL;
    CGameObject* m_pTurret = NULL; 
    XMFLOAT4X4 m_TurretBase; 
    CGameObject* m_pBarrel = NULL;
    XMFLOAT4X4 m_BarrelBase;
    CHeightMapTerrain* m_pTerrain = NULL;
    CPlayer* m_pTarget = NULL;

    ID3D12Device* m_pd3dDevice = NULL;
    CMesh* m_pBulletMesh = NULL;
    CShader* m_pColorShader = NULL;

    XMFLOAT3 m_xmf3Pos = { 0.0f, 0.0f, 0.0f };
    float m_fYaw = 0.0f;
    float m_fScale = 5.0f;

    int m_nHP = 6; 
    float m_fFireRange = 800.0f;
    float m_fFireCD = 1.2f;
    float m_fFireTimer = 0.0f;
    bool m_bDead = false;

  
    float m_fMaxPitchDeg = 40.0f;
    float m_fMinPitchDeg = -5.0f;

    float m_fModelYawOffset = 0.0f;
    float m_fTurretYawOffset = 180.0f;
};
