#pragma once
#include "Level.h"
#include <vector>

class CIlluminatedShader;
class CTerrainShader;
class CColorShader;
class CCamera;
class CLightManager;
class CHeightMapTerrain;
class CHumvee;
class CMesh;
class CGameObject;

class CLevel_Stage1 : public CLevel
{
public:
    CLevel_Stage1() {}
    virtual ~CLevel_Stage1() {}

    virtual void Initialize(ID3D12Device* pd3dDevice,
                            ID3D12GraphicsCommandList* pd3dCommandList,
                            ID3D12RootSignature* pd3dRootSignature) override;

    virtual int Update(float dt) override;
    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList) override;
    virtual void Release() override;
    virtual void ReleaseUploadBuffers() override;

private:

    enum GameState
    {
        PLAYING,
        WINNING,
        LOSING,
        WIN,
        LOSE
    };

    CIlluminatedShader* m_pShader = NULL; 
    CTerrainShader* m_pTerrainShader = NULL;
    CColorShader* m_pColorShader = NULL;
    CCamera* m_pCamera = NULL;
    CLightManager* m_pLightManager = NULL;
    CHeightMapTerrain* m_pTerrain = NULL;

    //cube bullet mesh
    CMesh* m_pPlayerBulletMesh = NULL;
    CMesh* m_pEnemyBulletMesh = NULL;

    GameState m_eState = PLAYING;

    float m_fEventTimer = 0.0f;
    const float m_fEventDelay = 2.0f;

    float m_fResultTimer = 0.0f;
    const float m_fResultHold = 3.0f;

    ID3D12Resource* m_pcbLine = NULL;
    void* m_pcbLineMapped = NULL;

    struct ResultCube
    {
        CGameObject* pObj;
        XMFLOAT3 localOffset;
    };
    void BuildResultText(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
                         int textId, XMFLOAT4 color, std::vector<ResultCube>& cubesOut);

    CCamera* m_pResultCamera = NULL;
    std::vector<ResultCube> m_WinCubes;
    std::vector<ResultCube> m_LoseCubes;
};
