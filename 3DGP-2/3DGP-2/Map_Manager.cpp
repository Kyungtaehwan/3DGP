#include "pch.h"
#include "Map_Manager.h"
#include "Mesh.h"
#include "Camera.h"
#include <cmath>
#include <cstring>

CMap_Manager* CMap_Manager::s_pInstance = nullptr;

CMap_Manager* CMap_Manager::Get_Instance()
{
    if (!s_pInstance)
        s_pInstance = new CMap_Manager();
    return s_pInstance;
}

void CMap_Manager::Destroy_Instance()
{
    if (s_pInstance)
    {
        delete s_pInstance;
        s_pInstance = nullptr;
    }
}

// ================================================================
//  MAP 1  "The Labyrinth"
//  F2(upper): START=(1,1)    STAIR=(16,17)
//  F1(lower): STAIR=(16,17)  END=(20,30)
//
//  Rooms [F2]:
//    Room A  rows 3-9,  cols 11-20  doors: N(2,15) S(10,15) W(6,10)
//    Room B  rows 13-18, cols 3-8   doors: N(12,5) W(15,2)
//    StairRoom rows 13-18, cols 14-20  doors: N(12,17) S(19,17) E(15,21)
//
//  Rooms [F1]:
//    Room D  rows 3-8,  cols 3-13   doors: N(2,8) S(9,8) W(5,2)
//    Room E  rows 12-18, cols 19-28  doors: N(11,23) S(19,24) E(15,29)
// ================================================================

static const uint8_t s_Map1_F2[MAP_ROWS][MAP_COLS] = {
//   0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 4, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    {0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
    {0, 1, 0, 6, 6, 6, 6, 6, 0, 1, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 6, 6, 6, 6, 0, 6, 6, 0, 1, 0},
    {0, 1, 3, 6, 6, 6, 6, 6, 0, 1, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 6, 6, 6, 6, 0, 6, 6, 0, 1, 0},
    {0, 1, 0, 6, 6, 6, 6, 6, 0, 1, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 6, 6, 6, 6, 0, 6, 6, 0, 1, 0},
    {0, 1, 0, 6, 6, 6, 6, 6, 0, 1, 3, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 6, 6, 6, 6, 0, 6, 6, 0, 1, 0},
    {0, 1, 0, 6, 6, 6, 6, 6, 0, 1, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 6, 6, 6, 6, 0, 6, 6, 0, 1, 0},
    {0, 1, 0, 6, 6, 6, 6, 6, 0, 1, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 6, 6, 6, 6, 0, 6, 6, 0, 1, 0},
    {0, 1, 0, 6, 6, 6, 6, 6, 0, 1, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 6, 6, 6, 6, 0, 6, 6, 0, 1, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    {0, 1, 0, 0, 0, 3, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 3, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0},
    {0, 1, 0, 6, 6, 6, 6, 6, 6, 0, 1, 0, 1, 0, 6, 6, 6, 2, 6, 6, 6, 0, 1, 0, 6, 6, 6, 6, 6, 0, 1, 0},
    {0, 1, 0, 6, 6, 6, 6, 6, 6, 0, 1, 0, 1, 0, 6, 6, 6, 2, 6, 6, 6, 0, 1, 0, 6, 6, 6, 6, 6, 0, 1, 0},
    {0, 1, 3, 6, 6, 6, 6, 6, 6, 0, 1, 0, 1, 0, 6, 6, 6, 2, 6, 6, 6, 3, 1, 0, 6, 6, 6, 6, 6, 0, 1, 0},
    {0, 1, 0, 6, 6, 6, 6, 6, 6, 0, 1, 1, 1, 0, 6, 6, 6, 2, 6, 6, 6, 0, 1, 0, 6, 6, 6, 6, 6, 0, 1, 0},
    {0, 1, 0, 6, 6, 6, 6, 6, 6, 0, 1, 0, 1, 0, 6, 6, 6, 2, 6, 6, 6, 0, 1, 0, 6, 6, 6, 6, 6, 0, 1, 0},
    {0, 1, 0, 6, 6, 6, 6, 6, 6, 0, 1, 0, 1, 0, 6, 6, 6, 2, 6, 6, 6, 0, 1, 0, 6, 6, 6, 6, 6, 0, 1, 0},
    {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 3, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0},
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};

static const uint8_t s_Map1_F1[MAP_ROWS][MAP_COLS] = {
//   0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 4, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    {0, 1, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
    {0, 1, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 1, 1, 0, 6, 6, 6, 6, 6, 6, 6, 0, 0, 0, 0, 0, 1, 0},
    {0, 1, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 1, 1, 3, 6, 6, 6, 6, 6, 6, 6, 0, 1, 1, 1, 0, 1, 0},
    {0, 1, 3, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 1, 1, 0, 6, 6, 6, 6, 6, 6, 6, 0, 1, 0, 1, 0, 1, 0},
    {0, 0, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 1, 1, 0, 6, 6, 6, 6, 6, 6, 6, 3, 1, 0, 1, 0, 1, 0},
    {0, 1, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 1, 1, 0, 6, 6, 6, 6, 6, 6, 6, 0, 0, 0, 1, 0, 1, 0},
    {0, 1, 3, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 1, 1, 0, 6, 6, 6, 6, 6, 6, 6, 0, 5, 1, 1, 0, 1, 0},
    {0, 1, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
    {0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0},
    {0, 1, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 1, 0},
    {0, 1, 0, 6, 6, 6, 0, 6, 6, 6, 6, 6, 6, 0, 1, 1, 0, 1, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 1, 0},
    {0, 1, 0, 6, 6, 6, 0, 6, 6, 6, 6, 6, 6, 0, 1, 1, 0, 1, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 1, 0},
    {0, 1, 3, 6, 6, 6, 0, 6, 6, 6, 6, 6, 6, 0, 1, 1, 0, 1, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 1, 0},
    {0, 1, 0, 6, 6, 6, 0, 6, 6, 6, 6, 6, 6, 0, 1, 1, 0, 1, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 3, 1, 0},
    {0, 1, 0, 6, 6, 6, 0, 6, 6, 6, 6, 6, 6, 0, 1, 1, 0, 0, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 1, 0},
    {0, 1, 0, 6, 6, 6, 0, 6, 6, 6, 6, 6, 6, 3, 1, 1, 1, 1, 3, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 1, 0},
    {0, 1, 0, 6, 6, 6, 0, 6, 6, 6, 6, 6, 6, 0, 1, 1, 1, 1, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 1, 0},
    {0, 1, 0, 6, 6, 6, 0, 0, 0, 0, 0, 3, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 1, 0},
    {0, 1, 0, 6, 6, 6, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};

// ================================================================
//  MAP 2  "The Citadel"
//  F2(upper): START=(20,1)   STAIR=(15,15)
//  F1(lower): STAIR=(15,15)  END=(1,30)
//
//  Rooms [F2]:
//    Room P    rows 3-8,  cols 3-12   doors: N(2,7) S(9,7) W(5,2)
//    Room Q    rows 3-8,  cols 16-28  doors: N(2,22) S(9,22) E(5,29)
//    StairRoom rows 12-18, cols 10-21 doors: N(11,15) S(19,15) W(15,9) E(15,22)
//
//  Rooms [F1]:
//    Room F    rows 3-9,  cols 22-28  doors: N(2,25) S(10,25) W(6,21) E(6,29)
//    Room G    rows 13-18, cols 3-9   doors: N(12,6) S(19,6) W(15,2)
// ================================================================

static const uint8_t s_Map2_F2[MAP_ROWS][MAP_COLS] = {
//   0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 1, 0},
    {0, 1, 3, 6, 6, 6, 6, 0, 6, 6, 6, 6, 6, 0, 1, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 1, 0},
    {0, 0, 0, 6, 6, 6, 6, 0, 6, 6, 6, 6, 6, 0, 1, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 1, 0},
    {0, 1, 3, 6, 6, 6, 6, 3, 6, 6, 6, 6, 6, 0, 1, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 3, 1, 0},
    {0, 1, 0, 6, 6, 6, 6, 0, 6, 6, 6, 6, 6, 0, 1, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 1, 0},
    {0, 1, 0, 6, 6, 6, 6, 0, 6, 6, 6, 6, 6, 0, 1, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 1, 0},
    {0, 1, 0, 6, 6, 6, 6, 0, 6, 6, 6, 6, 6, 0, 1, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 1, 0},
    {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 1, 0},
    {0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0},
    {0, 2, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0},
    {0, 2, 0, 6, 6, 6, 6, 0, 1, 0, 6, 6, 6, 0, 6, 6, 6, 6, 6, 6, 6, 6, 0, 1, 0, 6, 6, 6, 6, 0, 1, 0},
    {0, 2, 0, 6, 6, 6, 6, 0, 1, 0, 6, 6, 6, 0, 6, 6, 6, 6, 6, 6, 6, 6, 0, 1, 0, 6, 6, 6, 6, 0, 1, 0},
    {0, 2, 0, 0, 0, 3, 0, 0, 1, 0, 6, 6, 6, 0, 6, 6, 6, 6, 6, 6, 6, 6, 0, 1, 0, 6, 6, 6, 6, 0, 1, 0},
    {0, 2, 0, 1, 1, 1, 1, 1, 1, 3, 6, 6, 6, 0, 6, 6, 6, 6, 6, 6, 6, 6, 3, 1, 0, 6, 6, 6, 6, 0, 1, 0},
    {0, 2, 0, 0, 0, 1, 1, 1, 1, 0, 6, 6, 6, 0, 6, 6, 6, 6, 6, 6, 6, 6, 0, 1, 0, 6, 6, 6, 6, 0, 1, 0},
    {0, 2, 0, 6, 0, 1, 1, 1, 1, 0, 6, 6, 6, 0, 6, 6, 6, 6, 6, 6, 6, 6, 0, 1, 0, 6, 6, 6, 6, 0, 1, 0},
    {0, 2, 0, 6, 0, 1, 1, 1, 1, 0, 6, 6, 6, 0, 6, 6, 6, 6, 6, 6, 6, 6, 0, 1, 3, 6, 6, 6, 6, 3, 1, 0},
    {0, 0, 0, 3, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 1, 0, 6, 6, 6, 6, 0, 1, 0},
    {0, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 6, 6, 6, 6, 0, 1, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};

static const uint8_t s_Map2_F1[MAP_ROWS][MAP_COLS] = {
//   0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    {0, 0, 0, 0, 3, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 3, 0, 0, 0, 0, 1, 0},
    {0, 1, 0, 6, 6, 6, 6, 0, 1, 0, 6, 6, 6, 6, 6, 6, 0, 6, 6, 0, 1, 0, 6, 6, 6, 6, 6, 6, 6, 0, 1, 0},
    {0, 1, 0, 6, 6, 6, 6, 0, 1, 0, 6, 6, 6, 6, 6, 6, 0, 6, 6, 0, 1, 0, 6, 6, 6, 6, 6, 6, 6, 0, 1, 0},
    {0, 1, 0, 6, 6, 6, 6, 0, 1, 0, 6, 6, 6, 6, 6, 6, 3, 6, 6, 0, 1, 0, 6, 6, 6, 6, 6, 6, 6, 0, 1, 0},
    {0, 1, 0, 6, 6, 6, 6, 3, 1, 0, 6, 6, 6, 6, 6, 6, 0, 6, 6, 0, 1, 3, 6, 6, 6, 6, 6, 6, 6, 3, 1, 0},
    {0, 1, 0, 6, 6, 6, 6, 0, 1, 0, 6, 6, 6, 6, 6, 6, 0, 6, 6, 0, 1, 0, 6, 6, 6, 6, 6, 6, 6, 0, 1, 0},
    {0, 1, 0, 6, 6, 6, 6, 0, 1, 3, 6, 6, 6, 6, 6, 6, 0, 6, 6, 0, 1, 0, 6, 6, 6, 6, 6, 6, 6, 0, 1, 0},
    {0, 1, 0, 6, 6, 6, 6, 0, 1, 0, 6, 6, 6, 6, 6, 6, 0, 6, 6, 0, 1, 0, 6, 6, 6, 6, 6, 6, 6, 0, 1, 0},
    {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 3, 0, 0, 0, 0, 1, 0},
    {0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0},
    {0, 1, 0, 0, 0, 0, 3, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
    {0, 1, 0, 6, 6, 6, 6, 6, 6, 6, 0, 1, 0, 1, 0, 1, 0, 6, 6, 6, 6, 6, 6, 6, 0, 0, 0, 0, 0, 0, 1, 0},
    {0, 1, 0, 6, 6, 6, 6, 6, 6, 6, 0, 1, 0, 1, 0, 1, 0, 6, 6, 6, 6, 6, 6, 6, 0, 0, 1, 1, 1, 0, 1, 0},
    {0, 1, 3, 6, 6, 6, 6, 6, 6, 6, 0, 1, 0, 1, 0, 1, 0, 6, 6, 6, 6, 6, 6, 6, 0, 5, 0, 0, 1, 0, 1, 0},
    {0, 1, 0, 6, 6, 6, 6, 6, 6, 6, 0, 1, 0, 1, 0, 0, 0, 6, 6, 6, 6, 6, 6, 6, 0, 1, 1, 1, 1, 0, 1, 0},
    {0, 1, 0, 6, 6, 6, 6, 6, 6, 6, 0, 1, 0, 1, 1, 0, 0, 6, 6, 6, 6, 6, 6, 6, 0, 0, 0, 0, 1, 0, 1, 0},
    {0, 1, 0, 6, 6, 6, 6, 6, 6, 6, 0, 1, 0, 1, 0, 0, 0, 6, 6, 6, 6, 6, 6, 6, 3, 1, 1, 1, 1, 0, 1, 0},
    {0, 1, 0, 0, 0, 0, 3, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};

// ================================================================
//  CMap_Manager implementation
// ================================================================

bool CMap_Manager::InBounds(int r, int c) const
{
    return r >= 0 && r < MAP_ROWS && c >= 0 && c < MAP_COLS;
}

TileType CMap_Manager::GetTile(int r, int c) const
{
    if (!InBounds(r, c)) return TileType::WALL;
    return m_Tiles[r][c];
}

bool CMap_Manager::IsPassable(int r, int c) const
{
    if (!InBounds(r, c)) return false;
    TileType t = m_Tiles[r][c];
    if (t == TileType::DOOR) return m_DoorOpen[r][c];
    return (t == TileType::FLOOR || t == TileType::STAIR ||
            t == TileType::ROOM  || t == TileType::START ||
            t == TileType::END);
}

bool CMap_Manager::IsFlatFloor(int r, int c) const
{
    TileType t = GetTile(r, c);
    return (t == TileType::FLOOR || t == TileType::ROOM  ||
            t == TileType::START || t == TileType::END   ||
            t == TileType::STAIR);
}

bool CMap_Manager::IsStair(int r, int c) const
{
    return InBounds(r, c) && m_Tiles[r][c] == TileType::STAIR;
}

void CMap_Manager::ToggleDoor(int r, int c)
{
    if (InBounds(r, c) && m_Tiles[r][c] == TileType::DOOR)
        m_DoorOpen[r][c] = !m_DoorOpen[r][c];
}

bool CMap_Manager::IsDoorOpen(int r, int c) const
{
    return InBounds(r, c) && m_DoorOpen[r][c];
}

void CMap_Manager::Load(int mapIndex)
{
    const uint8_t (*srcF2)[MAP_COLS];
    const uint8_t (*srcF1)[MAP_COLS];

    if (mapIndex == 0) { srcF2 = s_Map1_F2; srcF1 = s_Map1_F1; }
    else               { srcF2 = s_Map2_F2; srcF1 = s_Map2_F1; }

    memset(m_DoorOpen, 0, sizeof(m_DoorOpen));
    m_CurrentFloor = 1;

    m_StartRow = m_StartCol = 0;
    m_EndRow   = m_EndCol   = 0;

    for (int r = 0; r < MAP_ROWS; ++r)
        for (int c = 0; c < MAP_COLS; ++c)
        {
            m_F2[r][c] = static_cast<TileType>(srcF2[r][c]);
            m_F1[r][c] = static_cast<TileType>(srcF1[r][c]);
        }

    m_Tiles = m_F2;

    for (int r = 0; r < MAP_ROWS; ++r)
        for (int c = 0; c < MAP_COLS; ++c)
            if (m_F2[r][c] == TileType::START)
                { m_StartRow = r; m_StartCol = c; }

    for (int r = 0; r < MAP_ROWS; ++r)
        for (int c = 0; c < MAP_COLS; ++c)
            if (m_F1[r][c] == TileType::END)
                { m_EndRow = r; m_EndCol = c; }

    m_StairF2TopRow = m_StairF2BotRow = m_StairF2Col = 0;
    m_StairF1Row    = m_StairF1Col = 0;
    {
        int minR = MAP_ROWS, maxR = -1;
        for (int r = 0; r < MAP_ROWS; ++r)
            for (int c = 0; c < MAP_COLS; ++c)
            {
                if (m_F2[r][c] == TileType::STAIR)
                {
                    if (r < minR) { minR = r; m_StairF2Col = c; }
                    if (r > maxR)   maxR = r;
                }
            }
        m_StairF2TopRow = (minR < MAP_ROWS) ? minR : 0;
        m_StairF2BotRow = (maxR >= 0)        ? maxR : 0;

        // F1 arrival: prefer STAIR tile, fall back to START tile
        bool foundF1Stair = false;
        int  f1StartR = 1, f1StartC = 1;
        for (int r = 0; r < MAP_ROWS; ++r)
            for (int c = 0; c < MAP_COLS; ++c)
            {
                if (m_F1[r][c] == TileType::STAIR && !foundF1Stair)
                    { m_StairF1Row = r; m_StairF1Col = c; foundF1Stair = true; }
                if (m_F1[r][c] == TileType::START)
                    { f1StartR = r; f1StartC = c; }
            }
        if (!foundF1Stair) { m_StairF1Row = f1StartR; m_StairF1Col = f1StartC; }
    }
}

void CMap_Manager::SwitchFloor()
{
    m_CurrentFloor ^= 1;
    m_Tiles = (m_CurrentFloor == 1) ? m_F2 : m_F1;
    memset(m_DoorOpen, 0, sizeof(m_DoorOpen));
}

XMFLOAT3 CMap_Manager::GetSpawnPos() const
{
    return XMFLOAT3(
        m_StartCol * TILE_SCALE + TILE_SCALE * 0.5f,
        0.0f,
        m_StartRow * TILE_SCALE + TILE_SCALE * 0.5f);
}

XMFLOAT3 CMap_Manager::GetStairPos() const
{
    // After switching to F1, return F1 stair arrival tile center.
    // While still on F2, return the top of the stair corridor.
    int r = (m_CurrentFloor == 0) ? m_StairF1Row    : m_StairF2TopRow;
    int c = (m_CurrentFloor == 0) ? m_StairF1Col    : m_StairF2Col;
    return XMFLOAT3(c * TILE_SCALE + TILE_SCALE * 0.5f, 0.0f, r * TILE_SCALE + TILE_SCALE * 0.5f);
}

std::vector<XMFLOAT3> CMap_Manager::GetEnemySpawnPositions() const
{
    std::vector<XMFLOAT3> positions;
    // Sample every 4 tiles so each room gets 1-2 enemies
    for (int r = 1; r < MAP_ROWS - 1; r += 4)
        for (int c = 1; c < MAP_COLS - 1; c += 4)
            if (m_Tiles[r][c] == TileType::ROOM)
                positions.push_back({
                    c * TILE_SCALE + TILE_SCALE * 0.5f,
                    0.0f,
                    r * TILE_SCALE + TILE_SCALE * 0.5f });
    return positions;
}

float CMap_Manager::GetStairProgress(float worldX, float worldZ) const
{
    int r = (int)floorf(worldZ / TILE_SCALE);
    int c = (int)floorf(worldX / TILE_SCALE);
    if (!IsStair(r, c)) return -1.0f;

    int totalSteps = m_StairF2BotRow - m_StairF2TopRow + 1;
    if (totalSteps <= 1) return 0.0f;

    float topZ = m_StairF2TopRow * TILE_SCALE;
    float botZ = (m_StairF2BotRow + 1) * TILE_SCALE;
    float t    = (worldZ - topZ) / (botZ - topZ);
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    return t;
}

bool CMap_Manager::IsAtStairBottom(int r, int c) const
{
    return IsStair(r, c) && r == m_StairF2BotRow && c == m_StairF2Col;
}

bool CMap_Manager::IsAtEnd(float worldX, float worldZ) const
{
    if (m_CurrentFloor != 0) return false;
    int r = (int)floorf(worldZ / TILE_SCALE);
    int c = (int)floorf(worldX / TILE_SCALE);
    return (r == m_EndRow && c == m_EndCol);
}

// ================================================================
//  Rendering (merged from CMapRenderer)
// ================================================================

static constexpr XMFLOAT4 COLOR_WALL  = { 0.38f, 0.38f, 0.40f, 1.0f };
static constexpr XMFLOAT4 COLOR_FLOOR = { 0.62f, 0.60f, 0.56f, 1.0f };
static constexpr XMFLOAT4 COLOR_STAIR = { 0.85f, 0.70f, 0.30f, 1.0f };
static constexpr XMFLOAT4 COLOR_DOOR  = { 0.45f, 0.28f, 0.10f, 1.0f };
static constexpr XMFLOAT4 COLOR_START = { 0.20f, 0.70f, 0.20f, 1.0f };
static constexpr XMFLOAT4 COLOR_END   = { 0.85f, 0.75f, 0.10f, 1.0f };
static constexpr XMFLOAT4 COLOR_ROOM  = { 0.55f, 0.52f, 0.60f, 1.0f };

static inline float TileX(int c) { return c * TILE_SCALE + TILE_SCALE * 0.5f; }
static inline float TileZ(int r) { return r * TILE_SCALE + TILE_SCALE * 0.5f; }

CBlock* CMap_Manager::MakeBlock(ID3D12Device* pd3dDevice,
                                 ID3D12GraphicsCommandList* pd3dCommandList,
                                 CShader* pShader,
                                 float x, float y, float z,
                                 float w, float h, float d,
                                 XMFLOAT4 color)
{
    CBlock* pBlock = new CBlock();
    pBlock->SetShader(pShader);
    pBlock->SetMesh(new CCubeMesh(pd3dDevice, pd3dCommandList, w, h, d, color));
    pBlock->CreateShaderVariables(pd3dDevice, pd3dCommandList);
    pBlock->SetPosition(x, y, z);
    return pBlock;
}

void CMap_Manager::BuildWall(ID3D12Device* pd3dDevice,
                              ID3D12GraphicsCommandList* pd3dCommandList,
                              CShader* pShader, int r, int c)
{
    m_Objects.push_back(MakeBlock(pd3dDevice, pd3dCommandList, pShader,
        TileX(c), 1.0f, TileZ(r),
        TILE_SCALE, 2.0f, TILE_SCALE, COLOR_WALL));
}

void CMap_Manager::BuildFloor(ID3D12Device* pd3dDevice,
                               ID3D12GraphicsCommandList* pd3dCommandList,
                               CShader* pShader, int r, int c,
                               float yCenter, XMFLOAT4 color)
{
    m_Objects.push_back(MakeBlock(pd3dDevice, pd3dCommandList, pShader,
        TileX(c), yCenter, TileZ(r),
        TILE_SCALE, 0.1f, TILE_SCALE, color));
}

void CMap_Manager::BuildStair(ID3D12Device* pd3dDevice,
                               ID3D12GraphicsCommandList* pd3dCommandList,
                               CShader* pShader, int r, int c)
{
    float x    = TileX(c), z = TileZ(r);
    float half = TILE_SCALE * 0.5f;

    int totalSteps = m_StairF2BotRow - m_StairF2TopRow + 1;
    if (totalSteps <= 1)
    {
        // F1 arrival tile: flat stair marker
        m_Objects.push_back(MakeBlock(pd3dDevice, pd3dCommandList, pShader,
            x, -0.05f, z, TILE_SCALE, 0.1f, TILE_SCALE, COLOR_STAIR));
        return;
    }

    // Compute which step index this tile is (0 = top, N-1 = bottom)
    int stepIndex = r - m_StairF2TopRow;
    float stepY   = -STAIR_DROP * (float)stepIndex / (float)(totalSteps - 1);

    // Tread (horizontal surface the player walks on)
    m_Objects.push_back(MakeBlock(pd3dDevice, pd3dCommandList, pShader,
        x, stepY - 0.05f, z, TILE_SCALE, 0.1f, TILE_SCALE, COLOR_STAIR));

    // Riser (vertical face on south edge connecting down to the next step)
    if (stepIndex < totalSteps - 1)
    {
        float nextY  = -STAIR_DROP * (float)(stepIndex + 1) / (float)(totalSteps - 1);
        float riserH = stepY - nextY;
        float riserY = stepY - riserH * 0.5f;

        m_Objects.push_back(MakeBlock(pd3dDevice, pd3dCommandList, pShader,
            x, riserY, z + TILE_SCALE * 0.5f - 0.05f, TILE_SCALE, riserH, 0.1f, COLOR_STAIR));
    }

    // Side walls enclosing the stair corridor (east and west)
    static constexpr float SWALL_THICK = 0.15f;
    float wallH  = 2.0f + STAIR_DROP;
    float wallCY = (2.0f - STAIR_DROP) * 0.5f;
    m_Objects.push_back(MakeBlock(pd3dDevice, pd3dCommandList, pShader,
        x - half + SWALL_THICK * 0.5f, wallCY, z,
        SWALL_THICK, wallH, TILE_SCALE, COLOR_WALL));
    m_Objects.push_back(MakeBlock(pd3dDevice, pd3dCommandList, pShader,
        x + half - SWALL_THICK * 0.5f, wallCY, z,
        SWALL_THICK, wallH, TILE_SCALE, COLOR_WALL));
}

void CMap_Manager::BuildDoor(ID3D12Device* pd3dDevice,
                              ID3D12GraphicsCommandList* pd3dCommandList,
                              CShader* pShader, int r, int c)
{
    float x = TileX(c);
    float z = TileZ(r);

    // N/S neighbors passable -> corridor N-S -> door faces E-W (thin Z) -> slides along X
    bool nsPassage = IsPassable(r - 1, c) || IsPassable(r + 1, c);

    CDoorBlock* pDoor = new CDoorBlock();
    pDoor->SetShader(pShader);
    if (nsPassage)
        pDoor->SetMesh(new CCubeMesh(pd3dDevice, pd3dCommandList,
                                     TILE_SCALE, 2.0f, 0.1f, COLOR_DOOR));
    else
        pDoor->SetMesh(new CCubeMesh(pd3dDevice, pd3dCommandList,
                                     0.1f, 2.0f, TILE_SCALE, COLOR_DOOR));
    pDoor->CreateShaderVariables(pd3dDevice, pd3dCommandList);
    pDoor->Init({ x, 1.0f, z }, nsPassage);

    m_Doors[r * MAP_COLS + c] = pDoor;
}

void CMap_Manager::Build(ID3D12Device* pd3dDevice,
                          ID3D12GraphicsCommandList* pd3dCommandList,
                          CShader* pShader)
{
    for (int r = 0; r < MAP_ROWS; ++r)
    {
        for (int c = 0; c < MAP_COLS; ++c)
        {
            switch (GetTile(r, c))
            {
            case TileType::WALL:
                BuildWall(pd3dDevice, pd3dCommandList, pShader, r, c);
                break;
            case TileType::FLOOR:
                BuildFloor(pd3dDevice, pd3dCommandList, pShader, r, c, -0.05f, COLOR_FLOOR);
                break;
            case TileType::STAIR:
                BuildStair(pd3dDevice, pd3dCommandList, pShader, r, c);
                break;
            case TileType::DOOR:
                BuildDoor(pd3dDevice, pd3dCommandList, pShader, r, c);
                break;
            case TileType::START:
                BuildFloor(pd3dDevice, pd3dCommandList, pShader, r, c, -0.05f, COLOR_START);
                break;
            case TileType::END:
                BuildFloor(pd3dDevice, pd3dCommandList, pShader, r, c, -0.05f, COLOR_END);
                break;
            case TileType::ROOM:
                BuildFloor(pd3dDevice, pd3dCommandList, pShader, r, c, 0.f, COLOR_ROOM);
                break;
            }
        }
    }
}

void CMap_Manager::UpdateDoors()
{
    for (auto& kv : m_Doors)
    {
        int r = kv.first / MAP_COLS;
        int c = kv.first % MAP_COLS;
        kv.second->SetOpen(IsDoorOpen(r, c));
    }
}

void CMap_Manager::Update(float dt)
{
    for (auto& kv : m_Doors)
        kv.second->Update(dt);
}

void CMap_Manager::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
    for (CBlock* pBlock : m_Objects)
        pBlock->Render(pd3dCommandList, pCamera);
    for (auto& kv : m_Doors)
        kv.second->Render(pd3dCommandList, pCamera);
}

void CMap_Manager::ReleaseUploadBuffers()
{
    for (CBlock* pBlock : m_Objects)
        pBlock->ReleaseUploadBuffers();
    for (auto& kv : m_Doors)
        kv.second->ReleaseUploadBuffers();
}

void CMap_Manager::ReleaseObjects()
{
    for (CBlock* pBlock : m_Objects)
        if (pBlock) delete pBlock;
    m_Objects.clear();

    for (auto& kv : m_Doors)
        if (kv.second) delete kv.second;
    m_Doors.clear();
}
