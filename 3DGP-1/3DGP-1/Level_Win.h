#pragma once
#include "Level.h"
#include "Button.h"
class CLevel_Win : public CLevel
{
public:
    CLevel_Win() {}
    virtual ~CLevel_Win() { Release(); }

    virtual void Initialize() override;
    virtual int  Update(float dt) override;
    virtual void Late_Update(float dt) override;
    virtual void Render(HDC hDC) override;
    virtual void Release() override;

private:
    CButton m_btnMenu;
};