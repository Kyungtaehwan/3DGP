#pragma once
#include "GameObject.h"

class CShader;
class CHeightMapTerrain;

// Auto-driving ground vehicle. Loads a model hierarchy, drives forward across
// the terrain, snapping to the surface height and tilting its "up" axis to the
// terrain normal. Lives in the object manager's OBJ_HUMVEE list, so its Update
// runs automatically each frame.
class CHumvee : public CGameObject
{
public:
    CHumvee();
    virtual ~CHumvee();

    bool LoadModel(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
                   CShader* pShader, const char* pstrFileName);

    void SetTerrain(CHeightMapTerrain* pTerrain) { m_pTerrain = pTerrain; }

    // Drive in a straight line from start -> end (world x,z), then stop. Heading
    // is derived from the start->end direction.
    void SetPath(XMFLOAT3 start, XMFLOAT3 end, float fSpeed, float fScale);

    // Overrides every material's diffuse color across the whole model hierarchy.
    void SetBodyColor(XMFLOAT4 xmf4Color);

    virtual int  Update(float fTimeElapsed) override;
    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera) override;
    virtual void ReleaseUploadBuffers() override;

    XMFLOAT3 GetPosition() const { return m_xmf3Pos; }

private:
    CGameObject*       m_pModel   = NULL;
    CHeightMapTerrain* m_pTerrain = NULL;

    XMFLOAT3 m_xmf3Pos = { 0.0f, 0.0f, 0.0f };
    XMFLOAT3 m_xmf3End = { 0.0f, 0.0f, 0.0f };
    float    m_fYaw    = 0.0f;     // heading, degrees
    float    m_fSpeed  = 40.0f;    // units / sec
    float    m_fScale  = 1.0f;
    bool     m_bArrived = false;
};
