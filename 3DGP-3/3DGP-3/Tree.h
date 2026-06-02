#pragma once
#include "GameObject.h"

// A static scenery tree. Many instances share one already-loaded prototype's
// mesh and materials (cheap), each with its own world transform. Stays upright.
// Lives in the object manager's OBJ_TREE list; Update is a no-op (inherited).
class CTree : public CGameObject
{
public:
    CTree() {}
    virtual ~CTree() {}

    // Share mesh + materials from pProto (a hierarchy root loaded once), then
    // place this instance upright at pos, scaled uniformly.
    void InitInstance(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
                      CGameObject* pProto, float fScale, XMFLOAT3 pos);
};
