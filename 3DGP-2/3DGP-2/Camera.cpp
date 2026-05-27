#include "pch.h"
#include "Player.h"
#include "Camera.h"

// ============================================================
// CCamera
// ============================================================

CCamera::CCamera()
{
    m_xmf4x4View        = Matrix4x4::Identity();
    m_xmf4x4Projection  = Matrix4x4::Identity();

    m_d3dViewport  = { 0.0f, 0.0f, (float)FRAME_BUFFER_WIDTH, (float)FRAME_BUFFER_HEIGHT, 0.0f, 1.0f };
    m_d3dScissorRect = { 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT };

    m_pcbMappedCamera = NULL;
    m_pd3dcbCamera    = NULL;
}

CCamera::CCamera(CCamera* pCamera)
{
    if (pCamera)
    {
        *this = *pCamera;
    }
    else
    {
        m_xmf4x4View        = Matrix4x4::Identity();
        m_xmf4x4Projection  = Matrix4x4::Identity();
        m_d3dViewport       = { 0.0f, 0.0f, (float)FRAME_BUFFER_WIDTH, (float)FRAME_BUFFER_HEIGHT, 0.0f, 1.0f };
        m_d3dScissorRect    = { 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT };
    }
    m_pcbMappedCamera = NULL;
    m_pd3dcbCamera    = NULL;
}

CCamera::~CCamera()
{
    ReleaseShaderVariables();
}

void CCamera::CreateShaderVariables(ID3D12Device* pd3dDevice,
                                    ID3D12GraphicsCommandList* pd3dCommandList)
{
    UINT cbSize = (sizeof(VS_CB_CAMERA_INFO) + 255) & ~255;

    m_pd3dcbCamera = ::CreateBufferResource(
        pd3dDevice, pd3dCommandList,
        NULL, cbSize,
        D3D12_HEAP_TYPE_UPLOAD,
        D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
        NULL);

    m_pd3dcbCamera->Map(0, NULL, (void**)&m_pcbMappedCamera);
}

void CCamera::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
    XMFLOAT4X4 xmf4x4View;
    XMStoreFloat4x4(&xmf4x4View, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4View)));
    XMFLOAT4X4 xmf4x4Projection;
    XMStoreFloat4x4(&xmf4x4Projection, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4Projection)));

    ::memcpy(&m_pcbMappedCamera->m_xmf4x4View,       &xmf4x4View,       sizeof(XMFLOAT4X4));
    ::memcpy(&m_pcbMappedCamera->m_xmf4x4Projection, &xmf4x4Projection, sizeof(XMFLOAT4X4));
    ::memcpy(&m_pcbMappedCamera->m_xmf3Position,     &m_xmf3Position,   sizeof(XMFLOAT3));
    m_pcbMappedCamera->padding = 0.0f;

    // Root parameter 0
    D3D12_GPU_VIRTUAL_ADDRESS cbGpuAddress = m_pd3dcbCamera->GetGPUVirtualAddress();
    pd3dCommandList->SetGraphicsRootConstantBufferView(0, cbGpuAddress);
}

void CCamera::ReleaseShaderVariables()
{
    if (m_pd3dcbCamera)
    {
        m_pd3dcbCamera->Unmap(0, NULL);
        m_pd3dcbCamera->Release();
        m_pd3dcbCamera    = NULL;
        m_pcbMappedCamera = NULL;
    }
}

void CCamera::SetViewport(int xTopLeft, int yTopLeft, int nWidth, int nHeight,
                          float fMinZ, float fMaxZ)
{
    m_d3dViewport.TopLeftX = (float)xTopLeft;
    m_d3dViewport.TopLeftY = (float)yTopLeft;
    m_d3dViewport.Width    = (float)nWidth;
    m_d3dViewport.Height   = (float)nHeight;
    m_d3dViewport.MinDepth = fMinZ;
    m_d3dViewport.MaxDepth = fMaxZ;
}

void CCamera::SetScissorRect(LONG xLeft, LONG yTop, LONG xRight, LONG yBottom)
{
    m_d3dScissorRect.left   = xLeft;
    m_d3dScissorRect.top    = yTop;
    m_d3dScissorRect.right  = xRight;
    m_d3dScissorRect.bottom = yBottom;
}

void CCamera::SetViewportsAndScissorRects(ID3D12GraphicsCommandList* pd3dCommandList)
{
    pd3dCommandList->RSSetViewports(1, &m_d3dViewport);
    pd3dCommandList->RSSetScissorRects(1, &m_d3dScissorRect);
}

void CCamera::GenerateViewMatrix()
{
    m_xmf4x4View = Matrix4x4::LookToLH(m_xmf3Position, m_xmf3Look, m_xmf3Up);
}

void CCamera::RegenerateViewMatrix()
{
    m_xmf3Look  = Vector3::Normalize(m_xmf3Look);
    m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look);
    m_xmf3Right = Vector3::Normalize(m_xmf3Right);
    m_xmf3Up    = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right);
    m_xmf3Up    = Vector3::Normalize(m_xmf3Up);

    GenerateViewMatrix();
}

void CCamera::GenerateProjectionMatrix(float fNearPlaneDistance, float fFarPlaneDistance,
                                       float fAspectRatio, float fFOVAngle)
{
    m_fNearPlane    = fNearPlaneDistance;
    m_fFarPlane     = fFarPlaneDistance;
    m_fAspectRatio  = fAspectRatio;
    m_fFOVAngle     = fFOVAngle;

    m_xmf4x4Projection = Matrix4x4::PerspectiveFovLH(
        DegreeToRadian(fFOVAngle), fAspectRatio,
        fNearPlaneDistance, fFarPlaneDistance);
}

void CCamera::SetLookAtMatrix(XMFLOAT3 xmf3LookAt, XMFLOAT3 xmf3Up)
{
    m_xmf4x4View = Matrix4x4::LookAtLH(m_xmf3Position, xmf3LookAt, xmf3Up);
}

void CCamera::Rotate(float fPitch, float fYaw, float fRoll)
{
    if (!IsZero(fPitch))
    {
        // 피치 클램핑: Look과 Up이 평행해지는 것 방지 (LookToLH 크래시 원인)
        m_fPitch += fPitch;
        if (m_fPitch >  89.0f) { fPitch -= (m_fPitch -  89.0f); m_fPitch =  89.0f; }
        if (m_fPitch < -89.0f) { fPitch -= (m_fPitch + 89.0f);  m_fPitch = -89.0f; }

        XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Right), DegreeToRadian(fPitch));
        m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, mtxRotate);
        m_xmf3Up   = Vector3::TransformNormal(m_xmf3Up,   mtxRotate);
    }
    if (!IsZero(fYaw))
    {
        XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), DegreeToRadian(fYaw));
        m_xmf3Look  = Vector3::TransformNormal(m_xmf3Look,  mtxRotate);
        m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, mtxRotate);
    }
    if (!IsZero(fRoll))
    {
        XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Look), DegreeToRadian(fRoll));
        m_xmf3Up    = Vector3::TransformNormal(m_xmf3Up,    mtxRotate);
        m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, mtxRotate);
    }
    RegenerateViewMatrix();
}



CSpaceShipCamera::CSpaceShipCamera(CCamera* pCamera) : CCamera(pCamera)
{
    m_nMode = SPACESHIP_CAMERA;
}

void CSpaceShipCamera::Update(XMFLOAT3& xmf3LookAt, float fTimeElapsed)
{
    if (m_pPlayer)
    {
        XMFLOAT4X4 xmf4x4Rotate;
        XMFLOAT3 xmf3Right  = m_pPlayer->GetRightVector();
        XMFLOAT3 xmf3Up     = m_pPlayer->GetUpVector();
        XMFLOAT3 xmf3Look   = m_pPlayer->GetLookVector();

        xmf4x4Rotate._11 = xmf3Right.x; xmf4x4Rotate._21 = xmf3Up.x; xmf4x4Rotate._31 = xmf3Look.x;
        xmf4x4Rotate._12 = xmf3Right.y; xmf4x4Rotate._22 = xmf3Up.y; xmf4x4Rotate._32 = xmf3Look.y;
        xmf4x4Rotate._13 = xmf3Right.z; xmf4x4Rotate._23 = xmf3Up.z; xmf4x4Rotate._33 = xmf3Look.z;
        xmf4x4Rotate._14 = 0.0f; xmf4x4Rotate._24 = 0.0f; xmf4x4Rotate._34 = 0.0f;
        xmf4x4Rotate._41 = 0.0f; xmf4x4Rotate._42 = 0.0f; xmf4x4Rotate._43 = 0.0f; xmf4x4Rotate._44 = 1.0f;

        XMFLOAT3 xmf3Offset = Vector3::TransformCoord(m_xmf3Offset, xmf4x4Rotate);
        XMFLOAT3 xmf3Position = Vector3::Add(m_pPlayer->GetPosition(), xmf3Offset);

        // fTimeElapsed==0 (초기화 시)에는 즉시 이동, 이후는 lag 보간
        if (m_fTimeLag > 0.0f && fTimeElapsed > 0.0f)
        {
            float fAlpha = fTimeElapsed / m_fTimeLag;
            m_xmf3Position.x += (xmf3Position.x - m_xmf3Position.x) * fAlpha;
            m_xmf3Position.y += (xmf3Position.y - m_xmf3Position.y) * fAlpha;
            m_xmf3Position.z += (xmf3Position.z - m_xmf3Position.z) * fAlpha;
        }
        else
        {
            m_xmf3Position = xmf3Position;
        }
        RegenerateViewMatrix();
    }
}

void CSpaceShipCamera::SetLookAt(XMFLOAT3& xmf3LookAt)
{
    XMFLOAT4X4 view = Matrix4x4::LookAtLH(m_xmf3Position, xmf3LookAt, m_pPlayer ? m_pPlayer->GetUpVector() : XMFLOAT3(0,1,0));
    m_xmf3Right = XMFLOAT3(view._11, view._21, view._31);
    m_xmf3Up    = XMFLOAT3(view._12, view._22, view._32);
    m_xmf3Look  = XMFLOAT3(view._13, view._23, view._33);
}


CFirstPersonCamera::CFirstPersonCamera(CCamera* pCamera) : CCamera(pCamera)
{
    m_nMode = FIRST_PERSON_CAMERA;
}

void CFirstPersonCamera::Update(XMFLOAT3& xmf3LookAt, float fTimeElapsed)
{
    if (m_pPlayer)
    {
        XMFLOAT4X4 xmf4x4Rotate;
        XMFLOAT3 xmf3Right  = m_pPlayer->GetRightVector();
        XMFLOAT3 xmf3Up     = m_pPlayer->GetUpVector();
        XMFLOAT3 xmf3Look   = m_pPlayer->GetLookVector();

        xmf4x4Rotate._11 = xmf3Right.x; xmf4x4Rotate._21 = xmf3Up.x; xmf4x4Rotate._31 = xmf3Look.x;
        xmf4x4Rotate._12 = xmf3Right.y; xmf4x4Rotate._22 = xmf3Up.y; xmf4x4Rotate._32 = xmf3Look.y;
        xmf4x4Rotate._13 = xmf3Right.z; xmf4x4Rotate._23 = xmf3Up.z; xmf4x4Rotate._33 = xmf3Look.z;
        xmf4x4Rotate._14 = 0.0f; xmf4x4Rotate._24 = 0.0f; xmf4x4Rotate._34 = 0.0f;
        xmf4x4Rotate._41 = 0.0f; xmf4x4Rotate._42 = 0.0f; xmf4x4Rotate._43 = 0.0f; xmf4x4Rotate._44 = 1.0f;

        XMFLOAT3 xmf3Offset   = Vector3::TransformCoord(m_xmf3Offset, xmf4x4Rotate);
        m_xmf3Position        = Vector3::Add(m_pPlayer->GetPosition(), xmf3Offset);
        m_xmf3Right           = xmf3Right;
        m_xmf3Up              = xmf3Up;
        m_xmf3Look            = xmf3Look;
        RegenerateViewMatrix();
    }
}

void CFirstPersonCamera::SetLookAt(XMFLOAT3& xmf3LookAt)
{
    XMFLOAT4X4 view = Matrix4x4::LookAtLH(m_xmf3Position, xmf3LookAt,
                                           m_pPlayer ? m_pPlayer->GetUpVector() : XMFLOAT3(0,1,0));
    m_xmf3Right = XMFLOAT3(view._11, view._21, view._31);
    m_xmf3Up    = XMFLOAT3(view._12, view._22, view._32);
    m_xmf3Look  = XMFLOAT3(view._13, view._23, view._33);
}

// ============================================================
// CThirdPersonCamera
// ============================================================

CThirdPersonCamera::CThirdPersonCamera(CCamera* pCamera) : CCamera(pCamera)
{
    m_nMode = THIRD_PERSON_CAMERA;
}

void CThirdPersonCamera::Update(XMFLOAT3& xmf3LookAt, float fTimeElapsed)
{
    if (m_pPlayer)
    {
        XMFLOAT4X4 xmf4x4Rotate;
        XMFLOAT3 xmf3Right  = m_pPlayer->GetRightVector();
        XMFLOAT3 xmf3Up     = m_pPlayer->GetUpVector();
        XMFLOAT3 xmf3Look   = m_pPlayer->GetLookVector();

        xmf4x4Rotate._11 = xmf3Right.x; xmf4x4Rotate._21 = xmf3Up.x; xmf4x4Rotate._31 = xmf3Look.x;
        xmf4x4Rotate._12 = xmf3Right.y; xmf4x4Rotate._22 = xmf3Up.y; xmf4x4Rotate._32 = xmf3Look.y;
        xmf4x4Rotate._13 = xmf3Right.z; xmf4x4Rotate._23 = xmf3Up.z; xmf4x4Rotate._33 = xmf3Look.z;
        xmf4x4Rotate._14 = 0.0f; xmf4x4Rotate._24 = 0.0f; xmf4x4Rotate._34 = 0.0f;
        xmf4x4Rotate._41 = 0.0f; xmf4x4Rotate._42 = 0.0f; xmf4x4Rotate._43 = 0.0f; xmf4x4Rotate._44 = 1.0f;

        XMFLOAT3 xmf3Offset   = Vector3::TransformCoord(m_xmf3Offset, xmf4x4Rotate);
        XMFLOAT3 xmf3Position = Vector3::Add(m_pPlayer->GetPosition(), xmf3Offset);

        // fTimeElapsed==0 (초기화 시)에는 즉시 이동, 이후는 lag 보간
        if (m_fTimeLag > 0.0f && fTimeElapsed > 0.0f)
        {
            float fAlpha = fTimeElapsed / m_fTimeLag;
            m_xmf3Position.x += (xmf3Position.x - m_xmf3Position.x) * fAlpha;
            m_xmf3Position.y += (xmf3Position.y - m_xmf3Position.y) * fAlpha;
            m_xmf3Position.z += (xmf3Position.z - m_xmf3Position.z) * fAlpha;
        }
        else
        {
            m_xmf3Position = xmf3Position;
        }

        // Always look at player
        XMFLOAT3 xmf3LookAtPlayer = m_pPlayer->GetPosition();
        xmf3LookAtPlayer.y += 3.0f;
        SetLookAt(xmf3LookAtPlayer);
        RegenerateViewMatrix();
    }
}

void CThirdPersonCamera::SetLookAt(XMFLOAT3& xmf3LookAt)
{
    XMFLOAT4X4 view = Matrix4x4::LookAtLH(m_xmf3Position, xmf3LookAt, XMFLOAT3(0.0f, 1.0f, 0.0f));
    m_xmf3Right = XMFLOAT3(view._11, view._21, view._31);
    m_xmf3Up    = XMFLOAT3(view._12, view._22, view._32);
    m_xmf3Look  = XMFLOAT3(view._13, view._23, view._33);
}
