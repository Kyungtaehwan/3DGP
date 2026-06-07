#pragma once
#include "GameObject.h"

class CShader;
class CMesh;
class CHeightMapTerrain;

class CBullet : public CGameObject
{
public:
    CBullet() {}
    virtual ~CBullet() {}

    void Initialize(ID3D12Device* pd3dDevice, CMesh* pMesh, CShader* pShader,
                    XMFLOAT3 pos, XMFLOAT3 dir, float fSpeed, float fLife, float fHalfExtent);

    // When set, the bullet dies on contact with the ground instead of passing through.
    void SetTerrain(CHeightMapTerrain* pTerrain) { m_pTerrain = pTerrain; }

    virtual int Update(float fTimeElapsed) override;

    void Kill() { m_bActive = false; }

private:
    XMFLOAT3 m_xmf3Pos = { 0.0f, 0.0f, 0.0f };
    XMFLOAT3 m_xmf3Dir = { 0.0f, 0.0f, 1.0f };
    float m_fSpeed = 300.0f;
    float m_fLife = 3.0f;
    float m_fAge = 0.0f;
    CHeightMapTerrain* m_pTerrain = nullptr;
};
