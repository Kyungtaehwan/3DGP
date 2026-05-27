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

//Map1
static const uint8_t s_Map1_F2[MAP_ROWS][MAP_COLS] = {

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
    {0, 0, 0, 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    {0, 1, 0, 0, 0, 3, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 3, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0},
    {0, 1, 0, 6, 6, 6, 6, 6, 6, 0, 1, 0, 1, 0, 6, 6, 6, 2, 6, 6, 6, 0, 1, 0, 6, 6, 6, 6, 6, 0, 1, 0},
    {0, 1, 0, 6, 6, 6, 6, 6, 6, 0, 1, 0, 1, 0, 6, 6, 6, 2, 6, 6, 6, 0, 1, 0, 6, 6, 6, 6, 6, 0, 1, 0},
    {0, 1, 3, 6, 6, 6, 6, 6, 6, 0, 1, 0, 1, 0, 6, 6, 6, 2, 6, 6, 6, 3, 1, 0, 6, 6, 6, 6, 6, 0, 1, 0},
    {0, 1, 0, 6, 6, 6, 6, 6, 6, 0, 1, 1, 1, 0, 6, 6, 6, 2, 6, 6, 6, 0, 1, 0, 6, 6, 6, 6, 6, 0, 1, 0},
    {0, 1, 0, 6, 6, 6, 6, 6, 6, 0, 1, 0, 1, 0, 6, 6, 6, 2, 6, 6, 6, 0, 1, 0, 6, 6, 6, 6, 6, 0, 1, 0},
    {0, 1, 0, 6, 6, 6, 6, 6, 6, 0, 1, 0, 1, 0, 6, 6, 6, 2, 6, 6, 6, 0, 1, 0, 6, 6, 6, 6, 6, 0, 1, 0},
    {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0},
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};

static const uint8_t s_Map1_F1[MAP_ROWS][MAP_COLS] = {
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

//Map2

static const uint8_t s_Map2_F2[MAP_ROWS][MAP_COLS] = {
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

    m_MapIndex = (mapIndex == 1) ? 1 : 0;
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

    int r = (m_CurrentFloor == 0) ? m_StairF1Row    : m_StairF2TopRow;
    int c = (m_CurrentFloor == 0) ? m_StairF1Col    : m_StairF2Col;
    return XMFLOAT3(c * TILE_SCALE + TILE_SCALE * 0.5f, 0.0f, r * TILE_SCALE + TILE_SCALE * 0.5f);
}

std::vector<XMFLOAT3> CMap_Manager::GetEnemySpawnPositions() const
{
    std::vector<XMFLOAT3> positions;

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


struct MapPalette
{
    XMFLOAT4 wall;
    XMFLOAT4 floor;
    XMFLOAT4 stair;
    XMFLOAT4 door;
    XMFLOAT4 start;
    XMFLOAT4 end;
    XMFLOAT4 room;
};

static const MapPalette s_Palettes[2] =
{
    {
        { 0.38f, 0.38f, 0.40f, 1.0f },  // wall 
        { 0.62f, 0.60f, 0.56f, 1.0f },  // floor
        { 0.85f, 0.70f, 0.30f, 1.0f },  // stair
        { 0.45f, 0.28f, 0.10f, 1.0f },  // door 
        { 0.20f, 0.70f, 0.20f, 1.0f },  // start
        { 0.85f, 0.75f, 0.10f, 1.0f },  // end  
        { 0.55f, 0.52f, 0.60f, 1.0f },  // room 
    },
    {
        { 0.18f, 0.22f, 0.34f, 1.0f },  // wall 
        { 0.22f, 0.36f, 0.42f, 1.0f },  // floor
        { 0.30f, 0.85f, 0.95f, 1.0f },  // stair
        { 0.50f, 0.10f, 0.25f, 1.0f },  // door 
        { 0.20f, 0.80f, 0.60f, 1.0f },  // start
        { 0.90f, 0.25f, 0.75f, 1.0f },  // end  
        { 0.32f, 0.22f, 0.55f, 1.0f },  // room 
    },
};

static inline const MapPalette& PaletteFor(int idx)
{
    return s_Palettes[(idx == 1) ? 1 : 0];
}

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
        TILE_SCALE, 2.0f, TILE_SCALE, PaletteFor(m_MapIndex).wall));
}

void CMap_Manager::BuildFloor(ID3D12Device* pd3dDevice,
                               ID3D12GraphicsCommandList* pd3dCommandList,
                               CShader* pShader, int r, int c,
                               float yCenter, XMFLOAT4 color)
{

    float yTop = yCenter + 0.05f;

    CBlock* pBlock = new CBlock();
    pBlock->SetShader(pShader);
    pBlock->SetMesh(new CFloorQuadMesh(pd3dDevice, pd3dCommandList,
                                       TILE_SCALE, TILE_SCALE, yTop, color));
    pBlock->CreateShaderVariables(pd3dDevice, pd3dCommandList);
    pBlock->SetPosition(TileX(c), 0.0f, TileZ(r));
    m_Objects.push_back(pBlock);
}

void CMap_Manager::BuildStair(ID3D12Device* pd3dDevice,
                               ID3D12GraphicsCommandList* pd3dCommandList,
                               CShader* pShader, int r, int c)
{
    const MapPalette& pal = PaletteFor(m_MapIndex);

    float x    = TileX(c), z = TileZ(r);
    float half = TILE_SCALE * 0.5f;

    int totalSteps = m_StairF2BotRow - m_StairF2TopRow + 1;
    if (totalSteps <= 1)
    {
 
        m_Objects.push_back(MakeBlock(pd3dDevice, pd3dCommandList, pShader,
            x, -0.05f, z, TILE_SCALE, 0.1f, TILE_SCALE, pal.stair));
        return;
    }

    int stepIndex = r - m_StairF2TopRow;
    float stepY   = -STAIR_DROP * (float)stepIndex / (float)(totalSteps - 1);


    m_Objects.push_back(MakeBlock(pd3dDevice, pd3dCommandList, pShader,
        x, stepY - 0.05f, z, TILE_SCALE, 0.1f, TILE_SCALE, pal.stair));

    if (stepIndex < totalSteps - 1)
    {
        float nextY  = -STAIR_DROP * (float)(stepIndex + 1) / (float)(totalSteps - 1);
        float riserH = stepY - nextY;
        float riserY = stepY - riserH * 0.5f;

        m_Objects.push_back(MakeBlock(pd3dDevice, pd3dCommandList, pShader,
            x, riserY, z + TILE_SCALE * 0.5f - 0.05f, TILE_SCALE, riserH, 0.1f, pal.stair));
    }

    static constexpr float SWALL_THICK = 0.15f;
    float wallH  = 2.0f + STAIR_DROP;
    float wallCY = (2.0f - STAIR_DROP) * 0.5f;
    m_Objects.push_back(MakeBlock(pd3dDevice, pd3dCommandList, pShader,
        x - half + SWALL_THICK * 0.5f, wallCY, z,
        SWALL_THICK, wallH, TILE_SCALE, pal.wall));
    m_Objects.push_back(MakeBlock(pd3dDevice, pd3dCommandList, pShader,
        x + half - SWALL_THICK * 0.5f, wallCY, z,
        SWALL_THICK, wallH, TILE_SCALE, pal.wall));
}

void CMap_Manager::BuildDoor(ID3D12Device* pd3dDevice,
                              ID3D12GraphicsCommandList* pd3dCommandList,
                              CShader* pShader, int r, int c)
{
    float x = TileX(c);
    float z = TileZ(r);


    bool nsPassage = IsPassable(r - 1, c) || IsPassable(r + 1, c);

    const XMFLOAT4& doorColor = PaletteFor(m_MapIndex).door;

    CDoorBlock* pDoor = new CDoorBlock();
    pDoor->SetShader(pShader);
    if (nsPassage)
        pDoor->SetMesh(new CCubeMesh(pd3dDevice, pd3dCommandList,
                                     TILE_SCALE, 2.0f, 0.1f, doorColor));
    else
        pDoor->SetMesh(new CCubeMesh(pd3dDevice, pd3dCommandList,
                                     0.1f, 2.0f, TILE_SCALE, doorColor));
    pDoor->CreateShaderVariables(pd3dDevice, pd3dCommandList);
    pDoor->Init({ x, 1.0f, z }, nsPassage);

    m_Doors[r * MAP_COLS + c] = pDoor;
}

void CMap_Manager::Build(ID3D12Device* pd3dDevice,
                          ID3D12GraphicsCommandList* pd3dCommandList,
                          CShader* pShader)
{
    const MapPalette& pal = PaletteFor(m_MapIndex);

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
                BuildFloor(pd3dDevice, pd3dCommandList, pShader, r, c, -0.06f, pal.floor);
                break;
            case TileType::STAIR:
                BuildStair(pd3dDevice, pd3dCommandList, pShader, r, c);
                break;
            case TileType::DOOR:
                BuildFloor(pd3dDevice, pd3dCommandList, pShader, r, c, -0.06f, pal.floor);
                BuildDoor(pd3dDevice, pd3dCommandList, pShader, r, c);
                break;
            case TileType::START:
                BuildFloor(pd3dDevice, pd3dCommandList, pShader, r, c, -0.06f, pal.start);
                break;
            case TileType::END:
                BuildFloor(pd3dDevice, pd3dCommandList, pShader, r, c, -0.06f, pal.end);
                break;
            case TileType::ROOM:
                BuildFloor(pd3dDevice, pd3dCommandList, pShader, r, c, -0.06f, pal.room);
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
