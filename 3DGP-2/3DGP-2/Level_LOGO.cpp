#include "pch.h"
#include "Level_LOGO.h"
#include "Level_Manager.h"
#include "Camera.h"
#include "Mesh.h"
#include "Input_Manager.h"

namespace
{
    // 3x5 grid: 1 = solid cube, 0 = empty. Row 0 = top of the letter.
    const int LETTER_3[5][3] = {
        {1,1,1},
        {0,0,1},
        {0,1,1},
        {0,0,1},
        {1,1,1},
    };
    const int LETTER_D[5][3] = {
        {1,1,0},
        {1,0,1},
        {1,0,1},
        {1,0,1},
        {1,1,0},
    };
    const int LETTER_G[5][3] = {
        {1,1,1},
        {1,0,0},
        {1,1,0},
        {1,0,1},
        {1,1,1},
    };
    const int LETTER_P[5][3] = {
        {1,1,1},
        {1,0,1},
        {1,1,1},
        {1,0,0},
        {1,0,0},
    };

    constexpr float CELL = 0.18f;       // cube edge length
    constexpr float LETTER_PITCH = 0.85f; // horizontal distance between letter centers
}

void CLevel_LOGO::Initialize(ID3D12Device* pd3dDevice,
                              ID3D12GraphicsCommandList* pd3dCommandList,
                              ID3D12RootSignature* pd3dRootSignature)
{
    m_pd3dDevice = pd3dDevice;

    // Show the OS cursor so users can aim at the play button.
    CInput_Manager::Get_Instance()->SetMouseLock(false);

    // Shader (same one used elsewhere; position+color).
    m_pShader = new CObjectShader();
    m_pShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dRootSignature);

    // Fixed camera at z=-4 looking +Z. Default Right/Up/Look from CCamera ctor is fine.
    m_pCamera = new CCamera();
    m_pCamera->CreateShaderVariables(pd3dDevice, pd3dCommandList);
    m_pCamera->SetPosition(XMFLOAT3(0.0f, 0.0f, -4.0f));
    m_pCamera->GenerateViewMatrix();
    m_pCamera->GenerateProjectionMatrix(0.1f, 100.0f, ASPECT_RATIO, 60.0f);
    m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
    m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);

    // Build "3DGP" — 4 letters, centered horizontally on x = 0.
    const XMFLOAT4 cyan   = { 0.4f, 1.0f, 1.0f, 1.0f };
    const XMFLOAT4 yellow = { 1.0f, 0.9f, 0.3f, 1.0f };
    const XMFLOAT4 pink   = { 1.0f, 0.5f, 0.7f, 1.0f };
    const XMFLOAT4 green  = { 0.4f, 1.0f, 0.5f, 1.0f };

    float startX = -1.5f * LETTER_PITCH;   // 4 letters: -1.5, -0.5, +0.5, +1.5
    BuildLetter(pd3dDevice, pd3dCommandList, LETTER_3, startX + 0 * LETTER_PITCH, cyan);
    BuildLetter(pd3dDevice, pd3dCommandList, LETTER_D, startX + 1 * LETTER_PITCH, yellow);
    BuildLetter(pd3dDevice, pd3dCommandList, LETTER_G, startX + 2 * LETTER_PITCH, pink);
    BuildLetter(pd3dDevice, pd3dCommandList, LETTER_P, startX + 3 * LETTER_PITCH, green);

    // Play button — dark grey box below the logo.
    {
        const XMFLOAT4 darkGrey = { 0.20f, 0.20f, 0.22f, 1.0f };
        m_pPlayButton = new CBlock();
        m_pPlayButton->SetShader(m_pShader);
        m_pPlayButton->SetMesh(new CCubeMesh(pd3dDevice, pd3dCommandList,
                                             0.9f, 0.5f, 0.2f, darkGrey));
        m_pPlayButton->CreateShaderVariables(pd3dDevice, pd3dCommandList);
        m_pPlayButton->SetPosition(0.0f, -1.3f, 0.0f);
    }

    // Play triangle — white, sitting slightly in front of the box face (z = -0.105).
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

void CLevel_LOGO::BuildLetter(ID3D12Device* pd3dDevice,
                              ID3D12GraphicsCommandList* pd3dCommandList,
                              const int pattern[5][3],
                              float letterCenterX, XMFLOAT4 color)
{
    for (int row = 0; row < 5; ++row)
    {
        for (int col = 0; col < 3; ++col)
        {
            if (!pattern[row][col]) continue;

            // col 0..2 -> -1..+1 cells. row 0 = top -> +2 cells, row 4 = bottom -> -2 cells.
            float localX = letterCenterX + (col - 1) * CELL;
            float localY = (2 - row) * CELL;

            CBlock* pCube = new CBlock();
            pCube->SetShader(m_pShader);
            pCube->SetMesh(new CCubeMesh(pd3dDevice, pd3dCommandList,
                                         CELL, CELL, CELL, color));
            pCube->CreateShaderVariables(pd3dDevice, pd3dCommandList);

            LogoCube lc;
            lc.pObj        = pCube;
            lc.localOffset = XMFLOAT3(localX, localY, 0.0f);
            m_LetterCubes.push_back(lc);
        }
    }
}

int CLevel_LOGO::Update(float dt)
{
    // Spin the whole logo group around Y.
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

    // Mouse click on the play button -> go to the menu.
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
