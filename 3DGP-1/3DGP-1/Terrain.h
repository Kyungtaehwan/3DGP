#pragma once
#include "GameObject.h"

class CTerrain : public CGameObject
{
public:
    CTerrain() {}
    ~CTerrain() { Release(); }

    virtual void    Initialize() override;
    virtual int     Update(float dt) override;
    virtual void    Late_Update(float dt) override;
    virtual void    Render(HDC hDC) override;
    virtual void    Release() override;

private:

    CTerrainMesh* m_pMesh = nullptr;
};