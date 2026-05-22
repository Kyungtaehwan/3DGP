#include "pch.h"
#include "Enemy.h"
#include "Mesh.h"
#include "Camera.h"
#include "Object_Manager.h"
#include <cmath>

void CEnemy::Init(ID3D12Device* pd3dDevice,
                  ID3D12GraphicsCommandList* pd3dCommandList,
                  CShader* pShader,
                  XMFLOAT3 spawnPos)
{
    SetShader(pShader);
    SetMesh(new CCubeMesh(pd3dDevice, pd3dCommandList,
        1.0f, 1.0f, 1.0f, XMFLOAT4(0.85f, 0.1f, 0.1f, 1.0f)));
    CreateShaderVariables(pd3dDevice, pd3dCommandList);

    m_xmLocalOBB = BoundingOrientedBox(
        XMFLOAT3(0.f, 0.f, 0.f),
        XMFLOAT3(0.5f, 0.5f, 0.5f),
        XMFLOAT4(0.f, 0.f, 0.f, 1.f));

    // y=0.5 so bottom of cube sits on floor (y=0)
    XMStoreFloat4x4(&m_xmf4x4World,
        XMMatrixTranslation(spawnPos.x, 0.5f, spawnPos.z));
}

int CEnemy::Update(float dt)
{
    if (m_bDead) return OBJ_DEAD;

    CGameObject* pPlayerObj = CObject_Manager::Get_Instance()->Get_Player();
    if (!pPlayerObj) return OBJ_NOEVENT;

    XMFLOAT3 myPos     = GetPosition();
    XMFLOAT3 playerPos = pPlayerObj->GetPosition();

    float dx   = playerPos.x - myPos.x;
    float dz   = playerPos.z - myPos.z;
    float dist = sqrtf(dx * dx + dz * dz);

    if (m_eState == EnemyState::IDLE && dist < m_fDetectRange)
        m_eState = EnemyState::CHASE;

    if (m_eState == EnemyState::CHASE && dist > 0.5f)
    {
        float nx = dx / dist;
        float nz = dz / dist;

        XMFLOAT3 prevPos = myPos;
        XMFLOAT3 newPos  = {
            myPos.x + nx * m_fSpeed * dt,
            myPos.y,
            myPos.z + nz * m_fSpeed * dt
        };

        // Tile-based wall collision (same logic as player)
        CMap_Manager*  pMap         = CMap_Manager::Get_Instance();
        XMFLOAT3       correctedPos = newPos;

        int cPlus  = (int)floorf((newPos.x + RADIUS) / TILE_SCALE);
        int cMinus = (int)floorf((newPos.x - RADIUS) / TILE_SCALE);
        int zRow   = (int)floorf(newPos.z / TILE_SCALE);
        if (!pMap->IsPassable(zRow, cPlus) || !pMap->IsPassable(zRow, cMinus))
            correctedPos.x = prevPos.x;

        int xCol   = (int)floorf(correctedPos.x / TILE_SCALE);
        int rPlus  = (int)floorf((newPos.z + RADIUS) / TILE_SCALE);
        int rMinus = (int)floorf((newPos.z - RADIUS) / TILE_SCALE);
        if (!pMap->IsPassable(rPlus, xCol) || !pMap->IsPassable(rMinus, xCol))
            correctedPos.z = prevPos.z;

        // Rotate to face player, then translate
        float yaw = atan2f(nx, nz);
        XMStoreFloat4x4(&m_xmf4x4World,
            XMMatrixRotationY(yaw) *
            XMMatrixTranslation(correctedPos.x, correctedPos.y, correctedPos.z));
    }

    return OBJ_NOEVENT;
}

void CEnemy::OnHit(int nDamage)
{
    m_iHP -= nDamage;
    if (m_iHP <= 0)
        m_bDead = true;
}

void CEnemy::Release()
{
    if (m_pMesh) { delete m_pMesh; m_pMesh = nullptr; }
    CGameObject::Release();
}

void CEnemy::ReleaseUploadBuffers()
{
    if (m_pMesh) m_pMesh->ReleaseUploadBuffers();
}
