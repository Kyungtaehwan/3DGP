#include "pch.h"
#include "Level_LOGO.h"
#include "Level_Manager.h"
#include "Camera.h"
#include "Mesh.h"
#include "Input_Manager.h"

namespace
{
    constexpr float CELL = 0.07f;        // cube size for the 16x16 Korean glyphs
    constexpr float SYLLABLE_PITCH = 1.30f;
}

void CLevel_LOGO::Initialize(ID3D12Device* pd3dDevice,
                              ID3D12GraphicsCommandList* pd3dCommandList,
                              ID3D12RootSignature* pd3dRootSignature)
{
    m_pd3dDevice = pd3dDevice;


    CInput_Manager::Get_Instance()->SetMouseLock(false);

    m_pShader = new CObjectShader();
    m_pShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dRootSignature);

    m_pCamera = new CCamera();
    m_pCamera->CreateShaderVariables(pd3dDevice, pd3dCommandList);
    m_pCamera->SetPosition(XMFLOAT3(0.0f, 0.0f, -4.0f));
    m_pCamera->GenerateViewMatrix();
    m_pCamera->GenerateProjectionMatrix(0.1f, 100.0f, ASPECT_RATIO, 60.0f);
    m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
    m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);

    const XMFLOAT4 cyan = { 0.4f, 1.0f, 1.0f, 1.0f };
    const XMFLOAT4 pink = { 1.0f, 0.5f, 0.7f, 1.0f };

    // Game name (Korean) placeholder "GE-IM" (2 syllables). Centered, two
    // syllables side by side. Replace with the real name later.
    float startX = -0.5f * SYLLABLE_PITCH;
    BuildGlyphCubes(pd3dDevice, pd3dCommandList, *KoreanGlyph(KOR_GE),
                    startX + 0 * SYLLABLE_PITCH, 0.0f, CELL, cyan);
    BuildGlyphCubes(pd3dDevice, pd3dCommandList, *KoreanGlyph(KOR_IM),
                    startX + 1 * SYLLABLE_PITCH, 0.0f, CELL, pink);

    {
        const XMFLOAT4 darkGrey = { 0.20f, 0.20f, 0.22f, 1.0f };
        m_pPlayButton = new CBlock();
        m_pPlayButton->SetShader(m_pShader);
        m_pPlayButton->SetMesh(new CCubeMesh(pd3dDevice, pd3dCommandList,
                                             0.9f, 0.5f, 0.2f, darkGrey));
        m_pPlayButton->CreateShaderVariables(pd3dDevice, pd3dCommandList);
        m_pPlayButton->SetPosition(0.0f, -1.3f, 0.0f);
    }

    {
        const XMFLOAT4 white = { 1.0f, 1.0f, 1.0f, 1.0f };
        m_pPlayTriangle = new CBlock();
        m_pPlayTriangle->SetShader(m_pShader);
        m_pPlayTriangle->SetMesh(new CTriangleMesh(pd3dDevice, pd3dCommandList,
                                                   0.28f, 0.28f, white));
        m_pPlayTriangle->CreateShaderVariables(pd3dDevice, pd3dCommandList);
        m_pPlayTriangle->SetPosition(0.0f, -1.3f, -0.105f);
    }
}

void CLevel_LOGO::BuildGlyphCubes(ID3D12Device* pd3dDevice,
                                  ID3D12GraphicsCommandList* pd3dCommandList,
                                  const Glyph& glyph, float centerX, float centerY,
                                  float cell, XMFLOAT4 color)
{
    std::vector<XMFLOAT3> cells;
    float left = centerX - (glyph.cols * cell) * 0.5f;
    LayoutGlyph(glyph, left, centerY, cell, 0.0f, cells);

    for (const XMFLOAT3& p : cells)
    {
        CBlock* pCube = new CBlock();
        pCube->SetShader(m_pShader);
        pCube->SetMesh(new CCubeMesh(pd3dDevice, pd3dCommandList,
                                     cell, cell, cell, color));
        pCube->CreateShaderVariables(pd3dDevice, pd3dCommandList);

        LogoCube lc;
        lc.pObj        = pCube;
        lc.localOffset = p;
        m_LetterCubes.push_back(lc);
    }
}

int CLevel_LOGO::Update(float dt)
{

    m_fRotation += dt * 0.8f;
    XMMATRIX groupRot = XMMatrixRotationY(m_fRotation);

    for (auto& lc : m_LetterCubes)
    {
        XMMATRIX local = XMMatrixTranslation(lc.localOffset.x,
                                             lc.localOffset.y,
                                             lc.localOffset.z);
        XMMATRIX world = local * groupRot;
        XMStoreFloat4x4(&lc.pObj->m_xmf4x4World, world);
    }


    CInput_Manager* pInput = CInput_Manager::Get_Instance();
    if (pInput->Key_Down(VK_LBUTTON))
    {
        POINT pt;
        GetCursorPos(&pt);
        ScreenToClient(g_hWnd, &pt);

        float ndcX = ((float)pt.x / (float)FRAME_BUFFER_WIDTH)  * 2.0f - 1.0f;
        float ndcY = -(((float)pt.y / (float)FRAME_BUFFER_HEIGHT) * 2.0f - 1.0f);

        if (ndcX >= BTN_NDC_X_MIN && ndcX <= BTN_NDC_X_MAX &&
            ndcY >= BTN_NDC_Y_MIN && ndcY <= BTN_NDC_Y_MAX)
        {
            CLevel_Manager::Get_Instance()->Request_Level_Change(LEVEL_MENU);
        }
    }

    return 0;
}

void CLevel_LOGO::Render(ID3D12GraphicsCommandList* pd3dCommandList)
{
    if (m_pCamera)
    {
        m_pCamera->SetViewportsAndScissorRects(pd3dCommandList);
        m_pCamera->UpdateShaderVariables(pd3dCommandList);
    }

    for (auto& lc : m_LetterCubes)
        if (lc.pObj) lc.pObj->Render(pd3dCommandList, m_pCamera);

    if (m_pPlayButton)   m_pPlayButton->Render(pd3dCommandList, m_pCamera);
    if (m_pPlayTriangle) m_pPlayTriangle->Render(pd3dCommandList, m_pCamera);
}

void CLevel_LOGO::Release()
{
    for (auto& lc : m_LetterCubes)
        if (lc.pObj) { delete lc.pObj; lc.pObj = nullptr; }
    m_LetterCubes.clear();

    if (m_pPlayTriangle) { delete m_pPlayTriangle; m_pPlayTriangle = nullptr; }
    if (m_pPlayButton)   { delete m_pPlayButton;   m_pPlayButton   = nullptr; }

    if (m_pCamera) { m_pCamera->ReleaseShaderVariables(); delete m_pCamera; m_pCamera = nullptr; }
    if (m_pShader) { delete m_pShader; m_pShader = nullptr; }
}
