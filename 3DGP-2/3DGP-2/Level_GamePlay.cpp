#include "pch.h"
#include "Level_GamePlay.h"
#include "Level_Manager.h"
#include "Object_Manager.h"
#include "Player.h"
#include "Camera.h"
#include "Mesh.h"
#include "Input_Manager.h"
#include "UI_Manager.h"
#include <cmath>

void CLevel_GamePlay::Initialize(ID3D12Device* pd3dDevice,
                                  ID3D12GraphicsCommandList* pd3dCommandList,
                                  ID3D12RootSignature* pd3dRootSignature)
{
    m_pd3dDevice = pd3dDevice;

    // Stage 1 -> map index 0, Stage 2 -> map index 1.
    int nStage = CLevel_Manager::Get_Instance()->GetCurrentStage();
    if (nStage < 1) nStage = 1;
    m_nCurrentMap = nStage - 1;

    m_pShader = new CObjectShader();
    m_pShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dRootSignature);

    CPlayer* pPlayer = new CPlayer();
    pPlayer->CreateBodyParts(pd3dDevice, pd3dCommandList, m_pShader);
    m_pCamera = pPlayer->ChangeCamera(FIRST_PERSON_CAMERA, 0.0f);
    pPlayer->CreateShaderVariables(pd3dDevice, pd3dCommandList);
    CObject_Manager::Get_Instance()->Add_Object(OBJ_PLAYER, pPlayer);

    CInput_Manager::Get_Instance()->SetMouseLock(true);

    CMap_Manager* pMap = CMap_Manager::Get_Instance();
    pMap->Load(m_nCurrentMap);
    pMap->Build(pd3dDevice, pd3dCommandList, m_pShader);

    COBBRenderer::Get_Instance()->Init(pd3dDevice, pd3dCommandList, pd3dRootSignature);
    CBullet::PrepareShared(pd3dDevice, pd3dCommandList, m_pShader);

    SpawnPlayer();
    SpawnEnemies(pd3dCommandList);
}

void CLevel_GamePlay::SpawnPlayer()
{
    CGameObject* pObj = CObject_Manager::Get_Instance()->Get_Player();
    if (!pObj) return;
    CPlayer* pPlayer = static_cast<CPlayer*>(pObj);

    XMFLOAT3 spawn = CMap_Manager::Get_Instance()->GetSpawnPos();
    pPlayer->SetPosition(spawn);

    CCamera* pCam = pPlayer->GetCamera();
    if (pCam)
    {
        XMFLOAT3 eyePos = { spawn.x, spawn.y + 1.7f, spawn.z };
        pCam->SetPosition(eyePos);
        pCam->RegenerateViewMatrix();
    }
}

void CLevel_GamePlay::SpawnAtStair()
{
    CGameObject* pObj = CObject_Manager::Get_Instance()->Get_Player();
    if (!pObj) return;
    CPlayer* pPlayer = static_cast<CPlayer*>(pObj);

    XMFLOAT3 stairPos = CMap_Manager::Get_Instance()->GetStairPos();
    pPlayer->SetPosition(stairPos);

    CCamera* pCam = pPlayer->GetCamera();
    if (pCam)
    {
        XMFLOAT3 eyePos = { stairPos.x, stairPos.y + 1.7f, stairPos.z };
        pCam->SetPosition(eyePos);
        pCam->RegenerateViewMatrix();
    }
}

void CLevel_GamePlay::SpawnEnemies(ID3D12GraphicsCommandList* pd3dCommandList)
{
    CObject_Manager::Get_Instance()->DeleteID(OBJ_ENEMY);

    auto spawnList = CMap_Manager::Get_Instance()->GetEnemySpawnPositions();
    for (const XMFLOAT3& pos : spawnList)
    {
        CEnemy* pEnemy = new CEnemy();
        pEnemy->Init(m_pd3dDevice, pd3dCommandList, m_pShader, pos);
        CObject_Manager::Get_Instance()->Add_Object(OBJ_ENEMY, pEnemy);
    }
}

int CLevel_GamePlay::Update(float dt)
{
    CPlayer* pPlayer = static_cast<CPlayer*>(CObject_Manager::Get_Instance()->Get_Player());

    XMFLOAT3 prevPos = { 0.0f, 0.0f, 0.0f };
    if (pPlayer) prevPos = pPlayer->GetPosition();

    CObject_Manager::Get_Instance()->Update(dt);
    CObject_Manager::Get_Instance()->CheckCollisions();

    if (!pPlayer) return 0;

    CMap_Manager* pMap = CMap_Manager::Get_Instance();
    XMFLOAT3 newPos = pPlayer->GetPosition();

    const float radius = 0.28f;

    XMFLOAT3 correctedPos = newPos;

    // X axis: probe new X against prev Z (move-and-slide).
    // Using prev Z avoids the "stuck inside wall" bug where newPos.z was already
    // intersecting a wall row, causing X check to always fail.
    // 4-corner AABB probe catches cases where the player's Z extent straddles tile rows.
    {
        int cP = (int)floorf((newPos.x + radius) / TILE_SCALE);
        int cM = (int)floorf((newPos.x - radius) / TILE_SCALE);
        int zP = (int)floorf((prevPos.z + radius) / TILE_SCALE);
        int zM = (int)floorf((prevPos.z - radius) / TILE_SCALE);

        if (!pMap->IsPassable(zP, cP) || !pMap->IsPassable(zP, cM) ||
            !pMap->IsPassable(zM, cP) || !pMap->IsPassable(zM, cM))
            correctedPos.x = prevPos.x;
    }

    // Z axis: probe new Z against the (already corrected) X.
    {
        int xP = (int)floorf((correctedPos.x + radius) / TILE_SCALE);
        int xM = (int)floorf((correctedPos.x - radius) / TILE_SCALE);
        int rP = (int)floorf((newPos.z + radius) / TILE_SCALE);
        int rM = (int)floorf((newPos.z - radius) / TILE_SCALE);

        if (!pMap->IsPassable(rP, xP) || !pMap->IsPassable(rP, xM) ||
            !pMap->IsPassable(rM, xP) || !pMap->IsPassable(rM, xM))
            correctedPos.z = prevPos.z;
    }

    if (correctedPos.x != newPos.x || correctedPos.z != newPos.z)
    {
        XMFLOAT3 camDelta = { correctedPos.x - newPos.x, 0.0f, correctedPos.z - newPos.z };
        pPlayer->SetPosition(correctedPos);
        pPlayer->GetCamera()->Move(camDelta);
    }

    CInput_Manager* pInput = CInput_Manager::Get_Instance();

    // E key: toggle door in front of player
    if (pInput->Key_Down('E'))
    {
        CCamera* pCam = pPlayer->GetCamera();
        XMFLOAT3 pos  = pPlayer->GetPosition();
        XMFLOAT3 look = pCam->GetLookVector();
        float dist = TILE_SCALE * 1.5f;
        int checkR = (int)floorf((pos.z + look.z * dist) / TILE_SCALE);
        int checkC = (int)floorf((pos.x + look.x * dist) / TILE_SCALE);
        if (pMap->InBounds(checkR, checkC) &&
            pMap->GetTile(checkR, checkC) == TileType::DOOR)
        {
            pMap->ToggleDoor(checkR, checkC);
            pMap->UpdateDoors();
        }
    }

    // Stair: adjust player Y based on position along the stair corridor (F2 only)
    if (pMap->GetCurrentFloor() == 1)
    {
        XMFLOAT3 pos = pPlayer->GetPosition();
        float progress = pMap->GetStairProgress(pos.x, pos.z);

        if (progress >= 0.0f)
        {
            float stairY = -STAIR_DROP * progress;
            pPlayer->SetPosition({ pos.x, stairY, pos.z });

            CCamera* pCam = pPlayer->GetCamera();
            XMFLOAT3 camPos = pCam->GetPosition();
            pCam->SetPosition({ camPos.x, stairY + 1.7f, camPos.z });

            // Trigger floor switch when player steps onto the bottom stair tile
            int pr = (int)floorf(pos.z / TILE_SCALE);
            int pc = (int)floorf(pos.x / TILE_SCALE);
            bool bOnBottom = pMap->IsAtStairBottom(pr, pc);
            if (bOnBottom && !m_bWasOnBottomStair)
            {
                m_bPendingFloorSwitch = true;
                m_bFloorSwitchIsStair = true;
            }
            m_bWasOnBottomStair = bOnBottom;
        }
        else
        {
            // Not on stair: ensure Y stays at floor level
            m_bWasOnBottomStair = false;
            if (pos.y < -0.01f)
            {
                pPlayer->SetPosition({ pos.x, 0.0f, pos.z });
                CCamera* pCam = pPlayer->GetCamera();
                XMFLOAT3 camPos = pCam->GetPosition();
                pCam->SetPosition({ camPos.x, 1.7f, camPos.z });
            }
        }
    }

    // F3: toggle OBB wireframe
    if (pInput->Key_Down(VK_F3))
        COBBRenderer::s_bShow = !COBBRenderer::s_bShow;

    // R key: manual map switch
    if (pInput->Key_Down('R'))
        m_bPendingMapSwitch = true;

    // End-of-stage detection (clear / game-over). Whichever happens first wins;
    // after that we just tick the timer and return to the menu.
    if (!m_bCleared && !m_bGameOver)
    {
        XMFLOAT3 pos = pPlayer->GetPosition();
        if (pMap->IsAtEnd(pos.x, pos.z))
        {
            m_bCleared = true;
            m_fEndTimer = CLEAR_DURATION;
        }
        else if (pPlayer->IsDead())
        {
            m_bGameOver = true;
            m_fEndTimer = GAMEOVER_DURATION;
        }
    }
    else
    {
        m_fEndTimer -= dt;
        if (m_fEndTimer <= 0.0f)
            CLevel_Manager::Get_Instance()->Request_Level_Change(LEVEL_MENU);
    }

    return 0;
}

void CLevel_GamePlay::Late_Update(float dt)
{
    CObject_Manager::Get_Instance()->Late_Update(dt);
    CMap_Manager::Get_Instance()->Update(dt);
}

void CLevel_GamePlay::Render(ID3D12GraphicsCommandList* pd3dCommandList)
{
    CMap_Manager* pMap = CMap_Manager::Get_Instance();

    if (m_bNeedReleaseUpload)
    {
        pMap->ReleaseUploadBuffers();
        m_bNeedReleaseUpload = false;
    }

    // Floor switch via stair
    if (m_bPendingFloorSwitch)
    {
        pMap->ReleaseObjects();
        pMap->SwitchFloor();
        pMap->Build(m_pd3dDevice, pd3dCommandList, m_pShader);
        if (m_bFloorSwitchIsStair)
        {
            SpawnAtStair();
            m_bFloorSwitchIsStair = false;
        }
        else
        {
            SpawnPlayer();
        }
        SpawnEnemies(pd3dCommandList);
        m_bPendingFloorSwitch = false;
        m_bNeedReleaseUpload  = true;
        m_bCleared            = false;
    }

    // Map switch (R key or clear condition)
    if (m_bPendingMapSwitch)
    {
        pMap->ReleaseObjects();
        m_nCurrentMap = 1 - m_nCurrentMap;
        pMap->Load(m_nCurrentMap);
        pMap->Build(m_pd3dDevice, pd3dCommandList, m_pShader);
        SpawnPlayer();
        SpawnEnemies(pd3dCommandList);
        m_bPendingMapSwitch  = false;
        m_bNeedReleaseUpload = true;
        m_bCleared           = false;
    }

    CGameObject* pPlayerObj = CObject_Manager::Get_Instance()->Get_Player();
    if (pPlayerObj)
        m_pCamera = static_cast<CPlayer*>(pPlayerObj)->GetCamera();

    if (m_pCamera)
    {
        m_pCamera->SetViewportsAndScissorRects(pd3dCommandList);
        m_pCamera->UpdateShaderVariables(pd3dCommandList);
    }

    CObject_Manager::Get_Instance()->Render(pd3dCommandList, m_pCamera);
    pMap->Render(pd3dCommandList, m_pCamera);

    // OBB wireframe overlay (F3 toggle)
    if (COBBRenderer::s_bShow)
    {
        COBBRenderer* pOBBR = COBBRenderer::Get_Instance();
        pOBBR->BeginFrame();

        CObject_Manager* pOM = CObject_Manager::Get_Instance();
        for (int i = 0; i < OBJ_END; i++)
        {
            for (CGameObject* pObj : *pOM->Get_List((OBJ_ID)i))
            {
                if (pObj && pObj->m_xmLocalOBB.Extents.x > 0.f)
                    pOBBR->AddOBB(pObj->m_xmOOBB);
            }
        }

        pOBBR->Render(pd3dCommandList);
    }

    // UI pass — drawn last so it sits on top of everything else.
    CUI_Manager* pUI = CUI_Manager::Get_Instance();
    pUI->Render(pd3dCommandList, m_pCamera);
    if (pPlayerObj)
        pUI->RenderHPBar(pd3dCommandList,
                         pPlayerObj->GetHP(), pPlayerObj->GetMaxHP());
    if (m_bCleared)
        pUI->RenderClearOverlay(pd3dCommandList);
    if (m_bGameOver)
        pUI->RenderGameOverOverlay(pd3dCommandList);
}

void CLevel_GamePlay::Release()
{
    CBullet::ReleaseShared();
    COBBRenderer::Destroy_Instance();
    // UI_Manager intentionally not destroyed here — its lifetime spans levels
    // (init'd by MainApp, freed at app exit).

    CMap_Manager* pMap = CMap_Manager::Get_Instance();
    pMap->ReleaseObjects();
    CMap_Manager::Destroy_Instance();
    CObject_Manager::Destroy_Instance();

    if (m_pShader)
    {
        delete m_pShader;
        m_pShader = NULL;
    }
}

void CLevel_GamePlay::ReleaseUploadBuffers()
{
    CBullet::ReleaseSharedUploadBuffers();
    CMap_Manager::Get_Instance()->ReleaseUploadBuffers();
    CObject_Manager::Get_Instance()->ReleaseUploadBuffers();
}
