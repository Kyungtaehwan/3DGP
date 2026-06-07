#pragma once
#include <vector>

struct Glyph;

class CWord
{
public:
    void Initialize(const std::vector<const Glyph*>& glyphs) { m_Glyphs = glyphs; }

    // 가로 폭
    float Measure(float cell, float spacing) const;

    // centerX, centerY
    void Layout(float centerX, float centerY, float cell, float spacing,
                std::vector<DirectX::XMFLOAT3>& outCells) const;

private:
    std::vector<const Glyph*> m_Glyphs;
};


class CText_Manager
{
public:
    enum TextId
    {
        TEXT_TITLE = 0, // 3D게임프로그래밍1
        TEXT_NAME, // 경태환
        TEXT_START, // START
        TEXT_TUTORIAL, // TUTORIAL
        TEXT_LEVEL1, // LEVEL-1
        TEXT_LEVEL2, // LEVEL-2
        TEXT_LEVEL3, // LEVEL-3
        TEXT_END, // END
        TEXT_WIN, // WIN
        TEXT_LOSE, // LOSE
        TEXT_COUNT
    };

    static CText_Manager* Get_Instance()
    {
        if(!m_pInstance)
            m_pInstance = new CText_Manager();
        return m_pInstance;
    }
    static void Destroy_Instance()
    {
        if(m_pInstance)
        {
            delete m_pInstance;
            m_pInstance = nullptr;
        }
    }

    const CWord* GetWord(int id) const
    {
        if(id >= 0 && id < TEXT_COUNT)
            return &m_Words[id];
        else
            return nullptr;
    }

    float MeasureText(int id, float cell, float spacing) const;
    void LayoutText(int id, float centerX, float centerY, float cell, float spacing,
                    std::vector<DirectX::XMFLOAT3>& outCells) const;

private:
    CText_Manager();
    ~CText_Manager() {}

    static CText_Manager* m_pInstance;
    CWord m_Words[TEXT_COUNT];
};
