#pragma once
#include "Level.h"

class CLevel_Menu : public CLevel
{
public:
    CLevel_Menu() {}
    virtual ~CLevel_Menu() {}

    virtual void Initialize(ID3D12Device* pd3dDevice,
                            ID3D12GraphicsCommandList* pd3dCommandList,
                            ID3D12RootSignature* pd3dRootSignature) override;

    virtual int  Update(float dt) override;
    virtual void Late_Update(float dt) override {}
    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList) override;
    virtual void Release() override {}

private:
    float m_fTimeAcc = 0.0f;

    static constexpr float STAGE1_NDC_X_MIN = -0.70f;
    static constexpr float STAGE1_NDC_X_MAX = -0.10f;
    static constexpr float STAGE1_NDC_Y_MIN = -0.50f;
    static constexpr float STAGE1_NDC_Y_MAX = +0.50f;

    static constexpr float STAGE2_NDC_X_MIN = +0.10f;
    static constexpr float STAGE2_NDC_X_MAX = +0.70f;
    static constexpr float STAGE2_NDC_Y_MIN = -0.50f;
    static constexpr float STAGE2_NDC_Y_MAX = +0.50f;
};
