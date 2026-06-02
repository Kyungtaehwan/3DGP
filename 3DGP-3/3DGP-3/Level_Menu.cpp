#include "pch.h"
#include "Level_Menu.h"
#include "Level_Manager.h"
#include "Camera.h"
#include "Mesh.h"
#include "Shader.h"
#include "GameObject.h"
#include "Input_Manager.h"
#include "TextManager.h"

// NDC y-center per menu item (top -> bottom), matching the ITEM_* order.
static const float s_ItemNdcY[6] = { 0.66f, 0.40f, 0.14f, -0.12f, -0.38f, -0.64f };

void CLevel_Menu::Initialize(ID3D12Device* pd3dDevice,
                              ID3D12GraphicsCommandList* pd3dCommandList,
                              ID3D12RootSignature* pd3dRootSignature)
{
    CInput_Manager::Get_Instance()->SetMouseLock(false);

    m_pShader = new CColorShader();
    m_pShader->AddRef();
    m_pShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dRootSignature);

    m_pCamera = new CCamera();
    m_pCamera->CreateShaderVariables(pd3dDevice, pd3dCommandList);
    m_pCamera->SetPosition(XMFLOAT3(0.0f, 0.0f, -4.0f));
    m_pCamera->GenerateViewMatrix();
    m_pCamera->GenerateProjectionMatrix(0.1f, 100.0f, ASPECT_RATIO, 60.0f);
    m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
    m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);

    const XMFLOAT4 white = { 0.90f, 0.90f, 0.95f, 1.0f };
    const XMFLOAT4 green = { 0.40f, 1.00f, 0.50f, 1.0f };
    const XMFLOAT4 red   = { 1.00f, 0.40f, 0.40f, 1.0f };

    BuildLabel(pd3dDevice, pd3dCommandList, CText_Manager::TEXT_TUTORIAL, ITEM_TUTORIAL, s_ItemNdcY[0] * kWorldPerNdcY, white);
    BuildLabel(pd3dDevice, pd3dCommandList, CText_Manager::TEXT_LEVEL1,   ITEM_LEVEL1,   s_ItemNdcY[1] * kWorldPerNdcY, white);
    BuildLabel(pd3dDevice, pd3dCommandList, CText_Manager::TEXT_LEVEL2,   ITEM_LEVEL2,   s_ItemNdcY[2] * kWorldPerNdcY, white);
    BuildLabel(pd3dDevice, pd3dCommandList, CText_Manager::TEXT_LEVEL3,   ITEM_LEVEL3,   s_ItemNdcY[3] * kWorldPerNdcY, white);
    BuildLabel(pd3dDevice, pd3dCommandList, CText_Manager::TEXT_START,    ITEM_START,    s_ItemNdcY[4] * kWorldPerNdcY, green);
    BuildLabel(pd3dDevice, pd3dCommandList, CText_Manager::TEXT_END,      ITEM_END,      s_ItemNdcY[5] * kWorldPerNdcY, red);

    // Shared highlight mesh: the selected stage's cubes swap to this blue cube.
    const XMFLOAT4 blue = { 0.25f, 0.5f, 1.0f, 1.0f };
    m_pBlueMesh = new CColorCubeMesh(pd3dDevice, pd3dCommandList, CELL, CELL, CELL, blue);
    m_pBlueMesh->AddRef();

    UpdateTransforms();
}

void CLevel_Menu::BuildLabel(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
                             int textId, int item, float centerY, XMFLOAT4 color)
{
    m_ItemCenterY[item] = centerY;

    std::vector<XMFLOAT3> cells;
    CText_Manager::Get_Instance()->LayoutText(textId, 0.0f, centerY, CELL, SPACING, cells);
    if (cells.empty()) return;

    // All cells of this label share one cube mesh (same size + color). Keep an
    // owning ref so it survives even while the cubes are showing the blue mesh.
    CColorCubeMesh* pMesh = new CColorCubeMesh(pd3dDevice, pd3dCommandList, CELL, CELL, CELL, color);
    m_ItemMesh[item] = pMesh;
    pMesh->AddRef();

    for (const XMFLOAT3& c : cells)
    {
        CGameObject* pCube = new CGameObject();
        pCube->SetMesh(pMesh);
        pCube->SetShader(m_pShader);
        pCube->CreateShaderVariables(pd3dDevice, pd3dCommandList);

        MenuCube mc;
        mc.pObj = pCube;
        mc.rel  = XMFLOAT3(c.x, c.y - centerY, 0.0f);
        mc.item = item;
        m_Cubes.push_back(mc);
    }
}

void CLevel_Menu::UpdateTransforms()
{
    int selectedItem = m_nSelectedStage;   // stage 0..3 maps directly to item index

    for (MenuCube& mc : m_Cubes)
    {
        // Fixed position (no scaling/pop).
        XMMATRIX world = XMMatrixTranslation(mc.rel.x,
                                             m_ItemCenterY[mc.item] + mc.rel.y,
                                             mc.rel.z);
        XMStoreFloat4x4(&mc.pObj->m_xmf4x4World, world);

        // Selected stage turns blue; everything else keeps its normal color.
        bool sel = (mc.item == selectedItem);
        mc.pObj->SetMesh(sel ? m_pBlueMesh : m_ItemMesh[mc.item]);
    }
}

int CLevel_Menu::HitTest(float ndcX, float ndcY) const
{
    if (ndcX < HIT_X_MIN || ndcX > HIT_X_MAX) return -1;
    for (int i = 0; i < ITEM_COUNT; ++i)
        if (ndcY >= s_ItemNdcY[i] - HIT_HALF_Y && ndcY <= s_ItemNdcY[i] + HIT_HALF_Y)
            return i;
    return -1;
}

int CLevel_Menu::Update(float /*dt*/)
{
    if (CInput_Manager::Get_Instance()->Key_Down(VK_LBUTTON))
    {
        POINT pt; GetCursorPos(&pt); ScreenToClient(g_hWnd, &pt);
        float ndcX = ((float)pt.x / (float)FRAME_BUFFER_WIDTH)  * 2.0f - 1.0f;
        float ndcY = -(((float)pt.y / (float)FRAME_BUFFER_HEIGHT) * 2.0f - 1.0f);

        int hit = HitTest(ndcX, ndcY);
        switch (hit)
        {
        case ITEM_TUTORIAL:
        case ITEM_LEVEL1:
        case ITEM_LEVEL2:
        case ITEM_LEVEL3:
            m_nSelectedStage = hit;
            UpdateTransforms();
            break;
        case ITEM_START:
            CLevel_Manager::Get_Instance()->Request_Level_Change(LEVEL_GAMEPLAY);
            break;
        case ITEM_END:
            PostQuitMessage(0);
            break;
        default:
            break;
        }
    }
    return 0;
}

void CLevel_Menu::Render(ID3D12GraphicsCommandList* pd3dCommandList)
{
    if (m_pCamera)
    {
        m_pCamera->SetViewportsAndScissorRects(pd3dCommandList);
        m_pCamera->UpdateShaderVariables(pd3dCommandList);
    }
    for (MenuCube& mc : m_Cubes)
        if (mc.pObj) mc.pObj->Render(pd3dCommandList, m_pCamera);
}

void CLevel_Menu::ReleaseUploadBuffers()
{
    // Free staging buffers via the meshes directly (a cube may currently point
    // at the blue mesh instead of its own).
    for (int i = 0; i < ITEM_COUNT; ++i)
        if (m_ItemMesh[i]) m_ItemMesh[i]->ReleaseUploadBuffers();
    if (m_pBlueMesh) m_pBlueMesh->ReleaseUploadBuffers();
}

void CLevel_Menu::Release()
{
    for (MenuCube& mc : m_Cubes)
        if (mc.pObj) mc.pObj->Release();
    m_Cubes.clear();

    for (int i = 0; i < ITEM_COUNT; ++i)
        if (m_ItemMesh[i]) { m_ItemMesh[i]->Release(); m_ItemMesh[i] = nullptr; }
    if (m_pBlueMesh) { m_pBlueMesh->Release(); m_pBlueMesh = nullptr; }

    if (m_pCamera) { m_pCamera->ReleaseShaderVariables(); delete m_pCamera; m_pCamera = nullptr; }
    if (m_pShader) { m_pShader->Release(); m_pShader = nullptr; }
}
