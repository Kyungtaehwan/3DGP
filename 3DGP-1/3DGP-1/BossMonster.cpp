#include "stdafx.h"
#include "BossMonster.h"
#include "GraphicsPipeline.h"
#include "Player.h"
#include "Bullet.h"
#include "Object_Manager.h"

CBossMonster::CBossMonster() {}
CBossMonster::~CBossMonster() { Release(); }

void CBossMonster::Initialize()
{
    m_pBodyMesh = new CTankBodyMesh(BODY_W, BODY_H, BODY_D);
    m_pHeadMesh = new CTankTurretMesh(HEAD_W, HEAD_H, HEAD_D);
    m_pLeftArmMesh = new CTankBodyMesh(ARM_W, ARM_H, ARM_D);
    m_pRightArmMesh = new CTankBodyMesh(ARM_W, ARM_H, ARM_D);
    m_pLeftBarrelMesh = new CTankBarrelMesh(BARREL_LEN, BARREL_R);
    m_pRightBarrelMesh = new CTankBarrelMesh(BARREL_LEN, BARREL_R);
    m_pExhaustMesh = new CTankBodyMesh(EXHAUST_W, EXHAUST_H, EXHAUST_D);

    m_xmLocalOBB = BoundingOrientedBox(
        XMFLOAT3(0.f, 0.f, 0.f),
        XMFLOAT3(BODY_W * 0.5f, BODY_H * 0.5f, BODY_D * 0.5f),
        XMFLOAT4(0.f, 0.f, 0.f, 1.f));

    UpdateBodyWorld();
    UpdateHeadWorld();
    UpdateArmWorld(true);
    UpdateArmWorld(false);
    UpdateBarrelWorld(true);
    UpdateBarrelWorld(false);
    UpdateExhaustWorld(true);
    UpdateExhaustWorld(false);

    m_iMaxHP = 1000;
    m_iHP = m_iMaxHP;
}

int CBossMonster::Update(float dt)
{
    if (m_bDead) return OBJ_NOEVENT;


    if (m_pPlayer)
    {
        XMFLOAT3 playerPos = m_pPlayer->GetPosition();
        XMFLOAT3 toPlayer = {
            playerPos.x - m_xmf3Position.x,
            0.f,
            playerPos.z - m_xmf3Position.z
        };
        float fLen = sqrtf(toPlayer.x * toPlayer.x + toPlayer.z * toPlayer.z);
        if (fLen > 0.1f)
        {
            m_xmf3Look = Vector3::Normalize(toPlayer);
            m_xmf3Right = Vector3::Normalize(
                Vector3::CrossProduct(m_xmf3Up, m_xmf3Look));
        }
    }

    // ── 미사일 공격 (배기구에서 포물선) ──────────────────────
    m_fMissileTimer += dt;
    if (m_fMissileTimer >= m_fMissileInterval)
    {
        m_fMissileTimer = 0.f;
        m_nMissileCount = 0;
        m_fMissileDelay = 0.f;
    }

    if (m_nMissileCount < 3 && m_pPlayer)
    {
        m_fMissileDelay += dt;
        if (m_fMissileDelay >= 0.2f * (m_nMissileCount + 1))
        {
            XMFLOAT3 playerPos = m_pPlayer->GetPosition();

            auto FireParabolic = [&](XMFLOAT3 vStart, XMFLOAT3 vTarget)
                {
                    float fSpeed = 60.f;
                    float fGravity = 30.f;  // Bullet의 중력값과 동일하게

                    // 수평 거리와 수직 거리
                    float dx = vTarget.x - vStart.x;
                    float dy = vTarget.y - vStart.y;
                    float dz = vTarget.z - vStart.z;
                    float fHorizDist = sqrtf(dx * dx + dz * dz);

                    // 물리 공식: tan(θ) = (v² ± sqrt(v⁴ - g(g*x² + 2y*v²))) / (g*x)
                    float v2 = fSpeed * fSpeed;
                    float g = fGravity;
                    float x = fHorizDist;
                    float y = dy;

                    float fDisc = v2 * v2 - g * (g * x * x + 2.f * y * v2);
                    float fAngle;

                    if (fDisc < 0.f)
                    {
                        // 사거리 초과 - 45도로 발사
                        fAngle = XMConvertToRadians(45.f);
                    }
                    else
                    {
                        // 높은 각도 선택 (+ 부호) → 더 높이 올라갔다가 내려옴
                        fAngle = atanf((v2 + sqrtf(fDisc)) / (g * x));
                    }

                    // 수평 방향 단위벡터
                    XMFLOAT3 horizDir = { dx / fHorizDist, 0.f, dz / fHorizDist };

                    // 발사 방향
                    float fCosA = cosf(fAngle);
                    float fSinA = sinf(fAngle);
                    XMFLOAT3 fireDir = {
                        horizDir.x * fCosA,
                        fSinA,
                        horizDir.z * fCosA
                    };

                    CBullet* pBullet = new CBullet();
                    pBullet->Initialize();
                    pBullet->Fire(vStart, fireDir, fSpeed, true);
                    CObject_Manager::Get_Instance()->Add_Object(OBJ_BOSS_BULLET, pBullet);
                };

            FireParabolic(m_xmf3LeftExhaustTip, playerPos);
            FireParabolic(m_xmf3RightExhaustTip, playerPos);

            m_nMissileCount++;
        }
    }

    // ── 직선탄 공격 (양쪽 배럴) ──────────────────────────────
    m_fBulletTimer += dt;
    if (m_fBulletTimer >= m_fBulletInterval && m_pPlayer)
    {
        m_fBulletTimer = 0.f;
        XMFLOAT3 playerPos = m_pPlayer->GetPosition();

        XMFLOAT3 leftBarrelPos = {
            m_xmf4x4LeftBarrelWorld._41,
            m_xmf4x4LeftBarrelWorld._42,
            m_xmf4x4LeftBarrelWorld._43
        };
        XMFLOAT3 rightBarrelPos = {
            m_xmf4x4RightBarrelWorld._41,
            m_xmf4x4RightBarrelWorld._42,
            m_xmf4x4RightBarrelWorld._43
        };

        XMFLOAT3 toPlayerL_raw = {
            playerPos.x - leftBarrelPos.x,
            playerPos.y - leftBarrelPos.y,
            playerPos.z - leftBarrelPos.z
        };
        XMFLOAT3 toPlayerR_raw = {
            playerPos.x - rightBarrelPos.x,
            playerPos.y - rightBarrelPos.y,
            playerPos.z - rightBarrelPos.z
        };

        XMFLOAT3 toPlayerL = Vector3::Normalize(toPlayerL_raw);
        XMFLOAT3 toPlayerR = Vector3::Normalize(toPlayerR_raw);

        CBullet* pBulletL = new CBullet();
        pBulletL->Initialize();
        pBulletL->Fire(leftBarrelPos, toPlayerL, 200.f, false);
        CObject_Manager::Get_Instance()->Add_Object(OBJ_BOSS_BULLET, pBulletL);

        CBullet* pBulletR = new CBullet();
        pBulletR->Initialize();
        pBulletR->Fire(rightBarrelPos, toPlayerR, 200.f, false);
        CObject_Manager::Get_Instance()->Add_Object(OBJ_BOSS_BULLET, pBulletR);
    }

    UpdateBodyWorld();
    UpdateHeadWorld();
    UpdateArmWorld(true);
    UpdateArmWorld(false);
    UpdateBarrelWorld(true);
    UpdateBarrelWorld(false);
    UpdateExhaustWorld(true);
    UpdateExhaustWorld(false);

    return OBJ_NOEVENT;
}

void CBossMonster::Late_Update(float dt)
{
    UpdateBoundingBox();
}

void CBossMonster::Render(HDC hDC)
{
    XMFLOAT4X4* pVP = CGraphicsPipeline::GetViewProjectMatrix();
    if (pVP)
    {
        XMMATRIX mat = XMLoadFloat4x4(pVP);
        XMVECTOR vPos = XMVectorSet(
            m_xmf3Position.x, m_xmf3Position.y, m_xmf3Position.z, 1.f);
        XMVECTOR vProj = XMVector4Transform(vPos, mat);
        XMFLOAT4 f4;
        XMStoreFloat4(&f4, vProj);
        if (f4.w <= 0.f) return;
    }

    DWORD bodyCol = m_bDead ? RGB(20, 5, 0) : RGB(80, 10, 0);
    DWORD headCol = m_bDead ? RGB(15, 3, 0) : RGB(80, 10, 0);
    DWORD armCol = m_bDead ? RGB(15, 3, 0) : RGB(80, 10, 0);
    DWORD barrelCol = m_bDead ? RGB(30, 30, 30) : RGB(100, 100, 100);
    DWORD exhaustCol = m_bDead ? RGB(15, 15, 15) : RGB(60, 60, 60);

    CGameObject::Render(hDC, &m_xmf4x4LeftExhaustWorld, m_pExhaustMesh, exhaustCol);
    CGameObject::Render(hDC, &m_xmf4x4RightExhaustWorld, m_pExhaustMesh, exhaustCol);
    CGameObject::Render(hDC, &m_xmf4x4HeadWorld, m_pHeadMesh, headCol);
    CGameObject::Render(hDC, &m_xmf4x4BodyWorld, m_pBodyMesh, bodyCol);
    CGameObject::Render(hDC, &m_xmf4x4LeftArmWorld, m_pLeftArmMesh, armCol);
    CGameObject::Render(hDC, &m_xmf4x4RightArmWorld, m_pRightArmMesh, armCol);
    CGameObject::Render(hDC, &m_xmf4x4LeftBarrelWorld, m_pLeftBarrelMesh, barrelCol);
    CGameObject::Render(hDC, &m_xmf4x4RightBarrelWorld, m_pRightBarrelMesh, barrelCol);

    RenderOBB(hDC);

    RenderOBB(hDC);
}

void CBossMonster::Release()
{
    if (m_pBodyMesh) { delete m_pBodyMesh;        m_pBodyMesh = nullptr; }
    if (m_pHeadMesh) { delete m_pHeadMesh;        m_pHeadMesh = nullptr; }
    if (m_pLeftArmMesh) { delete m_pLeftArmMesh;     m_pLeftArmMesh = nullptr; }
    if (m_pRightArmMesh) { delete m_pRightArmMesh;    m_pRightArmMesh = nullptr; }
    if (m_pLeftBarrelMesh) { delete m_pLeftBarrelMesh;  m_pLeftBarrelMesh = nullptr; }
    if (m_pRightBarrelMesh) { delete m_pRightBarrelMesh; m_pRightBarrelMesh = nullptr; }
    if (m_pExhaustMesh) { delete m_pExhaustMesh;     m_pExhaustMesh = nullptr; }
}

void CBossMonster::UpdateBodyWorld()
{
    m_xmf4x4BodyWorld._11 = m_xmf3Right.x; m_xmf4x4BodyWorld._12 = m_xmf3Right.y; m_xmf4x4BodyWorld._13 = m_xmf3Right.z; m_xmf4x4BodyWorld._14 = 0.f;
    m_xmf4x4BodyWorld._21 = m_xmf3Up.x;    m_xmf4x4BodyWorld._22 = m_xmf3Up.y;    m_xmf4x4BodyWorld._23 = m_xmf3Up.z;    m_xmf4x4BodyWorld._24 = 0.f;
    m_xmf4x4BodyWorld._31 = m_xmf3Look.x;  m_xmf4x4BodyWorld._32 = m_xmf3Look.y;  m_xmf4x4BodyWorld._33 = m_xmf3Look.z;  m_xmf4x4BodyWorld._34 = 0.f;
    m_xmf4x4BodyWorld._41 = m_xmf3Position.x;
    m_xmf4x4BodyWorld._42 = m_xmf3Position.y;
    m_xmf4x4BodyWorld._43 = m_xmf3Position.z;
    m_xmf4x4BodyWorld._44 = 1.f;

    m_xmf4x4World = m_xmf4x4BodyWorld;
}

void CBossMonster::UpdateHeadWorld()
{
    XMMATRIX mBody = XMLoadFloat4x4(&m_xmf4x4BodyWorld);
    XMMATRIX mHead = XMMatrixTranslation(0.f, BODY_HH + HEAD_HH, 0.f) * mBody;
    XMStoreFloat4x4(&m_xmf4x4HeadWorld, mHead);
}

void CBossMonster::UpdateArmWorld(bool bLeft)
{
    float fSide = bLeft ? -(BODY_W * 0.5f + ARM_HD) : +(BODY_W * 0.5f + ARM_HD);
    XMMATRIX mBodyWorld = XMLoadFloat4x4(&m_xmf4x4BodyWorld);
    XMMATRIX mArmWorld = XMMatrixTranslation(fSide, 0.f, 0.f) * mBodyWorld;

    if (bLeft)
        XMStoreFloat4x4(&m_xmf4x4LeftArmWorld, mArmWorld);
    else
        XMStoreFloat4x4(&m_xmf4x4RightArmWorld, mArmWorld);
}

void CBossMonster::UpdateBarrelWorld(bool bLeft)
{
    XMMATRIX mArmWorld = XMLoadFloat4x4(
        bLeft ? &m_xmf4x4LeftArmWorld : &m_xmf4x4RightArmWorld);
    XMMATRIX mBarrelWorld = XMMatrixTranslation(0.f, 0.f, ARM_HD) * mArmWorld;

    if (bLeft)
        XMStoreFloat4x4(&m_xmf4x4LeftBarrelWorld, mBarrelWorld);
    else
        XMStoreFloat4x4(&m_xmf4x4RightBarrelWorld, mBarrelWorld);
}

void CBossMonster::UpdateExhaustWorld(bool bLeft)
{
    XMMATRIX mBodyWorld = XMLoadFloat4x4(&m_xmf4x4BodyWorld);

    float fSide = bLeft ? -(BODY_W * 0.5f - 3.f) : +(BODY_W * 0.5f - 3.f);

    XMMATRIX mExhaustWorld =
        XMMatrixTranslation(fSide, BODY_HH * 0.3f, -(BODY_HD + EXHAUST_HD)) *
        mBodyWorld;

    XMVECTOR vTip = XMVector3TransformCoord(
        XMVectorSet(0.f, EXHAUST_HH, 0.f, 1.f), mExhaustWorld);

    if (bLeft)
    {
        XMStoreFloat4x4(&m_xmf4x4LeftExhaustWorld, mExhaustWorld);
        XMStoreFloat3(&m_xmf3LeftExhaustTip, vTip);
    }
    else
    {
        XMStoreFloat4x4(&m_xmf4x4RightExhaustWorld, mExhaustWorld);
        XMStoreFloat3(&m_xmf3RightExhaustTip, vTip);
    }
}