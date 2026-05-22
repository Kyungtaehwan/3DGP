#pragma once
#include "Level.h"
#include "Shader.h"
#include "GameObject.h"
#include "Map_Manager.h"
#include "OBBRenderer.h"
#include "Bullet.h"
#include "Enemy.h"

class CCamera;

class CLevel_GamePlay : public CLevel
{
public:
    CLevel_GamePlay() {}
    virtual ~CLevel_GamePlay() {}

    virtual void Initialize(ID3D12Device* pd3dDevice,
                            ID3D12GraphicsCommandList* pd3dCommandList,
                            ID3D12RootSignature* pd3dRootSignature) override;

    virtual int  Update(float dt) override;
    virtual void Late_Update(float dt) override;
    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList) override;
    virtual void Release() override;
    virtual void ReleaseUploadBuffers() override;

private:
    void SpawnPlayer();
    void SpawnAtStair();
    void SpawnEnemies(ID3D12GraphicsCommandList* pd3dCommandList);

    CObjectShader* m_pShader    = NULL;
    CCamera*       m_pCamera    = NULL;
    ID3D12Device*  m_pd3dDevice = NULL;

    int  m_nCurrentMap           = 1;
    bool m_bPendingMapSwitch     = false;
    bool m_bPendingFloorSwitch   = false;
    bool m_bFloorSwitchIsStair   = false;
    bool m_bNeedReleaseUpload    = false;
    bool m_bCleared              = false;
    bool m_bWasOnBottomStair     = false;
};
