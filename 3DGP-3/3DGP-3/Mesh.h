#pragma once

#define VERTEXT_POSITION 0x01
#define VERTEXT_COLOR    0x02
#define VERTEXT_NORMAL   0x04

class CMeshLoadInfo
{
public:
    CMeshLoadInfo() {}
    ~CMeshLoadInfo();

    char     m_pstrMeshName[256] = { 0 };

    UINT     m_nType = 0x00;

    XMFLOAT3 m_xmf3AABBCenter  = XMFLOAT3(0.0f, 0.0f, 0.0f);
    XMFLOAT3 m_xmf3AABBExtents = XMFLOAT3(0.0f, 0.0f, 0.0f);

    int       m_nVertices       = 0;
    XMFLOAT3* m_pxmf3Positions  = NULL;
    XMFLOAT4* m_pxmf4Colors     = NULL;
    XMFLOAT3* m_pxmf3Normals    = NULL;

    int       m_nIndices        = 0;
    UINT*     m_pnIndices       = NULL;

    int       m_nSubMeshes      = 0;
    int*      m_pnSubSetIndices = NULL;
    UINT**    m_ppnSubSetIndices = NULL;
};

class CMesh
{
public:
    CMesh() {}
    virtual ~CMesh();

    void AddRef()  { m_nReferences++; }
    void Release() { if (--m_nReferences <= 0) delete this; }

    virtual void ReleaseUploadBuffers();
    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList) {}
    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet) {}

    UINT GetType() const { return m_nType; }

protected:
    int  m_nReferences = 0;

    D3D12_PRIMITIVE_TOPOLOGY m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    UINT m_nSlot     = 0;
    UINT m_nVertices = 0;
    UINT m_nOffset   = 0;

    UINT m_nType = 0;
};

// Position-only mesh loaded from .bin file. Supports indexed sub-meshes.
class CMeshFromFile : public CMesh
{
public:
    CMeshFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
                  CMeshLoadInfo* pMeshInfo);
    virtual ~CMeshFromFile();

    virtual void ReleaseUploadBuffers() override;
    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet) override;

    int GetSubMeshCount() const { return m_nSubMeshes; }

protected:
    ID3D12Resource*          m_pd3dPositionBuffer        = NULL;
    ID3D12Resource*          m_pd3dPositionUploadBuffer  = NULL;
    D3D12_VERTEX_BUFFER_VIEW m_d3dPositionBufferView     = {};

    int   m_nSubMeshes      = 0;
    int*  m_pnSubSetIndices = NULL;

    ID3D12Resource**         m_ppd3dSubSetIndexBuffers       = NULL;
    ID3D12Resource**         m_ppd3dSubSetIndexUploadBuffers = NULL;
    D3D12_INDEX_BUFFER_VIEW* m_pd3dSubSetIndexBufferViews    = NULL;
};

// Position + Normal mesh: bound as two vertex buffers (slot 0 position, slot 1 normal).
class CMeshIlluminatedFromFile : public CMeshFromFile
{
public:
    CMeshIlluminatedFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
                             CMeshLoadInfo* pMeshInfo);
    virtual ~CMeshIlluminatedFromFile();

    virtual void ReleaseUploadBuffers() override;
    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet) override;

protected:
    ID3D12Resource*          m_pd3dNormalBuffer        = NULL;
    ID3D12Resource*          m_pd3dNormalUploadBuffer  = NULL;
    D3D12_VERTEX_BUFFER_VIEW m_d3dNormalBufferView     = {};
};

// Interleaved position + color vertex (unlit), used by the bitmap cube text.
struct CColorVertex
{
    XMFLOAT3 m_xmf3Position;
    XMFLOAT4 m_xmf4Color;
};

// A solid-color cube (36 vertices, one interleaved VB). Rendered with
// CColorShader through the engine's per-object CB (world matrix at b2).
class CColorCubeMesh : public CMesh
{
public:
    CColorCubeMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
                   float fWidth, float fHeight, float fDepth, XMFLOAT4 xmf4Color);
    virtual ~CColorCubeMesh();

    virtual void ReleaseUploadBuffers() override;
    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet) override;

protected:
    ID3D12Resource*          m_pd3dVertexBuffer       = NULL;
    ID3D12Resource*          m_pd3dVertexUploadBuffer = NULL;
    D3D12_VERTEX_BUFFER_VIEW m_d3dVertexBufferView    = {};
};
