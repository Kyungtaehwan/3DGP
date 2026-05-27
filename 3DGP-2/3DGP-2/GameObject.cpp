#include "pch.h"
#include "GameObject.h"
#include "Shader.h"
#include "Camera.h"

// ============================================================
// CGameObject
// ============================================================

CGameObject::CGameObject()
{
    m_xmf4x4World = Matrix4x4::Identity();
}

CGameObject::~CGameObject()
{
    Release();
}

void CGameObject::Release()
{
    ReleaseShaderVariables();

    if (m_pMesh)
    {

        delete m_pMesh;
        m_pMesh = NULL;
    }
    m_pShader = NULL;
}

void CGameObject::CreateShaderVariables(ID3D12Device* pd3dDevice,
                                         ID3D12GraphicsCommandList* pd3dCommandList)
{
    UINT cbSize = (sizeof(CB_WORLD_MATRIX) + 255) & ~255;

    m_pd3dcbWorldMatrix = ::CreateBufferResource(
        pd3dDevice, pd3dCommandList,
        NULL, cbSize,
        D3D12_HEAP_TYPE_UPLOAD,
        D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
        NULL);

    m_pd3dcbWorldMatrix->Map(0, NULL, (void**)&m_pcbMappedWorldMatrix);
}

void CGameObject::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
    XMFLOAT4X4 xmf4x4World;
    XMStoreFloat4x4(&xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4World)));
    ::memcpy(&m_pcbMappedWorldMatrix->m_xmf4x4World, &xmf4x4World, sizeof(XMFLOAT4X4));

    // Root parameter 1 -> b1
    D3D12_GPU_VIRTUAL_ADDRESS cbGpuAddress = m_pd3dcbWorldMatrix->GetGPUVirtualAddress();
    pd3dCommandList->SetGraphicsRootConstantBufferView(1, cbGpuAddress);
}

void CGameObject::ReleaseShaderVariables()
{
    if (m_pd3dcbWorldMatrix)
    {
        m_pd3dcbWorldMatrix->Unmap(0, NULL);
        m_pd3dcbWorldMatrix->Release();
        m_pd3dcbWorldMatrix    = NULL;
        m_pcbMappedWorldMatrix = NULL;
    }
}

void CGameObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
    if (!m_bActive) return;

    if (m_pShader)
        m_pShader->OnPrepareRender(pd3dCommandList);

    if (m_pd3dcbWorldMatrix)
        UpdateShaderVariables(pd3dCommandList);

    if (m_pMesh)
        m_pMesh->Render(pd3dCommandList);
}

void CGameObject::SetPosition(float x, float y, float z)
{
    m_xmf4x4World._41 = x;
    m_xmf4x4World._42 = y;
    m_xmf4x4World._43 = z;
}

void CGameObject::SetPosition(XMFLOAT3 xmf3Position)
{
    SetPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z);
}

XMFLOAT3 CGameObject::GetPosition() const
{
    return XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43);
}

XMFLOAT3 CGameObject::GetLook() const
{
    
    return Vector3::Normalize(XMFLOAT3(m_xmf4x4World._31, m_xmf4x4World._32, m_xmf4x4World._33));
}

XMFLOAT3 CGameObject::GetUp() const
{
    return Vector3::Normalize(XMFLOAT3(m_xmf4x4World._21, m_xmf4x4World._22, m_xmf4x4World._23));
}

XMFLOAT3 CGameObject::GetRight() const
{
    return Vector3::Normalize(XMFLOAT3(m_xmf4x4World._11, m_xmf4x4World._12, m_xmf4x4World._13));
}

void CGameObject::MoveForward(float fDistance)
{
    XMFLOAT3 xmf3Look = GetLook();
    SetPosition(Vector3::Add(GetPosition(), xmf3Look, fDistance));
}

void CGameObject::MoveStrafe(float fDistance)
{
    XMFLOAT3 xmf3Right = GetRight();
    SetPosition(Vector3::Add(GetPosition(), xmf3Right, fDistance));
}

void CGameObject::MoveUp(float fDistance)
{
    XMFLOAT3 xmf3Up = GetUp();
    SetPosition(Vector3::Add(GetPosition(), xmf3Up, fDistance));
}

void CGameObject::Rotate(float fPitch, float fYaw, float fRoll)
{
    XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(
        DegreeToRadian(fPitch), DegreeToRadian(fYaw), DegreeToRadian(fRoll));
    m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}

void CGameObject::Late_Update(float dt)
{
    UpdateBoundingBox();
}

void CGameObject::UpdateBoundingBox()
{
    if (m_xmLocalOBB.Extents.x <= 0.f &&
        m_xmLocalOBB.Extents.y <= 0.f &&
        m_xmLocalOBB.Extents.z <= 0.f)
        return;

    m_xmLocalOBB.Transform(m_xmOOBB, XMLoadFloat4x4(&m_xmf4x4World));
    XMStoreFloat4(&m_xmOOBB.Orientation,
        XMQuaternionNormalize(XMLoadFloat4(&m_xmOOBB.Orientation)));
}

void CGameObject::OnHit(int nDamage)
{
    m_iHP -= nDamage;
    if (m_iHP <= 0)
    {
        m_iHP   = 0;
        m_bDead = true;
    }
}
