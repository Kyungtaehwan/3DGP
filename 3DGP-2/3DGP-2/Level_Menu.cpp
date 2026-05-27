#include "pch.h"
#include "Level_Menu.h"
#include "Level_Manager.h"
#include "Input_Manager.h"
#include "UI_Manager.h"

void CLevel_Menu::Initialize(ID3D12Device* pd3dDevice,
                              ID3D12GraphicsCommandList* pd3dCommandList,
                              ID3D12RootSignature* pd3dRootSignature)
{

    m_fTimeAcc = 0.0f;
    CInput_Manager::Get_Instance()->SetMouseLock(false);
}

int CLevel_Menu::Update(float dt)
{
    m_fTimeAcc += dt;

    CInput_Manager* pInput = CInput_Manager::Get_Instance();


    if (pInput->Key_Down(VK_LBUTTON))
    {
        POINT pt;
        GetCursorPos(&pt);
        ScreenToClient(g_hWnd, &pt);

        float ndcX = ((float)pt.x / (float)FRAME_BUFFER_WIDTH)  * 2.0f - 1.0f;
        float ndcY = -(((float)pt.y / (float)FRAME_BUFFER_HEIGHT) * 2.0f - 1.0f);

        if (ndcX >= STAGE1_NDC_X_MIN && ndcX <= STAGE1_NDC_X_MAX &&
            ndcY >= STAGE1_NDC_Y_MIN && ndcY <= STAGE1_NDC_Y_MAX)
        {
            CLevel_Manager::Get_Instance()->Request_Level_Change(LEVEL_GAMEPLAY, 1);
        }
        else if (ndcX >= STAGE2_NDC_X_MIN && ndcX <= STAGE2_NDC_X_MAX &&
                 ndcY >= STAGE2_NDC_Y_MIN && ndcY <= STAGE2_NDC_Y_MAX)
        {
            CLevel_Manager::Get_Instance()->Request_Level_Change(LEVEL_GAMEPLAY, 2);
        }
    }

    return 0;
}

void CLevel_Menu::Render(ID3D12GraphicsCommandList* pd3dCommandList)
{
    CUI_Manager::Get_Instance()->RenderMenu(pd3dCommandList);
}
