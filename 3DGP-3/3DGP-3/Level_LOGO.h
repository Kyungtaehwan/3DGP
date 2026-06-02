#pragma once
#include "Level.h"
#include "TextManager.h"
#include <vector>

class CCamera;
class CColorShader;
class CGameObject;

// Title screen: game title (top) + author name (lower) as rotating 3D cube
// text. Clicking the name spawns an explosion effect (into the object manager);
// after a short delay the screen advances to the menu.
class CLevel_LOGO : public CLevel
{
public:
    CLevel_LOGO() {}
    virtual ~CLevel_LOGO() {}

    virtual void Initialize(ID3D12Device* pd3dDevice,
                            ID3D12GraphicsCommandList* pd3dCommandList,
                            ID3D12RootSignature* pd3dRootSignature) override;

    virtual int  Update(float dt) override;
    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList) override;
    virtual void Release() override;
    virtual void ReleaseUploadBuffers() override;

private:
    struct LogoCube
    {
        CGameObject* pObj;
        XMFLOAT3     localOffset;
    };


    void BuildTextCubes(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
                        int textId, float centerY, float cell, float spacing,
                        XMFLOAT4 color, std::vector<LogoCube>& out);

    ID3D12Device* m_pd3dDevice = nullptr;  // kept to spawn the effect on click
    CColorShader* m_pShader    = nullptr;
    CCamera*      m_pCamera    = nullptr;

    std::vector<LogoCube> m_TitleCubes;    // "3D게임프로그래밍1" (rotating, always)
    std::vector<LogoCube> m_NameCubes;     // "경태환" (rotating, clickable; hidden on explode)
    float m_fRotation = 0.0f;

    bool  m_bExploding    = false;
    float m_fExplodeTimer = 0.0f;
    float m_fNameY        = 0.0f;          // world Y where the name (and blast) sit

    static constexpr float CELL_NAME     = 0.03f;
    static constexpr float NAME_SPACING  = 0.07f;  // gap between glyphs
    static constexpr float TOP_Y         = 0.50f;  // title line
    static constexpr float EXPLODE_DELAY = 0.9f;   // wait this long, then go to menu
    static constexpr float kWorldPerNdcY = 2.309f;

    // Name click area (where the START button used to sit, lower-center).
    static constexpr float NAME_NDC_X_MIN = -0.30f;
    static constexpr float NAME_NDC_X_MAX = +0.30f;
    static constexpr float NAME_NDC_Y_MIN = -0.80f;
    static constexpr float NAME_NDC_Y_MAX = -0.55f;
};
