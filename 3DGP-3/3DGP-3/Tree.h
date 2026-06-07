#pragma once
#include "GameObject.h"

class CTree : public CGameObject
{
public:
    CTree() {}
    virtual ~CTree() {}

    void InitInstance(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
                      CGameObject* pProto, float fScale, XMFLOAT3 pos);
};
