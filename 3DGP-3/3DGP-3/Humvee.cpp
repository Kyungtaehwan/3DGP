#include "pch.h"
#include "Humvee.h"
#include "Terrain.h"
#include "ExplosionEffect.h"
#include "Object_Manager.h"

CHumvee::CHumvee() {}

CHumvee::~CHumvee()
{
    if(m_pModel)
    {
        m_pModel->Release();
        m_pModel = NULL;
    }
}

bool CHumvee::LoadModel(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
                        CShader* pShader, const char* pstrFileName)
{
    m_pModel = CGameObject::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, pstrFileName, pShader);
    if(!m_pModel)
        return false;
    m_pModel->AddRef();
    m_pModel->CreateShaderVariables(pd3dDevice, pd3dCommandList);
    return true;
}

static void TintHierarchy(CGameObject* pFrame, const XMFLOAT4& diffuse)
{
    if(!pFrame)
        return;
    for(int i = 0; i < pFrame->m_nMaterials; ++i)
    {
        if(pFrame->m_ppMaterials[i] && pFrame->m_ppMaterials[i]->m_pMaterialColors)
        {
            CMaterialColors* pColors = pFrame->m_ppMaterials[i]->m_pMaterialColors;
            pColors->m_xmf4Diffuse = diffuse;
            pColors->m_xmf4Ambient = XMFLOAT4(diffuse.x * 0.35f, diffuse.y * 0.35f,
                                              diffuse.z * 0.35f, 1.0f);
        }
    }
    TintHierarchy(pFrame->m_pSibling, diffuse);
    TintHierarchy(pFrame->m_pChild, diffuse);
}

void CHumvee::SetBodyColor(XMFLOAT4 xmf4Color)
{
    if(m_pModel)
        TintHierarchy(m_pModel, xmf4Color);
}

void CHumvee::SetPath(XMFLOAT3 start, XMFLOAT3 end, float fSpeed, float fScale)
{
    m_xmf3Pos = start;
    m_xmf3End = end;
    m_fSpeed = fSpeed;
    m_fScale = fScale;
    m_bArrived = false;

    float dx = end.x - start.x, dz = end.z - start.z;
    m_fYaw = XMConvertToDegrees(atan2f(dx, dz));

    Update(0.0f);
}

int CHumvee::Update(float fTimeElapsed)
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
            pFx->Initialize(m_pd3dDevice, m_pColorShader, XMFLOAT3(m_xmf3Pos.x, groundHeight + 4.0f, m_xmf3Pos.z), 130.0f);
            CObject_Manager::Get_Instance()->Add_Object(OBJ_EFFECT, pFx);
        }
        return OBJ_DEAD;
    }

    float yawR = XMConvertToRadians(m_fYaw);
    XMFLOAT3 fwd = XMFLOAT3(sinf(yawR), 0.0f, cosf(yawR));

    if(!m_bArrived)
    {
        float dx = m_xmf3End.x - m_xmf3Pos.x, dz = m_xmf3End.z - m_xmf3Pos.z;
        float distLeft = sqrtf(dx * dx + dz * dz);
        float stepLen = m_fSpeed * fTimeElapsed;

        if(stepLen >= distLeft)
        {
            m_xmf3Pos.x = m_xmf3End.x;
            m_xmf3Pos.z = m_xmf3End.z;
            m_bArrived = true;
        }
        else
        {
            m_xmf3Pos = Vector3::Add(m_xmf3Pos, fwd, stepLen);
        }
    }

    float groundHeight = m_pTerrain->GetHeight(m_xmf3Pos.x, m_xmf3Pos.z);
    XMFLOAT3 up = Vector3::Normalize(m_pTerrain->GetNormal(m_xmf3Pos.x, m_xmf3Pos.z));

    m_xmf3Pos.y = groundHeight;

    yawR = XMConvertToRadians(m_fYaw);
    XMFLOAT3 look = XMFLOAT3(sinf(yawR), 0.0f, cosf(yawR));
    XMFLOAT3 right = Vector3::Normalize(Vector3::CrossProduct(up, look));
    look = Vector3::Normalize(Vector3::CrossProduct(right, up));

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
    m_xmOOBB.Extents = XMFLOAT3(5.0f, 4.0f, 8.0f);

    return OBJ_NOEVENT;
}

void CHumvee::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
    if(m_pModel)
        m_pModel->Render(pd3dCommandList, pCamera);
}

void CHumvee::ReleaseUploadBuffers()
{
    if(m_pModel)
        m_pModel->ReleaseUploadBuffers();
}
