#include "stdafx.h"
#include "GameObject.h"
#include "GraphicsPipeline.h"

bool CGameObject::s_bShowOBB = true;

CGameObject::CGameObject() {}

CGameObject::~CGameObject()
{
    if (m_pMesh) delete m_pMesh;
}

void CGameObject::UpdateBoundingBox()
{

    BoundingOrientedBox& localOBB =
        (m_xmLocalOBB.Extents.x > 0.f) ? m_xmLocalOBB :
        (m_pMesh ? m_pMesh->m_xmOOBB : m_xmLocalOBB);

    localOBB.Transform(m_xmOOBB, XMLoadFloat4x4(&m_xmf4x4World));
    XMStoreFloat4(&m_xmOOBB.Orientation,
        XMQuaternionNormalize(XMLoadFloat4(&m_xmOOBB.Orientation)));
}

void CGameObject::Render(HDC hDC, XMFLOAT4X4* pWorld, CMesh* pMesh, DWORD dwColor)
{
    if (!pMesh) return;

    CGraphicsPipeline::SetWorldTransform(pWorld);

    HPEN hPen = ::CreatePen(PS_SOLID, 0, dwColor);
    HPEN hOldPen = (HPEN)::SelectObject(hDC, hPen);
    pMesh->m_dwColor = dwColor;
    pMesh->Render_Face(hDC);
    ::SelectObject(hDC, hOldPen);
    ::DeleteObject(hPen);
}

void CGameObject::RenderOBB(HDC hDC)
{
    if (!s_bShowOBB) return;

    XMFLOAT3 corners[8];
    m_xmOOBB.GetCorners(corners);


    XMFLOAT4X4 identity = Matrix4x4::Identity();
    CGraphicsPipeline::SetWorldTransform(&identity);

    XMFLOAT3 proj[8];
    bool valid[8];
    for (int i = 0; i < 8; i++)
    {
        proj[i] = CGraphicsPipeline::Project(corners[i]);
        valid[i] = (proj[i].z >= 0.f && proj[i].z <= 1.f);
    }

    HPEN hPen = ::CreatePen(PS_SOLID, 1, RGB(0, 255, 0));
    HPEN hOldPen = (HPEN)::SelectObject(hDC, hPen);


    auto DrawEdge = [&](int a, int b) {
        if (valid[a] && valid[b])
            Draw2DLine(hDC, proj[a], proj[b]);
        };

    // 앞면
    DrawEdge(0, 1); DrawEdge(1, 2); DrawEdge(2, 3); DrawEdge(3, 0);
    // 뒷면
    DrawEdge(4, 5); DrawEdge(5, 6); DrawEdge(6, 7); DrawEdge(7, 4);
    // 연결선
    DrawEdge(0, 4); DrawEdge(1, 5); DrawEdge(2, 6); DrawEdge(3, 7);

    ::SelectObject(hDC, hOldPen);
    ::DeleteObject(hPen);
}