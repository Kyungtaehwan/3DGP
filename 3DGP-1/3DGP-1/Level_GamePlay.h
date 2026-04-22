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
    CPlayer* m_pPlayer = nullptr; 
    CCamera* m_pCamera = nullptr; 
};