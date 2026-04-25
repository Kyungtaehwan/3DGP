#pragma once
#include "GameObject.h"

class CBullet : public CGameObject
{
public:
    CBullet();
    virtual ~CBullet();

    virtual void    Initialize() override;
    virtual int     Update(float dt) override;
    virtual void    Late_Update(float dt) override;
    virtual void    Render(HDC hDC) override;
    virtual void    Release() override;

    void Fire(XMFLOAT3 vPos, XMFLOAT3 vDir, float fSpeed = 80.f, bool bGravity = false);

    static void     PrepareExplosion();
    static void     ReleaseExplosion();


public:
    void            SetBlowingUp() {
    
        m_bBlowingUp = true;
        m_bActive = false;
    };


private:
    // ÀÌµ¿
    XMFLOAT3        m_xmf3Position = { 0,0,0 };
    XMFLOAT3        m_xmf3Direction = { 0,0,1 };
    float           m_fSpeed = 80.f;
    float           m_fMovingDistance = 0.f;
    float           m_fMaxRange = 400.f;

    // Æø¹ß
    bool            m_bBlowingUp = false;
    float           m_fElapsedTimes = 0.f;
    float           m_fDuration = 0.6f;
    float           m_fExplosionSpeed = 25.f;
    float           m_fExplosionRotation = 360.f;
    float           m_fGravity = 0.f;
    float           m_fVerticalVelocity = 0.f;
    
    XMFLOAT4X4      m_pxmf4x4Transforms[EXPLOSION_DEBRISES] = {};

    static XMFLOAT3 m_pxmf3SphereVectors[EXPLOSION_DEBRISES];
    static CMesh* m_pExplosionMesh;
};