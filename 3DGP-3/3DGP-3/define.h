#pragma once

extern HWND g_hWnd;

#define WINCX 1280
#define WINCY 720

#define FRAME_BUFFER_WIDTH WINCX
#define FRAME_BUFFER_HEIGHT WINCY

#define ASPECT_RATIO ((float)FRAME_BUFFER_WIDTH / (float)FRAME_BUFFER_HEIGHT)

#define OBJ_NOEVENT 0
#define OBJ_DEAD 1

#define DIR_FORWARD 0x01
#define DIR_BACKWARD 0x02
#define DIR_LEFT 0x04
#define DIR_RIGHT 0x08
#define DIR_UP 0x10
#define DIR_DOWN 0x20

#define DegreeToRadian(x) ((x) * (XM_PI / 180.0f))
#define EPSILON 1.0e-6f

//   ROOT_SLOT_CAMERA(b1)
//   ROOT_SLOT_GAMEOBJECT(b2) 
//   ROOT_SLOT_LIGHTS(b4)
//   ROOT_SLOT_MATIDX_32(b3) 
#define ROOT_SLOT_CAMERA 0
#define ROOT_SLOT_GAMEOBJECT 1
#define ROOT_SLOT_LIGHTS 2
#define ROOT_SLOT_MATIDX_32 3
#define ROOT_SLOT_TERRAINLINE 4

inline bool IsZero(float f) { return (fabsf(f) < EPSILON); }
inline bool IsEqual(float a, float b) { return IsZero(a - b); }

enum LEVEL_ID
{
    LEVEL_LOGO = 0,
    LEVEL_MENU,
    LEVEL_STAGE1,
    LEVEL_END
};

enum OBJ_ID
{
    OBJ_PLAYER = 0,
    OBJ_HUMVEE,
    OBJ_TREE,
    OBJ_TANK,
    OBJ_PLAYER_BULLET,
    OBJ_ENEMY_BULLET,
    OBJ_EFFECT,
    OBJ_END
};

template <typename T>
inline void Safe_Delete(T*& p)
{
    if(p)
    {
        delete p;
        p = nullptr;
    }
}

namespace Vector3
{
    inline XMFLOAT3 XMVectorToFloat3(XMVECTOR v)
    {
        XMFLOAT3 f;
        XMStoreFloat3(&f, v);
        return f;
    }

    inline XMFLOAT3 ScalarProduct(XMFLOAT3 v, float s)
    {
        return XMVectorToFloat3(XMVectorScale(XMLoadFloat3(&v), s));
    }

    inline XMFLOAT3 Add(XMFLOAT3 a, XMFLOAT3 b)
    {
        return XMVectorToFloat3(XMVectorAdd(XMLoadFloat3(&a), XMLoadFloat3(&b)));
    }

    inline XMFLOAT3 Add(XMFLOAT3 a, XMFLOAT3 b, float s)
    {
        return XMVectorToFloat3(XMVectorAdd(XMLoadFloat3(&a),
                                            XMVectorScale(XMLoadFloat3(&b), s)));
    }

    inline XMFLOAT3 Subtract(XMFLOAT3 a, XMFLOAT3 b)
    {
        return XMVectorToFloat3(XMVectorSubtract(XMLoadFloat3(&a), XMLoadFloat3(&b)));
    }

    inline float DotProduct(XMFLOAT3 a, XMFLOAT3 b)
    {
        XMFLOAT3 r;
        XMStoreFloat3(&r, XMVector3Dot(XMLoadFloat3(&a), XMLoadFloat3(&b)));
        return r.x;
    }

    inline XMFLOAT3 CrossProduct(XMFLOAT3 a, XMFLOAT3 b)
    {
        return XMVectorToFloat3(XMVector3Cross(XMLoadFloat3(&a), XMLoadFloat3(&b)));
    }

    inline XMFLOAT3 Normalize(XMFLOAT3 v)
    {
        return XMVectorToFloat3(XMVector3Normalize(XMLoadFloat3(&v)));
    }

    inline float Length(XMFLOAT3 v)
    {
        XMFLOAT3 r;
        XMStoreFloat3(&r, XMVector3Length(XMLoadFloat3(&v)));
        return r.x;
    }

    inline XMFLOAT3 TransformNormal(XMFLOAT3 v, XMMATRIX m)
    {
        return XMVectorToFloat3(XMVector3TransformNormal(XMLoadFloat3(&v), m));
    }

    inline XMFLOAT3 TransformCoord(XMFLOAT3 v, XMMATRIX m)
    {
        return XMVectorToFloat3(XMVector3TransformCoord(XMLoadFloat3(&v), m));
    }

    inline XMFLOAT3 TransformCoord(XMFLOAT3 v, XMFLOAT4X4 m)
    {
        return TransformCoord(v, XMLoadFloat4x4(&m));
    }
}

namespace Matrix4x4
{
    inline XMFLOAT4X4 Identity()
    {
        XMFLOAT4X4 m;
        XMStoreFloat4x4(&m, XMMatrixIdentity());
        return m;
    }

    inline XMFLOAT4X4 Multiply(XMFLOAT4X4 a, XMFLOAT4X4 b)
    {
        XMFLOAT4X4 m;
        XMStoreFloat4x4(&m, XMMatrixMultiply(XMLoadFloat4x4(&a), XMLoadFloat4x4(&b)));
        return m;
    }

    inline XMFLOAT4X4 Multiply(XMFLOAT4X4 a, XMMATRIX b)
    {
        XMFLOAT4X4 m;
        XMStoreFloat4x4(&m, XMMatrixMultiply(XMLoadFloat4x4(&a), b));
        return m;
    }

    inline XMFLOAT4X4 Multiply(XMMATRIX a, XMFLOAT4X4 b)
    {
        XMFLOAT4X4 m;
        XMStoreFloat4x4(&m, XMMatrixMultiply(a, XMLoadFloat4x4(&b)));
        return m;
    }

    inline XMFLOAT4X4 PerspectiveFovLH(float fovY, float aspect, float nearZ, float farZ)
    {
        XMFLOAT4X4 m;
        XMStoreFloat4x4(&m, XMMatrixPerspectiveFovLH(fovY, aspect, nearZ, farZ));
        return m;
    }

    inline XMFLOAT4X4 LookAtLH(XMFLOAT3 eye, XMFLOAT3 at, XMFLOAT3 up)
    {
        XMFLOAT4X4 m;
        XMStoreFloat4x4(&m,
                        XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&at), XMLoadFloat3(&up)));
        return m;
    }

    inline XMFLOAT4X4 LookToLH(XMFLOAT3 eye, XMFLOAT3 look, XMFLOAT3 up)
    {
        XMFLOAT4X4 m;
        XMStoreFloat4x4(&m,
                        XMMatrixLookToLH(XMLoadFloat3(&eye), XMLoadFloat3(&look), XMLoadFloat3(&up)));
        return m;
    }
}
