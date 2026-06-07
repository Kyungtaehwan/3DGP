#include "pch.h"
#include "Terrain.h"
#include "Shader.h"


CHeightMapImage::CHeightMapImage(const char* pFileName, int nWidth, int nLength, XMFLOAT3 xmf3Scale)
{
    m_nWidth = nWidth;
    m_nLength = nLength;
    m_xmf3Scale = xmf3Scale;

    const int nPixels = m_nWidth * m_nLength;
    unsigned short* pRawPixels = new unsigned short[nPixels];

    FILE* pInFile = NULL;
    ::fopen_s(&pInFile, pFileName, "rb");
    if(pInFile)
    {
        ::fread(pRawPixels, sizeof(unsigned short), nPixels, pInFile);
        ::fclose(pInFile);
    }
    else
    {
        ::ZeroMemory(pRawPixels, nPixels * sizeof(unsigned short));
        OutputDebugStringA("WARNING: height map file not found; using flat terrain.\n");
    }

    m_pHeights = new float[nPixels];
    for(int y = 0; y < m_nLength; y++)
        for(int x = 0; x < m_nWidth; x++)
            m_pHeights[x + ((m_nLength - 1 - y) * m_nWidth)] = pRawPixels[x + (y * m_nWidth)] / 257.0f;

    delete[] pRawPixels;
}

CHeightMapImage::~CHeightMapImage()
{
    if(m_pHeights)
        delete[] m_pHeights;
    m_pHeights = NULL;
}

XMFLOAT3 CHeightMapImage::GetHeightMapNormal(int x, int z)
{

    if((x < 0) || (z < 0) || (x >= m_nWidth) || (z >= m_nLength))
        return XMFLOAT3(0.0f, 1.0f, 0.0f);

    int nIndex = x + (z * m_nWidth);
    int xAdd;
    if(x < (m_nWidth - 1))
        xAdd = 1;
    else
        xAdd = -1;
    int zAdd;
    if(z < (m_nLength - 1))
        zAdd = m_nWidth;
    else
        zAdd = -m_nWidth;

    float y1 = m_pHeights[nIndex] * m_xmf3Scale.y;
    float y2 = m_pHeights[nIndex + xAdd] * m_xmf3Scale.y;
    float y3 = m_pHeights[nIndex + zAdd] * m_xmf3Scale.y;

    XMFLOAT3 xmf3Edge1 = XMFLOAT3(0.0f, y3 - y1, m_xmf3Scale.z);
    XMFLOAT3 xmf3Edge2 = XMFLOAT3(m_xmf3Scale.x, y2 - y1, 0.0f);
    return Vector3::Normalize(Vector3::CrossProduct(xmf3Edge1, xmf3Edge2));
}

#define _WITH_APPROXIMATE_OPPOSITE_CORNER
float CHeightMapImage::GetHeight(float fx, float fz)
{
    if((fx < 0.0f) || (fz < 0.0f) || (fx >= m_nWidth) || (fz >= m_nLength))
        return 0.0f;

    int x = (int)fx;
    int z = (int)fz;
    float fxPercent = fx - x;
    float fzPercent = fz - z;

    float fBottomLeft = m_pHeights[x + (z * m_nWidth)];
    float fBottomRight = m_pHeights[(x + 1) + (z * m_nWidth)];
    float fTopLeft = m_pHeights[x + ((z + 1) * m_nWidth)];
    float fTopRight = m_pHeights[(x + 1) + ((z + 1) * m_nWidth)];

#ifdef _WITH_APPROXIMATE_OPPOSITE_CORNER

    bool bRightToLeft = ((z % 2) != 0);
    if(bRightToLeft)
    {
        if(fzPercent >= fxPercent)
            fBottomRight = fBottomLeft + (fTopRight - fTopLeft);
        else
            fTopLeft = fTopRight + (fBottomLeft - fBottomRight);
    }
    else
    {
        if(fzPercent < (1.0f - fxPercent))
            fTopRight = fTopLeft + (fBottomRight - fBottomLeft);
        else
            fBottomLeft = fTopLeft + (fBottomRight - fTopRight);
    }
#endif

    float fTopHeight = fTopLeft * (1.0f - fxPercent) + fTopRight * fxPercent;
    float fBottomHeight = fBottomLeft * (1.0f - fxPercent) + fBottomRight * fxPercent;
    return fBottomHeight * (1.0f - fzPercent) + fTopHeight * fzPercent;
}


CHeightMapGridMesh::CHeightMapGridMesh(ID3D12Device* pd3dDevice,
                                       ID3D12GraphicsCommandList* pd3dCommandList,
                                       int xStart, int zStart, int nWidth, int nLength,
                                       XMFLOAT3 xmf3Scale, CHeightMapImage* pHeightMapImage)
{
    m_nWidth = nWidth;
    m_nLength = nLength;
    m_xmf3Scale = xmf3Scale;

    m_nVertices = nWidth * nLength;
    m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

    XMFLOAT3* pxmf3Positions = new XMFLOAT3[m_nVertices];
    XMFLOAT3* pxmf3Normals = new XMFLOAT3[m_nVertices];

    for(int i = 0, z = zStart; z < (zStart + nLength); z++)
    {
        for(int x = xStart; x < (xStart + nWidth); x++, i++)
        {
            float fHeight = pHeightMapImage->GetPixelHeight(x, z) * m_xmf3Scale.y;
            pxmf3Positions[i] = XMFLOAT3(x * m_xmf3Scale.x, fHeight, z * m_xmf3Scale.z);
            pxmf3Normals[i] = pHeightMapImage->GetHeightMapNormal(x, z);
        }
    }

    m_pd3dPositionBuffer = ::CreateBufferResource(
        pd3dDevice, pd3dCommandList, pxmf3Positions, sizeof(XMFLOAT3) * m_nVertices,
        D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
        &m_pd3dPositionUploadBuffer);
    m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
    m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
    m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

    m_pd3dNormalBuffer = ::CreateBufferResource(
        pd3dDevice, pd3dCommandList, pxmf3Normals, sizeof(XMFLOAT3) * m_nVertices,
        D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
        &m_pd3dNormalUploadBuffer);
    m_d3dNormalBufferView.BufferLocation = m_pd3dNormalBuffer->GetGPUVirtualAddress();
    m_d3dNormalBufferView.StrideInBytes = sizeof(XMFLOAT3);
    m_d3dNormalBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

    delete[] pxmf3Positions;
    delete[] pxmf3Normals;

    m_nIndices = ((nWidth * 2) * (nLength - 1)) + ((nLength - 1) - 1);
    UINT* pnIndices = new UINT[m_nIndices];

    for(int j = 0, z = 0; z < nLength - 1; z++)
    {
        if((z % 2) == 0) // left -> right
        {
            for(int x = 0; x < nWidth; x++)
            {
                if((x == 0) && (z > 0))
                    pnIndices[j++] = (UINT)(x + (z * nWidth));
                pnIndices[j++] = (UINT)(x + (z * nWidth));
                pnIndices[j++] = (UINT)((x + (z * nWidth)) + nWidth);
            }
        }
        else // right -> left
        {
            for(int x = nWidth - 1; x >= 0; x--)
            {
                if(x == (nWidth - 1))
                    pnIndices[j++] = (UINT)(x + (z * nWidth));
                pnIndices[j++] = (UINT)(x + (z * nWidth));
                pnIndices[j++] = (UINT)((x + (z * nWidth)) + nWidth);
            }
        }
    }

    m_pd3dIndexBuffer = ::CreateBufferResource(
        pd3dDevice, pd3dCommandList, pnIndices, sizeof(UINT) * m_nIndices,
        D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER,
        &m_pd3dIndexUploadBuffer);
    m_d3dIndexBufferView.BufferLocation = m_pd3dIndexBuffer->GetGPUVirtualAddress();
    m_d3dIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
    m_d3dIndexBufferView.SizeInBytes = sizeof(UINT) * m_nIndices;

    delete[] pnIndices;
}

CHeightMapGridMesh::~CHeightMapGridMesh()
{
    if(m_pd3dPositionBuffer)
        m_pd3dPositionBuffer->Release();
    if(m_pd3dNormalBuffer)
        m_pd3dNormalBuffer->Release();
    if(m_pd3dIndexBuffer)
        m_pd3dIndexBuffer->Release();
}

void CHeightMapGridMesh::ReleaseUploadBuffers()
{
    if(m_pd3dPositionUploadBuffer)
    {
        m_pd3dPositionUploadBuffer->Release();
        m_pd3dPositionUploadBuffer = NULL;
    }
    if(m_pd3dNormalUploadBuffer)
    {
        m_pd3dNormalUploadBuffer->Release();
        m_pd3dNormalUploadBuffer = NULL;
    }
    if(m_pd3dIndexUploadBuffer)
    {
        m_pd3dIndexUploadBuffer->Release();
        m_pd3dIndexUploadBuffer = NULL;
    }
}

void CHeightMapGridMesh::Render(ID3D12GraphicsCommandList* pd3dCommandList, int /*nSubSet*/)
{
    pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);

    D3D12_VERTEX_BUFFER_VIEW views[2] = { m_d3dPositionBufferView, m_d3dNormalBufferView };
    pd3dCommandList->IASetVertexBuffers(m_nSlot, 2, views);

    pd3dCommandList->IASetIndexBuffer(&m_d3dIndexBufferView);
    pd3dCommandList->DrawIndexedInstanced(m_nIndices, 1, 0, 0, 0);
}


CHeightMapTerrain::CHeightMapTerrain(ID3D12Device* pd3dDevice,
                                     ID3D12GraphicsCommandList* pd3dCommandList,
                                     const char* pFileName, int nWidth, int nLength,
                                     int nBlockWidth, int nBlockLength,
                                     XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color, CShader* pShader)
    : CGameObject()
{
    m_nWidth = nWidth;
    m_nLength = nLength;
    m_xmf3Scale = xmf3Scale;

    m_pHeightMapImage = new CHeightMapImage(pFileName, nWidth, nLength, xmf3Scale);

    int cxQuadsPerBlock = nBlockWidth - 1;
    int czQuadsPerBlock = nBlockLength - 1;

    long cxBlocks = (nWidth - 1 + cxQuadsPerBlock - 1) / cxQuadsPerBlock;
    long czBlocks = (nLength - 1 + czQuadsPerBlock - 1) / czQuadsPerBlock;

    m_nBlocks = cxBlocks * czBlocks;
    m_ppBlockMeshes = new CMesh*[m_nBlocks];
    for(int i = 0; i < m_nBlocks; i++) m_ppBlockMeshes[i] = NULL;

    for(int z = 0; z < czBlocks; z++)
    {
        for(int x = 0; x < cxBlocks; x++)
        {
            int xStart = x * cxQuadsPerBlock;
            int zStart = z * czQuadsPerBlock;

            int blockW;
            if(nBlockWidth < (nWidth - xStart))
                blockW = nBlockWidth;
            else
                blockW = (nWidth - xStart);
            int blockL;
            if(nBlockLength < (nLength - zStart))
                blockL = nBlockLength;
            else
                blockL = (nLength - zStart);

            if(blockW < 2 || blockL < 2)
                continue;

            CHeightMapGridMesh* pGridMesh = new CHeightMapGridMesh(
                pd3dDevice, pd3dCommandList, xStart, zStart, blockW, blockL,
                xmf3Scale, m_pHeightMapImage);
            pGridMesh->AddRef();
            m_ppBlockMeshes[x + (z * cxBlocks)] = pGridMesh;
        }
    }

    SetShader(pShader);
    if(m_ppMaterials && m_ppMaterials[0] && m_ppMaterials[0]->m_pMaterialColors)
    {
        CMaterialColors* pColors = m_ppMaterials[0]->m_pMaterialColors;
        pColors->m_xmf4Diffuse = XMFLOAT4(xmf4Color.x, xmf4Color.y, xmf4Color.z, 1.0f);
        pColors->m_xmf4Ambient = XMFLOAT4(xmf4Color.x * 0.4f, xmf4Color.y * 0.4f, xmf4Color.z * 0.4f, 1.0f);
        pColors->m_xmf4Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
    }

    CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

CHeightMapTerrain::~CHeightMapTerrain()
{
    if(m_ppBlockMeshes)
    {
        for(int i = 0; i < m_nBlocks; i++)
            if(m_ppBlockMeshes[i])
                m_ppBlockMeshes[i]->Release();
        delete[] m_ppBlockMeshes;
    }
    if(m_pHeightMapImage)
        delete m_pHeightMapImage;
}

void CHeightMapTerrain::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
    // CB
    UpdateShaderVariables(pd3dCommandList);

    if(m_nMaterials > 0 && m_ppMaterials[0] && m_ppMaterials[0]->m_pShader)
        m_ppMaterials[0]->m_pShader->OnPrepareRender(pd3dCommandList);

    pd3dCommandList->SetGraphicsRoot32BitConstant(ROOT_SLOT_MATIDX_32, 0, 0);

    for(int i = 0; i < m_nBlocks; i++)
        if(m_ppBlockMeshes[i])
            m_ppBlockMeshes[i]->Render(pd3dCommandList, 0);
}

void CHeightMapTerrain::ReleaseUploadBuffers()
{
    if(m_ppBlockMeshes)
        for(int i = 0; i < m_nBlocks; i++)
            if(m_ppBlockMeshes[i])
                m_ppBlockMeshes[i]->ReleaseUploadBuffers();
}

float CHeightMapTerrain::GetHeight(float x, float z)
{
    return m_pHeightMapImage->GetHeight(x / m_xmf3Scale.x, z / m_xmf3Scale.z) * m_xmf3Scale.y;
}

XMFLOAT3 CHeightMapTerrain::GetNormal(float x, float z)
{
    return m_pHeightMapImage->GetHeightMapNormal(int(x / m_xmf3Scale.x), int(z / m_xmf3Scale.z));
}

int CHeightMapTerrain::GetHeightMapWidth() { return m_pHeightMapImage->GetHeightMapWidth(); }
int CHeightMapTerrain::GetHeightMapLength() { return m_pHeightMapImage->GetHeightMapLength(); }
