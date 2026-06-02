#include "pch.h"
#include "Tank.h"
#include "Terrain.h"
#include "Humvee.h"

CTank::CTank() {}

CTank::~CTank()
{
    if (m_pModel) { m_pModel->Release(); m_pModel = NULL; }
}

bool CTank::LoadModel(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
                      CShader* pShader, const char* pstrFileName)
{
    m_pModel = CGameObject::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, pstrFileName, pShader);
    if (!m_pModel) return false;
    m_pModel->AddRef();
    m_pModel->CreateShaderVariables(pd3dDevice, pd3dCommandList);

    m_pTurret = m_pModel->FindFrame((char*)"TURRET");
    if (m_pTurret) m_TurretBase = m_pTurret->m_xmf4x4Transform;
    return true;
}

static void TintHierarchy(CGameObject* p, const XMFLOAT4& diffuse)
{
    if (!p) return;
    for (int i = 0; i < p->m_nMaterials; ++i)
        if (p->m_ppMaterials[i] && p->m_ppMaterials[i]->m_pMaterialColors)
        {
            CMaterialColors* c = p->m_ppMaterials[i]->m_pMaterialColors;
            c->m_xmf4Diffuse = diffuse;
            c->m_xmf4Ambient = XMFLOAT4(diffuse.x * 0.35f, diffuse.y * 0.35f, diffuse.z * 0.35f, 1.0f);
        }
    TintHierarchy(p->m_pSibling, diffuse);
    TintHierarchy(p->m_pChild,   diffuse);
}

void CTank::SetBodyColor(XMFLOAT4 xmf4Color)
{
    if (m_pModel) TintHierarchy(m_pModel, xmf4Color);
}

void CTank::SetSpawn(XMFLOAT3 pos, float fSpeed, float fScale)
{
    m_xmf3Pos = pos;
    m_fSpeed  = fSpeed;
    m_fScale  = fScale;
    Update(0.0f);   // place on the surface for frame 0
}

int CTank::Update(float fTimeElapsed)
{
    if (!m_pModel || !m_pTerrain) return OBJ_NOEVENT;

    float targetYaw = m_fYaw;
    float dist = 1e9f;
    if (m_pTarget)
    {
        XMFLOAT3 tgt = m_pTarget->GetPosition();
        float dx = tgt.x - m_xmf3Pos.x, dz = tgt.z - m_xmf3Pos.z;
        dist = sqrtf(dx * dx + dz * dz);
        targetYaw = XMConvertToDegrees(atan2f(dx, dz));

        if (!m_bLocked && dist < m_fLockDist) m_bLocked = true;

        // Drive toward the humvee until close enough, then hold position.
        if (dist > m_fStopDist)
        {
            m_fYaw = targetYaw;
            float yawR = XMConvertToRadians(m_fYaw);
            XMFLOAT3 fwd = XMFLOAT3(sinf(yawR), 0.0f, cosf(yawR));
            m_xmf3Pos = Vector3::Add(m_xmf3Pos, fwd, m_fSpeed * fTimeElapsed);
        }
    }

    // Terrain follow: height + tilt to surface normal (same as the humvee).
    float    h  = m_pTerrain->GetHeight(m_xmf3Pos.x, m_xmf3Pos.z);
    XMFLOAT3 up = Vector3::Normalize(m_pTerrain->GetNormal(m_xmf3Pos.x, m_xmf3Pos.z));

    // Body visual facing = heading + model offset (M26 faces -Z, so +180).
    float bodyYaw = m_fYaw + m_fModelYawOffset;
    float yawR = XMConvertToRadians(bodyYaw);
    XMFLOAT3 look  = XMFLOAT3(sinf(yawR), 0.0f, cosf(yawR));
    XMFLOAT3 right = Vector3::Normalize(Vector3::CrossProduct(up, look));
    look           = Vector3::Normalize(Vector3::CrossProduct(right, up));

    // Turret aims at a world azimuth: the movement heading before lock, the
    // humvee once locked. Local yaw is that world azimuth minus the body's
    // (offset) facing, so it stays correct despite the 180 body flip.
    if (m_pTurret)
    {
        float aimWorldYaw    = m_bLocked ? targetYaw : m_fYaw;
        float turretLocalYaw = aimWorldYaw - bodyYaw + m_fTurretYawOffset;
        XMMATRIX rot   = XMMatrixRotationY(XMConvertToRadians(turretLocalYaw));
        XMMATRIX trans = XMMatrixTranslation(m_TurretBase._41, m_TurretBase._42, m_TurretBase._43);
        XMStoreFloat4x4(&m_pTurret->m_xmf4x4Transform, rot * trans);
    }

    float s = m_fScale;
    XMFLOAT4X4 w;
    w._11 = right.x * s; w._12 = right.y * s; w._13 = right.z * s; w._14 = 0.0f;
    w._21 = up.x    * s; w._22 = up.y    * s; w._23 = up.z    * s; w._24 = 0.0f;
    w._31 = look.x  * s; w._32 = look.y  * s; w._33 = look.z  * s; w._34 = 0.0f;
    w._41 = m_xmf3Pos.x; w._42 = h; w._43 = m_xmf3Pos.z;          w._44 = 1.0f;

    m_pModel->m_xmf4x4Transform = w;
    m_pModel->UpdateTransform(NULL);

    return OBJ_NOEVENT;
}

void CTank::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
    if (m_pModel) m_pModel->Render(pd3dCommandList, pCamera);
}

void CTank::ReleaseUploadBuffers()
{
    if (m_pModel) m_pModel->ReleaseUploadBuffers();
}
