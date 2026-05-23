#pragma once
#include "Level.h"
#include "Shader.h"
#include "Block.h"
#include <vector>

class CCamera;

class CLevel_LOGO : public CLevel
{
public:
    CLevel_LOGO() {}
    virtual ~CLevel_LOGO() {}

    virtual void Initialize(ID3D12Device* pd3dDevice,
                            ID3D12GraphicsCommandList* pd3dCommandList,
                            ID3D12RootSignature* pd3dRootSignature) override;

    virtual int  Update(float dt) override;
    virtual void Late_Update(float dt) override {}
    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList) override;
    virtual void Release() override;

private:
    struct LogoCube
    {
        CBlock*  pObj;
        XMFLOAT3 localOffset;   // relative to logo group origin
    };

    void BuildLetter(ID3D12Device* pd3dDevice,
                     ID3D12GraphicsCommandList* pd3dCommandList,
                     const int pattern[5][3], float letterCenterX, XMFLOAT4 color);

    CObjectShader* m_pShader  = nullptr;
    CCamera*       m_pCamera  = nullptr;
    ID3D12Device*  m_pd3dDevice = nullptr;

    std::vector<LogoCube> m_LetterCubes;
    float m_fRotation = 0.0f;

    CBlock* m_pPlayButton   = nullptr;   // dark grey cube
    CBlock* m_pPlayTriangle = nullptr;   // white right-pointing triangle in front of the cube

    // Hard-coded NDC click region of the play button (camera is fixed,
    // so projected screen position is also fixed).
    static constexpr float BTN_NDC_X_MIN = -0.20f;
    static constexpr float BTN_NDC_X_MAX = +0.20f;
    static constexpr float BTN_NDC_Y_MIN = -0.70f;
    static constexpr float BTN_NDC_Y_MAX = -0.35f;
};
