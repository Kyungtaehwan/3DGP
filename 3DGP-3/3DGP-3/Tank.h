#pragma once
#include "GameObject.h"

class CShader;
class CHeightMapTerrain;
class CHumvee;

// Enemy tank. Drives across the terrain toward the humvee; once it first gets
// within lock range it stays "locked" and its turret tracks (aims at) the
// humvee from then on, regardless of distance. Terrain-follows like the humvee.
class CTank : public CGameObject
{
public:
    CTank();
    virtual ~CTank();

    bool LoadModel(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
                   CShader* pShader, const char* pstrFileName);

    void SetTerrain(CHeightMapTerrain* pTerrain) { m_pTerrain = pTerrain; }
    void SetTarget(CHumvee* pTarget)             { m_pTarget  = pTarget; }
    void SetSpawn(XMFLOAT3 pos, float fSpeed, float fScale);
    void SetBodyColor(XMFLOAT4 xmf4Color);

    virtual int  Update(float fTimeElapsed) override;
    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera) override;
    virtual void ReleaseUploadBuffers() override;

private:
    CGameObject*       m_pModel   = NULL;
    CGameObject*       m_pTurret  = NULL;   // cached frame, not owned
    XMFLOAT4X4         m_TurretBase;        // turret's original local transform
    CHeightMapTerrain* m_pTerrain = NULL;
    CHumvee*           m_pTarget  = NULL;

    XMFLOAT3 m_xmf3Pos = { 0.0f, 0.0f, 0.0f };
    float    m_fYaw    = 0.0f;      // body heading, degrees
    float    m_fSpeed  = 30.0f;
    float    m_fScale  = 4.0f;

    bool     m_bLocked   = false;   // true once it first reaches the humvee
    float    m_fLockDist = 250.0f;  // distance at which it locks on
    float    m_fStopDist = 120.0f;  // stops driving once this close

    float    m_fModelYawOffset  = 0.0f;
    // Extra turret azimuth offset (deg) if the barrel local axis isn't +Z.
    float    m_fTurretYawOffset = 180.0f;
};
