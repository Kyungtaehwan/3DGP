#pragma once
#include "Mesh.h"

class CCamera;
class CShader;

struct CB_GAMEOBJECT_INFO
{
    XMFLOAT4X4 m_xmf4x4World;
    XMFLOAT4   m_xmf4Color;
};

class CGameObject
{
public:
    CGameObject();
    virtual ~CGameObject();

    void AddRef();
    void Release();

    char m_pstrFrameName[64] = { 0 };

    CMesh* m_pMesh = NULL;

    XMFLOAT4 m_xmf4Color = XMFLOAT4(0.7f, 0.7f, 0.75f, 1.0f);

    XMFLOAT4X4 m_xmf4x4Transform;
    XMFLOAT4X4 m_xmf4x4World;

    CGameObject* m_pParent  = NULL;
    CGameObject* m_pChild   = NULL;
    CGameObject* m_pSibling = NULL;

    void SetMesh(CMesh* pMesh);
    void SetShader(CShader* pShader) { m_pShader = pShader; }
    void SetColor(XMFLOAT4 c) { m_xmf4Color = c; }

    void SetChild(CGameObject* pChild, bool bReferenceUpdate = false);

    virtual void OnInitialize() {}
    virtual int  Update(float fTimeElapsed) { return OBJ_NOEVENT; }
    virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent = NULL);

    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

    virtual void CreateShaderVariables(ID3D12Device* pd3dDevice,
                                       ID3D12GraphicsCommandList* pd3dCommandList);
    virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
    virtual void ReleaseShaderVariables();

    virtual void ReleaseUploadBuffers();

    XMFLOAT3 GetPosition();
    XMFLOAT3 GetLook();
    XMFLOAT3 GetUp();
    XMFLOAT3 GetRight();

    void SetPosition(float x, float y, float z);
    void SetPosition(XMFLOAT3 xmf3Position);
    void SetScale(float x, float y, float z);

    void MoveStrafe (float fDistance = 1.0f);
    void MoveUp     (float fDistance = 1.0f);
    void MoveForward(float fDistance = 1.0f);

    void Rotate(float fPitch, float fYaw, float fRoll);
    void Rotate(XMFLOAT3* pxmf3Axis, float fAngle);

    void UpdateTransform(XMFLOAT4X4* pxmf4x4Parent = NULL);
    CGameObject* FindFrame(char* pstrFrameName);

    static CGameObject* LoadFrameHierarchyFromFile(
        ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
        FILE* pInFile, CShader* pShader);
    static CGameObject* LoadGeometryFromFile(
        ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
        const char* pstrFileName, CShader* pShader);
    static CMeshLoadInfo* LoadMeshInfoFromFile(FILE* pInFile);

protected:
    int      m_nReferences = 0;
    CShader* m_pShader     = NULL;

    ID3D12Resource*     m_pd3dcbGameObject       = NULL;
    CB_GAMEOBJECT_INFO* m_pcbMappedGameObject    = NULL;
};
