#include "stdafx.h"
#include "Terrain.h"
#include "GraphicsPipeline.h"

void CTerrain::Initialize()
{
	m_pMesh = new CTerrainMesh(500.f, 500.f, 50, 50);
	XMStoreFloat4x4(&m_xmf4x4World, XMMatrixIdentity());
}

int CTerrain::Update(float dt)
{
	return OBJ_NOEVENT;
}

void CTerrain::Late_Update(float dt)
{
}

void CTerrain::Render(HDC hDC)
{
	if (!m_pMesh) return;
	CGraphicsPipeline::SetWorldTransform(&m_xmf4x4World);
	m_pMesh->Render_Terrain_Face(hDC);
}

void CTerrain::Release()
{
	if (m_pMesh) {
		delete m_pMesh; m_pMesh = nullptr;
	}
}
