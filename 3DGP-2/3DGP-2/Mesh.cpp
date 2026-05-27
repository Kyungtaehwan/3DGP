#include "pch.h"
#include "Mesh.h"

// ============================================================
// CMesh
// ============================================================

CMesh::~CMesh()
{
    if (m_pd3dVertexBuffer)      { m_pd3dVertexBuffer->Release();      m_pd3dVertexBuffer      = NULL; }
    if (m_pd3dVertexUploadBuffer){ m_pd3dVertexUploadBuffer->Release(); m_pd3dVertexUploadBuffer = NULL; }
    if (m_pd3dIndexBuffer)       { m_pd3dIndexBuffer->Release();        m_pd3dIndexBuffer       = NULL; }
    if (m_pd3dIndexUploadBuffer) { m_pd3dIndexUploadBuffer->Release();  m_pd3dIndexUploadBuffer  = NULL; }
}

void CMesh::ReleaseUploadBuffers()
{
    if (m_pd3dVertexUploadBuffer)
    {
        m_pd3dVertexUploadBuffer->Release();
        m_pd3dVertexUploadBuffer = NULL;
    }
    if (m_pd3dIndexUploadBuffer)
    {
        m_pd3dIndexUploadBuffer->Release();
        m_pd3dIndexUploadBuffer = NULL;
    }
}

void CMesh::Render(ID3D12GraphicsCommandList* pd3dCommandList)
{
    pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
    pd3dCommandList->IASetVertexBuffers(0, 1, &m_d3dVertexBufferView);

    if (m_nIndices > 0)
    {
        pd3dCommandList->IASetIndexBuffer(&m_d3dIndexBufferView);
        pd3dCommandList->DrawIndexedInstanced(m_nIndices, 1, 0, 0, 0);
    }
    else
    {
        pd3dCommandList->DrawInstanced(m_nVertices, 1, 0, 0);
    }
}

// ============================================================
// CCubeMesh
// ============================================================

CCubeMesh::CCubeMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
                     float fWidth, float fHeight, float fDepth, XMFLOAT4 xmf4Color)
{
    m_nVertices         = 8;
    m_nIndices          = 36;
    m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    float fHalfW = fWidth  * 0.5f;
    float fHalfH = fHeight * 0.5f;
    float fHalfD = fDepth  * 0.5f;

    
    CVertex pVertices[8] =
    {
        // front face
        CVertex(-fHalfW, -fHalfH, -fHalfD,  xmf4Color.x, xmf4Color.y, xmf4Color.z, xmf4Color.w), // 0
        CVertex( fHalfW, -fHalfH, -fHalfD,  xmf4Color.x, xmf4Color.y, xmf4Color.z, xmf4Color.w), // 1
        CVertex( fHalfW,  fHalfH, -fHalfD,  xmf4Color.x, xmf4Color.y, xmf4Color.z, xmf4Color.w), // 2
        CVertex(-fHalfW,  fHalfH, -fHalfD,  xmf4Color.x, xmf4Color.y, xmf4Color.z, xmf4Color.w), // 3
        // back face
        CVertex( fHalfW, -fHalfH,  fHalfD,  xmf4Color.x, xmf4Color.y, xmf4Color.z, xmf4Color.w), // 4
        CVertex(-fHalfW, -fHalfH,  fHalfD,  xmf4Color.x, xmf4Color.y, xmf4Color.z, xmf4Color.w), // 5
        CVertex(-fHalfW,  fHalfH,  fHalfD,  xmf4Color.x, xmf4Color.y, xmf4Color.z, xmf4Color.w), // 6
        CVertex( fHalfW,  fHalfH,  fHalfD,  xmf4Color.x, xmf4Color.y, xmf4Color.z, xmf4Color.w), // 7
    };

   
    UINT pIndices[36] =
    {
        // front  (z = -fHalfD, normal -Z)
        0, 1, 2,   0, 2, 3,
        // back   (z = +fHalfD, normal +Z)
        4, 5, 6,   4, 6, 7,
        // left   (x = -fHalfW, normal -X)
        5, 0, 3,   5, 3, 6,
        // right  (x = +fHalfW, normal +X)
        1, 4, 7,   1, 7, 2,
        // top    (y = +fHalfH, normal +Y)
        3, 2, 7,   3, 7, 6,
        // bottom (y = -fHalfH, normal -Y)
        5, 4, 1,   5, 1, 0,
    };

    m_pd3dVertexBuffer = ::CreateBufferResource(
        pd3dDevice, pd3dCommandList,
        pVertices, sizeof(CVertex) * m_nVertices,
        D3D12_HEAP_TYPE_DEFAULT,
        D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
        &m_pd3dVertexUploadBuffer);

    m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
    m_d3dVertexBufferView.StrideInBytes  = sizeof(CVertex);
    m_d3dVertexBufferView.SizeInBytes    = sizeof(CVertex) * m_nVertices;

    // Create index
    m_pd3dIndexBuffer = ::CreateBufferResource(
        pd3dDevice, pd3dCommandList,
        pIndices, sizeof(UINT) * m_nIndices,
        D3D12_HEAP_TYPE_DEFAULT,
        D3D12_RESOURCE_STATE_INDEX_BUFFER,
        &m_pd3dIndexUploadBuffer);

    m_d3dIndexBufferView.BufferLocation = m_pd3dIndexBuffer->GetGPUVirtualAddress();
    m_d3dIndexBufferView.Format         = DXGI_FORMAT_R32_UINT;
    m_d3dIndexBufferView.SizeInBytes    = sizeof(UINT) * m_nIndices;
}

CCubeMesh::~CCubeMesh()
{

}

void CCubeMesh::ReleaseUploadBuffers()
{
    CMesh::ReleaseUploadBuffers();
}

CFloorQuadMesh::CFloorQuadMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
                               float fWidth, float fDepth, float fYTop, XMFLOAT4 c)
{
    m_nVertices            = 4;
    m_nIndices             = 12;
    m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    float hW = fWidth * 0.5f;
    float hD = fDepth * 0.5f;

    CVertex pVertices[4] =
    {
        CVertex(-hW, fYTop, -hD, c.x, c.y, c.z, c.w), // 0
        CVertex( hW, fYTop, -hD, c.x, c.y, c.z, c.w), // 1
        CVertex( hW, fYTop,  hD, c.x, c.y, c.z, c.w), // 2
        CVertex(-hW, fYTop,  hD, c.x, c.y, c.z, c.w), // 3
    };


    UINT pIndices[12] =
    {
        0, 1, 2,   0, 2, 3,    // top
        0, 2, 1,   0, 3, 2,    // bottom
    };

    m_pd3dVertexBuffer = ::CreateBufferResource(
        pd3dDevice, pd3dCommandList,
        pVertices, sizeof(CVertex) * m_nVertices,
        D3D12_HEAP_TYPE_DEFAULT,
        D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
        &m_pd3dVertexUploadBuffer);

    m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
    m_d3dVertexBufferView.StrideInBytes  = sizeof(CVertex);
    m_d3dVertexBufferView.SizeInBytes    = sizeof(CVertex) * m_nVertices;

    m_pd3dIndexBuffer = ::CreateBufferResource(
        pd3dDevice, pd3dCommandList,
        pIndices, sizeof(UINT) * m_nIndices,
        D3D12_HEAP_TYPE_DEFAULT,
        D3D12_RESOURCE_STATE_INDEX_BUFFER,
        &m_pd3dIndexUploadBuffer);

    m_d3dIndexBufferView.BufferLocation = m_pd3dIndexBuffer->GetGPUVirtualAddress();
    m_d3dIndexBufferView.Format         = DXGI_FORMAT_R32_UINT;
    m_d3dIndexBufferView.SizeInBytes    = sizeof(UINT) * m_nIndices;
}

CFloorQuadMesh::~CFloorQuadMesh() {}

void CFloorQuadMesh::ReleaseUploadBuffers()
{
    CMesh::ReleaseUploadBuffers();
}

CTriangleMesh::CTriangleMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
                             float fWidth, float fHeight, XMFLOAT4 c)
{
    m_nVertices            = 6;   
    m_nIndices             = 0;
    m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    float hw = fWidth  * 0.5f;
    float hh = fHeight * 0.5f;

    CVertex pVertices[6] =
    {

        CVertex(-hw, -hh, 0.0f, c.x, c.y, c.z, c.w),
        CVertex( hw,  0.0f, 0.0f, c.x, c.y, c.z, c.w),
        CVertex(-hw,  hh, 0.0f, c.x, c.y, c.z, c.w),

        CVertex(-hw, -hh, 0.0f, c.x, c.y, c.z, c.w),
        CVertex(-hw,  hh, 0.0f, c.x, c.y, c.z, c.w),
        CVertex( hw,  0.0f, 0.0f, c.x, c.y, c.z, c.w),
    };

    m_pd3dVertexBuffer = ::CreateBufferResource(
        pd3dDevice, pd3dCommandList,
        pVertices, sizeof(CVertex) * m_nVertices,
        D3D12_HEAP_TYPE_DEFAULT,
        D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
        &m_pd3dVertexUploadBuffer);

    m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
    m_d3dVertexBufferView.StrideInBytes  = sizeof(CVertex);
    m_d3dVertexBufferView.SizeInBytes    = sizeof(CVertex) * m_nVertices;
}

CTriangleMesh::~CTriangleMesh() {}

void CTriangleMesh::ReleaseUploadBuffers()
{
    CMesh::ReleaseUploadBuffers();
}
