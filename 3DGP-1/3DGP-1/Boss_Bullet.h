#pragma once
#include "GameObject.h"

// 유도탄 상태
enum class EMissileState
{
    LAUNCH,     // 하늘로 솟구치는 중
    PEAK,       // 정점에서 잠시 대기
    DIVE        // 플레이어 위치로 낙하
};

class CBossBullet : public CGameObject
{
public:
    CBossBullet();
    virtual ~CBossBullet();

    virtual void    Initialize() override;
    virtual int     Update(float dt) override;
    virtual void    Late_Update(float dt) override;
    virtual void    Render(HDC hDC) override;
    virtual void    Release() override;

    // 유도탄 발사 (배기구에서 하늘로)
    void            FireMissile(XMFLOAT3 vPos, XMFLOAT3 vTargetPos);
    // 직선 총알 발사 (손 배럴에서)
    void            FireStraight(XMFLOAT3 vPos, XMFLOAT3 vDir, float fSpeed = 60.f);

    static void     PrepareShared();
    static void     ReleaseShared();
    void            StartExplosion();

private:
    void            Reset();

private:
    // 공통
    XMFLOAT3        m_xmf3Position = { 0,0,0 };
    XMFLOAT3        m_xmf3Direction = { 0,0,0 };
    float           m_fSpeed = 0.f;
    float           m_fMovedDistance = 0.f;
    float           m_fMaxRange = 500.f;
    bool            m_bIsMissile = false;  // true=유도탄, false=직선

    // 유도탄 전용
    EMissileState   m_eMissileState = EMissileState::LAUNCH;
    XMFLOAT3        m_xmf3TargetPos = { 0,0,0 };  // 낙하 목표 위치
    float           m_fStateTimer = 0.f;
    float           m_fPeakWaitTime = 0.5f;   // 정점 대기 시간
    float           m_fLaunchSpeed = 80.f;   // 솟구치는 속도
    float           m_fDiveSpeed = 120.f;  // 낙하 속도
    float           m_fLaunchHeight = 80.f;   // 목표 높이

    // 폭발
    bool            m_bBlowingUp = false;
    float           m_fElapsedTimes = 0.f;
    float           m_fDuration = 0.6f;
    float           m_fExplosionSpeed = 25.f;
    float           m_fExplosionRotation = 360.f;
    XMFLOAT4X4      m_pxmf4x4Transforms[EXPLOSION_DEBRISES] = {};

    static XMFLOAT3 m_pxmf3SphereVectors[EXPLOSION_DEBRISES];
    static CMesh* m_pExplosionMesh;
    static CMesh* m_pMissileMesh;
    static CMesh* m_pBulletMesh;
};