#pragma once
#include "Level.h"

class CObjectShader;
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
    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList) override;
    virtual void Release() override;
    virtual void ReleaseUploadBuffers() override;

private:
    CObjectShader* m_pShader = NULL;
    CCamera*       m_pCamera = NULL;
};
