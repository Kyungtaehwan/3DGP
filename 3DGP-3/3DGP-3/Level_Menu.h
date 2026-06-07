#pragma once
#include "Level.h"
#include "Text_Manager.h"
#include <vector>

class CCamera;
class CColorShader;
class CGameObject;
class CMesh;


class CLevel_Menu : public CLevel
{
public:
    CLevel_Menu() {}
    virtual ~CLevel_Menu() {}

    virtual void Initialize(ID3D12Device* pd3dDevice,
                            ID3D12GraphicsCommandList* pd3dCommandList,
                            ID3D12RootSignature* pd3dRootSignature) override;

    virtual int Update(float dt) override;
    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList) override;
    virtual void Release() override;
    virtual void ReleaseUploadBuffers() override;

private:
    enum
    {

        ITEM_TUTORIAL = 0,
        ITEM_LEVEL1,
        ITEM_LEVEL2,
        ITEM_LEVEL3,
        ITEM_START,
        ITEM_END,
        ITEM_COUNT
    };

    struct MenuCube
    {
        CGameObject* pObj;
        XMFLOAT3 rel;
        int item;
    };

    void BuildLabel(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
                    int textId, int item, float centerY, XMFLOAT4 color);
    void UpdateTransforms();
    int HitTest(float ndcX, float ndcY) const;

    CColorShader* m_pShader = nullptr;
    CCamera* m_pCamera = nullptr;

    std::vector<MenuCube> m_Cubes;
    float m_ItemCenterY[ITEM_COUNT] = { 0 };
    int m_nSelectedStage = 1;

    CMesh* m_pBlueMesh = nullptr;
    CMesh* m_ItemMesh[ITEM_COUNT] = {};

    static constexpr float CELL = 0.07f;
    static constexpr float SPACING = 0.02f;
    static constexpr float kWorldPerNdcY = 2.3f;
    static constexpr float HIT_HALF_Y = 0.11f;
    static constexpr float HIT_X_MIN = -0.55f;
    static constexpr float HIT_X_MAX = +0.55f;
};
