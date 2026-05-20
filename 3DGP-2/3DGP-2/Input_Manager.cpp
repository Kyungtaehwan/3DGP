#include "pch.h"
#include "Input_Manager.h"


CInput_Manager* CInput_Manager::m_pInstance = nullptr;

CInput_Manager::CInput_Manager()
{
    ZeroMemory(m_bKeyState, sizeof(m_bKeyState));
    ShowCursor(FALSE);
}

CInput_Manager::~CInput_Manager() {}

void CInput_Manager::Update()
{
    // 키 상태는 GetAsyncKeyState로 실시간 조회하므로 추가 갱신 불필요
}

void CInput_Manager::Update_Mouse(HWND hWnd)
{
    RECT rc;
    GetClientRect(hWnd, &rc);
    POINT ptCenterScreen = { (rc.right - rc.left) / 2, (rc.bottom - rc.top) / 2 };
    ClientToScreen(hWnd, &ptCenterScreen);

    //// 포커스 없으면 스킵 카운터 리셋 후 입력 0
    //if (GetForegroundWindow() != hWnd)
    //{
    //    m_iMouseDX = 0;
    //    m_iMouseDY = 0;
    //    return;
    //}

    //// 포커스 복귀 직후 / 시작 직후: SetCursorPos가 비동기이므로
    //// 커서가 실제로 중앙에 안착할 때까지 몇 프레임 대기
    //if (m_nFocusSkipFrames > 0)
    //{
    //    --m_nFocusSkipFrames;
    //    m_iMouseDX = 0;
    //    m_iMouseDY = 0;
    //    if (m_bMouseLock)
    //        SetCursorPos(ptCenterScreen.x, ptCenterScreen.y);
    //    return;
    //}

    POINT ptCurrent;
    GetCursorPos(&ptCurrent);

    if (m_bMouseLock)
    {
        m_iMouseDX = ptCurrent.x - ptCenterScreen.x;
        m_iMouseDY = ptCurrent.y - ptCenterScreen.y;
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