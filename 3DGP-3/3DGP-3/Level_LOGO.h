#pragma once
#include "Level.h"
#include "Text_Manager.h"
#include <vector>

class CCamera;
class CColorShader;
class CGameObject;

class CLevel_LOGO : public CLevel
{
public:
    CLevel_LOGO() {}
    virtual ~CLevel_LOGO() {}

    virtual void Initialize(ID3D12Device* pd3dDevice,
                            ID3D12GraphicsCommandList* pd3dCommandList,
                            ID3D12RootSignature* pd3dRootSignature) override;

    virtual int Update(float dt) override;
    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList) override;
    virtual void Release() override;
    virtual void ReleaseUploadBuffers() override;

private:
    struct LogoCube
    {
        CGameObject* pObj;
        XMFLOAT3 localOffset;
    };


    void BuildTextCubes(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
                        int textId, float centerY, float cell, float spacing,
                        XMFLOAT4 color, std::vector<LogoCube>& out);

    ID3D12Device* m_pd3dDevice = nullptr;
    CColorShader* m_pShader = nullptr;
    CCamera* m_pCamera = nullptr;

    std::vector<LogoCube> m_TitleCubes;
    std::vector<LogoCube> m_NameCubes;
    float m_fRotation = 0.0f;

    bool m_bExploding = false;
    float m_fExplodeTimer = 0.0f;
    float m_fNameY = 0.0f;

    static constexpr float CELL_NAME = 0.03f;
    static constexpr float NAME_SPACING = 0.07f;
    static constexpr float TOP_Y = 0.50f;
    static constexpr float EXPLODE_DELAY = 0.9f;
    static constexpr float kWorldPerNdcY = 2.309f;

    static constexpr float NAME_NDC_X_MIN = -0.30f;
    static constexpr float NAME_NDC_X_MAX = +0.30f;
    static constexpr float NAME_NDC_Y_MIN = -0.80f;
    static constexpr float NAME_NDC_Y_MAX = -0.55f;
};
