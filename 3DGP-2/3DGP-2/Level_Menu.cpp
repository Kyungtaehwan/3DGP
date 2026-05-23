#include "pch.h"
#include "Level_Menu.h"
#include "Level_Manager.h"
#include "Input_Manager.h"
#include "UI_Manager.h"

void CLevel_Menu::Initialize(ID3D12Device* pd3dDevice,
                              ID3D12GraphicsCommandList* pd3dCommandList,
                              ID3D12RootSignature* pd3dRootSignature)
{
    // Menu has no scene objects — the background is the cleared render target.
    // UI_Manager is already initialized by MainApp.
    m_fTimeAcc = 0.0f;

    // Show OS cursor in the menu; gameplay re-locks it on entry.
    CInput_Manager::Get_Instance()->SetMouseLock(false);
}

int CLevel_Menu::Update(float dt)
{
    m_fTimeAcc += dt;

    CInput_Manager* pInput = CInput_Manager::Get_Instance();

    // '1' / '2' — pick stage and enter gameplay.
    if (pInput->Key_Down('1'))
        CLevel_Manager::Get_Instance()->Request_Level_Change(LEVEL_GAMEPLAY, 1);
    else if (pInput->Key_Down('2'))
        CLevel_Manager::Get_Instance()->Request_Level_Change(LEVEL_GAMEPLAY, 2);

    return 0;
}

void CLevel_Menu::Render(ID3D12GraphicsCommandList* pd3dCommandList)
{
    // Draw the menu UI (two stage panels). UI is in NDC so no camera needed.
    CUI_Manager::Get_Instance()->RenderMenu(pd3dCommandList);
}
