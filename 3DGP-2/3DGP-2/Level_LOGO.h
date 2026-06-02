#pragma once
#include "Level.h"
#include "Shader.h"
#include "Block.h"
#include "BitmapFont.h"
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
        XMFLOAT3 localOffset; 
    };

    // Builds one glyph as a group of cubes centered at (centerX, centerY).
    void BuildGlyphCubes(ID3D12Device* pd3dDevice,
                         ID3D12GraphicsCommandList* pd3dCommandList,
                         const Glyph& glyph, float centerX, float centerY,
                         float cell, XMFLOAT4 color);

    CObjectShader* m_pShader  = nullptr;
    CCamera*       m_pCamera  = nullptr;
    ID3D12Device*  m_pd3dDevice = nullptr;

    std::vector<LogoCube> m_LetterCubes;
    float m_fRotation = 0.0f;

    CBlock* m_pPlayButton   = nullptr; 
    CBlock* m_pPlayTriangle = nullptr; 

    static constexpr float BTN_NDC_X_MIN = -0.20f;
    static constexpr float BTN_NDC_X_MAX = +0.20f;
    static constexpr float BTN_NDC_Y_MIN = -0.70f;
    static constexpr float BTN_NDC_Y_MAX = -0.35f;
};
