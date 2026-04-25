#pragma once
#include "Level.h"

class CPlayer;
class CCamera;

class CLevel_GamePlay : public CLevel
{
public:
    CLevel_GamePlay();
    virtual ~CLevel_GamePlay();

    virtual void Initialize() override;
    virtual int  Update(float dt) override;
    virtual void Late_Update(float dt) override;
    virtual void Render(HDC hDC) override;
    virtual void Release() override;


private:
    void Set_GameWorld();
    float m_fDeathTimer = 0.f;
    float m_fDeathDelay = 3.f;  // 3초 뒤 레벨 전환
    bool  m_bDeathDetected = false;
private:
    CPlayer* m_pPlayer = nullptr; 
    CCamera* m_pCamera = nullptr; 
};