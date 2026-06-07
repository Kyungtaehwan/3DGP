#pragma once
#include "Mesh.h"

class CCamera;
class CShader;

#define MATERIALS_IN_HIERARCHY 8

struct MATERIALLOADINFO
{
    XMFLOAT4 m_xmf4AlbedoColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    XMFLOAT4 m_xmf4EmissiveColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    XMFLOAT4 m_xmf4SpecularColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

    float m_fGlossiness = 0.0f;
    float m_fSmoothness = 0.0f;
    float m_fSpecularHighlight = 0.0f;
    float m_fMetallic = 0.0f;
    float m_fGlossyReflection = 0.0f;
};

struct MATERIALSLOADINFO
{
    int m_nMaterials = 0;
    MATERIALLOADINFO* m_pMaterials = NULL;
    ~MATERIALSLOADINFO()
    {
        if(m_pMaterials)
            delete[] m_pMaterials;
    }
};

class CMaterialColors
{
public:
    CMaterialColors() {}
    CMaterialColors(MATERIALLOADINFO* p);
    ~CMaterialColors() {}

    void AddRef() { m_nReferences++; }
    void Release()
    {
        if(--m_nReferences <= 0)
            delete this;
    }

    XMFLOAT4 m_xmf4Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    XMFLOAT4 m_xmf4Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    XMFLOAT4 m_xmf4Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f); // a = specular power
    XMFLOAT4 m_xmf4Emissive = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

private:
    int m_nReferences = 0;
};

class CMaterial
{
public:
    CMaterial();
    ~CMaterial();

    void AddRef() { m_nReferences++; }
    void Release()
    {
        if(--m_nReferences <= 0)
            delete this;
    }

    void SetShader(CShader* pShader);
    void SetMaterialColors(CMaterialColors* p);

    CShader* m_pShader = NULL;
    CMaterialColors* m_pMaterialColors = NULL;

private:
    int m_nReferences = 0;
};

// One per-material payload in the GameObject constant buffer (matches HLSL MATERIAL).
struct MATERIAL_CB
{
    XMFLOAT4 m_xmf4Ambient;
    XMFLOAT4 m_xmf4Diffuse;
    XMFLOAT4 m_xmf4Specular; // a = power
    XMFLOAT4 m_xmf4Emissive;
};

struct CB_GAMEOBJECT_INFO
{
    XMFLOAT4X4 m_xmf4x4World;
    MATERIAL_CB m_Materials[MATERIALS_IN_HIERARCHY];
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

    int m_nMaterials = 0;
    CMaterial** m_ppMaterials = NULL;

    XMFLOAT4X4 m_xmf4x4Transform;
    XMFLOAT4X4 m_xmf4x4World;

    // World-space collision box, recomputed each frame by the owning object.
    // Used by CObject_Manager::CheckCollisions() (DirectXCollision Intersects).
    BoundingOrientedBox m_xmOOBB;
    bool m_bActive = true; // false = scheduled for removal

    CGameObject* m_pParent = NULL;
    CGameObject* m_pChild = NULL;
    CGameObject* m_pSibling = NULL;

    void SetMesh(CMesh* pMesh);
    // Creates a single default material that uses pShader (used when a frame
    // has a mesh but no <Materials> block).
    void SetShader(CShader* pShader);
    void SetShader(int nMaterial, CShader* pShader);
    void SetMaterial(int nMaterial, CMaterial* pMaterial);

    void SetChild(CGameObject* pChild, bool bReferenceUpdate = false);

    virtual void OnInitialize() {}
    virtual int Update(float fTimeElapsed) { return OBJ_NOEVENT; }
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

    void MoveStrafe(float fDistance = 1.0f);
    void MoveUp(float fDistance = 1.0f);
    void MoveForward(float fDistance = 1.0f);

    void Rotate(float fPitch, float fYaw, float fRoll);
    void Rotate(XMFLOAT3* pxmf3Axis, float fAngle);

    void UpdateTransform(XMFLOAT4X4* pxmf4x4Parent = NULL);
    CGameObject* FindFrame(char* pstrFrameName);

    UINT GetMeshType() const
    {
        if(m_pMesh)
            return m_pMesh->GetType();
        else
            return 0;
    }

    static CGameObject* LoadFrameHierarchyFromFile(
        ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
        FILE* pInFile, CShader* pShader);
    static CGameObject* LoadGeometryFromFile(
        ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
        const char* pstrFileName, CShader* pShader);
    static CMeshLoadInfo* LoadMeshInfoFromFile(FILE* pInFile);
    static MATERIALSLOADINFO* LoadMaterialsInfoFromFile(FILE* pInFile);

protected:
    int m_nReferences = 0;

    ID3D12Resource* m_pd3dcbGameObject = NULL;
    CB_GAMEOBJECT_INFO* m_pcbMappedGameObject = NULL;
};
