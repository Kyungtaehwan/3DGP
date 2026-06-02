#pragma once
#include <vector>

// ============================================================
// BitmapFont — engine-agnostic bitmap glyph data + layout.
//
// A glyph is a rows x cols grid of 0/1 bits (row 0 = top, 1 = filled cell).
// Callers turn each filled cell into something concrete (e.g. a cube). This
// keeps all DirectX code out of the font module so it can be shared by the
// LOGO (Korean, rotating 3D) and the menu (ASCII uppercase 3D) screens.
// ============================================================

struct Glyph
{
    int                  rows;
    int                  cols;
    const unsigned char* bits;   // rows*cols, row-major, 1 = filled
};

// Korean glyph ids (hand-authored 16x16). Extend as more syllables are needed.
enum KoreanGlyphId
{
    KOR_GE = 0,   // GE
    KOR_IM,       // IM
    KOR_COUNT
};

// ASCII glyph for 'A'..'Z', '0'..'9', '-' and ' ' (5 wide x 7 tall).
// Lowercase is promoted to uppercase. Unknown chars return the space glyph.
const Glyph* AsciiGlyph(char c);

// Hand-authored Korean glyph (16x16) by id.
const Glyph* KoreanGlyph(int id);

// Append the local offsets of a glyph's filled cells. The glyph block is laid
// out with its LEFT edge at penXLeft and vertically centered on centerY; each
// returned offset is the CENTER of one cell. Returns the horizontal advance
// (cols*cell + spacing) to move the pen to the next glyph.
float LayoutGlyph(const Glyph& g, float penXLeft, float centerY,
                  float cell, float spacing,
                  std::vector<DirectX::XMFLOAT3>& outCells);

// Total width (world units) of an ASCII string laid out with LayoutGlyph,
// used to center text horizontally (startX = -MeasureAscii(...) * 0.5f).
float MeasureAscii(const char* s, float cell, float spacing);
