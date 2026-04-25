#pragma once
#include "define.h"

class CButton
{
public:
    CButton() {}
    ~CButton() {}

    void    Setup(int x, int y, int nWidth, int nHeight,
        LPCTSTR szText, DWORD dwColor = RGB(60, 60, 60));
    bool    Update();   // 클릭 시 true 반환
    void    Render(HDC hDC);

private:
    RECT    m_rc = {};
    TCHAR   m_szText[64] = {};
    DWORD   m_dwColor = RGB(60, 60, 60);
    DWORD   m_dwHoverColor = RGB(100, 100, 100);
    bool    m_bHover = false;
};