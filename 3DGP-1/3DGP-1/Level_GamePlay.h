#pragma once
#include "Level.h"
#include "Player.h"
#include "Camera.h"

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
    CPlayer* m_pPlayer = nullptr;  // ← 추가
    CCamera* m_pCamera = nullptr;  // ← 추가
};