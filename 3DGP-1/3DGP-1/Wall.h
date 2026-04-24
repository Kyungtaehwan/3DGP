#pragma once
#include "GameObject.h"
class CWall : public CGameObject
{
public:
    CWall();
    virtual ~CWall();

    virtual void    Initialize() override;
    virtual int     Update(float dt) override;
    virtual void    Late_Update(float dt) override;
    virtual void    Render(HDC hDC) override;
    virtual void    Release() override;

    void            SetupWall(float x, float z, float fWidth, float fHeight, float fDepth);

private:
    CMesh* m_pMesh = nullptr;
    float           m_fWidth = 0.f;
    float           m_fHeight = 0.f;
    float           m_fDepth = 0.f;

};

