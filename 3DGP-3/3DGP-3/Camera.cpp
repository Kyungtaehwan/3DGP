#include "pch.h"
#include "Player.h"
#include "Camera.h"

CCamera::CCamera()
{
    m_xmf4x4View       = Matrix4x4::Identity();
    m_xmf4x4Projection = Matrix4x4::Identity();

    m_d3dViewport    = { 0.0f, 0.0f, (float)FRAME_BUFFER_WIDTH, (float)FRAME_BUFFER_HEIGHT, 0.0f, 1.0f };
    m_d3dScissorRect = { 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT };
}

CCamera::CCamera(CCamera* pCamera)
{
    if (pCamera)
    {
        *this = *pCamera;
    }
    else
    {
        m_xmf4x4View       = Matrix4x4::Identity();
        m_xmf4x4Projection = Matrix4x4::Identity();
        m_d3dViewport      = { 0.0f, 0.0f, (float)FRAME_BUFFER_WIDTH, (float)FRAME_BUFFER_HEIGHT, 0.0f, 1.0f };
        m_d3dScissorRect   = { 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT };
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
    m_fNearPlane   = fNearPlaneDistance;
    m_fFarPlane    = fFarPlaneDistance;
    m_fAspectRatio = fAspectRatio;
    m_fFOVAngle    = fFOVAngle;

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

// ============================================================
// CThirdPersonCamera (orbit / GTA-style)
// ============================================================

CThirdPersonCamera::CThirdPersonCamera(CCamera* pCamera) : CCamera(pCamera)
{
    m_nMode = THIRD_PERSON_CAMERA;
}

void CThirdPersonCamera::OrbitInput(float fYawDelta, float fPitchDelta)
{
    m_fOrbitYaw += fYawDelta;
    if (m_fOrbitYaw >  360.0f) m_fOrbitYaw -= 360.0f;
    if (m_fOrbitYaw <    0.0f) m_fOrbitYaw += 360.0f;

    m_fOrbitPitch += fPitchDelta;
    if (m_fOrbitPitch >  80.0f) m_fOrbitPitch =  80.0f;
    if (m_fOrbitPitch < -30.0f) m_fOrbitPitch = -30.0f;
}

XMFLOAT3 CThirdPersonCamera::GetHorizontalForward()
{
    float yawR = DegreeToRadian(m_fOrbitYaw);
    return XMFLOAT3(sinf(yawR), 0.0f, cosf(yawR));
}

XMFLOAT3 CThirdPersonCamera::GetHorizontalRight()
{
    float yawR = DegreeToRadian(m_fOrbitYaw);
    return XMFLOAT3(cosf(yawR), 0.0f, -sinf(yawR));
}

void CThirdPersonCamera::Update(XMFLOAT3& xmf3LookAt, float fTimeElapsed)
{
    if (!m_pPlayer) return;

    XMFLOAT3 playerPos = m_pPlayer->GetPosition();

    float yawR   = DegreeToRadian(m_fOrbitYaw);
    float pitchR = DegreeToRadian(m_fOrbitPitch);
    float cy = cosf(yawR),   sy = sinf(yawR);
    float cp = cosf(pitchR), sp = sinf(pitchR);

    // forward = direction from camera toward player (after orbit yaw/pitch).
    // pitch > 0 => camera is above player; forward.y becomes negative (looks down).
    XMFLOAT3 forward = XMFLOAT3(sy * cp, -sp, cy * cp);

    // Camera sits opposite to forward, offset by distance.
    m_xmf3Position.x = playerPos.x - forward.x * m_fOrbitDistance;
    m_xmf3Position.y = playerPos.y - forward.y * m_fOrbitDistance + m_fLookHeight;
    m_xmf3Position.z = playerPos.z - forward.z * m_fOrbitDistance;

    XMFLOAT3 lookTarget = playerPos;
    lookTarget.y += m_fLookHeight;
    SetLookAt(lookTarget);
    RegenerateViewMatrix();
}

void CThirdPersonCamera::SetLookAt(XMFLOAT3& xmf3LookAt)
{
    XMFLOAT4X4 view = Matrix4x4::LookAtLH(m_xmf3Position, xmf3LookAt, XMFLOAT3(0.0f, 1.0f, 0.0f));
    m_xmf3Right = XMFLOAT3(view._11, view._21, view._31);
    m_xmf3Up    = XMFLOAT3(view._12, view._22, view._32);
    m_xmf3Look  = XMFLOAT3(view._13, view._23, view._33);
}
