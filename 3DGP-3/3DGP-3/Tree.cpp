#include "pch.h"
#include "Tree.h"

void CTree::InitInstance(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
                         CGameObject* pProto, float fScale, XMFLOAT3 pos)
{
    if(!pProto)
        return;

    SetMesh(pProto->m_pMesh);
    if(pProto->m_nMaterials > 0)
    {
        m_nMaterials = pProto->m_nMaterials;
        m_ppMaterials = new CMaterial*[m_nMaterials];
        for(int i = 0; i < m_nMaterials; ++i)
        {
            m_ppMaterials[i] = NULL;
            SetMaterial(i, pProto->m_ppMaterials[i]);
        }
    }

    CreateShaderVariables(pd3dDevice, pd3dCommandList);

    XMMATRIX wm = XMMatrixScaling(fScale, fScale, fScale) *
        XMMatrixTranslation(pos.x, pos.y, pos.z);
    XMStoreFloat4x4(&m_xmf4x4Transform, wm);
    UpdateTransform(NULL);
}
