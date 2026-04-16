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

    void Update();

    bool Key_Pressing(int _iKey);
    bool Key_Down(int _iKey);
    bool Key_Up(int _iKey);

private:
    CInput_Manager();
    ~CInput_Manager();

    static CInput_Manager* m_pInstance;
    bool m_bKeyState[256] = {};
};