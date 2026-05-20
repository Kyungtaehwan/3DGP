#pragma once

class CCamera;

class CLevel
{
public:
    CLevel() {}
    virtual ~CLevel() {}

    virtual void Initialize(ID3D12Device* pd3dDevice,
                            ID3D12GraphicsCommandList* pd3dCommandList,
                            ID3D12RootSignature* pd3dRootSignature) = 0;

    virtual int  Update(float dt)     = 0;
    virtual void Late_Update(float dt)= 0;

    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList) = 0;
    virtual void Release()     = 0;
    virtual void ReleaseUploadBuffers() {}
};
