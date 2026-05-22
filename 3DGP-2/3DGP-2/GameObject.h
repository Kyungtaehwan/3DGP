#pragma once
#include "Mesh.h"

class CCamera;
class CShader;
class COBBRenderer;

struct CB_WORLD_MATRIX
{
    XMFLOAT4X4 m_xmf4x4World;
};

class CGameObject
{
public:
    CGameObject();
    virtual ~CGameObject();

    virtual void Initialize() {}
    virtual int  Update(float dt) { return OBJ_NOEVENT; }
    virtual void Late_Update(float dt);
    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
    virtual void Release();

    virtual void CreateShaderVariables(ID3D12Device* pd3dDevice,
                                       ID3D12GraphicsCommandList* pd3dCommandList);
    virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
    virtual void ReleaseShaderVariables();
    virtual void ReleaseUploadBuffers() { if (m_pMesh) m_pMesh->ReleaseUploadBuffers(); }

    void SetMesh(CMesh* pMesh)     { m_pMesh   = pMesh; }
    void SetShader(CShader* pShader) { m_pShader = pShader; }

    void SetPosition(float x, float y, float z);
    void SetPosition(XMFLOAT3 xmf3Position);
    XMFLOAT3 GetPosition() const;
    XMFLOAT3 GetLook()     const;
    XMFLOAT3 GetUp()       const;
    XMFLOAT3 GetRight()    const;

    void MoveForward(float fDistance);
    void MoveStrafe(float fDistance);
    void MoveUp(float fDistance);
    void Rotate(float fPitch, float fYaw, float fRoll);

    virtual void OnHit(int nDamage);
    bool IsDead() const { return m_bDead; }
    int  GetHP()  const { return m_iHP; }

    bool                m_bActive = true;
    XMFLOAT4X4          m_xmf4x4World;
    BoundingOrientedBox m_xmOOBB;
    BoundingOrientedBox m_xmLocalOBB;

protected:
    void UpdateBoundingBox();

    CMesh*   m_pMesh   = NULL;
    CShader* m_pShader = NULL;

    // GPU constant buffer for world matrix (b1)
    ID3D12Resource*    m_pd3dcbWorldMatrix    = NULL;
    CB_WORLD_MATRIX*   m_pcbMappedWorldMatrix = NULL;

    bool m_bDead  = false;
    int  m_iMaxHP = 100;
    int  m_iHP    = 100;
};
