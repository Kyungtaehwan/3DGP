#include "pch.h"
#include "Block.h"
#include "MapData.h"

void CDoorBlock::Init(XMFLOAT3 closedPos, bool bNSPassage)
{
    m_ClosedPos   = closedPos;
    m_bNSPassage  = bNSPassage;
    SetPosition(closedPos.x, closedPos.y, closedPos.z);
}

void CDoorBlock::SetOpen(bool bOpen)
{
    m_bTargetOpen = bOpen;
}

int CDoorBlock::Update(float dt)
{
    float target = m_bTargetOpen ? 1.0f : 0.0f;

    if (m_fAnimT < target)
        m_fAnimT = std::min(m_fAnimT + ANIM_SPEED * dt, target);
    else if (m_fAnimT > target)
        m_fAnimT = std::max(m_fAnimT - ANIM_SPEED * dt, target);

    // Slide one edge of the door to the wall boundary while shrinking width to 0.
    // The far edge stays fixed; the near edge moves toward it.
    // The door never enters wall geometry, so depth-test clipping is not an issue.
    float scale = 1.0f - m_fAnimT;
    float shift = (TILE_SCALE * 0.5f) * m_fAnimT;

    if (m_bNSPassage)
    {
        SetPosition(m_ClosedPos.x + shift, m_ClosedPos.y, m_ClosedPos.z);
        m_xmf4x4World._11 = scale;
    }
    else
    {
        SetPosition(m_ClosedPos.x, m_ClosedPos.y, m_ClosedPos.z + shift);
        m_xmf4x4World._33 = scale;
    }

    m_bActive = (m_fAnimT < 1.0f);

    return OBJ_NOEVENT;
}
