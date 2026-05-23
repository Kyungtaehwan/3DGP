#include "pch.h"
#include "GameObject.h"
#include "Shader.h"
#include "Camera.h"

CGameObject::CGameObject()
{
    m_xmf4x4Transform = Matrix4x4::Identity();
    m_xmf4x4World     = Matrix4x4::Identity();
}

CGameObject::~CGameObject()
{
    ReleaseShaderVariables();
    if (m_pMesh) m_pMesh->Release();
}

void CGameObject::AddRef()
{
    m_nReferences++;
    if (m_pSibling) m_pSibling->AddRef();
    if (m_pChild)   m_pChild->AddRef();
}

void CGameObject::Release()
{
    if (m_pChild)   m_pChild->Release();
    if (m_pSibling) m_pSibling->Release();
    if (--m_nReferences <= 0) delete this;
}

void CGameObject::SetMesh(CMesh* pMesh)
{
    if (m_pMesh) m_pMesh->Release();
    m_pMesh = pMesh;
    if (m_pMesh) m_pMesh->AddRef();
}

void CGameObject::SetChild(CGameObject* pChild, bool bReferenceUpdate)
{
    if (pChild)
    {
        pChild->m_pParent = this;
        if (bReferenceUpdate) pChild->AddRef();
    }
    if (m_pChild)
    {
        if (pChild) pChild->m_pSibling = m_pChild->m_pSibling;
        m_pChild->m_pSibling = pChild;
    }
    else
    {
        m_pChild = pChild;
    }
}

void CGameObject::Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent)
{
    if (m_pSibling) m_pSibling->Animate(fTimeElapsed, pxmf4x4Parent);
    if (m_pChild)   m_pChild->Animate(fTimeElapsed, &m_xmf4x4World);
}

CGameObject* CGameObject::FindFrame(char* pstrFrameName)
{
    CGameObject* pFrame = NULL;
    size_t lenA = strlen(m_pstrFrameName);
    size_t lenB = strlen(pstrFrameName);
    if (!strncmp(m_pstrFrameName, pstrFrameName, (lenA > lenB) ? lenA : lenB))
        return this;

    if (m_pSibling) if ((pFrame = m_pSibling->FindFrame(pstrFrameName))) return pFrame;
    if (m_pChild)   if ((pFrame = m_pChild  ->FindFrame(pstrFrameName))) return pFrame;
    return NULL;
}

void CGameObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
    if (m_pMesh)
    {
        if (m_pShader) m_pShader->OnPrepareRender(pd3dCommandList);

        if (m_pd3dcbGameObject) UpdateShaderVariables(pd3dCommandList);

        // Render all sub-meshes
        if (CMeshFromFile* pMF = dynamic_cast<CMeshFromFile*>(m_pMesh))
        {
            // Internally handles >=0 sub-meshes (nSubSet < m_nSubMeshes draws indexed)
            // For safety call once per submesh from outside if needed.
            // CMeshFromFile::Render currently draws one submesh at a time, so
            // we loop over the submeshes here:
            // We don't know the submesh count outside; render index 0..N until exhausted.
            // Simple approach: try index 0; if loader put everything in one submesh this
            // suffices. Apache.bin has 1 submesh per frame for our needs.
            m_pMesh->Render(pd3dCommandList, 0);
        }
    }

    if (m_pSibling) m_pSibling->Render(pd3dCommandList, pCamera);
    if (m_pChild)   m_pChild  ->Render(pd3dCommandList, pCamera);
}

void CGameObject::CreateShaderVariables(ID3D12Device* pd3dDevice,
                                        ID3D12GraphicsCommandList* pd3dCommandList)
{
    UINT cbSize = (sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255;

    m_pd3dcbGameObject = ::CreateBufferResource(
        pd3dDevice, pd3dCommandList,
        NULL, cbSize,
        D3D12_HEAP_TYPE_UPLOAD,
        D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
        NULL);

    m_pd3dcbGameObject->Map(0, NULL, (void**)&m_pcbMappedGameObject);

    if (m_pSibling) m_pSibling->CreateShaderVariables(pd3dDevice, pd3dCommandList);
    if (m_pChild)   m_pChild  ->CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CGameObject::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
    XMFLOAT4X4 xmf4x4World;
    XMStoreFloat4x4(&xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4World)));
    ::memcpy(&m_pcbMappedGameObject->m_xmf4x4World, &xmf4x4World, sizeof(XMFLOAT4X4));
    ::memcpy(&m_pcbMappedGameObject->m_xmf4Color,   &m_xmf4Color, sizeof(XMFLOAT4));

    D3D12_GPU_VIRTUAL_ADDRESS cbGpu = m_pd3dcbGameObject->GetGPUVirtualAddress();
    pd3dCommandList->SetGraphicsRootConstantBufferView(1, cbGpu);
}

void CGameObject::ReleaseShaderVariables()
{
    if (m_pd3dcbGameObject)
    {
        m_pd3dcbGameObject->Unmap(0, NULL);
        m_pd3dcbGameObject->Release();
        m_pd3dcbGameObject    = NULL;
        m_pcbMappedGameObject = NULL;
    }
    if (m_pSibling) m_pSibling->ReleaseShaderVariables();
    if (m_pChild)   m_pChild  ->ReleaseShaderVariables();
}

void CGameObject::ReleaseUploadBuffers()
{
    if (m_pMesh) m_pMesh->ReleaseUploadBuffers();
    if (m_pSibling) m_pSibling->ReleaseUploadBuffers();
    if (m_pChild)   m_pChild  ->ReleaseUploadBuffers();
}

void CGameObject::UpdateTransform(XMFLOAT4X4* pxmf4x4Parent)
{
    m_xmf4x4World = (pxmf4x4Parent) ? Matrix4x4::Multiply(m_xmf4x4Transform, *pxmf4x4Parent) : m_xmf4x4Transform;

    if (m_pSibling) m_pSibling->UpdateTransform(pxmf4x4Parent);
    if (m_pChild)   m_pChild  ->UpdateTransform(&m_xmf4x4World);
}

void CGameObject::SetPosition(float x, float y, float z)
{
    m_xmf4x4Transform._41 = x;
    m_xmf4x4Transform._42 = y;
    m_xmf4x4Transform._43 = z;
    UpdateTransform(NULL);
}

void CGameObject::SetPosition(XMFLOAT3 xmf3Position)
{
    SetPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z);
}

void CGameObject::SetScale(float x, float y, float z)
{
    XMMATRIX mtxScale = XMMatrixScaling(x, y, z);
    m_xmf4x4Transform = Matrix4x4::Multiply(mtxScale, m_xmf4x4Transform);
    UpdateTransform(NULL);
}

XMFLOAT3 CGameObject::GetPosition()
{
    return XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43);
}

XMFLOAT3 CGameObject::GetLook()
{
    return Vector3::Normalize(XMFLOAT3(m_xmf4x4World._31, m_xmf4x4World._32, m_xmf4x4World._33));
}

XMFLOAT3 CGameObject::GetUp()
{
    return Vector3::Normalize(XMFLOAT3(m_xmf4x4World._21, m_xmf4x4World._22, m_xmf4x4World._23));
}

XMFLOAT3 CGameObject::GetRight()
{
    return Vector3::Normalize(XMFLOAT3(m_xmf4x4World._11, m_xmf4x4World._12, m_xmf4x4World._13));
}

void CGameObject::MoveStrafe(float fDistance)
{
    XMFLOAT3 pos   = GetPosition();
    XMFLOAT3 right = GetRight();
    SetPosition(Vector3::Add(pos, right, fDistance));
}

void CGameObject::MoveUp(float fDistance)
{
    XMFLOAT3 pos = GetPosition();
    XMFLOAT3 up  = GetUp();
    SetPosition(Vector3::Add(pos, up, fDistance));
}

void CGameObject::MoveForward(float fDistance)
{
    XMFLOAT3 pos  = GetPosition();
    XMFLOAT3 look = GetLook();
    SetPosition(Vector3::Add(pos, look, fDistance));
}

void CGameObject::Rotate(float fPitch, float fYaw, float fRoll)
{
    XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(
        XMConvertToRadians(fPitch), XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
    m_xmf4x4Transform = Matrix4x4::Multiply(mtxRotate, m_xmf4x4Transform);
    UpdateTransform(NULL);
}

void CGameObject::Rotate(XMFLOAT3* pxmf3Axis, float fAngle)
{
    XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(pxmf3Axis), XMConvertToRadians(fAngle));
    m_xmf4x4Transform = Matrix4x4::Multiply(mtxRotate, m_xmf4x4Transform);
    UpdateTransform(NULL);
}

// ============================================================
// File loading
// ============================================================
static int ReadIntegerFromFile(FILE* pInFile)
{
    int nValue = 0;
    ::fread(&nValue, sizeof(int), 1, pInFile);
    return nValue;
}

static float ReadFloatFromFile(FILE* pInFile)
{
    float fValue = 0;
    ::fread(&fValue, sizeof(float), 1, pInFile);
    return fValue;
}

static BYTE ReadStringFromFile(FILE* pInFile, char* pstrToken)
{
    BYTE nStrLength = 0;
    ::fread(&nStrLength, sizeof(BYTE), 1, pInFile);
    ::fread(pstrToken, sizeof(char), nStrLength, pInFile);
    pstrToken[nStrLength] = '\0';
    return nStrLength;
}

CMeshLoadInfo* CGameObject::LoadMeshInfoFromFile(FILE* pInFile)
{
    char pstrToken[64] = { '\0' };

    CMeshLoadInfo* pMeshInfo = new CMeshLoadInfo;

    pMeshInfo->m_nVertices = ReadIntegerFromFile(pInFile);
    ReadStringFromFile(pInFile, pMeshInfo->m_pstrMeshName);

    for (;;)
    {
        ReadStringFromFile(pInFile, pstrToken);

        if (!strcmp(pstrToken, "<Bounds>:"))
        {
            ::fread(&pMeshInfo->m_xmf3AABBCenter,  sizeof(XMFLOAT3), 1, pInFile);
            ::fread(&pMeshInfo->m_xmf3AABBExtents, sizeof(XMFLOAT3), 1, pInFile);
        }
        else if (!strcmp(pstrToken, "<Positions>:"))
        {
            int nPositions = ReadIntegerFromFile(pInFile);
            if (nPositions > 0)
            {
                pMeshInfo->m_nType |= VERTEXT_POSITION;
                pMeshInfo->m_pxmf3Positions = new XMFLOAT3[nPositions];
                ::fread(pMeshInfo->m_pxmf3Positions, sizeof(XMFLOAT3), nPositions, pInFile);
            }
        }
        else if (!strcmp(pstrToken, "<Colors>:"))
        {
            int nColors = ReadIntegerFromFile(pInFile);
            if (nColors > 0)
            {
                pMeshInfo->m_nType |= VERTEXT_COLOR;
                pMeshInfo->m_pxmf4Colors = new XMFLOAT4[nColors];
                ::fread(pMeshInfo->m_pxmf4Colors, sizeof(XMFLOAT4), nColors, pInFile);
            }
        }
        else if (!strcmp(pstrToken, "<Normals>:"))
        {
            int nNormals = ReadIntegerFromFile(pInFile);
            if (nNormals > 0)
            {
                pMeshInfo->m_nType |= VERTEXT_NORMAL;
                pMeshInfo->m_pxmf3Normals = new XMFLOAT3[nNormals];
                ::fread(pMeshInfo->m_pxmf3Normals, sizeof(XMFLOAT3), nNormals, pInFile);
            }
        }
        else if (!strcmp(pstrToken, "<Indices>:"))
        {
            int nIndices = ReadIntegerFromFile(pInFile);
            if (nIndices > 0)
            {
                pMeshInfo->m_pnIndices = new UINT[nIndices];
                ::fread(pMeshInfo->m_pnIndices, sizeof(int), nIndices, pInFile);
            }
        }
        else if (!strcmp(pstrToken, "<SubMeshes>:"))
        {
            pMeshInfo->m_nSubMeshes = ReadIntegerFromFile(pInFile);
            if (pMeshInfo->m_nSubMeshes > 0)
            {
                pMeshInfo->m_pnSubSetIndices  = new int[pMeshInfo->m_nSubMeshes];
                pMeshInfo->m_ppnSubSetIndices = new UINT*[pMeshInfo->m_nSubMeshes];
                for (int i = 0; i < pMeshInfo->m_nSubMeshes; i++)
                {
                    pMeshInfo->m_ppnSubSetIndices[i] = NULL;
                    ReadStringFromFile(pInFile, pstrToken);
                    if (!strcmp(pstrToken, "<SubMesh>:"))
                    {
                        ReadIntegerFromFile(pInFile); // sub-mesh index (ignored)
                        pMeshInfo->m_pnSubSetIndices[i] = ReadIntegerFromFile(pInFile);
                        if (pMeshInfo->m_pnSubSetIndices[i] > 0)
                        {
                            pMeshInfo->m_ppnSubSetIndices[i] = new UINT[pMeshInfo->m_pnSubSetIndices[i]];
                            ::fread(pMeshInfo->m_ppnSubSetIndices[i],
                                    sizeof(UINT), pMeshInfo->m_pnSubSetIndices[i], pInFile);
                        }
                    }
                }
            }
        }
        else if (!strcmp(pstrToken, "</Mesh>"))
        {
            break;
        }
    }
    return pMeshInfo;
}

// Skip "<Materials>:" block since this build has no lighting/materials —
// we just want to advance the file pointer past it.
static void SkipMaterialsBlock(FILE* pInFile)
{
    char pstrToken[64] = { '\0' };
    int nMaterials = ReadIntegerFromFile(pInFile);
    (void)nMaterials;

    for (;;)
    {
        ReadStringFromFile(pInFile, pstrToken);
        if (!strcmp(pstrToken, "<Material>:"))
        {
            ReadIntegerFromFile(pInFile);
        }
        else if (!strcmp(pstrToken, "<AlbedoColor>:")  ||
                 !strcmp(pstrToken, "<EmissiveColor>:") ||
                 !strcmp(pstrToken, "<SpecularColor>:"))
        {
            float dummy[4];
            ::fread(dummy, sizeof(float), 4, pInFile);
        }
        else if (!strcmp(pstrToken, "<Glossiness>:")         ||
                 !strcmp(pstrToken, "<Smoothness>:")         ||
                 !strcmp(pstrToken, "<Metallic>:")           ||
                 !strcmp(pstrToken, "<SpecularHighlight>:")  ||
                 !strcmp(pstrToken, "<GlossyReflection>:"))
        {
            float dummy;
            ::fread(&dummy, sizeof(float), 1, pInFile);
        }
        else if (!strcmp(pstrToken, "</Materials>"))
        {
            break;
        }
    }
}

CGameObject* CGameObject::LoadFrameHierarchyFromFile(
    ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
    FILE* pInFile, CShader* pShader)
{
    char pstrToken[64] = { '\0' };

    CGameObject* pGameObject = NULL;

    for (;;)
    {
        ReadStringFromFile(pInFile, pstrToken);
        if (!strcmp(pstrToken, "<Frame>:"))
        {
            pGameObject = new CGameObject();
            ReadIntegerFromFile(pInFile); // frame index (ignored)
            ReadStringFromFile(pInFile, pGameObject->m_pstrFrameName);
            pGameObject->SetShader(pShader);
        }
        else if (!strcmp(pstrToken, "<Transform>:"))
        {
            XMFLOAT3 xmf3Position, xmf3Rotation, xmf3Scale;
            XMFLOAT4 xmf4Rotation;
            ::fread(&xmf3Position, sizeof(float), 3, pInFile);
            ::fread(&xmf3Rotation, sizeof(float), 3, pInFile);
            ::fread(&xmf3Scale,    sizeof(float), 3, pInFile);
            ::fread(&xmf4Rotation, sizeof(float), 4, pInFile);
        }
        else if (!strcmp(pstrToken, "<TransformMatrix>:"))
        {
            ::fread(&pGameObject->m_xmf4x4Transform, sizeof(float), 16, pInFile);
        }
        else if (!strcmp(pstrToken, "<Mesh>:") || !strcmp(pstrToken, "<SkinnedMesh>:"))
        {
            CMeshLoadInfo* pMeshInfo = CGameObject::LoadMeshInfoFromFile(pInFile);
            if (pMeshInfo)
            {
                CMesh* pMesh = new CMeshFromFile(pd3dDevice, pd3dCommandList, pMeshInfo);
                pGameObject->SetMesh(pMesh);
                delete pMeshInfo;
            }
        }
        else if (!strcmp(pstrToken, "<Materials>:"))
        {
            SkipMaterialsBlock(pInFile);
        }
        else if (!strcmp(pstrToken, "<Children>:"))
        {
            int nChilds = ReadIntegerFromFile(pInFile);
            for (int i = 0; i < nChilds; i++)
            {
                CGameObject* pChild = CGameObject::LoadFrameHierarchyFromFile(
                    pd3dDevice, pd3dCommandList, pInFile, pShader);
                if (pChild) pGameObject->SetChild(pChild);
            }
        }
        else if (!strcmp(pstrToken, "</Frame>"))
        {
            break;
        }
    }
    return pGameObject;
}

CGameObject* CGameObject::LoadGeometryFromFile(
    ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
    const char* pstrFileName, CShader* pShader)
{
    FILE* pInFile = NULL;
    ::fopen_s(&pInFile, pstrFileName, "rb");
    if (!pInFile) return NULL;
    ::rewind(pInFile);

    CGameObject* pGameObject = NULL;
    char pstrToken[64] = { '\0' };

    for (;;)
    {
        if (ReadStringFromFile(pInFile, pstrToken) == 0) break;

        if (!strcmp(pstrToken, "<Hierarchy>:"))
        {
            pGameObject = CGameObject::LoadFrameHierarchyFromFile(
                pd3dDevice, pd3dCommandList, pInFile, pShader);
        }
        else if (!strcmp(pstrToken, "</Hierarchy>"))
        {
            break;
        }
    }
    ::fclose(pInFile);
    return pGameObject;
}
