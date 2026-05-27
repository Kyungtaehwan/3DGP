#pragma once
#include "MapData.h"
#include "Block.h"
#include "Shader.h"
#include <DirectXMath.h>
#include <d3d12.h>
#include <vector>
#include <unordered_map>
using namespace DirectX;

class CCamera;

class CMap_Manager
{
public:
    static CMap_Manager* Get_Instance();
    static void          Destroy_Instance();

    // Map data
    void     Load(int mapIndex);
    void     SwitchFloor();
    int      GetCurrentFloor() const { return m_CurrentFloor; }

    TileType GetTile(int r, int c) const;
    bool     IsPassable(int r, int c) const;
    bool     IsFlatFloor(int r, int c) const;
    bool     IsStair(int r, int c) const;
    bool     InBounds(int r, int c) const;

    void     ToggleDoor(int r, int c);
    bool     IsDoorOpen(int r, int c) const;

    XMFLOAT3 GetSpawnPos() const;
    XMFLOAT3 GetStairPos() const;
    std::vector<XMFLOAT3> GetEnemySpawnPositions() const;
    float    GetStairProgress(float worldX, float worldZ) const;
    bool     IsAtStairBottom(int r, int c) const;
    bool     IsAtEnd(float worldX, float worldZ) const;

    // Rendering
    void Build(ID3D12Device* pd3dDevice,
               ID3D12GraphicsCommandList* pd3dCommandList,
               CShader* pShader);
    void UpdateDoors();
    void Update(float dt);
    void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
    void ReleaseUploadBuffers();
    void ReleaseObjects();

private:
    CMap_Manager()  = default;
    ~CMap_Manager() = default;
    CMap_Manager(const CMap_Manager&)            = delete;
    CMap_Manager& operator=(const CMap_Manager&) = delete;

    static CMap_Manager* s_pInstance;

    // Map data members
    TileType  m_F2[MAP_ROWS][MAP_COLS];
    TileType  m_F1[MAP_ROWS][MAP_COLS];
    int       m_MapIndex     = 0;   // 0 = stage 1, 1 = stage 2 (drives palette)
    int       m_CurrentFloor = 1;
    TileType(*m_Tiles)[MAP_COLS] = nullptr;
    bool      m_DoorOpen[MAP_ROWS][MAP_COLS] = {};
    int       m_StartRow    = 0, m_StartCol    = 0;
    int       m_EndRow      = 0, m_EndCol      = 0;
    int       m_StairF2TopRow = 0, m_StairF2BotRow = 0, m_StairF2Col = 0;
    int       m_StairF1Row    = 0, m_StairF1Col    = 0;


    CBlock*  MakeBlock(ID3D12Device*, ID3D12GraphicsCommandList*, CShader*,
                       float x, float y, float z,
                       float w, float h, float d, XMFLOAT4 color);
    void BuildWall (ID3D12Device*, ID3D12GraphicsCommandList*, CShader*, int r, int c);
    void BuildFloor(ID3D12Device*, ID3D12GraphicsCommandList*, CShader*, int r, int c,
                    float yCenter, XMFLOAT4 color);
    void BuildStair(ID3D12Device*, ID3D12GraphicsCommandList*, CShader*, int r, int c);
    void BuildDoor (ID3D12Device*, ID3D12GraphicsCommandList*, CShader*, int r, int c);

    std::vector<CBlock*>                 m_Objects;
    std::unordered_map<int, CDoorBlock*> m_Doors;   // key = r*MAP_COLS + c
};
