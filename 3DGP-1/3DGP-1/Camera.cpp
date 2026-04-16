#include "stdafx.h"
#include "Camera.h"
#include "Player.h"

void CViewport::SetViewport(int nLeft, int nTop, int nWidth, int nHeight)
{
    m_nLeft = nLeft; m_nTop = nTop;
    m_nWidth = nWidth; m_nHeight = nHeight;
}

CCamera::CCamera(CPlayer* pPlayer, XMFLOAT3 xmf3Offset)
    : m_pPlayer(pPlayer), m_xmf3Offset(xmf3Offset)
{
}

CCamera::~CCamera()
{
}

void CCamera::SetViewport(int l, int t, int w, int h)
{
    m_Viewport.SetViewport(l, t, w, h);
    m_fAspectRatio = float(w) / float(h);
}

void CCamera::GeneratePerspectiveProjectionMatrix(float fNear, float fFar, float fFOV)
{
    XMMATRIX xmmtxProj = XMMatrixPerspectiveFovLH(
        XMConvertToRadians(fFOV), m_fAspectRatio, fNear, fFar);
    XMStoreFloat4x4(&m_xmf4x4Project, xmmtxProj);

    BoundingFrustum::CreateFromMatrix(m_xmFrustumView, xmmtxProj);
}

void CCamera::GenerateViewMatrix()
{
    XMFLOAT3 pPos = m_pPlayer->GetPosition();
    XMFLOAT3 pRight = m_pPlayer->GetRight();
    XMFLOAT3 pUp = m_pPlayer->GetUp();
    XMFLOAT3 pLook = m_pPlayer->GetLook();

    // 오프셋을 플레이어 로컬 축 기준으로 변환
    m_xmf3Position.x = pPos.x + pRight.x * m_xmf3Offset.x + pUp.x * m_xmf3Offset.y + pLook.x * m_xmf3Offset.z;
    m_xmf3Position.y = pPos.y + pRight.y * m_xmf3Offset.x + pUp.y * m_xmf3Offset.y + pLook.y * m_xmf3Offset.z;
    m_xmf3Position.z = pPos.z + pRight.z * m_xmf3Offset.x + pUp.z * m_xmf3Offset.y + pLook.z * m_xmf3Offset.z;

    // 플레이어를 바라보도록 축 계산
    m_xmf3Look = Vector3::Normalize(Vector3::Subtract(pPos, m_xmf3Position));
    m_xmf3Right = Vector3::Normalize(Vector3::CrossProduct(pUp, m_xmf3Look));
    m_xmf3Up = Vector3::Normalize(Vector3::CrossProduct(m_xmf3Look, m_xmf3Right));

    // View 행렬
    m_xmf4x4View._11 = m_xmf3Right.x; m_xmf4x4View._12 = m_xmf3Up.x; m_xmf4x4View._13 = m_xmf3Look.x; m_xmf4x4View._14 = 0;
    m_xmf4x4View._21 = m_xmf3Right.y; m_xmf4x4View._22 = m_xmf3Up.y; m_xmf4x4View._23 = m_xmf3Look.y; m_xmf4x4View._24 = 0;
    m_xmf4x4View._31 = m_xmf3Right.z; m_xmf4x4View._32 = m_xmf3Up.z; m_xmf4x4View._33 = m_xmf3Look.z; m_xmf4x4View._34 = 0;
    m_xmf4x4View._41 = -Vector3::DotProduct(m_xmf3Position, m_xmf3Right);
    m_xmf4x4View._42 = -Vector3::DotProduct(m_xmf3Position, m_xmf3Up);
    m_xmf4x4View._43 = -Vector3::DotProduct(m_xmf3Position, m_xmf3Look);
    m_xmf4x4View._44 = 1;

    m_xmf4x4ViewProject = Matrix4x4::Multiply(m_xmf4x4View, m_xmf4x4Project);

    // 프러스텀 월드 변환 (InverseView = 카메라→월드)
    XMFLOAT4X4 invView = Matrix4x4::Identity();
    invView._11 = m_xmf3Right.x; invView._12 = m_xmf3Right.y; invView._13 = m_xmf3Right.z;
    invView._21 = m_xmf3Up.x;    invView._22 = m_xmf3Up.y;    invView._23 = m_xmf3Up.z;
    invView._31 = m_xmf3Look.x;  invView._32 = m_xmf3Look.y;  invView._33 = m_xmf3Look.z;
    invView._41 = m_xmf3Position.x; invView._42 = m_xmf3Position.y; invView._43 = m_xmf3Position.z;
    m_xmFrustumView.Transform(m_xmFrustumWorld, XMLoadFloat4x4(&invView));
}

void CCamera::GenerateViewMatrix_Fixed()
{
    XMFLOAT3 pPos = m_pPlayer->GetPosition();

    // 플레이어 위치 + 고정 오프셋
    m_xmf3Position.x = pPos.x + m_xmf3Offset.x;
    m_xmf3Position.y = pPos.y + m_xmf3Offset.y;
    m_xmf3Position.z = pPos.z + m_xmf3Offset.z;

    // 축 고정
    m_xmf3Look = XMFLOAT3(0.f, 0.f, 1.f);
    m_xmf3Right = XMFLOAT3(1.f, 0.f, 0.f);
    m_xmf3Up = XMFLOAT3(0.f, 1.f, 0.f);

    m_xmf4x4View._11 = m_xmf3Right.x; m_xmf4x4View._12 = m_xmf3Up.x; m_xmf4x4View._13 = m_xmf3Look.x; m_xmf4x4View._14 = 0;
    m_xmf4x4View._21 = m_xmf3Right.y; m_xmf4x4View._22 = m_xmf3Up.y; m_xmf4x4View._23 = m_xmf3Look.y; m_xmf4x4View._24 = 0;
    m_xmf4x4View._31 = m_xmf3Right.z; m_xmf4x4View._32 = m_xmf3Up.z; m_xmf4x4View._33 = m_xmf3Look.z; m_xmf4x4View._34 = 0;
    m_xmf4x4View._41 = -Vector3::DotProduct(m_xmf3Position, m_xmf3Right);
    m_xmf4x4View._42 = -Vector3::DotProduct(m_xmf3Position, m_xmf3Up);
    m_xmf4x4View._43 = -Vector3::DotProduct(m_xmf3Position, m_xmf3Look);
    m_xmf4x4View._44 = 1;

    m_xmf4x4ViewProject = Matrix4x4::Multiply(m_xmf4x4View, m_xmf4x4Project);
}

bool CCamera::IsInFrustum(BoundingOrientedBox& xmBoundingBox)
{
    return m_xmFrustumWorld.Intersects(xmBoundingBox);
}