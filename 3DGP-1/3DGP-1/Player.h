#pragma once
#include "GameObject.h"


class CPlayer : public CGameObject
{
public:
    CPlayer();
    virtual ~CPlayer();

    virtual void    Initialize() override;
    virtual int     Update(float dt) override;
    virtual void    Late_Update(float dt) override;
    virtual void    Render(HDC hDC) override;
    virtual void    Release() override;

    // ── 카메라가 필요한 Getter ────────────────────────────────
    XMFLOAT3        GetPosition() const { return m_xmf3Position; }
    XMFLOAT3        GetRight()    const { return m_xmf3Right; }
    XMFLOAT3        GetUp()       const { return m_xmf3Up; }
    XMFLOAT3        GetLook()     const { return m_xmf3Look; }

    // ── 포탑/포신 제어 ────────────────────────────────────────
    void            SetTurretYaw(float fYaw) { m_fTurretYaw = fYaw; }
    void            SetBarrelPitch(float fPitch) { m_fBarrelPitch = fPitch; }
    float           GetTurretYaw()   const { return m_fTurretYaw; }
    float           GetBarrelPitch() const { return m_fBarrelPitch; }

    // ── 포신 끝 정보 (총알 발사용) ────────────────────────────
    XMFLOAT3        GetBarrelTipWorld()  const { return m_xmf3BarrelTip; }
    XMFLOAT3        GetBarrelDirection() const { return m_xmf3BarrelDir; }

    void SetVelocityZero() { m_xmf3Velocity = { 0.f, 0.f, 0.f }; }
    void RollbackPosition() { m_xmf3Position = m_xmf3PrevPosition; }
    void ReflectVelocity(XMFLOAT3 vNormal)
    {
        XMVECTOR vel = XMLoadFloat3(&m_xmf3Velocity);
        XMVECTOR nor = XMVector3Normalize(XMLoadFloat3(&vNormal));
        // 반사 벡터 계산
        vel = XMVector3Reflect(vel, nor);
        // 반사 후 속도를 약간 줄여서 자연스럽게
        vel = XMVectorScale(vel, 0.5f);
        XMStoreFloat3(&m_xmf3Velocity, vel);

        m_xmf3Position = m_xmf3PrevPosition;
    }

private:
    void            Key_Input(float dt);
    void            Move(XMFLOAT3& vVel, float dt);
    void            Rotate(float fPitch, float fYaw, float fRoll);
    void            OnUpdateTransform();
    void            UpdateTurretWorld();
    void            UpdateBarrelWorld();

private:
    // ── 메쉬 ──────────────────────────────────────────────────
    CMesh* m_pBodyMesh = nullptr;
    CMesh* m_pTurretMesh = nullptr;
    CMesh* m_pBarrelMesh = nullptr;

    // ── 파트별 World 행렬 ─────────────────────────────────────
    XMFLOAT4X4 m_xmf4x4TurretWorld = Matrix4x4::Identity();
    XMFLOAT4X4 m_xmf4x4BarrelWorld = Matrix4x4::Identity();

    // ── 포탑/포신 각도 ────────────────────────────────────────
    float m_fTurretYaw = 0.f;
    float m_fBarrelPitch = 0.f;
    float m_fBarrelPitchMin = -5.f;
    float m_fBarrelPitchMax = 20.f;

    // ── 포신 끝 정보 ──────────────────────────────────────────
    XMFLOAT3 m_xmf3BarrelTip = { 0,0,0 };
    XMFLOAT3 m_xmf3BarrelDir = { 0,0,1 };

    // ── 이동 파라미터 ─────────────────────────────────────────
    XMFLOAT3 m_xmf3Velocity = { 0,0,0 };
    float m_fMoveSpeed = 2.f;   // 가속도 0.5 m/s²
    float m_fMaxSpeed = 20.f;   // 최대 15 m/s = 54 km/h
    float m_fFriction = 15.f;    // 제동력 8 m/s²
    float m_fRotSpeed = 60.f;   // 45 도/s

    // ── 차체 축 / 위치 ────────────────────────────────────────
    XMFLOAT3 m_xmf3Right = { 1,0,0 };
    XMFLOAT3 m_xmf3Up = { 0,1,0 };
    XMFLOAT3 m_xmf3Look = { 0,0,1 };
    XMFLOAT3 m_xmf3Position = { 0,0,0 };
    XMFLOAT3 m_xmf3PrevPosition = { 0,0,0 };
    // ── 메쉬 크기 상수 ────────────────────────────────────────
    static constexpr float BODY_W = 4.f;
    static constexpr float BODY_H = 2.4f;
    static constexpr float BODY_D = 8.f;
    static constexpr float BODY_HH = BODY_H * 0.5f;   // 1.2m

    // 포탑: 3m x 1.5m x 3m
    static constexpr float TURRET_W = 3.f;
    static constexpr float TURRET_H = 1.5f;
    static constexpr float TURRET_D = 3.f;
    static constexpr float TURRET_HH = TURRET_H * 0.5f;  // 0.75m
    static constexpr float TURRET_HD = TURRET_D * 0.5f;  // 1.5m

    // 포신: 길이 5m, 반폭 0.2m
    static constexpr float BARREL_LEN = 5.f;
    static constexpr float BARREL_R = 0.2f;


};

