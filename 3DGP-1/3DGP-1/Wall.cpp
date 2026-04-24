#include "stdafx.h"
#include "Wall.h"

#include "GraphicsPipeline.h"

CWall::CWall() {}
CWall::~CWall() { Release(); }

void CWall::Initialize() {


}

void CWall::SetupWall(float x, float z, float fWidth, float fHeight, float fDepth)
{
    m_fWidth = fWidth;
    m_fHeight = fHeight;
    m_fDepth = fDepth;

    m_pMesh = new CCubeMesh(fWidth, fHeight, fDepth);

    XMStoreFloat4x4(&m_xmf4x4World,
        XMMatrixTranslation(x, fHeight * 0.5f, z));

    m_xmLocalOBB = BoundingOrientedBox(
        XMFLOAT3(0.f, 0.f, 0.f),
        XMFLOAT3(fWidth * 0.5f, fHeight * 0.5f, fDepth * 0.5f),
        XMFLOAT4(0.f, 0.f, 0.f, 1.f));
    UpdateBoundingBox();
}

int CWall::Update(float dt)
{
    return OBJ_NOEVENT;
}

void CWall::Late_Update(float dt) {}

void CWall::Render(HDC hDC)
{
    CGameObject::Render(hDC, &m_xmf4x4World, m_pMesh, RGB(80, 80, 80));
    CGraphicsPipeline::SetWorldTransform(&m_xmf4x4World);
    HPEN hPen = ::CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
    HPEN hOldPen = (HPEN)::SelectObject(hDC, hPen);
    m_pMesh->Render_Outline(hDC);
    ::SelectObject(hDC, hOldPen);
    ::DeleteObject(hPen);
    RenderOBB(hDC);
}

void CWall::Release()
{
    if (m_pMesh) { delete m_pMesh; m_pMesh = nullptr; }
}