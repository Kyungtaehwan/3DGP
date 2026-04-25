#include "stdafx.h"
#include "Level_Menu.h"
#include "Bmp_Manager.h"
#include "Input_Manager.h"
#include "Level_Manager.h"

CLevel_Menu::CLevel_Menu()
{
}

CLevel_Menu::~CLevel_Menu()
{
    Release();
}

void CLevel_Menu::Initialize()
{
    CInput_Manager::Get_Instance()->SetMouseLock(false);
    CBmp_Manager::Get_Instance()->Insert_Bmp(L"../Resource/BackGround.bmp", L"Back");
    int nBtnW = 200, nBtnH = 60;
    int nBtnX = (WINCX - nBtnW) / 2;
    int nBtnY = WINCY / 2;
    m_btnStart.Setup(nBtnX, nBtnY, nBtnW, nBtnH,
        _T("START GAME"), RGB(30, 80, 30));

}

int CLevel_Menu::Update(float dt)
{
    if (m_btnStart.Update())
        CLevel_Manager::Get_Instance()->Level_Change(LEVEL_GAMEPLAY);
    return 0;
}

void CLevel_Menu::Late_Update(float dt)
{
}

void CLevel_Menu::Render(HDC hDC)
{
    HDC	hMemDC = CBmp_Manager::Get_Instance()->Find_Img(L"Back");
    BitBlt(hDC, 0, 0, WINCX, WINCY, hMemDC, 0, 0, SRCCOPY);
   

    // ≈∏¿Ã∆≤
    SetBkMode(hDC, TRANSPARENT);
    SetTextColor(hDC, RGB(255, 215, 0));
    HFONT hFont = CreateFont(70, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH, _T("Arial"));
    HFONT hOldFont = (HFONT)SelectObject(hDC, hFont);

    RECT rcTitle = { 0, WINCY / 4, WINCX, WINCY / 2 };
    DrawText(hDC, _T("TANK BOSS"), -1, &rcTitle,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    SelectObject(hDC, hOldFont);
    DeleteObject(hFont);

    m_btnStart.Render(hDC);
}

void CLevel_Menu::Release()
{
}
