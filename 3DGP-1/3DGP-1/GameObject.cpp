#include "stdafx.h"
#include "GameObject.h"
#include "GraphicsPipeline.h"

CGameObject::CGameObject() {}

CGameObject::~CGameObject()
{
    if (m_pMesh) delete m_pMesh;
}

void CGameObject::UpdateBoundingBox()
{
    if (m_pMesh)
    {
        m_pMesh->m_xmOOBB.Transform(m_xmOOBB, XMLoadFloat4x4(&m_xmf4x4World));
        XMStoreFloat4(&m_xmOOBB.Orientation, XMQuaternionNormalize(XMLoadFloat4(&m_xmOOBB.Orientation)));
    }
}

void CGameObject::Render(HDC hDC, XMFLOAT4X4* pWorld, CMesh* pMesh, DWORD dwColor)
{
    if (!pMesh) return;

    CGraphicsPipeline::SetWorldTransform(pWorld);

    HPEN hPen = ::CreatePen(PS_SOLID, 0, dwColor);
    HPEN hOldPen = (HPEN)::SelectObject(hDC, hPen);
    pMesh->Render(hDC);
    ::SelectObject(hDC, hOldPen);
    ::DeleteObject(hPen);
}