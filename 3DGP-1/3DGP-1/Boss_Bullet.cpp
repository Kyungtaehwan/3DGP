#include "stdafx.h"
#include "Boss_Bullet.h"
#include "GraphicsPipeline.h"

XMFLOAT3 CBossBullet::m_pxmf3SphereVectors[EXPLOSION_DEBRISES];
CMesh* CBossBullet::m_pExplosionMesh = nullptr;
CMesh* CBossBullet::m_pMissileMesh = nullptr;
CMesh* CBossBullet::m_pBulletMesh = nullptr;

inline float RandF_Boss(float fMin, float fMax)
{
    return fMin + ((float)rand() / (float)RAND_MAX) * (fMax - fMin);
}

void CBossBullet::PrepareShared()
{
    for (int i = 0; i < EXPLOSION_DEBRISES; i++)
    {
        XMVECTOR v;
        do {
            float x = RandF_Boss(-1.f, 1.f);
            float y = RandF_Boss(-1.f, 1.f);
            float z = RandF_Boss(-1.f, 1.f);
            v = XMVectorSet(x, y, z, 0.f);
        } while (XMVectorGetX(XMVector3LengthSq(v)) > 1.f);
        XMStoreFloat3(&m_pxmf3SphereVectors[i], XMVector3Normalize(v));
    }

    m_pMissileMesh = new CCubeMesh(1.f, 5.f, 1.f);   // 유도탄 - 길쭉한 큐브
    m_pBulletMesh = new CCubeMesh(0.5f, 0.5f, 1.5f); // 직선탄
    m_pExplosionMesh = new CCubeMesh(0.5f, 0.5f, 0.5f);
}

void CBossBullet::ReleaseShared()
{
    if (m_pMissileMesh) { delete m_pMissileMesh;   m_pMissileMesh = nullptr; }
    if (m_pBulletMesh) { delete m_pBulletMesh;    m_pBulletMesh = nullptr; }
    if (m_pExplosionMesh) { delete m_pExplosionMesh; m_pExplosionMesh = nullptr; }
}

CBossBullet::CBossBullet() {}
CBossBullet::~CBossBullet() { Release(); }

void CBossBullet::Initialize()
{
    m_bActive = false;
    m_bBlowingUp = false;
}

void CBossBullet::FireMissile(XMFLOAT3 vPos, XMFLOAT3 vTargetPos)
{
    m_xmf3Position = vPos;
    m_xmf3TargetPos = vTargetPos;
    m_xmf3Direction = { 0.f, 1.f, 0.f };  // 처음엔 위로
    m_fSpeed = m_fLaunchSpeed;
    m_fMovedDistance = 0.f;
    m_fElapsedTimes = 0.f;
    m_fStateTimer = 0.f;
    m_bBlowingUp = false;
    m_bIsMissile = true;
    m_eMissileState = EMissileState::LAUNCH;
    m_bActive = true;

    XMStoreFloat4x4(&m_xmf4x4World,
        XMMatrixTranslation(vPos.x, vPos.y, vPos.z));

    m_xmLocalOBB = BoundingOrientedBox(
        XMFLOAT3(0.f, 0.f, 0.f),
        XMFLOAT3(0.5f, 0.5f, 1.5f),
        XMFLOAT4(0.f, 0.f, 0.f, 1.f));
}

void CBossBullet::FireStraight(XMFLOAT3 vPos, XMFLOAT3 vDir, float fSpeed)
{
    m_xmf3Position = vPos;
    m_xmf3Direction = vDir;
    m_fSpeed = fSpeed;
    m_fMovedDistance = 0.f;
    m_fElapsedTimes = 0.f;
    m_bBlowingUp = false;
    m_bIsMissile = false;
    m_bActive = true;

    XMStoreFloat4x4(&m_xmf4x4World,
        XMMatrixTranslation(vPos.x, vPos.y, vPos.z));

    m_xmLocalOBB = BoundingOrientedBox(
        XMFLOAT3(0.f, 0.f, 0.f),
        XMFLOAT3(0.3f, 0.3f, 0.8f),
        XMFLOAT4(0.f, 0.f, 0.f, 1.f));
}

int CBossBullet::Update(float dt)
{
    if (!m_bActive) return OBJ_NOEVENT;

    // 폭발 처리
    if (m_bBlowingUp)
    {
        m_fElapsedTimes += dt;
        if (m_fElapsedTimes <= m_fDuration)
        {
            for (int i = 0; i < EXPLOSION_DEBRISES; i++)
            {
                m_pxmf4x4Transforms[i] = Matrix4x4::Identity();
                m_pxmf4x4Transforms[i]._41 = m_xmf3Position.x + m_pxmf3SphereVectors[i].x * m_fExplosionSpeed * m_fElapsedTimes;
                m_pxmf4x4Transforms[i]._42 = m_xmf3Position.y + m_pxmf3SphereVectors[i].y * m_fExplosionSpeed * m_fElapsedTimes;
                m_pxmf4x4Transforms[i]._43 = m_xmf3Position.z + m_pxmf3SphereVectors[i].z * m_fExplosionSpeed * m_fElapsedTimes;
                m_pxmf4x4Transforms[i] = Matrix4x4::Multiply(
                    Matrix4x4::RotationAxis(m_pxmf3SphereVectors[i], m_fExplosionRotation * m_fElapsedTimes),
                    m_pxmf4x4Transforms[i]);
            }
        }
        else
        {
            m_bActive = false;
            return OBJ_DEAD;
        }
        return OBJ_NOEVENT;
    }

    if (m_bIsMissile)
    {
        // 유도탄 상태머신
        switch (m_eMissileState)
        {
        case EMissileState::LAUNCH:
        {
            // 위로 솟구침
            float fDist = m_fSpeed * dt;
            m_xmf3Position.y += fDist;
            m_fMovedDistance += fDist;

            // 목표 높이 도달 시 PEAK 상태
            if (m_xmf3Position.y >= m_xmf3TargetPos.y + m_fLaunchHeight)
            {
                m_eMissileState = EMissileState::PEAK;
                m_fStateTimer = 0.f;
            }
            break;
        }
        case EMissileState::PEAK:
        {
            // 잠시 대기
            m_fStateTimer += dt;
            if (m_fStateTimer >= m_fPeakWaitTime)
            {
                // 초기 방향을 아래쪽으로 → Lerp로 목표를 향해 꺾임
                m_xmf3Direction = { 0.f, -1.f, 0.f };
                m_fSpeed = m_fDiveSpeed;
                m_eMissileState = EMissileState::DIVE;
            }
            break;
        }
        case EMissileState::DIVE:
        {
            float fDist = m_fSpeed * dt;

            XMFLOAT3 toTarget = {
                m_xmf3TargetPos.x - m_xmf3Position.x,
                m_xmf3TargetPos.y - m_xmf3Position.y,
                m_xmf3TargetPos.z - m_xmf3Position.z
            };

            XMVECTOR vCurrent = XMVector3Normalize(XMLoadFloat3(&m_xmf3Direction));
            XMVECTOR vTarget = XMVector3Normalize(XMLoadFloat3(&toTarget));

            // 3.f → 좀 더 빠르게 꺾임
            XMVECTOR vNewDir = XMVector3Normalize(
                XMVectorLerp(vCurrent, vTarget, 3.f * dt));
            XMStoreFloat3(&m_xmf3Direction, vNewDir);

            m_xmf3Position.x += m_xmf3Direction.x * fDist;
            m_xmf3Position.y += m_xmf3Direction.y * fDist;
            m_xmf3Position.z += m_xmf3Direction.z * fDist;
            m_fMovedDistance += fDist;

            if (m_xmf3Position.y <= 0.f)
            {
                m_xmf3Position.y = 0.f;
                StartExplosion();
                return OBJ_NOEVENT;
            }
            break;
        }
        }
    }
    else
    {
        // 직선 이동
        float fDist = m_fSpeed * dt;
        m_xmf3Position.x += m_xmf3Direction.x * fDist;
        m_xmf3Position.y += m_xmf3Direction.y * fDist;
        m_xmf3Position.z += m_xmf3Direction.z * fDist;
        m_fMovedDistance += fDist;

        if (m_xmf3Position.y <= 0.f)
        {
            m_xmf3Position.y = 0.f;
            StartExplosion();
            return OBJ_NOEVENT;
        }
    }

    // 사거리 초과
    if (m_fMovedDistance > m_fMaxRange)
    {
        m_bActive = false;
        return OBJ_DEAD;
    }

    XMStoreFloat4x4(&m_xmf4x4World,
        XMMatrixTranslation(m_xmf3Position.x, m_xmf3Position.y, m_xmf3Position.z));
    UpdateBoundingBox();

    return OBJ_NOEVENT;
}

void CBossBullet::StartExplosion()
{
    m_bBlowingUp = true;
    m_fElapsedTimes = 0.f;
    for (int i = 0; i < EXPLOSION_DEBRISES; i++)
    {
        m_pxmf4x4Transforms[i] = Matrix4x4::Identity();
        m_pxmf4x4Transforms[i]._41 = m_xmf3Position.x;
        m_pxmf4x4Transforms[i]._42 = m_xmf3Position.y;
        m_pxmf4x4Transforms[i]._43 = m_xmf3Position.z;
    }
}

void CBossBullet::Late_Update(float dt) {}

void CBossBullet::Render(HDC hDC)
{
    if (!m_bActive) return;

    if (m_bBlowingUp)
    {
        if (!m_pExplosionMesh) return;
        for (int i = 0; i < EXPLOSION_DEBRISES; i++)
        {
            if (m_pxmf4x4Transforms[i]._42 <= 0.f) continue;
            DWORD col = (i % 2 == 0) ? RGB(255, 80, 0) : RGB(255, 200, 0);
            CGameObject::Render(hDC, &m_pxmf4x4Transforms[i], m_pExplosionMesh, col);
        }
    }
    else
    {
        CMesh* pMesh = m_bIsMissile ? m_pMissileMesh : m_pBulletMesh;
        DWORD  col = m_bIsMissile ? RGB(255, 50, 50) : RGB(255, 200, 0);
        if (!pMesh) return;
        CGameObject::Render(hDC, &m_xmf4x4World, pMesh, col);
    }
}

void CBossBullet::Release() {}
void CBossBullet::Reset()
{
    m_bActive = false;
    m_bBlowingUp = false;
    m_fMovedDistance = 0.f;
    m_fElapsedTimes = 0.f;
}