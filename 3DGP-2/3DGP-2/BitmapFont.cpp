#include "pch.h"
#include "BitmapFont.h"

using namespace DirectX;

// ============================================================
// ASCII glyphs — 7 rows x 5 cols (uppercase + digits + '-' + space).
// Only the characters actually used by the menu are defined.
// ============================================================
namespace
{
    #define G5x7 static const unsigned char

    G5x7 A_bits[] = {
        0,1,1,1,0,
        1,0,0,0,1,
        1,0,0,0,1,
        1,1,1,1,1,
        1,0,0,0,1,
        1,0,0,0,1,
        1,0,0,0,1,
    };
    G5x7 D_bits[] = {
        1,1,1,1,0,
        1,0,0,0,1,
        1,0,0,0,1,
        1,0,0,0,1,
        1,0,0,0,1,
        1,0,0,0,1,
        1,1,1,1,0,
    };
    G5x7 E_bits[] = {
        1,1,1,1,1,
        1,0,0,0,0,
        1,0,0,0,0,
        1,1,1,1,0,
        1,0,0,0,0,
        1,0,0,0,0,
        1,1,1,1,1,
    };
    G5x7 I_bits[] = {
        1,1,1,1,1,
        0,0,1,0,0,
        0,0,1,0,0,
        0,0,1,0,0,
        0,0,1,0,0,
        0,0,1,0,0,
        1,1,1,1,1,
    };
    G5x7 L_bits[] = {
        1,0,0,0,0,
        1,0,0,0,0,
        1,0,0,0,0,
        1,0,0,0,0,
        1,0,0,0,0,
        1,0,0,0,0,
        1,1,1,1,1,
    };
    G5x7 N_bits[] = {
        1,0,0,0,1,
        1,1,0,0,1,
        1,0,1,0,1,
        1,0,1,0,1,
        1,0,0,1,1,
        1,0,0,0,1,
        1,0,0,0,1,
    };
    G5x7 O_bits[] = {
        0,1,1,1,0,
        1,0,0,0,1,
        1,0,0,0,1,
        1,0,0,0,1,
        1,0,0,0,1,
        1,0,0,0,1,
        0,1,1,1,0,
    };
    G5x7 R_bits[] = {
        1,1,1,1,0,
        1,0,0,0,1,
        1,0,0,0,1,
        1,1,1,1,0,
        1,0,1,0,0,
        1,0,0,1,0,
        1,0,0,0,1,
    };
    G5x7 S_bits[] = {
        0,1,1,1,1,
        1,0,0,0,0,
        1,0,0,0,0,
        0,1,1,1,0,
        0,0,0,0,1,
        0,0,0,0,1,
        1,1,1,1,0,
    };
    G5x7 T_bits[] = {
        1,1,1,1,1,
        0,0,1,0,0,
        0,0,1,0,0,
        0,0,1,0,0,
        0,0,1,0,0,
        0,0,1,0,0,
        0,0,1,0,0,
    };
    G5x7 U_bits[] = {
        1,0,0,0,1,
        1,0,0,0,1,
        1,0,0,0,1,
        1,0,0,0,1,
        1,0,0,0,1,
        1,0,0,0,1,
        0,1,1,1,0,
    };
    G5x7 V_bits[] = {
        1,0,0,0,1,
        1,0,0,0,1,
        1,0,0,0,1,
        1,0,0,0,1,
        1,0,0,0,1,
        0,1,0,1,0,
        0,0,1,0,0,
    };
    G5x7 D1_bits[] = {
        0,0,1,0,0,
        0,1,1,0,0,
        0,0,1,0,0,
        0,0,1,0,0,
        0,0,1,0,0,
        0,0,1,0,0,
        0,1,1,1,0,
    };
    G5x7 D2_bits[] = {
        0,1,1,1,0,
        1,0,0,0,1,
        0,0,0,0,1,
        0,0,0,1,0,
        0,0,1,0,0,
        0,1,0,0,0,
        1,1,1,1,1,
    };
    G5x7 D3_bits[] = {
        1,1,1,1,1,
        0,0,0,1,0,
        0,0,1,0,0,
        0,0,0,1,0,
        0,0,0,0,1,
        1,0,0,0,1,
        0,1,1,1,0,
    };
    G5x7 HYPHEN_bits[] = {
        0,0,0,0,0,
        0,0,0,0,0,
        0,0,0,0,0,
        0,1,1,1,0,
        0,0,0,0,0,
        0,0,0,0,0,
        0,0,0,0,0,
    };
    G5x7 SPACE_bits[] = {
        0,0,0,0,0,
        0,0,0,0,0,
        0,0,0,0,0,
        0,0,0,0,0,
        0,0,0,0,0,
        0,0,0,0,0,
        0,0,0,0,0,
    };

    const Glyph kSpace = { 7, 5, SPACE_bits };

    // ============================================================
    // Korean glyphs — 16 rows x 16 cols (hand-authored, approximate).
    // ============================================================
    G5x7 GE_bits[] = {  // GE  (consonant g + vowel e)
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,0,
        0,0,1,1,1,1,1,1,1,0,0,1,0,0,1,0,
        0,0,0,0,0,0,0,0,1,0,0,1,0,0,1,0,
        0,0,0,0,0,0,0,0,1,0,0,1,0,0,1,0,
        0,0,0,0,0,0,0,0,1,0,0,1,0,0,1,0,
        0,0,0,0,0,0,0,0,1,0,0,1,0,0,1,0,
        0,0,0,0,0,0,0,0,1,0,0,1,1,1,1,0,
        0,0,0,0,0,0,0,0,1,0,0,1,0,0,1,0,
        0,0,0,0,0,0,0,0,1,0,0,1,0,0,1,0,
        0,0,0,0,0,0,0,0,1,0,0,1,0,0,1,0,
        0,0,0,0,0,0,0,0,1,0,0,1,0,0,1,0,
        0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    };
    G5x7 IM_bits[] = {  // IM  (consonant ng + vowel i + final m)
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,1,1,1,1,0,0,0,0,0,0,1,0,0,0,
        0,0,1,0,0,1,0,0,0,0,0,0,1,0,0,0,
        0,0,1,0,0,1,0,0,0,0,0,0,1,0,0,0,
        0,0,1,0,0,1,0,0,0,0,0,0,1,0,0,0,
        0,0,1,0,0,1,0,0,0,0,0,0,1,0,0,0,
        0,0,1,1,1,1,0,0,0,0,0,0,1,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,
        0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
        0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,
        0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,
        0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,
        0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,
        0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    };

    const Glyph kKorean[KOR_COUNT] = {
        { 16, 16, GE_bits },
        { 16, 16, IM_bits },
    };

    #undef G5x7
}

const Glyph* AsciiGlyph(char c)
{
    if (c >= 'a' && c <= 'z') c = (char)(c - 'a' + 'A');
    switch (c)
    {
    case 'A': { static const Glyph g = { 7, 5, A_bits }; return &g; }
    case 'D': { static const Glyph g = { 7, 5, D_bits }; return &g; }
    case 'E': { static const Glyph g = { 7, 5, E_bits }; return &g; }
    case 'I': { static const Glyph g = { 7, 5, I_bits }; return &g; }
    case 'L': { static const Glyph g = { 7, 5, L_bits }; return &g; }
    case 'N': { static const Glyph g = { 7, 5, N_bits }; return &g; }
    case 'O': { static const Glyph g = { 7, 5, O_bits }; return &g; }
    case 'R': { static const Glyph g = { 7, 5, R_bits }; return &g; }
    case 'S': { static const Glyph g = { 7, 5, S_bits }; return &g; }
    case 'T': { static const Glyph g = { 7, 5, T_bits }; return &g; }
    case 'U': { static const Glyph g = { 7, 5, U_bits }; return &g; }
    case 'V': { static const Glyph g = { 7, 5, V_bits }; return &g; }
    case '1': { static const Glyph g = { 7, 5, D1_bits }; return &g; }
    case '2': { static const Glyph g = { 7, 5, D2_bits }; return &g; }
    case '3': { static const Glyph g = { 7, 5, D3_bits }; return &g; }
    case '-': { static const Glyph g = { 7, 5, HYPHEN_bits }; return &g; }
    default:  return &kSpace;
    }
}

const Glyph* KoreanGlyph(int id)
{
    if (id < 0 || id >= KOR_COUNT) return &kSpace;
    return &kKorean[id];
}

float LayoutGlyph(const Glyph& g, float penXLeft, float centerY,
                  float cell, float spacing,
                  std::vector<XMFLOAT3>& outCells)
{
    // Top edge so the glyph block is vertically centered on centerY.
    float top = centerY + (g.rows * cell) * 0.5f;
    for (int row = 0; row < g.rows; ++row)
    {
        for (int col = 0; col < g.cols; ++col)
        {
            if (g.bits[row * g.cols + col])
            {
                float x = penXLeft + (col + 0.5f) * cell;
                float y = top - (row + 0.5f) * cell;
                outCells.push_back(XMFLOAT3(x, y, 0.0f));
            }
        }
    }
    return g.cols * cell + spacing;
}

float MeasureAscii(const char* s, float cell, float spacing)
{
    float width = 0.0f;
    for (const char* p = s; *p; ++p)
        width += AsciiGlyph(*p)->cols * cell + spacing;
    if (width > 0.0f) width -= spacing;   // no trailing spacing
    return width;
}
