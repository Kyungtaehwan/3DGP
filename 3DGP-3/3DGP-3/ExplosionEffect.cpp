#include "pch.h"
#include "ExplosionEffect.h"
#include "Mesh.h"

CMesh* CExplosionEffect::s_pDebrisMesh = nullptr;

// 단위 구면 위의 임의 방향 (3DGP-2 Bullet 의 데브리 방식과 동일).
static XMFLOAT3 RandomSphereVec()
{
    while (true)
    {
        float x = (rand() / (float)RAND_MAX) * 2.0f - 1.0f;
        float y = (rand() / (float)RAND_MAX) * 2.0f - 1.0f;
        float z = (rand() / (float)RAND_MAX) * 2.0f - 1.0f;
        float len = sqrtf(x * x + y * y + z * z);
        if (len > 0.001f && len <= 1.0f) return { x / len, y / len, z / len };
    }
}

void CExplosionEffect::PrepareShared(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
    if (s_pDebrisMesh) return;
    s_pDebrisMesh = new CColorCubeMesh(pd3dDevice, pd3dCommandList,
        0.05f, 0.05f, 0.05f, XMFLOAT4(1.0f, 0.55f, 0.10f, 1.0f));
    s_pDebrisMesh->AddRef();
}

void CExplosionEffect::ReleaseShared()
{
    if (s_pDebrisMesh) { s_pDebrisMesh->Release(); s_pDebrisMesh = nullptr; }
}

void CExplosionEffect::ReleaseSharedUploadBuffers()
{
    if (s_pDebrisMesh) s_pDebrisMesh->ReleaseUploadBuffers();
}

CExplosionEffect::~CExplosionEffect()
{
    for (Debris& d : m_Debris) if (d.pObj) d.pObj->Release();
    m_Debris.clear();
}

void CExplosionEffect::Init(ID3D12Device* pd3dDevice, CShader* pShader, XMFLOAT3 center)
{
    m_Center = center;

    srand(2026);
    m_Debris.reserve(DEBRIS_CNT);
    for (int i = 0; i < DEBRIS_CNT; ++i)
    {
        CGameObject* pObj = new CGameObject();
        pObj->SetMesh(s_pDebrisMesh);
        pObj->SetShader(pShader);
        pObj->CreateShaderVariables(pd3dDevice, nullptr);   // CB only — no command list needed
        XMStoreFloat4x4(&pObj->m_xmf4x4World,
            XMMatrixTranslation(center.x, center.y, center.z));

        Debris d;
        d.pObj = pObj;
        d.dir  = RandomSphereVec();
        d.spin = XMFLOAT3((rand() / (float)RAND_MAX) * 6.0f,
                          (rand() / (float)RAND_MAX) * 6.0f,
                          (rand() / (float)RAND_MAX) * 6.0f);
        m_Debris.push_back(d);
    }
}

int CExplosionEffect::Update(float dt)
{
    if (m_fTimer >= BLOW_DUR) return OBJ_DEAD;   // 끝났으면 더 움직이지 않는다

    m_fTimer += dt;
    float t = m_fTimer;
    float s = 1.0f - (t / BLOW_DUR);             // 시간이 지날수록 조각이 작아진다
    for (Debris& d : m_Debris)
    {
        XMFLOAT3 p = Vector3::Add(m_Center, d.dir, DEBRIS_SPD * t);
        XMMATRIX w = XMMatrixScaling(s, s, s) *
                     XMMatrixRotationRollPitchYaw(d.spin.x * t, d.spin.y * t, d.spin.z * t) *
                     XMMatrixTranslation(p.x, p.y, p.z);
        XMStoreFloat4x4(&d.pObj->m_xmf4x4World, w);
    }
    return OBJ_NOEVENT;
}

void CExplosionEffect::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
    if (m_fTimer >= BLOW_DUR) return;            // 끝났으면 그리지 않는다
    for (Debris& d : m_Debris) if (d.pObj) d.pObj->Render(pd3dCommandList, pCamera);
}
