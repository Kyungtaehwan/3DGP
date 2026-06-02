#pragma once
#include "GameObject.h"
#include <vector>

class CShader;

// GameObject 를 상속한 폭발 이펙트.
// 이름 클릭 시 new 해서 Object_Manager 에 add 하면(fire-and-forget) 매니저가
// 매 프레임 Update/Render 한다. 데브리(작은 큐브들)가 중심에서 사방으로 퍼졌다가
// BLOW_DUR 후 멈춘다. 호출측은 이펙트를 들고 있을 필요가 없다.
//
// 데브리 큐브 메쉬는 커맨드리스트가 필요하므로 PrepareShared 로 레벨 init 때 한 번만
// 만들어 공유한다. 이펙트 자신은 CB 만 만들면 되어 클릭 시점(device 만 있는 곳)에서
// 생성할 수 있다.
class CExplosionEffect : public CGameObject
{
public:
    CExplosionEffect() = default;
    virtual ~CExplosionEffect();

    static void PrepareShared(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
    static void ReleaseShared();
    static void ReleaseSharedUploadBuffers();

    // 중심(center)에서 폭발 시작. CB 만 생성하므로 device 만으로 충분.
    void Init(ID3D12Device* pd3dDevice, CShader* pShader, XMFLOAT3 center);

    virtual int  Update(float dt) override;
    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera) override;

private:
    static constexpr int   DEBRIS_CNT = 28;
    static constexpr float BLOW_DUR   = 0.8f;    // 지속 시간(초)
    static constexpr float DEBRIS_SPD = 1.3f;    // 퍼지는 속도(월드/초)

    struct Debris
    {
        CGameObject* pObj = nullptr;
        XMFLOAT3     dir  = {};                   // 퍼지는 방향(단위 구면)
        XMFLOAT3     spin = {};                   // 회전 속도(rad/초)
    };

    std::vector<Debris> m_Debris;
    XMFLOAT3 m_Center = { 0.0f, 0.0f, 0.0f };
    float    m_fTimer = 0.0f;

    static CMesh* s_pDebrisMesh;                  // 공유 큐브 메쉬
};
