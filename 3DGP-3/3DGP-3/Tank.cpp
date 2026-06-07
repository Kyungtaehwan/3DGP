#include "pch.h"
#include "Tank.h"
#include "Terrain.h"
#include "Player.h"
#include "Bullet.h"
#include "ExplosionEffect.h"
#include "Object_Manager.h"

CTank::CTank() {}

CTank::~CTank()
{
    if(m_pModel)
    {
        m_pModel->Release();
        m_pModel = NULL;
    }
}

bool CTank::LoadModel(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
                      CShader* pShader, const char* pstrFileName)
{
    m_pModel = CGameObject::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, pstrFileName, pShader);
    if(!m_pModel)
        return false;
    m_pModel->AddRef();
    m_pModel->CreateShaderVariables(pd3dDevice, pd3dCommandList);

    m_pTurret = m_pModel->FindFrame((char*)"TURRET");
    if(m_pTurret)
        m_TurretBase = m_pTurret->m_xmf4x4Transform;

    m_pBarrel = m_pModel->FindFrame((char*)"shield");
    if(m_pBarrel)
        m_BarrelBase = m_pBarrel->m_xmf4x4Transform;
    return true;
}

static void TintHierarchy(CGameObject* pFrame, const XMFLOAT4& diffuse)
{
    if(!pFrame)
        return;
    for(int i = 0; i < pFrame->m_nMaterials; ++i)
        if(pFrame->m_ppMaterials[i] && pFrame->m_ppMaterials[i]->m_pMaterialColors)
        {
            CMaterialColors* pColors = pFrame->m_ppMaterials[i]->m_pMaterialColors;
            pColors->m_xmf4Diffuse = diffuse;
            pColors->m_xmf4Ambient = XMFLOAT4(diffuse.x * 0.35f, diffuse.y * 0.35f, diffuse.z * 0.35f, 1.0f);
        }
    TintHierarchy(pFrame->m_pSibling, diffuse);
    TintHierarchy(pFrame->m_pChild, diffuse);
}

void CTank::SetBodyColor(XMFLOAT4 xmf4Color)
{
    if(m_pModel)
        TintHierarchy(m_pModel, xmf4Color);
}

void CTank::SetSpawn(XMFLOAT3 pos, float fBodyYaw, float fScale)
{
    m_xmf3Pos = pos;
    m_fYaw = fBodyYaw;
    m_fScale = fScale;
    Update(0.0f);
}

void CTank::Fire(XMFLOAT3 aimDir)
{
    if(!m_pd3dDevice || !m_pBulletMesh || !m_pColorShader)
        return;

    float groundHeight;
    if(m_pTerrain)
        groundHeight = m_pTerrain->GetHeight(m_xmf3Pos.x, m_xmf3Pos.z);
    else
        groundHeight = m_xmf3Pos.y;
    XMFLOAT3 muzzle = XMFLOAT3(m_xmf3Pos.x, groundHeight + 6.0f, m_xmf3Pos.z);
    muzzle = Vector3::Add(muzzle, aimDir, 10.0f);

    CBullet* pBullet = new CBullet();
    pBullet->Initialize(m_pd3dDevice, m_pBulletMesh, m_pColorShader,
                        muzzle, aimDir, 220.0f, 4.0f, 2.0f);
    pBullet->SetTerrain(m_pTerrain);
    CObject_Manager::Get_Instance()->Add_Object(OBJ_ENEMY_BULLET, pBullet);
}

int CTank::Update(float fTimeElapsed)
{
    if(!m_pModel || !m_pTerrain)
        return OBJ_NOEVENT;


    if(m_nHP <= 0 && !m_bDead)
    {
        m_bDead = true;
        m_bActive = false;
        float groundHeight = m_pTerrain->GetHeight(m_xmf3Pos.x, m_xmf3Pos.z);
        if(m_pd3dDevice && m_pColorShader)
        {
            CExplosionEffect* pFx = new CExplosionEffect();
            pFx->Initialize(m_pd3dDevice, m_pColorShader, XMFLOAT3(m_xmf3Pos.x, groundHeight + 4.0f, m_xmf3Pos.z), 150.0f);
            CObject_Manager::Get_Instance()->Add_Object(OBJ_EFFECT, pFx);
        }
        return OBJ_DEAD;
    }

    float aimWorldYaw = m_fYaw;
    float dist = 1e9f;
    float elevDeg = 0.0f;
    XMFLOAT3 aimDir = XMFLOAT3(0.0f, 0.0f, 1.0f);
    bool bInRange = false;
    bool bCanElevate = false;
    if(m_pTarget && !m_pTarget->IsDestroyed())
    {
        XMFLOAT3 tgt = m_pTarget->GetPosition();
        float dx = tgt.x - m_xmf3Pos.x, dz = tgt.z - m_xmf3Pos.z;
        dist = sqrtf(dx * dx + dz * dz);
        aimWorldYaw = XMConvertToDegrees(atan2f(dx, dz));
        bInRange = (dist < m_fFireRange);

        float groundHeight = m_pTerrain->GetHeight(m_xmf3Pos.x, m_xmf3Pos.z);
        XMFLOAT3 muzzle = XMFLOAT3(m_xmf3Pos.x, groundHeight + 6.0f, m_xmf3Pos.z);
        XMFLOAT3 target = XMFLOAT3(tgt.x, tgt.y + 2.0f, tgt.z);
        aimDir = Vector3::Normalize(Vector3::Subtract(target, muzzle));

        float sy = aimDir.y;
        if(sy > 1.0f)
            sy = 1.0f;
        if(sy < -1.0f)
            sy = -1.0f;
        elevDeg = XMConvertToDegrees(asinf(sy));
        bCanElevate = (elevDeg <= m_fMaxPitchDeg && elevDeg >= m_fMinPitchDeg);
    }

    float groundHeight = m_pTerrain->GetHeight(m_xmf3Pos.x, m_xmf3Pos.z);
    XMFLOAT3 up = Vector3::Normalize(m_pTerrain->GetNormal(m_xmf3Pos.x, m_xmf3Pos.z));

    float bodyYaw = m_fYaw + m_fModelYawOffset;
    float yawR = XMConvertToRadians(bodyYaw);
    XMFLOAT3 look = XMFLOAT3(sinf(yawR), 0.0f, cosf(yawR));
    XMFLOAT3 right = Vector3::Normalize(Vector3::CrossProduct(up, look));
    look = Vector3::Normalize(Vector3::CrossProduct(right, up));

    if(m_pTurret)
    {
        float aimYaw;
        if(bInRange)
            aimYaw = aimWorldYaw;
        else
            aimYaw = m_fYaw;
        float turretLocalYaw = aimYaw - bodyYaw + m_fTurretYawOffset;
        XMMATRIX rot = XMMatrixRotationY(XMConvertToRadians(turretLocalYaw));
        XMMATRIX trans = XMMatrixTranslation(m_TurretBase._41, m_TurretBase._42, m_TurretBase._43);
        XMStoreFloat4x4(&m_pTurret->m_xmf4x4Transform, rot * trans);
    }


    if(m_pBarrel)
    {
        float pitch;
        if(bInRange)
            pitch = elevDeg;
        else
            pitch = 0.0f;
        if(pitch > m_fMaxPitchDeg)
            pitch = m_fMaxPitchDeg;
        if(pitch < m_fMinPitchDeg)
            pitch = m_fMinPitchDeg;
        XMMATRIX rot = XMMatrixRotationX(XMConvertToRadians(pitch));
        XMMATRIX trans = XMMatrixTranslation(m_BarrelBase._41, m_BarrelBase._42, m_BarrelBase._43);
        XMStoreFloat4x4(&m_pBarrel->m_xmf4x4Transform, rot * trans);
    }

    float scale = m_fScale;
    XMFLOAT4X4 world;
    world._11 = right.x * scale;
    world._12 = right.y * scale;
    world._13 = right.z * scale;
    world._14 = 0.0f;
    world._21 = up.x * scale;
    world._22 = up.y * scale;
    world._23 = up.z * scale;
    world._24 = 0.0f;
    world._31 = look.x * scale;
    world._32 = look.y * scale;
    world._33 = look.z * scale;
    world._34 = 0.0f;
    world._41 = m_xmf3Pos.x;
    world._42 = groundHeight;
    world._43 = m_xmf3Pos.z;
    world._44 = 1.0f;

    m_pModel->m_xmf4x4Transform = world;
    m_pModel->UpdateTransform(NULL);

    XMVECTOR vScale, vRotQ, vTrans;
    XMMatrixDecompose(&vScale, &vRotQ, &vTrans, XMLoadFloat4x4(&m_pModel->m_xmf4x4World));
    XMStoreFloat4(&m_xmOOBB.Orientation, vRotQ);
    m_xmOOBB.Center = XMFLOAT3(m_xmf3Pos.x, groundHeight + 3.0f, m_xmf3Pos.z);
    m_xmOOBB.Extents = XMFLOAT3(7.0f, 5.0f, 9.0f);

    if(m_fFireTimer > 0.0f)
        m_fFireTimer -= fTimeElapsed;
    if(bInRange && bCanElevate && m_fFireTimer <= 0.0f)
    {
        Fire(aimDir);
        m_fFireTimer = m_fFireCD;
    }

    return OBJ_NOEVENT;
}

void CTank::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
    if(m_pModel)
        m_pModel->Render(pd3dCommandList, pCamera);
}

void CTank::ReleaseUploadBuffers()
{
    if(m_pModel)
        m_pModel->ReleaseUploadBuffers();
}
