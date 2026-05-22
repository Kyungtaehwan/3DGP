#pragma once
#include "GameObject.h"

class CBlock : public CGameObject
{
public:
    CBlock()          = default;
    virtual ~CBlock() = default;
};

class CDoorBlock : public CBlock
{
public:
    // bNSPassage=true : N/S neighbors passable -> door faces E-W -> slides along X
    // bNSPassage=false: E/W neighbors passable -> door faces N-S -> slides along Z
    void Init(XMFLOAT3 closedPos, bool bNSPassage);
    void SetOpen(bool bOpen);
    bool IsTargetOpen() const { return m_bTargetOpen; }

    virtual int Update(float dt) override;

private:
    XMFLOAT3 m_ClosedPos   = { 0.f, 0.f, 0.f };
    float    m_fAnimT      = 0.0f;
    bool     m_bTargetOpen = false;
    bool     m_bNSPassage  = false;

    static constexpr float ANIM_SPEED = 3.0f; // full open in ~0.33s
};
