#include "pch.h"
#include "Level_GamePlay.h"
#include "Shader.h"
#include "Player.h"
#include "Camera.h"
#include "Object_Manager.h"
#include "Input_Manager.h"
#include "LightManager.h"
#include "Terrain.h"
#include "GameObject.h"
#include "Humvee.h"
#include "Tree.h"
#include "Tank.h"
#include <cstdlib>

// Matches cbTerrainLine (b5) in Shaders.hlsl.
struct CB_TERRAIN_LINE
{
    XMFLOAT4 p0p1;        // xy = start(x,z), zw = end(x,z)
    XMFLOAT4 color;       // rgb + opacity
    float    halfWidth;
    float    pad[3];
};

void CLevel_GamePlay::Initialize(ID3D12Device* pd3dDevice,
                                  ID3D12GraphicsCommandList* pd3dCommandList,
                                  ID3D12RootSignature* pd3dRootSignature)
{
    m_pShader = new CIlluminatedShader();
    m_pShader->AddRef();
    m_pShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dRootSignature);

    m_pTerrainShader = new CTerrainShader();
    m_pTerrainShader->AddRef();
    m_pTerrainShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dRootSignature);

    m_pLightManager = new CLightManager();
    m_pLightManager->BuildDefaultLights();
    m_pLightManager->CreateShaderVariables(pd3dDevice, pd3dCommandList);

    // --- Outdoor terrain (LabProject14) -------------------------------------
    // Height map is a header-less 8-bit grayscale RAW image. Width/Length MUST
    // match the actual pixel dimensions of the file. NOTE: this builds one
    // vertex per pixel, so 4096x4096 ~= 16.7M vertices (heavy). Test with a
    // smaller map (e.g. 1025x1025) first, then scale up.
    const int      TERRAIN_W       = 1024;
    const int      TERRAIN_L       = 1024;
    const int      BLOCK_W         = 257;   // 256 quads per block
    const int      BLOCK_L         = 257;
    const XMFLOAT3 TERRAIN_SCALE   = XMFLOAT3(4.0f, 2.0f, 4.0f); // x/z 2x bigger map, same height
    const XMFLOAT4 TERRAIN_COLOR   = XMFLOAT4(0.7f, 0.45f, 0.35f, 1.0f); // grass green

    m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList,
        "Model/HeightMap.raw", TERRAIN_W, TERRAIN_L, BLOCK_W, BLOCK_L,
        TERRAIN_SCALE, TERRAIN_COLOR, m_pTerrainShader);

    const float fMargin = 40.0f;
    const float fW = m_pTerrain->GetWidth();
    const float fL = m_pTerrain->GetLength();

    // Diagonal route: start corner -> opposite corner.
    XMFLOAT3 startPos = XMFLOAT3(fMargin,       0.0f, fMargin);
    XMFLOAT3 endPos   = XMFLOAT3(fW - fMargin,  0.0f, fL - fMargin);

    CPlayer* pPlayer = new CPlayer();
    pPlayer->LoadModel(pd3dDevice, pd3dCommandList, m_pShader, "Model/Apache.bin");

    // Helicopter starts above the route's start corner (same as the humvee).
    float fGround = m_pTerrain->GetHeight(startPos.x, startPos.z);
    pPlayer->SetPosition(XMFLOAT3(startPos.x, fGround + 200.0f, startPos.z));

    m_pCamera = pPlayer->ChangeCamera(THIRD_PERSON_CAMERA, 0.0f);
    pPlayer->CreateShaderVariables(pd3dDevice, pd3dCommandList);
    CObject_Manager::Get_Instance()->Add_Object(OBJ_PLAYER, pPlayer);

    // --- Humvee: drives the diagonal from startPos to endPos, then stops. ----
    CHumvee* pHumvee = new CHumvee();
    if (pHumvee->LoadModel(pd3dDevice, pd3dCommandList, m_pShader, "Model/Hummer.bin"))
    {
        pHumvee->SetTerrain(m_pTerrain);
        pHumvee->SetPath(startPos, endPos, 40.0f /*speed*/, 4.0f /*scale*/);
        pHumvee->SetBodyColor(XMFLOAT4(0.58f, 0.52f, 0.33f, 1.0f)); // khaki
        CObject_Manager::Get_Instance()->Add_Object(OBJ_HUMVEE, pHumvee);
    }
    else
    {
        delete pHumvee;
        pHumvee = nullptr;
    }

    // --- Enemy tank: spawns beside the road, drives at the humvee, then its
    //     turret tracks it once it gets close. -------------------------------
    if (pHumvee)
    {
        CTank* pTank = new CTank();
        if (pTank->LoadModel(pd3dDevice, pd3dCommandList, m_pShader, "Model/M26.bin"))
        {
            // Spawn near the middle of the route, offset to one side.
            XMFLOAT3 mid  = Vector3::ScalarProduct(Vector3::Add(startPos, endPos), 0.5f);
            XMFLOAT3 dir  = Vector3::Normalize(Vector3::Subtract(endPos, startPos));
            XMFLOAT3 perp = XMFLOAT3(-dir.z, 0.0f, dir.x);
            XMFLOAT3 spawn = Vector3::Add(mid, perp, 350.0f);
            if (spawn.x < fMargin) spawn.x = fMargin; if (spawn.x > fW - fMargin) spawn.x = fW - fMargin;
            if (spawn.z < fMargin) spawn.z = fMargin; if (spawn.z > fL - fMargin) spawn.z = fL - fMargin;

            pTank->SetTerrain(m_pTerrain);
            pTank->SetTarget(pHumvee);
            pTank->SetSpawn(spawn, 30.0f /*speed*/, 5.0f /*scale*/);
            pTank->SetBodyColor(XMFLOAT4(0.28f, 0.32f, 0.24f, 1.0f)); // dark olive
            CObject_Manager::Get_Instance()->Add_Object(OBJ_TANK, pTank);
        }
        else
        {
            delete pTank;
        }
    }

    // --- Route line constant buffer (b5): painted on the terrain. ------------
    UINT cbSize = (sizeof(CB_TERRAIN_LINE) + 255) & ~255;
    m_pcbLine = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, cbSize,
        D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
    m_pcbLine->Map(0, NULL, &m_pcbLineMapped);
    {
        CB_TERRAIN_LINE* p = (CB_TERRAIN_LINE*)m_pcbLineMapped;
        p->p0p1      = XMFLOAT4(startPos.x, startPos.z, endPos.x, endPos.z);
        p->color     = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.8f);  // gray road, 90% opacity
        p->halfWidth = 30.0f;                              // wider path (world units)
    }

    // --- Trees: one shared mesh, many upright instances at random spots. -----
    CGameObject* pTreeProto = CGameObject::LoadGeometryFromFile(
        pd3dDevice, pd3dCommandList, "Model/Tree.bin", m_pShader);
    if (pTreeProto)
    {
        pTreeProto->AddRef();

        srand(20260531);
        const int nTrees = 60;
        for (int i = 0; i < nTrees; ++i)
        {
            float rx = fMargin + (fW - 2 * fMargin) * (rand() / (float)RAND_MAX);
            float rz = fMargin + (fL - 2 * fMargin) * (rand() / (float)RAND_MAX);
            float ry = m_pTerrain->GetHeight(rx, rz);

            CTree* pTree = new CTree();
            pTree->InitInstance(pd3dDevice, pd3dCommandList, pTreeProto,
                                5.0f /*scale*/, XMFLOAT3(rx, ry, rz));
            CObject_Manager::Get_Instance()->Add_Object(OBJ_TREE, pTree);
        }

        // Instances now hold their own refs on the shared mesh/materials.
        pTreeProto->Release();
    }

    CInput_Manager::Get_Instance()->SetMouseLock(true);
}

int CLevel_GamePlay::Update(float dt)
{
    CObject_Manager::Get_Instance()->Update(dt);

    CPlayer* pPlayer = static_cast<CPlayer*>(CObject_Manager::Get_Instance()->Get_Player());

    // Spot light 1 follows the player (matches Sample::AnimateObjects).
    if (pPlayer && m_pLightManager)
        m_pLightManager->SetSpotLight(1, pPlayer->GetPosition(), pPlayer->GetLookVector());

    if (pPlayer)
    {
        static int s_tick = 0;
        if (++s_tick % 20 == 0)
        {
            XMFLOAT3 p = pPlayer->GetPosition();
            wchar_t buf[256];
            swprintf_s(buf, 256, L"3DGP-3 Helicopter  Pos(%.1f, %.1f, %.1f)",
                       p.x, p.y, p.z);
            SetWindowTextW(g_hWnd, buf);
        }
    }
    return 0;
}

void CLevel_GamePlay::Render(ID3D12GraphicsCommandList* pd3dCommandList)
{
    CPlayer* pPlayer = static_cast<CPlayer*>(CObject_Manager::Get_Instance()->Get_Player());
    if (pPlayer) m_pCamera = pPlayer->GetCamera();

    if (m_pCamera)
    {
        m_pCamera->SetViewportsAndScissorRects(pd3dCommandList);
        m_pCamera->UpdateShaderVariables(pd3dCommandList);
    }

    if (m_pLightManager) m_pLightManager->UpdateShaderVariables(pd3dCommandList);

    if (m_pcbLine)
        pd3dCommandList->SetGraphicsRootConstantBufferView(
            ROOT_SLOT_TERRAINLINE, m_pcbLine->GetGPUVirtualAddress());

    if (m_pTerrain) m_pTerrain->Render(pd3dCommandList, m_pCamera);

    // Player (heli) + humvee + trees all live in the object manager now.
    CObject_Manager::Get_Instance()->Render(pd3dCommandList, m_pCamera);
}

void CLevel_GamePlay::Release()
{
    CObject_Manager::Destroy_Instance();   // deletes player + humvee + trees

    if (m_pcbLine)
    {
        m_pcbLine->Unmap(0, NULL);
        m_pcbLine->Release();
        m_pcbLine       = NULL;
        m_pcbLineMapped = NULL;
    }

    if (m_pTerrain)       { m_pTerrain->Release();        m_pTerrain       = NULL; }
    if (m_pLightManager)  { delete m_pLightManager;       m_pLightManager  = NULL; }
    if (m_pTerrainShader) { m_pTerrainShader->Release();  m_pTerrainShader = NULL; }
    if (m_pShader)        { m_pShader->Release();          m_pShader        = NULL; }
}

void CLevel_GamePlay::ReleaseUploadBuffers()
{
    CObject_Manager::Get_Instance()->ReleaseUploadBuffers();   // player + humvee + trees
    if (m_pTerrain) m_pTerrain->ReleaseUploadBuffers();
}
