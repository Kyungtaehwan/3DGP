#pragma once

// ============================================================
// Global window handle (3DGP-1 style)
// ============================================================
extern HWND g_hWnd;

// ============================================================
// Window / Frame Buffer
// ============================================================
#define WINCX  1280
#define WINCY   720

#define FRAME_BUFFER_WIDTH  WINCX
#define FRAME_BUFFER_HEIGHT WINCY

#define ASPECT_RATIO ((float)FRAME_BUFFER_WIDTH / (float)FRAME_BUFFER_HEIGHT)

// ============================================================
// Object event return codes
// ============================================================
#define OBJ_NOEVENT  0
#define OBJ_DEAD     1

// ============================================================
// Movement direction flags
// ============================================================
#define DIR_FORWARD   0x01
#define DIR_BACKWARD  0x02
#define DIR_LEFT      0x04
#define DIR_RIGHT     0x08
#define DIR_UP        0x10
#define DIR_DOWN      0x20

// ============================================================
// Math utilities
// ============================================================
#define DegreeToRadian(x) ((x) * (XM_PI / 180.0f))
#define EPSILON 1.0e-6f

#define RANDOM_COLOR XMFLOAT4((float)(rand()) / RAND_MAX, \
                               (float)(rand()) / RAND_MAX, \
                               (float)(rand()) / RAND_MAX, 1.0f)

inline bool IsZero(float f)  { return (fabsf(f) < EPSILON); }
inline bool IsEqual(float a, float b) { return IsZero(a - b); }

// ============================================================
// Enumerations
// ============================================================
enum LEVEL_ID
{
    LEVEL_LOGO = 0,
    LEVEL_MENU,
    LEVEL_GAMEPLAY,
    LEVEL_END
};

enum OBJ_ID
{
    OBJ_PLAYER = 0,
    OBJ_ENEMY,
    OBJ_PLAYER_BULLET,
    OBJ_ENEMY_BULLET,
    OBJ_END
};

// ============================================================
// Safe_Delete template
// ============================================================
template <typename T>
inline void Safe_Delete(T*& p)
{
    if (p) { delete p; p = nullptr; }
}

// ============================================================
// CDeleteMap – functor that deletes mapped pointer values
// ============================================================
class CDeleteMap
{
public:
    template <typename K, typename V>
    void operator()(std::pair<K, V*> pair) { delete pair.second; }
};

// ============================================================
// CTagFinder – functor for std::find_if by tag string
// ============================================================
class CTagFinder
{
public:
    explicit CTagFinder(const TCHAR* pszTag) : m_pszTag(pszTag) {}
    template <typename T>
    bool operator()(T* pObj) const
    {
        return (lstrcmp(m_pszTag, pObj->GetTag()) == 0);
    }
private:
    const TCHAR* m_pszTag;
};

// ============================================================
// Vector3 namespace
// ============================================================
namespace Vector3
{
    inline XMFLOAT3 XMVectorToFloat3(XMVECTOR v)
    {
        XMFLOAT3 f; XMStoreFloat3(&f, v); return f;
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

    inline float Distance(XMFLOAT3 a, XMFLOAT3 b)
    {
        return Length(Subtract(a, b));
    }

    inline float Angle(XMVECTOR a, XMVECTOR b)
    {
        XMFLOAT3 r;
        XMStoreFloat3(&r, XMVector3AngleBetweenNormals(a, b));
        return r.x;
    }

    inline float Angle(XMFLOAT3 a, XMFLOAT3 b)
    {
        return Angle(XMLoadFloat3(&a), XMLoadFloat3(&b));
    }

    inline XMFLOAT3 TransformNormal(XMFLOAT3 v, XMMATRIX m)
    {
        return XMVectorToFloat3(XMVector3TransformNormal(XMLoadFloat3(&v), m));
    }

    inline XMFLOAT3 TransformNormal(XMFLOAT3 v, XMFLOAT4X4 m)
    {
        return TransformNormal(v, XMLoadFloat4x4(&m));
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

// ============================================================
// Vector4 namespace
// ============================================================
namespace Vector4
{
    inline XMFLOAT4 Add(XMFLOAT4 a, XMFLOAT4 b)
    {
        XMFLOAT4 r;
        XMStoreFloat4(&r, XMVectorAdd(XMLoadFloat4(&a), XMLoadFloat4(&b)));
        return r;
    }
}

// ============================================================
// Matrix4x4 namespace
// ============================================================
namespace Matrix4x4
{
    inline XMFLOAT4X4 Identity()
    {
        XMFLOAT4X4 m;
        XMStoreFloat4x4(&m, XMMatrixIdentity());
        return m;
    }

    inline XMFLOAT4X4 Translate(float tx, float ty, float tz)
    {
        XMFLOAT4X4 m;
        XMStoreFloat4x4(&m, XMMatrixTranslation(tx, ty, tz));
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

    inline XMFLOAT4X4 RotationYawPitchRoll(float yaw, float pitch, float roll)
    {
        XMFLOAT4X4 m;
        XMStoreFloat4x4(&m, XMMatrixRotationRollPitchYaw(pitch, yaw, roll));
        return m;
    }

    inline XMFLOAT4X4 RotationAxis(XMFLOAT3 axis, float angle)
    {
        XMFLOAT4X4 m;
        XMStoreFloat4x4(&m, XMMatrixRotationAxis(XMLoadFloat3(&axis), angle));
        return m;
    }

    inline XMFLOAT4X4 Inverse(XMFLOAT4X4 m)
    {
        XMFLOAT4X4 r;
        XMStoreFloat4x4(&r, XMMatrixInverse(NULL, XMLoadFloat4x4(&m)));
        return r;
    }

    inline XMFLOAT4X4 Transpose(XMFLOAT4X4 m)
    {
        XMFLOAT4X4 r;
        XMStoreFloat4x4(&r, XMMatrixTranspose(XMLoadFloat4x4(&m)));
        return r;
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

// ============================================================
// Triangle namespace
// ============================================================
namespace Triangle
{
    inline bool Intersect(XMFLOAT3 p0, XMFLOAT3 p1, XMFLOAT3 p2,
                          XMFLOAT3 rayOrigin, XMFLOAT3 rayDir, float& dist)
    {
        XMVECTOR v0 = XMLoadFloat3(&p0);
        XMVECTOR v1 = XMLoadFloat3(&p1);
        XMVECTOR v2 = XMLoadFloat3(&p2);
        XMVECTOR ro = XMLoadFloat3(&rayOrigin);
        XMVECTOR rd = XMLoadFloat3(&rayDir);
        return TriangleTests::Intersects(ro, rd, v0, v1, v2, dist);
    }
}

// ============================================================
// Plane namespace
// ============================================================
namespace Plane
{
    inline XMFLOAT4 Normalize(XMFLOAT4 plane)
    {
        XMFLOAT4 r;
        XMStoreFloat4(&r, XMPlaneNormalize(XMLoadFloat4(&plane)));
        return r;
    }
}
