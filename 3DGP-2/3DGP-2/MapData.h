#pragma once
#include <cstdint>

enum class TileType : uint8_t {
    WALL  = 0,
    FLOOR = 1,
    STAIR = 2,   // step on to switch floors
    DOOR  = 3,
    START = 4,
    END   = 5,
    ROOM  = 6,
};

static constexpr int   MAP_ROWS   = 22;
static constexpr int   MAP_COLS   = 32;
static constexpr float TILE_SCALE = 2.0f;
static constexpr float STAIR_DROP = 2.5f; // world units descended from top to bottom of stair
