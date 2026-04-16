#pragma once
#include "GameObject.h"


class CPlayer : public CGameObject
{
public:
    CPlayer();
    virtual ~CPlayer();

public:
    virtual void Initialize() override;
    virtual int  Update(float dt) override;
    virtual void Late_Update(float dt) override;
    virtual void Render(HDC hDC) override;
    virtual void Release() override;

    void Key_Input(float dt);

    // 외부(카메라)에서 읽어야 하니까 getter
    XMFLOAT3 GetPosition() const { return m_xmf3Position; }
    XMFLOAT3 GetLook()     const { return m_xmf3Look; }
    XMFLOAT3 GetUp()       const { return m_xmf3Up; }
    XMFLOAT3 GetRight()    const { return m_xmf3Right; }

private:
    // 이동/회전
    void Move(XMFLOAT3& vDir,float dt);
    void Rotate(float fPitch, float fYaw, float fRoll);
    void OnUpdateTransform(); // World 행렬 갱신

private:
    XMFLOAT3  m_xmf3Position = XMFLOAT3(0.f, 0.f, 0.f);
    XMFLOAT3  m_xmf3Right = XMFLOAT3(1.f, 0.f, 0.f);
    XMFLOAT3  m_xmf3Up = XMFLOAT3(0.f, 1.f, 0.f);
    XMFLOAT3  m_xmf3Look = XMFLOAT3(0.f, 0.f, 1.f);

    XMFLOAT3  m_xmf3Velocity = XMFLOAT3(0.f, 0.f, 0.f);
    float m_fFriction = 10.f;
    float m_fMoveSpeed = 30.f;
    float m_fRotSpeed = 90.f;
    float m_fMaxSpeed = 50.f;
    float     m_fPitch = 0.f;
    float     m_fYaw = 0.f;
    float     m_fRoll = 0.f;

    DWORD     m_dwColor = RGB(0, 200, 255); // ← 추가
};

