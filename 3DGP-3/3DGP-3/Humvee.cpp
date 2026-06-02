#include "pch.h"
#include "Humvee.h"
#include "Terrain.h"

CHumvee::CHumvee() {}

CHumvee::~CHumvee()
{
    if (m_pModel) { m_pModel->Release(); m_pModel = NULL; }
}

bool CHumvee::LoadModel(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
                        CShader* pShader, const char* pstrFileName)
{
    m_pModel = CGameObject::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, pstrFileName, pShader);
    if (!m_pModel) return false;
    m_pModel->AddRef();
    m_pModel->CreateShaderVariables(pd3dDevice, pd3dCommandList);
    return true;
}

// Recursively tint every material in a frame hierarchy.
static void TintHierarchy(CGameObject* p, const XMFLOAT4& diffuse)
{
    if (!p) return;
    for (int i = 0; i < p->m_nMaterials; ++i)
    {
        if (p->m_ppMaterials[i] && p->m_ppMaterials[i]->m_pMaterialColors)
        {
            CMaterialColors* c = p->m_ppMaterials[i]->m_pMaterialColors;
            c->m_xmf4Diffuse = diffuse;
            c->m_xmf4Ambient = XMFLOAT4(diffuse.x * 0.35f, diffuse.y * 0.35f,
                                        diffuse.z * 0.35f, 1.0f);
        }
    }
    TintHierarchy(p->m_pSibling, diffuse);
    TintHierarchy(p->m_pChild,   diffuse);
}

void CHumvee::SetBodyColor(XMFLOAT4 xmf4Color)
{
    if (m_pModel) TintHierarchy(m_pModel, xmf4Color);
}

void CHumvee::SetPath(XMFLOAT3 start, XMFLOAT3 end, float fSpeed, float fScale)
{
    m_xmf3Pos = start;
    m_xmf3End = end;
    m_fSpeed  = fSpeed;
    m_fScale  = fScale;
    m_bArrived = false;

    // Heading from start -> end (forward = (sin yaw, 0, cos yaw)).
    float dx = end.x - start.x, dz = end.z - start.z;
    m_fYaw = XMConvertToDegrees(atan2f(dx, dz));

    Update(0.0f);   // place on the surface for frame 0
}

int CHumvee::Update(float fTimeElapsed)
{
    if (!m_pModel || !m_pTerrain) return OBJ_NOEVENT;

    // Advance toward the destination along the (fixed) heading, then stop.
    float yawR = XMConvertToRadians(m_fYaw);
    XMFLOAT3 fwd = XMFLOAT3(sinf(yawR), 0.0f, cosf(yawR));

    if (!m_bArrived)
    {
        float dx = m_xmf3End.x - m_xmf3Pos.x, dz = m_xmf3End.z - m_xmf3Pos.z;
        float distLeft = sqrtf(dx * dx + dz * dz);
        float stepLen  = m_fSpeed * fTimeElapsed;

        if (stepLen >= distLeft)   // would reach/overshoot -> snap and stop
        {
            m_xmf3Pos.x = m_xmf3End.x;
            m_xmf3Pos.z = m_xmf3End.z;
            m_bArrived  = true;
        }
        else
        {
            m_xmf3Pos = Vector3::Add(m_xmf3Pos, fwd, stepLen);
        }
    }

    float    h  = m_pTerrain->GetHeight(m_xmf3Pos.x, m_xmf3Pos.z);
    XMFLOAT3 up = Vector3::Normalize(m_pTerrain->GetNormal(m_xmf3Pos.x, m_xmf3Pos.z));

    yawR = XMConvertToRadians(m_fYaw);
    XMFLOAT3 look  = XMFLOAT3(sinf(yawR), 0.0f, cosf(yawR));
    XMFLOAT3 right = Vector3::Normalize(Vector3::CrossProduct(up, look));   // up x look
    look           = Vector3::Normalize(Vector3::CrossProduct(right, up));  // re-orthonormalize

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

void CHumvee::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
    if (m_pModel) m_pModel->Render(pd3dCommandList, pCamera);
}

void CHumvee::ReleaseUploadBuffers()
{
    if (m_pModel) m_pModel->ReleaseUploadBuffers();
}
