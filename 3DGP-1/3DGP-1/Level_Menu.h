#pragma once
#include "Level.h"
#include "Button.h"

class CLevel_Menu : public CLevel
{
public:
    CLevel_Menu();
    virtual ~CLevel_Menu();

    virtual void Initialize() override;
    virtual int  Update(float dt) override;
    virtual void Late_Update(float dt) override;
    virtual void Render(HDC hDC) override;
    virtual void Release() override;

private:
    CButton m_btnStart;
};