#include "stdafx.h"
#include "Bullet.h"
#include "GraphicsPipeline.h"

XMFLOAT3 CBullet::m_pxmf3SphereVectors[EXPLOSION_DEBRISES];
CMesh* CBullet::m_pExplosionMesh = nullptr;

inline float RandF_B(float fMin, float fMax)
{
    return fMin + ((float)rand() / (float)RAND_MAX) * (fMax - fMin);
}

XMVECTOR RandomUnitVectorOnSphere_B()
{
    XMVECTOR xmvOne = XMVectorSet(1.f, 1.f, 1.f, 1.f);
    while (true)
    {
        XMVECTOR v = XMVectorSet(RandF_B(-1.f, 1.f), RandF_B(-1.f, 1.f), RandF_B(-1.f, 1.f), 0.f);
        if (!XMVector3Greater(XMVector3LengthSq(v), xmvOne))
            return XMVector3Normalize(v);
    }
}

void CBullet::PrepareExplosion()
{
    for (int i = 0; i < EXPLOSION_DEBRISES; i++)
        XMStoreFloat3(&m_pxmf3SphereVectors[i], ::RandomUnitVectorOnSphere_B());

    m_pExplosionMesh = new CCubeMesh(0.5f, 0.5f, 0.5f);
}

void CBullet::ReleaseExplosion()
{
    if (m_pExplosionMesh) { delete m_pExplosionMesh; m_pExplosionMesh = nullptr; }
}

CBullet::CBullet() {}
CBullet::~CBullet() { Release(); }

void CBullet::Initialize()
{
    m_bActive = false;
    m_bBlowingUp = false;
}

void CBullet::Fire(XMFLOAT3 vPos, XMFLOAT3 vDir, float fSpeed, bool bGravity)
{
    m_xmf3Position = vPos;
    m_xmf3Direction = vDir;
    m_fSpeed = fSpeed;
    m_fMovingDistance = 0.f;
    m_fElapsedTimes = 0.f;
    m_bBlowingUp = false;
    m_bActive = true;
    m_fVerticalVelocity = vDir.y * fSpeed;  // 초기 수직 속도
    m_fGravity = bGravity ? 30.f : 0.f;  // 중력값
    m_xmLocalOBB = BoundingOrientedBox(
        XMFLOAT3(0.f, 0.f, 0.f),
        XMFLOAT3(0.4f, 0.4f, 0.6f),
        XMFLOAT4(0.f, 0.f, 0.f, 1.f));
    XMStoreFloat4x4(&m_xmf4x4World,
        XMMatrixTranslation(vPos.x, vPos.y, vPos.z));
}

int CBullet::Update(float dt)
{
    if (!m_bActive) return OBJ_NOEVENT;

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
            m_bBlowingUp = false;
            m_fElapsedTimes = 0.f;
            m_bActive = false;
            return OBJ_DEAD;
        }
        return OBJ_NOEVENT;
    }

    // 이동
    float fDist = m_fSpeed * dt;
    m_xmf3Position.x += m_xmf3Direction.x * fDist;
    m_xmf3Position.z += m_xmf3Direction.z * fDist;

    if (m_fGravity > 0.f)
    {
        // 중력 누적 → 수직 속도 감소
        m_fVerticalVelocity -= m_fGravity * dt;
        m_xmf3Position.y += m_fVerticalVelocity * dt;
    }
    else
    {
        m_xmf3Position.y += m_xmf3Direction.y * fDist;
    }

    m_fMovingDistance += fDist;

    XMStoreFloat4x4(&m_xmf4x4World,
        XMMatrixTranslation(m_xmf3Position.x, m_xmf3Position.y, m_xmf3Position.z));

    if (m_xmf3Position.y <= 0.f)
    {
        m_xmf3Position.y = 0.f;
        m_bBlowingUp = true;
        m_fElapsedTimes = 0.f;
        return OBJ_NOEVENT;
    }

    if (m_fMovingDistance > m_fMaxRange)
    {
        m_bActive = false;
        return OBJ_DEAD;
    }



    return OBJ_NOEVENT;
}

void CBullet::Late_Update(float dt) {

    UpdateBoundingBox();
}

void CBullet::Render(HDC hDC)
{
    if (!m_bActive) return;

    if (m_bBlowingUp)
    {
        for (int i = 0; i < EXPLOSION_DEBRISES; i++)
        {
            if (m_pxmf4x4Transforms[i]._42 <= 0.f) continue;

            CGameObject::Render(hDC, &m_pxmf4x4Transforms[i], m_pExplosionMesh, RGB(255, 140, 0));
        }
    }
    else
    {
        CGameObject::Render(hDC, &m_xmf4x4World, m_pExplosionMesh, RGB(255, 255, 0));
    }

    RenderOBB(hDC);
}

void CBullet::Release() {}