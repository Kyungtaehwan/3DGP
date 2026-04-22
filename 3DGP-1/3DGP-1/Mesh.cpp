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

		bool bVisible = false;
		for (int v = 0; v < nVerts; v++)
		{
			XMFLOAT3 proj = CGraphicsPipeline::Project(pVerts[v].m_xmf3Position);
			if (proj.z >= 0.f && proj.z <= 1.f) { bVisible = true; break; }
		}
		if (!bVisible) continue;

		std::vector<POINT> pts(nVerts);
		for (int v = 0; v < nVerts; v++)
		{
			XMFLOAT3 proj = CGraphicsPipeline::Project(pVerts[v].m_xmf3Position);
			XMFLOAT3 screen = CGraphicsPipeline::ScreenTransform(proj);
			pts[v] = { (long)screen.x, (long)screen.y };
		}

		HBRUSH hBrush = CreateSolidBrush(m_dwColor);
		HBRUSH hOldBrush = (HBRUSH)SelectObject(hDCFrameBuffer, hBrush);
		HPEN   hPen = CreatePen(PS_SOLID, 0, m_dwColor);
		HPEN   hOldPen = (HPEN)SelectObject(hDCFrameBuffer, hPen);
		::Polygon(hDCFrameBuffer, pts.data(), nVerts);
		SelectObject(hDCFrameBuffer, hOldBrush);
		SelectObject(hDCFrameBuffer, hOldPen);
		DeleteObject(hBrush);
		DeleteObject(hPen);
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

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
CWallMesh::CWallMesh(float fWidth, float fHeight, float fDepth, int nSubRects) : CMesh((4 * nSubRects * nSubRects) + 2)
{
	float fHalfWidth = fWidth * 0.5f;
	float fHalfHeight = fHeight * 0.5f;
	float fHalfDepth = fDepth * 0.5f;
	float fCellWidth = fWidth * (1.0f / nSubRects);
	float fCellHeight = fHeight * (1.0f / nSubRects);
	float fCellDepth = fDepth * (1.0f / nSubRects);

	int k = 0;
	CPolygon* pLeftFace;
	for (int i = 0; i < nSubRects; i++)
	{
		for (int j = 0; j < nSubRects; j++)
		{
			pLeftFace = new CPolygon(4);
			pLeftFace->SetVertex(0, CVertex(-fHalfWidth, -fHalfHeight + (i * fCellHeight), -fHalfDepth + (j * fCellDepth)));
			pLeftFace->SetVertex(1, CVertex(-fHalfWidth, -fHalfHeight + ((i + 1) * fCellHeight), -fHalfDepth + (j * fCellDepth)));
			pLeftFace->SetVertex(2, CVertex(-fHalfWidth, -fHalfHeight + ((i + 1) * fCellHeight), -fHalfDepth + ((j + 1) * fCellDepth)));
			pLeftFace->SetVertex(3, CVertex(-fHalfWidth, -fHalfHeight + (i * fCellHeight), -fHalfDepth + ((j + 1) * fCellDepth)));
			SetPolygon(k++, pLeftFace);
		}
	}

	CPolygon* pRightFace;
	for (int i = 0; i < nSubRects; i++)
	{
		for (int j = 0; j < nSubRects; j++)
		{
			pRightFace = new CPolygon(4);
			pRightFace->SetVertex(0, CVertex(+fHalfWidth, -fHalfHeight + (i * fCellHeight), -fHalfDepth + (j * fCellDepth)));
			pRightFace->SetVertex(1, CVertex(+fHalfWidth, -fHalfHeight + ((i + 1) * fCellHeight), -fHalfDepth + (j * fCellDepth)));
			pRightFace->SetVertex(2, CVertex(+fHalfWidth, -fHalfHeight + ((i + 1) * fCellHeight), -fHalfDepth + ((j + 1) * fCellDepth)));
			pRightFace->SetVertex(3, CVertex(+fHalfWidth, -fHalfHeight + (i * fCellHeight), -fHalfDepth + ((j + 1) * fCellDepth)));
			SetPolygon(k++, pRightFace);
		}
	}

	CPolygon* pTopFace;
	for (int i = 0; i < nSubRects; i++)
	{
		for (int j = 0; j < nSubRects; j++)
		{
			pTopFace = new CPolygon(4);
			pTopFace->SetVertex(0, CVertex(-fHalfWidth + (i * fCellWidth), +fHalfHeight, -fHalfDepth + (j * fCellDepth)));
			pTopFace->SetVertex(1, CVertex(-fHalfWidth + ((i + 1) * fCellWidth), +fHalfHeight, -fHalfDepth + (j * fCellDepth)));
			pTopFace->SetVertex(2, CVertex(-fHalfWidth + ((i + 1) * fCellWidth), +fHalfHeight, -fHalfDepth + ((j + 1) * fCellDepth)));
			pTopFace->SetVertex(3, CVertex(-fHalfWidth + (i * fCellWidth), +fHalfHeight, -fHalfDepth + ((j + 1) * fCellDepth)));
			SetPolygon(k++, pTopFace);
		}
	}

	CPolygon* pBottomFace;
	for (int i = 0; i < nSubRects; i++)
	{
		for (int j = 0; j < nSubRects; j++)
		{
			pBottomFace = new CPolygon(4);
			pBottomFace->SetVertex(0, CVertex(-fHalfWidth + (i * fCellWidth), -fHalfHeight, -fHalfDepth + (j * fCellDepth)));
			pBottomFace->SetVertex(1, CVertex(-fHalfWidth + ((i + 1) * fCellWidth), -fHalfHeight, -fHalfDepth + (j * fCellDepth)));
			pBottomFace->SetVertex(2, CVertex(-fHalfWidth + ((i + 1) * fCellWidth), -fHalfHeight, -fHalfDepth + ((j + 1) * fCellDepth)));
			pBottomFace->SetVertex(3, CVertex(-fHalfWidth + (i * fCellWidth), -fHalfHeight, -fHalfDepth + ((j + 1) * fCellDepth)));
			SetPolygon(k++, pBottomFace);
		}
	}

	CPolygon* pFrontFace = new CPolygon(4);
	pFrontFace->SetVertex(0, CVertex(-fHalfWidth, +fHalfHeight, -fHalfDepth));
	pFrontFace->SetVertex(1, CVertex(+fHalfWidth, +fHalfHeight, -fHalfDepth));
	pFrontFace->SetVertex(2, CVertex(+fHalfWidth, -fHalfHeight, -fHalfDepth));
	pFrontFace->SetVertex(3, CVertex(-fHalfWidth, -fHalfHeight, -fHalfDepth));
	SetPolygon(k++, pFrontFace);

	CPolygon* pBackFace = new CPolygon(4);
	pBackFace->SetVertex(0, CVertex(-fHalfWidth, -fHalfHeight, +fHalfDepth));
	pBackFace->SetVertex(1, CVertex(+fHalfWidth, -fHalfHeight, +fHalfDepth));
	pBackFace->SetVertex(2, CVertex(+fHalfWidth, +fHalfHeight, +fHalfDepth));
	pBackFace->SetVertex(3, CVertex(-fHalfWidth, +fHalfHeight, +fHalfDepth));
	SetPolygon(k++, pBackFace);

	m_xmOOBB = BoundingOrientedBox(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(fHalfWidth, fHalfHeight, fHalfDepth), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
CAirplaneMesh::CAirplaneMesh(float fWidth, float fHeight, float fDepth) : CMesh(24)
{
	float fx = fWidth * 0.5f, fy = fHeight * 0.5f, fz = fDepth * 0.5f;

	float x1 = fx * 0.2f, y1 = fy * 0.2f, x2 = fx * 0.1f, y3 = fy * 0.3f, y2 = ((y1 - (fy - y3)) / x1) * x2 + (fy - y3);
	int i = 0;

	//Upper Plane
	CPolygon* pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(0.0f, +(fy + y3), -fz));
	pFace->SetVertex(1, CVertex(+x1, -y1, -fz));
	pFace->SetVertex(2, CVertex(0.0f, 0.0f, -fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(0.0f, +(fy + y3), -fz));
	pFace->SetVertex(1, CVertex(0.0f, 0.0f, -fz));
	pFace->SetVertex(2, CVertex(-x1, -y1, -fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(+x2, +y2, -fz));
	pFace->SetVertex(1, CVertex(+fx, -y3, -fz));
	pFace->SetVertex(2, CVertex(+x1, -y1, -fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(-x2, +y2, -fz));
	pFace->SetVertex(1, CVertex(-x1, -y1, -fz));
	pFace->SetVertex(2, CVertex(-fx, -y3, -fz));
	SetPolygon(i++, pFace);

	//Lower Plane
	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(0.0f, +(fy + y3), +fz));
	pFace->SetVertex(1, CVertex(0.0f, 0.0f, +fz));
	pFace->SetVertex(2, CVertex(+x1, -y1, +fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(0.0f, +(fy + y3), +fz));
	pFace->SetVertex(1, CVertex(-x1, -y1, +fz));
	pFace->SetVertex(2, CVertex(0.0f, 0.0f, +fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(+x2, +y2, +fz));
	pFace->SetVertex(1, CVertex(+x1, -y1, +fz));
	pFace->SetVertex(2, CVertex(+fx, -y3, +fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(-x2, +y2, +fz));
	pFace->SetVertex(1, CVertex(-fx, -y3, +fz));
	pFace->SetVertex(2, CVertex(-x1, -y1, +fz));
	SetPolygon(i++, pFace);

	//Right Plane
	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(0.0f, +(fy + y3), -fz));
	pFace->SetVertex(1, CVertex(0.0f, +(fy + y3), +fz));
	pFace->SetVertex(2, CVertex(+x2, +y2, -fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(+x2, +y2, -fz));
	pFace->SetVertex(1, CVertex(0.0f, +(fy + y3), +fz));
	pFace->SetVertex(2, CVertex(+x2, +y2, +fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(+x2, +y2, -fz));
	pFace->SetVertex(1, CVertex(+x2, +y2, +fz));
	pFace->SetVertex(2, CVertex(+fx, -y3, -fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(+fx, -y3, -fz));
	pFace->SetVertex(1, CVertex(+x2, +y2, +fz));
	pFace->SetVertex(2, CVertex(+fx, -y3, +fz));
	SetPolygon(i++, pFace);

	//Back/Right Plane
	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(+x1, -y1, -fz));
	pFace->SetVertex(1, CVertex(+fx, -y3, -fz));
	pFace->SetVertex(2, CVertex(+fx, -y3, +fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(+x1, -y1, -fz));
	pFace->SetVertex(1, CVertex(+fx, -y3, +fz));
	pFace->SetVertex(2, CVertex(+x1, -y1, +fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(0.0f, 0.0f, -fz));
	pFace->SetVertex(1, CVertex(+x1, -y1, -fz));
	pFace->SetVertex(2, CVertex(+x1, -y1, +fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(0.0f, 0.0f, -fz));
	pFace->SetVertex(1, CVertex(+x1, -y1, +fz));
	pFace->SetVertex(2, CVertex(0.0f, 0.0f, +fz));
	SetPolygon(i++, pFace);

	//Left Plane
	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(0.0f, +(fy + y3), +fz));
	pFace->SetVertex(1, CVertex(0.0f, +(fy + y3), -fz));
	pFace->SetVertex(2, CVertex(-x2, +y2, -fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(0.0f, +(fy + y3), +fz));
	pFace->SetVertex(1, CVertex(-x2, +y2, -fz));
	pFace->SetVertex(2, CVertex(-x2, +y2, +fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(-x2, +y2, +fz));
	pFace->SetVertex(1, CVertex(-x2, +y2, -fz));
	pFace->SetVertex(2, CVertex(-fx, -y3, -fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(-x2, +y2, +fz));
	pFace->SetVertex(1, CVertex(-fx, -y3, -fz));
	pFace->SetVertex(2, CVertex(-fx, -y3, +fz));
	SetPolygon(i++, pFace);

	//Back/Left Plane
	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(0.0f, 0.0f, -fz));
	pFace->SetVertex(1, CVertex(0.0f, 0.0f, +fz));
	pFace->SetVertex(2, CVertex(-x1, -y1, +fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(0.0f, 0.0f, -fz));
	pFace->SetVertex(1, CVertex(-x1, -y1, +fz));
	pFace->SetVertex(2, CVertex(-x1, -y1, -fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(-x1, -y1, -fz));
	pFace->SetVertex(1, CVertex(-x1, -y1, +fz));
	pFace->SetVertex(2, CVertex(-fx, -y3, +fz));
	SetPolygon(i++, pFace);

	pFace = new CPolygon(3);
	pFace->SetVertex(0, CVertex(-x1, -y1, -fz));
	pFace->SetVertex(1, CVertex(-fx, -y3, +fz));
	pFace->SetVertex(2, CVertex(-fx, -y3, -fz));
	SetPolygon(i++, pFace);

	m_xmOOBB = BoundingOrientedBox(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(fx, fy, fz), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
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
	// 원점(후미)에서 -Z 방향으로 fLength 만큼 뻗음
	float zNear = -fLength;   // 포신 끝(앞)
	float zFar = 0.0f;      // 포신 뿌리(후미, 포탑 연결)

	// Front (포신 끝 캡)
	CPolygon* pFront = new CPolygon(4);
	pFront->SetVertex(0, CVertex(-hr, +hr, zNear));
	pFront->SetVertex(1, CVertex(+hr, +hr, zNear));
	pFront->SetVertex(2, CVertex(+hr, -hr, zNear));
	pFront->SetVertex(3, CVertex(-hr, -hr, zNear));
	SetPolygon(0, pFront);

	// Back (포신 뿌리 캡)
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
	pBottom->SetVertex(0, CVertex(-hr, -hr, zFar));
	pBottom->SetVertex(1, CVertex(+hr, -hr, zFar));
	pBottom->SetVertex(2, CVertex(+hr, -hr, zNear));
	pBottom->SetVertex(3, CVertex(-hr, -hr, zNear));
	SetPolygon(3, pBottom);

	// Left
	CPolygon* pLeft = new CPolygon(4);
	pLeft->SetVertex(0, CVertex(-hr, +hr, zFar));
	pLeft->SetVertex(1, CVertex(-hr, +hr, zNear));
	pLeft->SetVertex(2, CVertex(-hr, -hr, zNear));
	pLeft->SetVertex(3, CVertex(-hr, -hr, zFar));
	SetPolygon(4, pLeft);

	// Right
	CPolygon* pRight = new CPolygon(4);
	pRight->SetVertex(0, CVertex(+hr, +hr, zNear));
	pRight->SetVertex(1, CVertex(+hr, +hr, zFar));
	pRight->SetVertex(2, CVertex(+hr, -hr, zFar));
	pRight->SetVertex(3, CVertex(+hr, -hr, zNear));
	SetPolygon(5, pRight);

	m_xmOOBB = BoundingOrientedBox(
		XMFLOAT3(0.f, 0.f, zNear * 0.5f),
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