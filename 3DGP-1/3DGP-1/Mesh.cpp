#include "stdafx.h"
#include "Mesh.h"
#include "GraphicsPipeline.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
CPolygon::CPolygon(int nVertices)
{
	m_nVertices = nVertices;
	m_pVertices = new CVertex[nVertices];
}

CPolygon::~CPolygon()
{
	if (m_pVertices) delete[] m_pVertices;
}

void CPolygon::SetVertex(int nIndex, CVertex& vertex)
{
	if ((0 <= nIndex) && (nIndex < m_nVertices) && m_pVertices)
	{
		m_pVertices[nIndex] = vertex;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
CMesh::CMesh(int nPolygons)
{
	m_nPolygons = nPolygons;
	m_ppPolygons = new CPolygon * [nPolygons];
}

CMesh::~CMesh()
{
	if (m_ppPolygons)
	{
		for (int i = 0; i < m_nPolygons; i++) if (m_ppPolygons[i]) delete m_ppPolygons[i];
		delete[] m_ppPolygons;
	}
}

void CMesh::SetPolygon(int nIndex, CPolygon* pPolygon)
{
	if ((0 <= nIndex) && (nIndex < m_nPolygons)) m_ppPolygons[nIndex] = pPolygon;
}

void Draw2DLine(HDC hDCFrameBuffer, XMFLOAT3& f3PreviousProject, XMFLOAT3& f3CurrentProject)
{
	XMFLOAT3 f3Previous = CGraphicsPipeline::ScreenTransform(f3PreviousProject);
	XMFLOAT3 f3Current = CGraphicsPipeline::ScreenTransform(f3CurrentProject);
	::MoveToEx(hDCFrameBuffer, (long)f3Previous.x, (long)f3Previous.y, NULL);
	::LineTo(hDCFrameBuffer, (long)f3Current.x, (long)f3Current.y);
}

void CMesh::Render(HDC hDCFrameBuffer)
{
	for (int j = 0; j < m_nPolygons; j++)
	{
		int nVertices = m_ppPolygons[j]->m_nVertices;
		CVertex* pVertices = m_ppPolygons[j]->m_pVertices;

		XMFLOAT3 f3InitialProject = CGraphicsPipeline::Project(pVertices[0].m_xmf3Position);
		XMFLOAT3 f3PreviousProject = f3InitialProject;
		bool bInitialValid = (f3InitialProject.z >= 0.0f && f3InitialProject.z <= 1.0f);
		bool bPreviousValid = bInitialValid;

		for (int i = 1; i < nVertices; i++)
		{
			XMFLOAT3 f3CurrentProject = CGraphicsPipeline::Project(pVertices[i].m_xmf3Position);
			bool bCurrentValid = (f3CurrentProject.z >= 0.0f && f3CurrentProject.z <= 1.0f);

			// z 유효한 두 점이면 무조건 그림 (x,y 범위 체크 없음)
			if (bPreviousValid && bCurrentValid)
			{
				Draw2DLine(hDCFrameBuffer, f3PreviousProject, f3CurrentProject);
			}

			f3PreviousProject = f3CurrentProject;
			bPreviousValid = bCurrentValid;
		}

		// 마지막 닫기 선
		if (bPreviousValid && bInitialValid)
		{
			Draw2DLine(hDCFrameBuffer, f3PreviousProject, f3InitialProject);
		}
	}
}
void CMesh::Render_Face(HDC hDCFrameBuffer)
{
	for (int j = 0; j < m_nPolygons; j++)
	{
		int nVerts = m_ppPolygons[j]->m_nVertices;
		CVertex* pVerts = m_ppPolygons[j]->m_pVertices;

		std::vector<XMFLOAT3> projs(nVerts);
		bool bSkip = false;

		for (int v = 0; v < nVerts; v++)
		{
			// World * ViewProject 합성
			XMFLOAT4X4 xmf4x4Transform = Matrix4x4::Multiply(
				*CGraphicsPipeline::GetWorldMatrix(),
				*CGraphicsPipeline::GetViewProjectMatrix());
			XMMATRIX mat = XMLoadFloat4x4(&xmf4x4Transform);

			// w 나누기 전 좌표 계산
			XMVECTOR vIn = XMVectorSet(
				pVerts[v].m_xmf3Position.x,
				pVerts[v].m_xmf3Position.y,
				pVerts[v].m_xmf3Position.z, 1.f);
			XMVECTOR vOut = XMVector4Transform(vIn, mat);
			XMFLOAT4 f4;
			XMStoreFloat4(&f4, vOut);

			// w <= 0 이면 카메라 뒤 → 폴리곤 스킵
			if (f4.w <= 0.f) { bSkip = true; break; }

			projs[v] = CGraphicsPipeline::Project(pVerts[v].m_xmf3Position);
		}
		if (bSkip) continue;

		std::vector<POINT> pts(nVerts);
		for (int v = 0; v < nVerts; v++)
		{
			XMFLOAT3 screen = CGraphicsPipeline::ScreenTransform(projs[v]);
			pts[v] = { (long)screen.x, (long)screen.y };
		}

		HBRUSH hBrush = CreateSolidBrush(m_dwColor);
		HBRUSH hOldBrush = (HBRUSH)SelectObject(hDCFrameBuffer, hBrush);
		HPEN hPen = CreatePen(PS_SOLID, 0, m_dwColor);
		HPEN hOldPen = (HPEN)SelectObject(hDCFrameBuffer, hPen);
		::Polygon(hDCFrameBuffer, pts.data(), nVerts);
		SelectObject(hDCFrameBuffer, hOldBrush);
		SelectObject(hDCFrameBuffer, hOldPen);
		DeleteObject(hBrush);
		DeleteObject(hPen);
	}
}

void CMesh::Render_Outline(HDC hDCFrameBuffer)
{
	for (int j = 0; j < m_nPolygons; j++)
	{
		int nVerts = m_ppPolygons[j]->m_nVertices;
		CVertex* pVerts = m_ppPolygons[j]->m_pVertices;

		// Render_Face와 동일한 w 체크
		XMFLOAT4X4 xmf4x4Transform = Matrix4x4::Multiply(
			*CGraphicsPipeline::GetWorldMatrix(),
			*CGraphicsPipeline::GetViewProjectMatrix());
		XMMATRIX mat = XMLoadFloat4x4(&xmf4x4Transform);

		bool bSkip = false;
		std::vector<XMFLOAT3> projs(nVerts);
		for (int v = 0; v < nVerts; v++)
		{
			XMVECTOR vIn = XMVectorSet(
				pVerts[v].m_xmf3Position.x,
				pVerts[v].m_xmf3Position.y,
				pVerts[v].m_xmf3Position.z, 1.f);
			XMVECTOR vOut = XMVector4Transform(vIn, mat);
			XMFLOAT4 f4;
			XMStoreFloat4(&f4, vOut);

			if (f4.w <= 0.f) { bSkip = true; break; }

			projs[v] = CGraphicsPipeline::Project(pVerts[v].m_xmf3Position);
		}
		if (bSkip) continue;

		// 폴리곤 외곽선 그리기
		for (int i = 0; i < nVerts; i++)
		{
			int next = (i + 1) % nVerts;
			Draw2DLine(hDCFrameBuffer, projs[i], projs[next]);
		}
	}
}

void CMesh::Render_Terrain_Face(HDC hDCFrameBuffer)
{
	for (int j = 0; j < m_nPolygons; j++)
	{
		int nVerts = m_ppPolygons[j]->m_nVertices;
		CVertex* pVerts = m_ppPolygons[j]->m_pVertices;

		std::vector<XMFLOAT3> projs(nVerts);
		bool bSkip = false;
		for (int v = 0; v < nVerts; v++)
		{
			projs[v] = CGraphicsPipeline::Project(pVerts[v].m_xmf3Position);
			if (projs[v].z < 0.0f || projs[v].z > 1.0f)
			{
				bSkip = true;
				break;
			}
		}
		if (bSkip) continue;

		std::vector<POINT> pts(nVerts);
		for (int v = 0; v < nVerts; v++)
		{
			XMFLOAT3 screen = CGraphicsPipeline::ScreenTransform(projs[v]);
			pts[v] = { (long)screen.x, (long)screen.y };
		}

		COLORREF color = RGB(15, 30, 15);
		HBRUSH hBrush = CreateSolidBrush(color);
		HBRUSH hOldBrush = (HBRUSH)SelectObject(hDCFrameBuffer, hBrush);
		HPEN hOldPen = (HPEN)SelectObject(hDCFrameBuffer, GetStockObject(NULL_PEN));
		::Polygon(hDCFrameBuffer, pts.data(), nVerts);
		SelectObject(hDCFrameBuffer, hOldBrush);
		SelectObject(hDCFrameBuffer, hOldPen);
		DeleteObject(hBrush);
	}
}
BOOL CMesh::RayIntersectionByTriangle(XMVECTOR& xmRayOrigin, XMVECTOR& xmRayDirection, XMVECTOR v0, XMVECTOR v1, XMVECTOR v2, float* pfNearHitDistance)
{
	float fHitDistance;
	BOOL bIntersected = TriangleTests::Intersects(xmRayOrigin, xmRayDirection, v0, v1, v2, fHitDistance);
	if (bIntersected && (fHitDistance < *pfNearHitDistance)) *pfNearHitDistance = fHitDistance;

	return(bIntersected);
}

int CMesh::CheckRayIntersection(XMVECTOR& xmvPickRayOrigin, XMVECTOR& xmvPickRayDirection, float* pfNearHitDistance)
{
	int nIntersections = 0;
	bool bIntersected = m_xmOOBB.Intersects(xmvPickRayOrigin, xmvPickRayDirection, *pfNearHitDistance);
	if (bIntersected)
	{
		for (int i = 0; i < m_nPolygons; i++)
		{
			switch (m_ppPolygons[i]->m_nVertices)
			{
			case 3:
			{
				XMVECTOR v0 = XMLoadFloat3(&(m_ppPolygons[i]->m_pVertices[0].m_xmf3Position));
				XMVECTOR v1 = XMLoadFloat3(&(m_ppPolygons[i]->m_pVertices[1].m_xmf3Position));
				XMVECTOR v2 = XMLoadFloat3(&(m_ppPolygons[i]->m_pVertices[2].m_xmf3Position));
				BOOL bIntersected = RayIntersectionByTriangle(xmvPickRayOrigin, xmvPickRayDirection, v0, v1, v2, pfNearHitDistance);
				if (bIntersected) nIntersections++;
				break;
			}
			case 4:
			{
				XMVECTOR v0 = XMLoadFloat3(&(m_ppPolygons[i]->m_pVertices[0].m_xmf3Position));
				XMVECTOR v1 = XMLoadFloat3(&(m_ppPolygons[i]->m_pVertices[1].m_xmf3Position));
				XMVECTOR v2 = XMLoadFloat3(&(m_ppPolygons[i]->m_pVertices[2].m_xmf3Position));
				BOOL bIntersected = RayIntersectionByTriangle(xmvPickRayOrigin, xmvPickRayDirection, v0, v1, v2, pfNearHitDistance);
				if (bIntersected) nIntersections++;
				v0 = XMLoadFloat3(&(m_ppPolygons[i]->m_pVertices[0].m_xmf3Position));
				v1 = XMLoadFloat3(&(m_ppPolygons[i]->m_pVertices[2].m_xmf3Position));
				v2 = XMLoadFloat3(&(m_ppPolygons[i]->m_pVertices[3].m_xmf3Position));
				bIntersected = RayIntersectionByTriangle(xmvPickRayOrigin, xmvPickRayDirection, v0, v1, v2, pfNearHitDistance);
				if (bIntersected) nIntersections++;
				break;
			}
			}
		}
	}
	return(nIntersections);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
CCubeMesh::CCubeMesh(float fWidth, float fHeight, float fDepth) : CMesh(6)
{
	float fHalfWidth = fWidth * 0.5f;
	float fHalfHeight = fHeight * 0.5f;
	float fHalfDepth = fDepth * 0.5f;

	CPolygon* pFrontFace = new CPolygon(4);
	pFrontFace->SetVertex(0, CVertex(-fHalfWidth, +fHalfHeight, -fHalfDepth));
	pFrontFace->SetVertex(1, CVertex(+fHalfWidth, +fHalfHeight, -fHalfDepth));
	pFrontFace->SetVertex(2, CVertex(+fHalfWidth, -fHalfHeight, -fHalfDepth));
	pFrontFace->SetVertex(3, CVertex(-fHalfWidth, -fHalfHeight, -fHalfDepth));
	SetPolygon(0, pFrontFace);

	CPolygon* pTopFace = new CPolygon(4);
	pTopFace->SetVertex(0, CVertex(-fHalfWidth, +fHalfHeight, +fHalfDepth));
	pTopFace->SetVertex(1, CVertex(+fHalfWidth, +fHalfHeight, +fHalfDepth));
	pTopFace->SetVertex(2, CVertex(+fHalfWidth, +fHalfHeight, -fHalfDepth));
	pTopFace->SetVertex(3, CVertex(-fHalfWidth, +fHalfHeight, -fHalfDepth));
	SetPolygon(1, pTopFace);

	CPolygon* pBackFace = new CPolygon(4);
	pBackFace->SetVertex(0, CVertex(-fHalfWidth, -fHalfHeight, +fHalfDepth));
	pBackFace->SetVertex(1, CVertex(+fHalfWidth, -fHalfHeight, +fHalfDepth));
	pBackFace->SetVertex(2, CVertex(+fHalfWidth, +fHalfHeight, +fHalfDepth));
	pBackFace->SetVertex(3, CVertex(-fHalfWidth, +fHalfHeight, +fHalfDepth));
	SetPolygon(2, pBackFace);

	CPolygon* pBottomFace = new CPolygon(4);
	pBottomFace->SetVertex(0, CVertex(-fHalfWidth, -fHalfHeight, -fHalfDepth));
	pBottomFace->SetVertex(1, CVertex(+fHalfWidth, -fHalfHeight, -fHalfDepth));
	pBottomFace->SetVertex(2, CVertex(+fHalfWidth, -fHalfHeight, +fHalfDepth));
	pBottomFace->SetVertex(3, CVertex(-fHalfWidth, -fHalfHeight, +fHalfDepth));
	SetPolygon(3, pBottomFace);

	CPolygon* pLeftFace = new CPolygon(4);
	pLeftFace->SetVertex(0, CVertex(-fHalfWidth, +fHalfHeight, +fHalfDepth));
	pLeftFace->SetVertex(1, CVertex(-fHalfWidth, +fHalfHeight, -fHalfDepth));
	pLeftFace->SetVertex(2, CVertex(-fHalfWidth, -fHalfHeight, -fHalfDepth));
	pLeftFace->SetVertex(3, CVertex(-fHalfWidth, -fHalfHeight, +fHalfDepth));
	SetPolygon(4, pLeftFace);

	CPolygon* pRightFace = new CPolygon(4);
	pRightFace->SetVertex(0, CVertex(+fHalfWidth, +fHalfHeight, -fHalfDepth));
	pRightFace->SetVertex(1, CVertex(+fHalfWidth, +fHalfHeight, +fHalfDepth));
	pRightFace->SetVertex(2, CVertex(+fHalfWidth, -fHalfHeight, +fHalfDepth));
	pRightFace->SetVertex(3, CVertex(+fHalfWidth, -fHalfHeight, -fHalfDepth));
	SetPolygon(5, pRightFace);

	m_xmOOBB = BoundingOrientedBox(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(fHalfWidth, fHalfHeight, fHalfDepth), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
}



CAxisMesh::CAxisMesh(float fWidth, float fHeight, float fDepth) : CMesh(3)
{
	float fHalfWidth = fWidth * 0.5f;
	float fHalfHeight = fHeight * 0.5f;
	float fHalfDepth = fDepth * 0.5f;

	CPolygon* pxAxis = new CPolygon(2);
	pxAxis->SetVertex(0, CVertex(-fHalfWidth, 0.0f, 0.0f));
	pxAxis->SetVertex(1, CVertex(+fHalfWidth, 0.0f, 0.0f));
	SetPolygon(0, pxAxis);

	CPolygon* pyAxis = new CPolygon(2);
	pyAxis->SetVertex(0, CVertex(0.0f, -fHalfWidth, 0.0f));
	pyAxis->SetVertex(1, CVertex(0.0f, +fHalfWidth, 0.0f));
	SetPolygon(1, pyAxis);

	CPolygon* pzAxis = new CPolygon(2);
	pzAxis->SetVertex(0, CVertex(0.0f, 0.0f, -fHalfWidth));
	pzAxis->SetVertex(1, CVertex(0.0f, 0.0f, +fHalfWidth));
	SetPolygon(2, pzAxis);
}

void CAxisMesh::Render(HDC hDCFrameBuffer)
{
	XMFLOAT3 f3PreviousProject = CGraphicsPipeline::Project(m_ppPolygons[0]->m_pVertices[0].m_xmf3Position);
	XMFLOAT3 f3CurrentProject = CGraphicsPipeline::Project(m_ppPolygons[0]->m_pVertices[1].m_xmf3Position);
	HPEN hPen = ::CreatePen(PS_SOLID, 0, RGB(255, 0, 0));
	HPEN hOldPen = (HPEN)::SelectObject(hDCFrameBuffer, hPen);
	::Draw2DLine(hDCFrameBuffer, f3PreviousProject, f3CurrentProject);
	::SelectObject(hDCFrameBuffer, hOldPen);
	::DeleteObject(hPen);

	f3PreviousProject = CGraphicsPipeline::Project(m_ppPolygons[1]->m_pVertices[0].m_xmf3Position);
	f3CurrentProject = CGraphicsPipeline::Project(m_ppPolygons[1]->m_pVertices[1].m_xmf3Position);
	hPen = ::CreatePen(PS_SOLID, 0, RGB(0, 0, 255));
	hOldPen = (HPEN)::SelectObject(hDCFrameBuffer, hPen);
	::Draw2DLine(hDCFrameBuffer, f3PreviousProject, f3CurrentProject);
	::SelectObject(hDCFrameBuffer, hOldPen);
	::DeleteObject(hPen);

	f3PreviousProject = CGraphicsPipeline::Project(m_ppPolygons[2]->m_pVertices[0].m_xmf3Position);
	f3CurrentProject = CGraphicsPipeline::Project(m_ppPolygons[2]->m_pVertices[1].m_xmf3Position);
	hPen = ::CreatePen(PS_SOLID, 0, RGB(0, 255, 0));
	hOldPen = (HPEN)::SelectObject(hDCFrameBuffer, hPen);
	::Draw2DLine(hDCFrameBuffer, f3PreviousProject, f3CurrentProject);
	::SelectObject(hDCFrameBuffer, hOldPen);
	::DeleteObject(hPen);
}



CTankBodyMesh :: CTankBodyMesh(float fWidth, float fHeight, float fDepth) : CMesh(6)
{
	float hw = fWidth * 0.5f;
	float hh = fHeight * 0.5f;
	float hd = fDepth * 0.5f;

	// Front  (-Z)
	CPolygon* pFront = new CPolygon(4);
	pFront->SetVertex(0, CVertex(-hw, +hh, -hd));
	pFront->SetVertex(1, CVertex(+hw, +hh, -hd));
	pFront->SetVertex(2, CVertex(+hw, -hh, -hd));
	pFront->SetVertex(3, CVertex(-hw, -hh, -hd));
	SetPolygon(0, pFront);

	// Back   (+Z)
	CPolygon* pBack = new CPolygon(4);
	pBack->SetVertex(0, CVertex(+hw, +hh, +hd));
	pBack->SetVertex(1, CVertex(-hw, +hh, +hd));
	pBack->SetVertex(2, CVertex(-hw, -hh, +hd));
	pBack->SetVertex(3, CVertex(+hw, -hh, +hd));
	SetPolygon(1, pBack);

	// Top    (+Y)
	CPolygon* pTop = new CPolygon(4);
	pTop->SetVertex(0, CVertex(-hw, +hh, -hd));
	pTop->SetVertex(1, CVertex(+hw, +hh, -hd));
	pTop->SetVertex(2, CVertex(+hw, +hh, +hd));
	pTop->SetVertex(3, CVertex(-hw, +hh, +hd));
	SetPolygon(2, pTop);

	// Bottom (-Y)
	CPolygon* pBottom = new CPolygon(4);
	pBottom->SetVertex(0, CVertex(-hw, -hh, +hd));
	pBottom->SetVertex(1, CVertex(+hw, -hh, +hd));
	pBottom->SetVertex(2, CVertex(+hw, -hh, -hd));
	pBottom->SetVertex(3, CVertex(-hw, -hh, -hd));
	SetPolygon(3, pBottom);

	// Left   (-X)
	CPolygon* pLeft = new CPolygon(4);
	pLeft->SetVertex(0, CVertex(-hw, +hh, +hd));
	pLeft->SetVertex(1, CVertex(-hw, +hh, -hd));
	pLeft->SetVertex(2, CVertex(-hw, -hh, -hd));
	pLeft->SetVertex(3, CVertex(-hw, -hh, +hd));
	SetPolygon(4, pLeft);

	// Right  (+X)
	CPolygon* pRight = new CPolygon(4);
	pRight->SetVertex(0, CVertex(+hw, +hh, -hd));
	pRight->SetVertex(1, CVertex(+hw, +hh, +hd));
	pRight->SetVertex(2, CVertex(+hw, -hh, +hd));
	pRight->SetVertex(3, CVertex(+hw, -hh, -hd));
	SetPolygon(5, pRight);

	m_xmOOBB = BoundingOrientedBox(
		XMFLOAT3(0.f, 0.f, 0.f),
		XMFLOAT3(hw, hh, hd),
		XMFLOAT4(0.f, 0.f, 0.f, 1.f));
}




CTankTurretMesh::CTankTurretMesh(float fWidth, float fHeight, float fDepth) : CMesh(6)
{
	float hw = fWidth * 0.5f;
	float hh = fHeight * 0.5f;
	float hd = fDepth * 0.5f;

	// Front
	CPolygon* pFront = new CPolygon(4);
	pFront->SetVertex(0, CVertex(-hw, +hh, -hd));
	pFront->SetVertex(1, CVertex(+hw, +hh, -hd));
	pFront->SetVertex(2, CVertex(+hw, -hh, -hd));
	pFront->SetVertex(3, CVertex(-hw, -hh, -hd));
	SetPolygon(0, pFront);

	// Back
	CPolygon* pBack = new CPolygon(4);
	pBack->SetVertex(0, CVertex(+hw, +hh, +hd));
	pBack->SetVertex(1, CVertex(-hw, +hh, +hd));
	pBack->SetVertex(2, CVertex(-hw, -hh, +hd));
	pBack->SetVertex(3, CVertex(+hw, -hh, +hd));
	SetPolygon(1, pBack);

	// Top
	CPolygon* pTop = new CPolygon(4);
	pTop->SetVertex(0, CVertex(-hw, +hh, -hd));
	pTop->SetVertex(1, CVertex(+hw, +hh, -hd));
	pTop->SetVertex(2, CVertex(+hw, +hh, +hd));
	pTop->SetVertex(3, CVertex(-hw, +hh, +hd));
	SetPolygon(2, pTop);

	// Bottom
	CPolygon* pBottom = new CPolygon(4);
	pBottom->SetVertex(0, CVertex(-hw, -hh, +hd));
	pBottom->SetVertex(1, CVertex(+hw, -hh, +hd));
	pBottom->SetVertex(2, CVertex(+hw, -hh, -hd));
	pBottom->SetVertex(3, CVertex(-hw, -hh, -hd));
	SetPolygon(3, pBottom);

	// Left
	CPolygon* pLeft = new CPolygon(4);
	pLeft->SetVertex(0, CVertex(-hw, +hh, +hd));
	pLeft->SetVertex(1, CVertex(-hw, +hh, -hd));
	pLeft->SetVertex(2, CVertex(-hw, -hh, -hd));
	pLeft->SetVertex(3, CVertex(-hw, -hh, +hd));
	SetPolygon(4, pLeft);

	// Right
	CPolygon* pRight = new CPolygon(4);
	pRight->SetVertex(0, CVertex(+hw, +hh, -hd));
	pRight->SetVertex(1, CVertex(+hw, +hh, +hd));
	pRight->SetVertex(2, CVertex(+hw, -hh, +hd));
	pRight->SetVertex(3, CVertex(+hw, -hh, -hd));
	SetPolygon(5, pRight);

	m_xmOOBB = BoundingOrientedBox(
		XMFLOAT3(0.f, 0.f, 0.f),
		XMFLOAT3(hw, hh, hd),
		XMFLOAT4(0.f, 0.f, 0.f, 1.f));
}



CTankBarrelMesh::CTankBarrelMesh(float fLength, float fRadius) : CMesh(6)
{
	float hr = fRadius;
	float zNear = 0.0f;      // 포신 뿌리(포탑 연결)
	float zFar = fLength;   // 포신 끝(앞) - +Z 방향으로 변경

	// Front (포신 뿌리 캡)
	CPolygon* pFront = new CPolygon(4);
	pFront->SetVertex(0, CVertex(-hr, +hr, zNear));
	pFront->SetVertex(1, CVertex(+hr, +hr, zNear));
	pFront->SetVertex(2, CVertex(+hr, -hr, zNear));
	pFront->SetVertex(3, CVertex(-hr, -hr, zNear));
	SetPolygon(0, pFront);

	// Back (포신 끝 캡)
	CPolygon* pBack = new CPolygon(4);
	pBack->SetVertex(0, CVertex(+hr, +hr, zFar));
	pBack->SetVertex(1, CVertex(-hr, +hr, zFar));
	pBack->SetVertex(2, CVertex(-hr, -hr, zFar));
	pBack->SetVertex(3, CVertex(+hr, -hr, zFar));
	SetPolygon(1, pBack);

	// Top
	CPolygon* pTop = new CPolygon(4);
	pTop->SetVertex(0, CVertex(-hr, +hr, zNear));
	pTop->SetVertex(1, CVertex(+hr, +hr, zNear));
	pTop->SetVertex(2, CVertex(+hr, +hr, zFar));
	pTop->SetVertex(3, CVertex(-hr, +hr, zFar));
	SetPolygon(2, pTop);

	// Bottom
	CPolygon* pBottom = new CPolygon(4);
	pBottom->SetVertex(0, CVertex(-hr, -hr, zNear));
	pBottom->SetVertex(1, CVertex(+hr, -hr, zNear));
	pBottom->SetVertex(2, CVertex(+hr, -hr, zFar));
	pBottom->SetVertex(3, CVertex(-hr, -hr, zFar));
	SetPolygon(3, pBottom);

	// Left
	CPolygon* pLeft = new CPolygon(4);
	pLeft->SetVertex(0, CVertex(-hr, +hr, zNear));
	pLeft->SetVertex(1, CVertex(-hr, +hr, zFar));
	pLeft->SetVertex(2, CVertex(-hr, -hr, zFar));
	pLeft->SetVertex(3, CVertex(-hr, -hr, zNear));
	SetPolygon(4, pLeft);

	// Right
	CPolygon* pRight = new CPolygon(4);
	pRight->SetVertex(0, CVertex(+hr, +hr, zFar));
	pRight->SetVertex(1, CVertex(+hr, +hr, zNear));
	pRight->SetVertex(2, CVertex(+hr, -hr, zNear));
	pRight->SetVertex(3, CVertex(+hr, -hr, zFar));
	SetPolygon(5, pRight);

	m_xmOOBB = BoundingOrientedBox(
		XMFLOAT3(0.f, 0.f, fLength * 0.5f),
		XMFLOAT3(hr, hr, fLength * 0.5f),
		XMFLOAT4(0.f, 0.f, 0.f, 1.f));
}


CTerrainMesh::CTerrainMesh(float fWidth, float fDepth, int nSubX, int nSubZ)
	: CMesh(nSubX* nSubZ)
{
	float hw = fWidth * 0.5f;
	float hd = fDepth * 0.5f;
	float cellW = fWidth / nSubX;
	float cellD = fDepth / nSubZ;

	int k = 0;
	for (int i = 0; i < nSubZ; i++)
	{
		for (int j = 0; j < nSubX; j++)
		{
			float x0 = -hw + j * cellW;
			float x1 = x0 + cellW;
			float z0 = -hd + i * cellD;
			float z1 = z0 + cellD;

			CPolygon* pFace = new CPolygon(4);
			pFace->SetVertex(0, CVertex(x0, 0.f, z0));
			pFace->SetVertex(1, CVertex(x1, 0.f, z0));
			pFace->SetVertex(2, CVertex(x1, 0.f, z1));
			pFace->SetVertex(3, CVertex(x0, 0.f, z1));
			SetPolygon(k++, pFace);
		}
	}
}