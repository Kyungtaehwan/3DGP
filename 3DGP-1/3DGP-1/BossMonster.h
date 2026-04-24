#pragma once
#include "GameObject.h"
class CPlayer;

class CBossMonster : public CGameObject
{
public:
    CBossMonster();
    virtual ~CBossMonster();

    virtual void    Initialize() override;
    virtual int     Update(float dt) override;
    virtual void    Late_Update(float dt) override;
    virtual void    Render(HDC hDC) override;
    virtual void    Release() override;

    void            SetPlayer(CPlayer* pPlayer) { m_pPlayer = pPlayer; }
    XMFLOAT3        GetPosition() const { return m_xmf3Position; }
    XMFLOAT3        GetLeftExhaustTip()  const { return m_xmf3LeftExhaustTip; }
    XMFLOAT3        GetRightExhaustTip() const { return m_xmf3RightExhaustTip; }
    XMFLOAT3        GetExhaustDir()      const { return XMFLOAT3(0.f, 1.f, 0.f); }

private:
    void            UpdateBodyWorld();
    void            UpdateHeadWorld();
    void            UpdateArmWorld(bool bLeft);
    void            UpdateBarrelWorld(bool bLeft);
    void            UpdateExhaustWorld(bool bLeft);

private:
    CPlayer* m_pPlayer = nullptr;

    CMesh* m_pBodyMesh = nullptr;
    CMesh* m_pHeadMesh = nullptr;
    CMesh* m_pLeftArmMesh = nullptr;
    CMesh* m_pRightArmMesh = nullptr;
    CMesh* m_pLeftBarrelMesh = nullptr;
    CMesh* m_pRightBarrelMesh = nullptr;
    CMesh* m_pExhaustMesh = nullptr;

    XMFLOAT4X4 m_xmf4x4BodyWorld = Matrix4x4::Identity();
    XMFLOAT4X4 m_xmf4x4HeadWorld = Matrix4x4::Identity();
    XMFLOAT4X4 m_xmf4x4LeftArmWorld = Matrix4x4::Identity();
    XMFLOAT4X4 m_xmf4x4RightArmWorld = Matrix4x4::Identity();
    XMFLOAT4X4 m_xmf4x4LeftBarrelWorld = Matrix4x4::Identity();
    XMFLOAT4X4 m_xmf4x4RightBarrelWorld = Matrix4x4::Identity();
    XMFLOAT4X4 m_xmf4x4LeftExhaustWorld = Matrix4x4::Identity();
    XMFLOAT4X4 m_xmf4x4RightExhaustWorld = Matrix4x4::Identity();

    XMFLOAT3 m_xmf3Position = { 0.f, BODY_HH, 0.f };
    XMFLOAT3 m_xmf3Right = { 1.f, 0.f, 0.f };
    XMFLOAT3 m_xmf3Up = { 0.f, 1.f, 0.f };
    XMFLOAT3 m_xmf3Look = { 0.f, 0.f, -1.f };

    XMFLOAT3 m_xmf3LeftExhaustTip = { 0,0,0 };
    XMFLOAT3 m_xmf3RightExhaustTip = { 0,0,0 };

    static constexpr float BODY_W = 25.f;
    static constexpr float BODY_H = 40.f;
    static constexpr float BODY_D = 15.f;
    static constexpr float BODY_HH = BODY_H * 0.5f;
    static constexpr float BODY_HD = BODY_D * 0.5f;

    static constexpr float HEAD_W = 4.f;
    static constexpr float HEAD_H = 4.f;
    static constexpr float HEAD_D = 12.f;
    static constexpr float HEAD_HH = HEAD_H * 0.5f;

    static constexpr float ARM_W = 4.f;
    static constexpr float ARM_H = 4.f;
    static constexpr float ARM_D = 10.f;
    static constexpr float ARM_HD = ARM_D * 0.5f;

    static constexpr float BARREL_LEN = 8.f;
    static constexpr float BARREL_R = 0.8f;

    static constexpr float EXHAUST_W = 2.f;
    static constexpr float EXHAUST_H = 40.f;
    static constexpr float EXHAUST_D = 5.f;
    static constexpr float EXHAUST_HH = EXHAUST_H * 0.5f;
    static constexpr float EXHAUST_HD = EXHAUST_D * 0.5f;
};