#pragma once
#include "Level.h"

class CIlluminatedShader;
class CTerrainShader;
class CCamera;
class CLightManager;
class CHeightMapTerrain;

class CLevel_GamePlay : public CLevel
{
public:
    CLevel_GamePlay() {}
    virtual ~CLevel_GamePlay() {}

    virtual void Initialize(ID3D12Device* pd3dDevice,
                            ID3D12GraphicsCommandList* pd3dCommandList,
                            ID3D12RootSignature* pd3dRootSignature) override;

    virtual int  Update(float dt) override;
    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList) override;
    virtual void Release() override;
    virtual void ReleaseUploadBuffers() override;

private:
    CIlluminatedShader* m_pShader        = NULL;   // objects (heli/humvee/trees)
    CTerrainShader*     m_pTerrainShader = NULL;   // terrain + route-line overlay
    CCamera*            m_pCamera        = NULL;
    CLightManager*      m_pLightManager  = NULL;
    CHeightMapTerrain*  m_pTerrain       = NULL;

    // Route line painted on the terrain (start -> destination), bound at b5.
    ID3D12Resource* m_pcbLine       = NULL;
    void*           m_pcbLineMapped = NULL;
};
