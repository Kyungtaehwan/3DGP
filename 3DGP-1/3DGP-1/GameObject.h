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
	BoundingOrientedBox m_xmLocalOBB = BoundingOrientedBox();

public:
	virtual void OnHit(int nDamage)
	{
		m_iHP -= nDamage;
		if (m_iHP <= 0)
		{
			m_iHP = 0;
			m_bDead = true;
		}
	}
	bool IsDead() const { return m_bDead; }
	int  GetHP()  const { return m_iHP; }
	int	 GetMaxHp() { return m_iMaxHP; }

	bool				m_bActive = true;
	bool                m_bDead = false;
	static bool			s_bShowOBB;

protected:
	int                 m_iMaxHP;
	int                 m_iHP;
	void UpdateBoundingBox();
	void RenderOBB(HDC hDC);
	void Render(HDC hDC, XMFLOAT4X4* pWorld, CMesh* pMesh, DWORD dwColor = RGB(255, 255, 255));


};