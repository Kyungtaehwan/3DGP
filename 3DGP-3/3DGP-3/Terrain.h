#pragma once
#include "GameObject.h"   // CGameObject (and Mesh.h -> CMesh)

// ============================================================
// Height map terrain (LabProject14)
// ============================================================

// Loads a header-less 16-bit grayscale RAW image (little-endian unsigned short)
// and answers height/normal queries in height-map (image) coordinates.
//
// Heights are normalized into a 0..255 range (16-bit value / 257) so the terrain
// scale values keep the same meaning as an 8-bit map, but with 256x finer height
// resolution — this removes the "stair-step" terracing you get from 8-bit data.
class CHeightMapImage
{
public:
    CHeightMapImage(const char* pFileName, int nWidth, int nLength, XMFLOAT3 xmf3Scale);
    ~CHeightMapImage();

    // Height at the (possibly fractional) image coordinate (fx, fz), bilinearly
    // interpolated across the quad the point falls in.
    float    GetHeight(float fx, float fz);
    // Surface normal at integer image coordinate (x, z).
    XMFLOAT3 GetHeightMapNormal(int x, int z);
    // Stored (un-scaled) height at an integer pixel.
    float    GetPixelHeight(int x, int z) const { return m_pHeights[x + (z * m_nWidth)]; }

    XMFLOAT3 GetScale()           const { return m_xmf3Scale; }
    int      GetHeightMapWidth()  const { return m_nWidth; }
    int      GetHeightMapLength() const { return m_nLength; }

private:
    float*   m_pHeights = NULL;   // per-pixel height (0..255 range), flipped so image +z == terrain +z
    int      m_nWidth  = 0;
    int      m_nLength = 0;
    XMFLOAT3 m_xmf3Scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
};

// One grid block of the terrain. Produces a POSITION (slot 0) + NORMAL (slot 1)
// triangle-strip mesh so it renders through the engine's lighting pipeline
// (CIlluminatedShader). Vertex world positions already include the terrain scale.
class CHeightMapGridMesh : public CMesh
{
public:
    CHeightMapGridMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
                       int xStart, int zStart, int nWidth, int nLength,
                       XMFLOAT3 xmf3Scale, CHeightMapImage* pHeightMapImage);
    virtual ~CHeightMapGridMesh();

    virtual void ReleaseUploadBuffers() override;
    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet) override;

    XMFLOAT3 GetScale()  const { return m_xmf3Scale; }
    int      GetWidth()  const { return m_nWidth; }
    int      GetLength() const { return m_nLength; }

protected:
    int      m_nWidth  = 0;
    int      m_nLength = 0;
    XMFLOAT3 m_xmf3Scale = XMFLOAT3(1.0f, 1.0f, 1.0f);

    ID3D12Resource*          m_pd3dPositionBuffer       = NULL;
    ID3D12Resource*          m_pd3dPositionUploadBuffer = NULL;
    D3D12_VERTEX_BUFFER_VIEW m_d3dPositionBufferView    = {};

    ID3D12Resource*          m_pd3dNormalBuffer       = NULL;
    ID3D12Resource*          m_pd3dNormalUploadBuffer = NULL;
    D3D12_VERTEX_BUFFER_VIEW m_d3dNormalBufferView    = {};

    ID3D12Resource*          m_pd3dIndexBuffer       = NULL;
    ID3D12Resource*          m_pd3dIndexUploadBuffer = NULL;
    D3D12_INDEX_BUFFER_VIEW  m_d3dIndexBufferView    = {};
    UINT                     m_nIndices              = 0;
};

// Outdoor terrain (LabProject14). Splits a height map into a grid of block
// meshes and renders them through the engine's lighting pipeline using a single
// shared material. World transform is identity; vertex positions already include
// the terrain scale, so GetHeight()/GetNormal() take WORLD (x, z) coordinates.
class CHeightMapTerrain : public CGameObject
{
public:
    CHeightMapTerrain(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
                      const char* pFileName, int nWidth, int nLength,
                      int nBlockWidth, int nBlockLength,
                      XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color, CShader* pShader);
    virtual ~CHeightMapTerrain();

    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera) override;
    virtual void ReleaseUploadBuffers() override;

    float    GetHeight(float x, float z);     // world-space height at (x, z)
    XMFLOAT3 GetNormal(float x, float z);     // world-space surface normal at (x, z)

    int      GetHeightMapWidth();
    int      GetHeightMapLength();
    XMFLOAT3 GetScale()  { return m_xmf3Scale; }
    float    GetWidth()  { return m_nWidth  * m_xmf3Scale.x; }
    float    GetLength() { return m_nLength * m_xmf3Scale.z; }

private:
    CHeightMapImage* m_pHeightMapImage = NULL;
    int              m_nWidth  = 0;
    int              m_nLength = 0;
    XMFLOAT3         m_xmf3Scale = XMFLOAT3(1.0f, 1.0f, 1.0f);

    CMesh**          m_ppBlockMeshes = NULL;   // grid of block meshes
    int              m_nBlocks       = 0;
};
