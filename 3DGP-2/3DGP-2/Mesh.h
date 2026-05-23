#pragma once

struct CVertex
{
    XMFLOAT3 position;
    XMFLOAT4 color;

    CVertex() : position(0,0,0), color(1,1,1,1) {}
    CVertex(float x, float y, float z, float r, float g, float b, float a = 1.0f)
        : position(x, y, z), color(r, g, b, a) {}
    CVertex(XMFLOAT3 pos, XMFLOAT4 col) : position(pos), color(col) {}
};

class CMesh
{
public:
    CMesh() {}
    virtual ~CMesh();

    virtual void ReleaseUploadBuffers();
    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList);

protected:
    UINT m_nVertices = 0;
    ID3D12Resource* m_pd3dVertexBuffer       = NULL;
    ID3D12Resource* m_pd3dVertexUploadBuffer = NULL;
    D3D12_VERTEX_BUFFER_VIEW m_d3dVertexBufferView = {};

    UINT m_nIndices = 0;
    ID3D12Resource* m_pd3dIndexBuffer        = NULL;
    ID3D12Resource* m_pd3dIndexUploadBuffer  = NULL;
    D3D12_INDEX_BUFFER_VIEW  m_d3dIndexBufferView = {};

    D3D12_PRIMITIVE_TOPOLOGY m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
};

class CCubeMesh : public CMesh
{
public:
    CCubeMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
              float fWidth = 2.0f, float fHeight = 2.0f, float fDepth = 2.0f,
              XMFLOAT4 xmf4Color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
    virtual ~CCubeMesh();
    virtual void ReleaseUploadBuffers() override;
};

// Single horizontal quad at y=fYTop, spanning fWidth (X) x fDepth (Z), centered on origin.
// Has no side faces, so adjacent tiles can't produce coplanar-side artifacts.
class CFloorQuadMesh : public CMesh
{
public:
    CFloorQuadMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
                   float fWidth, float fDepth, float fYTop, XMFLOAT4 xmf4Color);
    virtual ~CFloorQuadMesh();
    virtual void ReleaseUploadBuffers() override;
};

// Flat right-pointing triangle (play-icon) on the XY plane (z = 0), double-sided.
// Apex on +X, base on -X. Centered on origin.
class CTriangleMesh : public CMesh
{
public:
    CTriangleMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
                  float fWidth, float fHeight, XMFLOAT4 xmf4Color);
    virtual ~CTriangleMesh();
    virtual void ReleaseUploadBuffers() override;
};


class CGlockMesh : public CMesh
{
public:
    CGlockMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
    virtual ~CGlockMesh();
    virtual void ReleaseUploadBuffers() override;
};
