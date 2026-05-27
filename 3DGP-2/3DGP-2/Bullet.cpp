#include "pch.h"
#include "Bullet.h"
#include "Mesh.h"
#include "Camera.h"

XMFLOAT3 CBullet::s_SphereVecs[CBullet::DEBRIS_CNT];
CMesh*   CBullet::s_pBulletMesh    = nullptr;
CMesh*   CBullet::s_pDebrisMesh    = nullptr;
CShader* CBullet::s_pSharedShader  = nullptr;


static XMFLOAT3 RandomSphereVec()
{
    while (true)
    {
        float x = (rand() / (float)RAND_MAX) * 2.f - 1.f;
        float y = (rand() / (float)RAND_MAX) * 2.f - 1.f;
        float z = (rand() / (float)RAND_MAX) * 2.f - 1.f;
        float len = sqrtf(x*x + y*y + z*z);
        if (len > 0.001f && len <= 1.f)
            return { x/len, y/len, z/len };
    }
}

void CBullet::PrepareShared(ID3D12Device* pd3dDevice,
                             ID3D12GraphicsCommandList* pd3dCommandList,
                             CShader* pShader)
{
    s_pSharedShader = pShader;

    srand(12345);
    for (int i = 0; i < DEBRIS_CNT; ++i)
        s_SphereVecs[i] = RandomSphereVec();

    s_pBulletMesh = new CCubeMesh(pd3dDevice, pd3dCommandList,
        0.05f, 0.05f, 0.10f, XMFLOAT4(1.f, 1.f, 0.f, 1.f));

    s_pDebrisMesh = new CCubeMesh(pd3dDevice, pd3dCommandList,
        0.07f, 0.07f, 0.07f, XMFLOAT4(1.f, 0.5f, 0.f, 1.f));
}

void CBullet::ReleaseShared()
{
    if (s_pBulletMesh) { delete s_pBulletMesh; s_pBulletMesh = nullptr; }
    if (s_pDebrisMesh) { delete s_pDebrisMesh; s_pDebrisMesh = nullptr; }
    s_pSharedShader = nullptr;
}

void CBullet::ReleaseSharedUploadBuffers()
{
    if (s_pBulletMesh) s_pBulletMesh->ReleaseUploadBuffers();
    if (s_pDebrisMesh) s_pDebrisMesh->ReleaseUploadBuffers();
}

void CBullet::Fire(ID3D12Device* pd3dDevice,
                   XMFLOAT3 vPos, XMFLOAT3 vDir,
                   CShader* pShader, float fSpeed)
{
    m_vPos       = vPos;
    m_vDir       = Vector3::Normalize(vDir);
    m_fSpeed     = fSpeed;
    m_pShader    = pShader;
    m_bBlowingUp = false;
    m_bActive    = true;
    m_fTravelled = 0.f;
    m_fBlowTimer = 0.f;

    XMStoreFloat4x4(&m_xmf4x4World,
        XMMatrixTranslation(vPos.x, vPos.y, vPos.z));

    m_xmLocalOBB = BoundingOrientedBox(
        XMFLOAT3(0.f, 0.f, 0.f),
        XMFLOAT3(0.05f, 0.05f, 0.10f),
        XMFLOAT4(0.f, 0.f, 0.f, 1.f));

    if (!m_pd3dcbWorldMatrix)
        CGameObject::CreateShaderVariables(pd3dDevice, nullptr);

    if (!m_pDebrisCB)
    {
        constexpr UINT SLOT  = (sizeof(XMFLOAT4X4) + 255) & ~255;
        constexpr UINT TOTAL = SLOT * DEBRIS_CNT;

        D3D12_HEAP_PROPERTIES hp = {};
        hp.Type = D3D12_HEAP_TYPE_UPLOAD;

        D3D12_RESOURCE_DESC rd = {};
        rd.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;
        rd.Width              = TOTAL;
        rd.Height             = 1;
        rd.DepthOrArraySize   = 1;
        rd.MipLevels          = 1;
        rd.Format             = DXGI_FORMAT_UNKNOWN;
        rd.SampleDesc.Count   = 1;
        rd.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

        pd3dDevice->CreateCommittedResource(
            &hp, D3D12_HEAP_FLAG_NONE, &rd,
            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
            __uuidof(ID3D12Resource), (void**)&m_pDebrisCB);

        m_pDebrisCB->Map(0, nullptr, (void**)&m_pMappedDebris);
    }
}


void CBullet::SetBlowingUp()
{
    m_bBlowingUp = true;
    m_bActive    = false;
    m_fBlowTimer = 0.f;

    for (int i = 0; i < DEBRIS_CNT; ++i)
    {
        m_DebrisTx[i] = Matrix4x4::Identity();
        m_DebrisTx[i]._41 = m_vPos.x;
        m_DebrisTx[i]._42 = m_vPos.y;
        m_DebrisTx[i]._43 = m_vPos.z;
    }
}


int CBullet::Update(float dt)
{
    if (m_bBlowingUp)
    {
        m_fBlowTimer += dt;
        if (m_fBlowTimer >= BLOW_DUR)
        {
            m_bBlowingUp = false;
            m_bActive    = false;
            return OBJ_DEAD;
        }

        for (int i = 0; i < DEBRIS_CNT; ++i)
        {
            float t = m_fBlowTimer;
            m_DebrisTx[i] = Matrix4x4::Identity();
            m_DebrisTx[i]._41 = m_vPos.x + s_SphereVecs[i].x * DEBRIS_SPD * t;
            m_DebrisTx[i]._42 = m_vPos.y + s_SphereVecs[i].y * DEBRIS_SPD * t;
            m_DebrisTx[i]._43 = m_vPos.z + s_SphereVecs[i].z * DEBRIS_SPD * t;
        }
        return OBJ_NOEVENT;
    }

    if (!m_bActive) return OBJ_DEAD;


    float fDist = m_fSpeed * dt;
    m_vPos.x += m_vDir.x * fDist;
    m_vPos.y += m_vDir.y * fDist;
    m_vPos.z += m_vDir.z * fDist;
    m_fTravelled += fDist;

    XMStoreFloat4x4(&m_xmf4x4World,
        XMMatrixTranslation(m_vPos.x, m_vPos.y, m_vPos.z));

    CMap_Manager* pMap = CMap_Manager::Get_Instance();
    if (pMap)
    {
        int r = (int)floorf(m_vPos.z / TILE_SCALE);
        int c = (int)floorf(m_vPos.x / TILE_SCALE);
        if (!pMap->IsPassable(r, c))
        {
            SetBlowingUp();
            return OBJ_NOEVENT;
        }
    }

    if (m_fTravelled > MAX_RANGE)
    {
        m_bActive = false;
        return OBJ_DEAD;
    }

    return OBJ_NOEVENT;
}


void CBullet::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
    if (!m_bActive && !m_bBlowingUp) return;
    if (!m_pd3dcbWorldMatrix) return;

    CShader* pShader = m_pShader ? m_pShader : s_pSharedShader;
    if (!pShader) return;
    pShader->OnPrepareRender(pd3dCommandList);

    if (!m_bBlowingUp)
    {
        UpdateShaderVariables(pd3dCommandList);
        if (s_pBulletMesh) s_pBulletMesh->Render(pd3dCommandList);
    }
    else
    {
        if (s_pDebrisMesh && m_pDebrisCB)
        {
            constexpr UINT SLOT = (sizeof(XMFLOAT4X4) + 255) & ~255;
            D3D12_GPU_VIRTUAL_ADDRESS base = m_pDebrisCB->GetGPUVirtualAddress();

            for (int i = 0; i < DEBRIS_CNT; ++i)
            {
                XMFLOAT4X4 transposed;
                XMStoreFloat4x4(&transposed,
                    XMMatrixTranspose(XMLoadFloat4x4(&m_DebrisTx[i])));
                memcpy(m_pMappedDebris + i * SLOT, &transposed, sizeof(XMFLOAT4X4));

                pd3dCommandList->SetGraphicsRootConstantBufferView(1, base + i * SLOT);
                s_pDebrisMesh->Render(pd3dCommandList);
            }
        }
    }
}


void CBullet::Release()
{
    if (m_pDebrisCB)
    {
        m_pDebrisCB->Unmap(0, nullptr);
        m_pDebrisCB->Release();
        m_pDebrisCB     = nullptr;
        m_pMappedDebris = nullptr;
    }
    m_pMesh   = nullptr;
    m_pShader = nullptr;
    CGameObject::Release();
}

void CBullet::ReleaseUploadBuffers()
{
    if (s_pBulletMesh) s_pBulletMesh->ReleaseUploadBuffers();
    if (s_pDebrisMesh) s_pDebrisMesh->ReleaseUploadBuffers();
}
