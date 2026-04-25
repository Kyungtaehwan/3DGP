#pragma once
#include "Level.h"

class CLevel_Lose : public CLevel
{
public:
    CLevel_Lose() {}
    virtual ~CLevel_Lose() { Release(); }

    virtual void Initialize() override;
    virtual int  Update(float dt) override;
    virtual void Late_Update(float dt) override;
    virtual void Render(HDC hDC) override;
    virtual void Release() override;
};