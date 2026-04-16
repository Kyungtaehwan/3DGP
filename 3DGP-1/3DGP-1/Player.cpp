#include "stdafx.h"
#include "Player.h"
#include "GraphicsPipeline.h"
#include "Input_Manager.h"

CPlayer::CPlayer() {}
CPlayer::~CPlayer() { Release(); }

void CPlayer::Initialize()
{
    m_pMesh = new CAirplaneMesh(6.0f, 6.0f, 1.0f);

    m_xmf3Position = XMFLOAT3(0.f, 0.f, 0.f);
    m_xmf3Right = XMFLOAT3(1.f, 0.f, 0.f);
    m_xmf3Up = XMFLOAT3(0.f, 1.f, 0.f);
    m_xmf3Look = XMFLOAT3(0.f, 0.f, 1.f);  // Z+가 전방

    OnUpdateTransform();


}

int CPlayer::Update(float dt)
{
    Key_Input(dt);

    XMVECTOR vel = XMLoadFloat3(&m_xmf3Velocity);
    float fLength = XMVectorGetX(XMVector3Length(vel));
    if (fLength > m_fMaxSpeed)
    {
        vel = XMVectorScale(XMVector3Normalize(vel), m_fMaxSpeed);
        XMStoreFloat3(&m_xmf3Velocity, vel);
    }

    // 마찰력
    if (fLength > 0.f)
    {
        float fDecel = min(m_fFriction * dt, fLength);
        vel = XMVectorSubtract(vel, XMVectorScale(XMVector3Normalize(vel), fDecel));
        XMStoreFloat3(&m_xmf3Velocity, vel);
    }

    Move(m_xmf3Velocity, dt);
    OnUpdateTransform();
    UpdateBoundingBox();
    TCHAR szDebug[256];
    _stprintf_s(szDebug, _T("Pos: %.2f, %.2f, %.2f"),
        m_xmf3Position.x, m_xmf3Position.y, m_xmf3Position.z);
    SetWindowText(g_hWnd, szDebug);
    return OBJ_NOEVENT;
}

void CPlayer::Late_Update(float dt) {}

void CPlayer::Render(HDC hDC)
{
    CGameObject::Render(hDC, &m_xmf4x4World, m_pMesh, m_dwColor);
}

void CPlayer::Release()
{
    if (m_pMesh) { delete m_pMesh; m_pMesh = nullptr; }
}

void CPlayer::Key_Input(float dt)
{
    auto* pInput = CInput_Manager::Get_Instance();

    if (pInput->Key_Pressing('W'))
    {
        XMVECTOR v = XMLoadFloat3(&m_xmf3Velocity);
        XMVECTOR look = XMLoadFloat3(&m_xmf3Look);
        XMStoreFloat3(&m_xmf3Velocity, XMVectorAdd(v, XMVectorScale(look, m_fMoveSpeed * dt)));
    }
    if (pInput->Key_Pressing('S'))
    {
        XMVECTOR v = XMLoadFloat3(&m_xmf3Velocity);
        XMVECTOR look = XMLoadFloat3(&m_xmf3Look);
        XMStoreFloat3(&m_xmf3Velocity, XMVectorSubtract(v, XMVectorScale(look, m_fMoveSpeed * dt)));
    }
    if (pInput->Key_Pressing('A')) Rotate(0.f, -m_fRotSpeed * dt, 0.f);
    if (pInput->Key_Pressing('D')) Rotate(0.f, +m_fRotSpeed * dt, 0.f);
    if (pInput->Key_Pressing('Q')) Rotate(-m_fRotSpeed * dt, 0.f, 0.f);
    if (pInput->Key_Pressing('E')) Rotate(+m_fRotSpeed * dt, 0.f, 0.f);
}

void CPlayer::Move(XMFLOAT3& vDir, float dt)
{
    m_xmf3Position.x += vDir.x * dt;
    m_xmf3Position.y += vDir.y * dt;
    m_xmf3Position.z += vDir.z * dt;
}

void CPlayer::Rotate(float fPitch, float fYaw, float fRoll)
{
     if (fYaw != 0.f)
    {
        XMMATRIX R = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(fYaw));
        m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, R);
        m_xmf3Look  = Vector3::TransformNormal(m_xmf3Look, R);
    }

    // Pitch
    if (fPitch != 0.f)
    {
        XMMATRIX R = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Right), XMConvertToRadians(fPitch));
        m_xmf3Up   = Vector3::TransformNormal(m_xmf3Up, R);
        m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, R);
    }

    // Roll
    if (fRoll != 0.f)
    {
        XMMATRIX R = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Look), XMConvertToRadians(fRoll));
        m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, R);
        m_xmf3Up    = Vector3::TransformNormal(m_xmf3Up, R);
    }

    // 
    m_xmf3Look = Vector3::Normalize(m_xmf3Look);
    m_xmf3Right = Vector3::Normalize(Vector3::CrossProduct(m_xmf3Up, m_xmf3Look));
    m_xmf3Up = Vector3::Normalize(Vector3::CrossProduct(m_xmf3Look, m_xmf3Right));
}

void CPlayer::OnUpdateTransform()
{
    m_xmf4x4World._11 = m_xmf3Right.x; m_xmf4x4World._12 = m_xmf3Right.y; m_xmf4x4World._13 = m_xmf3Right.z;
    m_xmf4x4World._21 = m_xmf3Up.x;    m_xmf4x4World._22 = m_xmf3Up.y;    m_xmf4x4World._23 = m_xmf3Up.z;
    m_xmf4x4World._31 = m_xmf3Look.x;  m_xmf4x4World._32 = m_xmf3Look.y;  m_xmf4x4World._33 = m_xmf3Look.z;
    m_xmf4x4World._41 = m_xmf3Position.x;
    m_xmf4x4World._42 = m_xmf3Position.y;
    m_xmf4x4World._43 = m_xmf3Position.z;

    m_xmf4x4World = Matrix4x4::Multiply(
        XMMatrixRotationRollPitchYaw(XMConvertToRadians(90.0f), 0.0f, 0.0f),
        m_xmf4x4World);
}