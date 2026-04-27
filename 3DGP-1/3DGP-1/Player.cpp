#include "stdafx.h"
#include "Player.h"
#include "GraphicsPipeline.h"
#include "Input_Manager.h"
#include "Object_Manager.h"
#include "Bullet.h"

CPlayer::CPlayer() {}
CPlayer::~CPlayer() { Release(); }

void CPlayer::Initialize()
{
    m_pBodyMesh   = new CTankBodyMesh(BODY_W, BODY_H, BODY_D);
    m_pTurretMesh = new CTankTurretMesh(TURRET_W, TURRET_H, TURRET_D);
    m_pBarrelMesh = new CTankBarrelMesh(BARREL_LEN, BARREL_R);
    m_xmf3Position = XMFLOAT3(0.f, BODY_HH, -150.f); 
    m_xmf3Right = XMFLOAT3(1.f, 0.f, 0.f);
    m_xmf3Up = XMFLOAT3(0.f, 1.f, 0.f);
    m_xmf3Look = XMFLOAT3(0.f, 0.f, 1.f);

    m_xmLocalOBB = BoundingOrientedBox(
        XMFLOAT3(0.f, 0.f, 0.f), 
        XMFLOAT3(BODY_W * 0.5f, BODY_HH, BODY_D * 0.5f),
        XMFLOAT4(0.f, 0.f, 0.f, 1.f));

    OnUpdateTransform();
    m_iMaxHP = 100;
    m_iHP = m_iMaxHP;

}

int CPlayer::Update(float dt)
{
    if (m_bDead) return OBJ_NOEVENT;

    Key_Input(dt);

    // 속도 제한
    XMVECTOR vel = XMLoadFloat3(&m_xmf3Velocity);
    float fLen = XMVectorGetX(XMVector3Length(vel));
    if (fLen > m_fMaxSpeed)
    {
        vel = XMVectorScale(XMVector3Normalize(vel), m_fMaxSpeed);
        XMStoreFloat3(&m_xmf3Velocity, vel);
    }

    // 마찰력
    if (fLen > 0.f)
    {
        float fDecel = min(m_fFriction * dt, fLen);
        vel = XMVectorSubtract(vel, XMVectorScale(XMVector3Normalize(vel), fDecel));
        XMStoreFloat3(&m_xmf3Velocity, vel);
    }

    Move(m_xmf3Velocity, dt);

    const float TERRAIN_HALF = 245.f;
    if (m_xmf3Position.x < -TERRAIN_HALF || m_xmf3Position.x > TERRAIN_HALF ||
        m_xmf3Position.z < -TERRAIN_HALF || m_xmf3Position.z > TERRAIN_HALF)
    {
        Respawn();
    }

    OnUpdateTransform();        // 차체 World 갱신
    UpdateTurretWorld();        // 포탑 World 갱신
    UpdateBarrelWorld();        // 포신 World 갱신


    return OBJ_NOEVENT;
}

void CPlayer::Late_Update(float dt) {

    UpdateBoundingBox();

}

void CPlayer::Render(HDC hDC)
{
    if (m_bDead)
    {
        // 어두운 색으로
        CGameObject::Render(hDC, &m_xmf4x4World, m_pBodyMesh, RGB(60, 0, 0));
        CGameObject::Render(hDC, &m_xmf4x4BarrelWorld, m_pBarrelMesh, RGB(20, 20, 20));
        CGameObject::Render(hDC, &m_xmf4x4TurretWorld, m_pTurretMesh, RGB(40, 0, 0));
    }
    else
    {
        CGameObject::Render(hDC, &m_xmf4x4World, m_pBodyMesh, RGB(255, 0, 0));
        CGameObject::Render(hDC, &m_xmf4x4BarrelWorld, m_pBarrelMesh, RGB(0, 0, 255));
        CGameObject::Render(hDC, &m_xmf4x4TurretWorld, m_pTurretMesh, RGB(0, 255, 0));
    }
    RenderOBB(hDC);
}

void CPlayer::Release()
{
    if (m_pBodyMesh) { delete m_pBodyMesh;   m_pBodyMesh = nullptr; }
    if (m_pTurretMesh) { delete m_pTurretMesh; m_pTurretMesh = nullptr; }
    if (m_pBarrelMesh) { delete m_pBarrelMesh; m_pBarrelMesh = nullptr; }
}

void CPlayer::Key_Input(float dt)
{
    auto* pInput = CInput_Manager::Get_Instance();
   
    if (pInput->Key_Down('R'))
        Respawn();

    // 전진 / 후진
    if (pInput->Key_Pressing('W'))
    {
        XMVECTOR v = XMLoadFloat3(&m_xmf3Velocity);
        XMVECTOR look = XMLoadFloat3(&m_xmf3Look);
        XMStoreFloat3(&m_xmf3Velocity,
            XMVectorAdd(v, XMVectorScale(look, m_fMoveSpeed)));
    }
    if (pInput->Key_Pressing('S'))
    {
        XMVECTOR v = XMLoadFloat3(&m_xmf3Velocity);
        XMVECTOR look = XMLoadFloat3(&m_xmf3Look);
        XMStoreFloat3(&m_xmf3Velocity,
            XMVectorSubtract(v, XMVectorScale(look, m_fMoveSpeed)));
    }
    if (pInput->Key_Pressing('A')) Rotate(0.f, -m_fRotSpeed * dt, 0.f);
    if (pInput->Key_Pressing('D')) Rotate(0.f, +m_fRotSpeed * dt, 0.f);

    int dx = pInput->GetMouseDX();
    int dy = pInput->GetMouseDY();

    if (pInput->Key_Down(VK_LBUTTON))
    {
        CBullet* pBullet = new CBullet();
        pBullet->Initialize();
        pBullet->Fire(m_xmf3BarrelTip, m_xmf3BarrelDir, 200.f, true);
        CObject_Manager::Get_Instance()->Add_Object(OBJ_PLAYER_BULLET, pBullet);
    }

    if (dx != 0)
        m_fTurretYaw += dx * 0.15f;       // 포탑 Yaw
    if (dy != 0)
    {
        m_fBarrelPitch -= dy * 0.15f;  
        m_fBarrelPitch = max(m_fBarrelPitchMin, min(m_fBarrelPitchMax, m_fBarrelPitch));
    }
}

void CPlayer::Move(XMFLOAT3& vVel, float dt)
{
    m_xmf3PrevPosition = m_xmf3Position;

    m_xmf3Position.x += vVel.x * dt;
    m_xmf3Position.y += vVel.y * dt;
    m_xmf3Position.z += vVel.z * dt;
}

void CPlayer::Rotate(float fPitch, float fYaw, float fRoll)
{
    if (fYaw != 0.f)
    {
        XMMATRIX R = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up),
            XMConvertToRadians(fYaw));
        m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, R);
        m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, R);

        m_fTurretYaw -= fYaw;
    }

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
    m_xmf4x4World._14 = 0.f; m_xmf4x4World._24 = 0.f;
    m_xmf4x4World._34 = 0.f; m_xmf4x4World._44 = 1.f;
}



void CPlayer::UpdateTurretWorld()
{
    // 포탑 로컬 행렬: Y축 회전 후 차체 위로 이동
    XMMATRIX mTurretLocal =
        XMMatrixRotationY(XMConvertToRadians(m_fTurretYaw)) *
        XMMatrixTranslation(0.f, BODY_HH + TURRET_HH, 0.f);

    // 수정 - 먼저 차체 위로 올리고, 그 다음 Yaw 회전
    // → 차체 회전이 먼저 적용되고 그 위에서 포탑이 추가 회전
    XMMATRIX mBodyWorld = XMLoadFloat4x4(&m_xmf4x4World);
    XMMATRIX mTurretWorld =
        XMMatrixTranslation(0.f, BODY_HH + TURRET_HH, 0.f) *   //  차체 위로
        XMMatrixRotationY(XMConvertToRadians(m_fTurretYaw)) *   // 포탑 Yaw
        mBodyWorld;                                              // 차체 World 적용

    XMStoreFloat4x4(&m_xmf4x4TurretWorld, mTurretWorld);
}


void CPlayer::UpdateBarrelWorld()
{
    XMMATRIX mBarrelLocal =
        XMMatrixRotationX(XMConvertToRadians(-m_fBarrelPitch)) *
        XMMatrixTranslation(0.f, 0.f, +TURRET_HD);  // 포탑 앞면(+Z)에 배치

    XMMATRIX mTurretWorld = XMLoadFloat4x4(&m_xmf4x4TurretWorld);
    XMMATRIX mBarrelWorld = mBarrelLocal * mTurretWorld;

    XMStoreFloat4x4(&m_xmf4x4BarrelWorld, mBarrelWorld);

    XMVECTOR vTip = XMVector3TransformCoord(
        XMVectorSet(0.f, 0.f, +BARREL_LEN, 1.f), mBarrelWorld);
    XMStoreFloat3(&m_xmf3BarrelTip, vTip);

    XMVECTOR vDir = XMVector3TransformNormal(
        XMVectorSet(0.f, 0.f, +1.f, 0.f), mBarrelWorld);
    vDir = XMVector3Normalize(vDir);
    XMStoreFloat3(&m_xmf3BarrelDir, vDir);
}