#pragma once

class CInput_Manager
{
public:
    static CInput_Manager* Get_Instance()
    {
        if (!m_pInstance) m_pInstance = new CInput_Manager();
        return m_pInstance;
    }
    static void Destroy_Instance()
    {
        if (m_pInstance) { delete m_pInstance; m_pInstance = nullptr; }
    }

    void    Update();
    void    Update_Mouse(HWND hWnd);

    bool    Key_Pressing(int _iKey);
    bool    Key_Down(int _iKey);
    bool    Key_Up(int _iKey);

    int     GetMouseDX() const { return m_iMouseDX; }
    int     GetMouseDY() const { return m_iMouseDY; }

    void SetMouseLock(bool bLock)
    {
        m_bMouseLock = bLock;
        if (bLock)
        {
            while (ShowCursor(FALSE) >= 0);
        }
        else
        {
            while (ShowCursor(TRUE) < 0);
        }
    }
    bool    GetMouseLock() const { return m_bMouseLock; }
    POINT   GetMousePos() const { return m_ptMouse; }


private:
    CInput_Manager();
    ~CInput_Manager();

    static CInput_Manager* m_pInstance;

    bool    m_bKeyState[256];
    POINT   m_ptMouse = {};
    int     m_iMouseDX = 0;
    int     m_iMouseDY = 0;
    bool    m_bMouseLock = false;
};