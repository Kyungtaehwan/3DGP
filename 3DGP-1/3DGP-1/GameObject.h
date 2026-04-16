#pragma once
#include "define.h"
#include "Mesh.h"

class CGameObject
{
public:
	CGameObject();
	virtual ~CGameObject();

public:
	virtual void Initialize() = 0;
	virtual int	 Update(float dt) = 0;
	virtual void Late_Update(float dt) = 0;
	virtual void Render(HDC hDC) = 0;
	virtual void Release(void) = 0;
public:
	CMesh* m_pMesh = NULL;
	XMFLOAT4X4   m_xmf4x4World = Matrix4x4::Identity();
	BoundingOrientedBox m_xmOOBB = BoundingOrientedBox();
	bool         m_bActive = true;

protected:
	void UpdateBoundingBox();
	void Render(HDC hDC, XMFLOAT4X4* pWorld, CMesh* pMesh, DWORD dwColor = RGB(255, 255, 255));

};