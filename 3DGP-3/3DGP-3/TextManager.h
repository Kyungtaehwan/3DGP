#pragma once
#include <vector>

struct Glyph;   // 비트맵 글리프(0/1 행렬). 정의는 TextManager.cpp 내부에 있다.

// 한 단어(예: "경태환", "START")를 표현하는 클래스. 자신을 이루는 글리프
// 시퀀스를 들고, 주어진 cell/spacing으로 스스로를 큐브 셀 좌표로 배치한다.
class CWord
{
public:
    void Init(const std::vector<const Glyph*>& glyphs) { m_Glyphs = glyphs; }

    // 단어 전체의 가로 폭(월드 단위).
    float Measure(float cell, float spacing) const;

    // (centerX, centerY) 기준 가로/세로 중앙 정렬로 각 셀의 중심을 outCells에 추가.
    void Layout(float centerX, float centerY, float cell, float spacing,
                std::vector<DirectX::XMFLOAT3>& outCells) const;

private:
    std::vector<const Glyph*> m_Glyphs;
};

// 글자(텍스트) 매니저 — 화면에 쓰이는 단어들을 enum 단위로 보관/제공한다.
// 각 단어는 CWord 객체로 클래스화되어 있고, 글리프 비트맵 데이터까지 이 모듈이
// 직접 관리한다(예전 BitmapFont 를 통합).
class CText_Manager
{
public:
    enum TextId
    {
        TEXT_TITLE = 0,   // 3D게임프로그래밍1
        TEXT_NAME,        // 경태환
        TEXT_START,       // START
        TEXT_TUTORIAL,    // TUTORIAL
        TEXT_LEVEL1,      // LEVEL-1
        TEXT_LEVEL2,      // LEVEL-2
        TEXT_LEVEL3,      // LEVEL-3
        TEXT_END,         // END
        TEXT_COUNT
    };

    static CText_Manager* Get_Instance()
    {
        if (!m_pInstance) m_pInstance = new CText_Manager();
        return m_pInstance;
    }
    static void Destroy_Instance()
    {
        if (m_pInstance) { delete m_pInstance; m_pInstance = nullptr; }
    }

    // enum에 해당하는 단어 객체.
    const CWord* GetWord(int id) const
    {
        return (id >= 0 && id < TEXT_COUNT) ? &m_Words[id] : nullptr;
    }

    // enum에 해당하는 단어를 바로 측정/배치하는 편의 래퍼.
    float MeasureText(int id, float cell, float spacing) const;
    void  LayoutText(int id, float centerX, float centerY, float cell, float spacing,
                     std::vector<DirectX::XMFLOAT3>& outCells) const;

private:
    CText_Manager();
    ~CText_Manager() {}

    static CText_Manager* m_pInstance;
    CWord m_Words[TEXT_COUNT];
};
