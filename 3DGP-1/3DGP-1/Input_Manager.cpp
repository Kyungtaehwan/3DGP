#include "stdafx.h"
#include "Input_Manager.h"


CInput_Manager* CInput_Manager::m_pInstance = nullptr;

CInput_Manager::CInput_Manager()
{
    ZeroMemory(m_bKeyState, sizeof(m_bKeyState));
    ShowCursor(FALSE);
}

CInput_Manager::~CInput_Manager() {}


void CInput_Manager::Update_Mouse(HWND hWnd)
{

    GetCursorPos(&m_ptMouse);
    ScreenToClient(g_hWnd, &m_ptMouse);
    // 클라이언트 영역 중앙 계산
    RECT rc;
    GetClientRect(hWnd, &rc);
    POINT ptCenter = { (rc.right - rc.left) / 2, (rc.bottom - rc.top) / 2 };

    // 클라이언트 → 스크린 좌표 변환
    POINT ptCenterScreen = ptCenter;
    ClientToScreen(hWnd, &ptCenterScreen);

    // 현재 마우스 스크린 좌표
    POINT ptCurrent;
    GetCursorPos(&ptCurrent);

    // 델타 = 현재 - 중앙
    m_iMouseDX = ptCurrent.x - ptCenterScreen.x;
    m_iMouseDY = ptCurrent.y - ptCenterScreen.y;

    // 마우스 중앙 고정
    if (m_bMouseLock)
    {
        SetCursorPos(ptCenterScreen.x, ptCenterScreen.y);
    }
    else
    {
        m_iMouseDX = 0;
        m_iMouseDY = 0;
    }
}

bool CInput_Manager::Key_Pressing(int _iKey)
{
    return (GetAsyncKeyState(_iKey) & 0x8000) != 0;
}

bool CInput_Manager::Key_Down(int _iKey)
{
    if (GetAsyncKeyState(_iKey) & 0x8000)
    {
        if (!m_bKeyState[_iKey])
        {
            m_bKeyState[_iKey] = true;
            return true;
        }
    }
    else
    {
        m_bKeyState[_iKey] = false;
    }
    return false;
}

bool CInput_Manager::Key_Up(int _iKey)
{
    if (!(GetAsyncKeyState(_iKey) & 0x8000))
    {
        if (m_bKeyState[_iKey])
        {
            m_bKeyState[_iKey] = false;
            return true;
        }
    }
    else
    {
        m_bKeyState[_iKey] = true;
    }
    return false;
}