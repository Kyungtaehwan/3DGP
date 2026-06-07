#include "pch.h"
#include "ExplosionEffect.h"
#include "Mesh.h"

CMesh* CExplosionEffect::s_pDebrisMesh = nullptr;

static XMFLOAT3 RandomSphereVec()
{
    while(true)
    {
        float randomX = (rand() / (float)RAND_MAX) * 2.0f - 1.0f;
        float randomY = (rand() / (float)RAND_MAX) * 2.0f - 1.0f;
        float randomZ = (rand() / (float)RAND_MAX) * 2.0f - 1.0f;
        float len = sqrtf(randomX * randomX + randomY * randomY + randomZ * randomZ);
        if(len > 0.001f && len <= 1.0f)
            return { randomX / len, randomY / len, randomZ / len };
    }
}

void CExplosionEffect::PrepareShared(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
    if(s_pDebrisMesh)
        return;
    s_pDebrisMesh = new CColorCubeMesh(pd3dDevice, pd3dCommandList,
                                       0.05f, 0.05f, 0.05f, XMFLOAT4(1.0f, 0.55f, 0.10f, 1.0f));
    s_pDebrisMesh->AddRef();
}

void CExplosionEffect::ReleaseShared()
{
    if(s_pDebrisMesh)
    {
        s_pDebrisMesh->Release();
        s_pDebrisMesh = nullptr;
    }
}

void CExplosionEffect::ReleaseSharedUploadBuffers()
{
    if(s_pDebrisMesh)
        s_pDebrisMesh->ReleaseUploadBuffers();
}

CExplosionEffect::~CExplosionEffect()
{
    for(Debris& debris : m_Debris)
        if(debris.pObj)
            debris.pObj->Release();
    m_Debris.clear();
}

void CExplosionEffect::Initialize(ID3D12Device* pd3dDevice, CShader* pShader, XMFLOAT3 center, float fWorldScale)
{
    m_Center = center;
    m_fScale = fWorldScale;

    srand(2026);
    m_Debris.reserve(DEBRIS_CNT);
    for(int i = 0; i < DEBRIS_CNT; ++i)
    {
        CGameObject* pObj = new CGameObject();
        pObj->SetMesh(s_pDebrisMesh);
        pObj->SetShader(pShader);
        pObj->CreateShaderVariables(pd3dDevice, nullptr);
        XMStoreFloat4x4(&pObj->m_xmf4x4World,
                        XMMatrixTranslation(center.x, center.y, center.z));

        Debris debris;
        debris.pObj = pObj;
        debris.dir = RandomSphereVec();
        debris.spin = XMFLOAT3((rand() / (float)RAND_MAX) * 6.0f,
                               (rand() / (float)RAND_MAX) * 6.0f,
                               (rand() / (float)RAND_MAX) * 6.0f);
        m_Debris.push_back(debris);
    }
}

int CExplosionEffect::Update(float dt)
{
    if(m_fTimer >= BLOW_DUR)
        return OBJ_DEAD;

    m_fTimer += dt;
    float elapsed = m_fTimer;
    float scale = (1.0f - (elapsed / BLOW_DUR)) * m_fScale;
    for(Debris& debris : m_Debris)
    {
        XMFLOAT3 position = Vector3::Add(m_Center, debris.dir, DEBRIS_SPD * m_fScale * elapsed);
        XMMATRIX world = XMMatrixScaling(scale, scale, scale) *
            XMMatrixRotationRollPitchYaw(debris.spin.x * elapsed, debris.spin.y * elapsed, debris.spin.z * elapsed) *
            XMMatrixTranslation(position.x, position.y, position.z);
        XMStoreFloat4x4(&debris.pObj->m_xmf4x4World, world);
    }
    return OBJ_NOEVENT;
}

void CExplosionEffect::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
    if(m_fTimer >= BLOW_DUR)
        return;

    for(Debris& debris : m_Debris)
        if(debris.pObj)
            debris.pObj->Render(pd3dCommandList, pCamera);
}
