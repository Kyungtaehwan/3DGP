#include "pch.h"
#include "Bullet.h"
#include "Shader.h"
#include "Mesh.h"
#include "Terrain.h"

void CBullet::Initialize(ID3D12Device* pd3dDevice, CMesh* pMesh, CShader* pShader,
                         XMFLOAT3 pos, XMFLOAT3 dir, float fSpeed, float fLife, float fHalfExtent)
{
    m_xmf3Pos = pos;
    m_xmf3Dir = Vector3::Normalize(dir);
    m_fSpeed = fSpeed;
    m_fLife = fLife;
    m_fAge = 0.0f;
    m_bActive = true;

    SetMesh(pMesh);
    SetShader(pShader);
    CreateShaderVariables(pd3dDevice, nullptr);

    m_xmOOBB.Center = pos;
    m_xmOOBB.Extents = XMFLOAT3(fHalfExtent, fHalfExtent, fHalfExtent);
    m_xmOOBB.Orientation = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

    XMStoreFloat4x4(&m_xmf4x4World, XMMatrixTranslation(pos.x, pos.y, pos.z));
}

int CBullet::Update(float fTimeElapsed)
{
    if(!m_bActive)
        return OBJ_DEAD;

    m_fAge += fTimeElapsed;
    if(m_fAge >= m_fLife)
        return OBJ_DEAD;

    m_xmf3Pos = Vector3::Add(m_xmf3Pos, m_xmf3Dir, m_fSpeed * fTimeElapsed);

    if(m_pTerrain && m_xmf3Pos.y <= m_pTerrain->GetHeight(m_xmf3Pos.x, m_xmf3Pos.z))
        return OBJ_DEAD;

    m_xmOOBB.Center = m_xmf3Pos;
    XMStoreFloat4x4(&m_xmf4x4World,
                    XMMatrixTranslation(m_xmf3Pos.x, m_xmf3Pos.y, m_xmf3Pos.z));

    return OBJ_NOEVENT;
}
