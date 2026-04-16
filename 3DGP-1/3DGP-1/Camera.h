#pragma once
#include "define.h"

class CPlayer;

class CViewport {
public:
    int m_nLeft = 0, m_nTop = 0, m_nWidth = 800, m_nHeight = 600;
    void SetViewport(int l, int t, int w, int h);
};

class CCamera
{
public:
    CCamera(CPlayer* pPlayer, XMFLOAT3 xmf3Offset = XMFLOAT3(0.f, 5.f, -10.f));
    ~CCamera();

public:
    void SetViewport(int l, int t, int w, int h);
    void GeneratePerspectiveProjectionMatrix(float fNear, float fFar, float fFOV);
    void GenerateViewMatrix(); // Update + View행렬계산 통합
    void GenerateViewMatrix_Fixed(); // 고정 카메라용

    bool IsInFrustum(BoundingOrientedBox& obb);

    XMFLOAT4X4* GetViewProjectMatrix() { return &m_xmf4x4ViewProject; }
    CViewport* GetViewport() { return &m_Viewport; }
private:
    CPlayer* m_pPlayer = NULL;
    XMFLOAT3    m_xmf3Offset;

    XMFLOAT3    m_xmf3Position = XMFLOAT3(0, 0, 0);
    XMFLOAT3    m_xmf3Right = XMFLOAT3(1, 0, 0);
    XMFLOAT3    m_xmf3Up = XMFLOAT3(0, 1, 0);
    XMFLOAT3    m_xmf3Look = XMFLOAT3(0, 0, 1);

    XMFLOAT4X4  m_xmf4x4View = Matrix4x4::Identity();
    XMFLOAT4X4  m_xmf4x4Project = Matrix4x4::Identity();
    XMFLOAT4X4  m_xmf4x4ViewProject = Matrix4x4::Identity();

    CViewport   m_Viewport;
    float       m_fAspectRatio = 4.f / 3.f;

    BoundingFrustum m_xmFrustumView;
    BoundingFrustum m_xmFrustumWorld;
};