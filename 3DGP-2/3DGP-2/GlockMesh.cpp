#include "pch.h"
#include "Mesh.h"

// Simplified Glock: 4 boxes, 32 verts, 144 indices  (2x scale for visibility)
//
// Local axes:  +Z = barrel/forward,  +Y = up,  X = width
//
//  y=+0.16  [=========== SLIDE (Box0) ===========][BARREL(Box3)]
//  y=+0.04  |                                     |
//           [GRIP(Box1)][== TRIGGER GUARD (Box2) ==]
//  y=-0.13  [    ]
//           z=-0.10  z=0.00    z=0.32  z=0.44
//
// Origin (0,0,0) = grip/frame junction

CGlockMesh::CGlockMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
    const float sR=0.20f,sG=0.20f,sB=0.22f; // slide  : dark gunmetal
    const float gR=0.15f,gG=0.13f,gB=0.12f; // grip   : dark polymer
    const float tR=0.20f,tG=0.20f,tB=0.22f; // guard  : gunmetal
    const float bR=0.10f,bG=0.10f,bB=0.12f; // barrel : darker metal

    // Vertex layout per box (offset B):
    //   B+0(-x,bot,minZ)  B+1(+x,bot,minZ)  B+2(+x,top,minZ)  B+3(-x,top,minZ)
    //   B+4(+x,bot,maxZ)  B+5(-x,bot,maxZ)  B+6(-x,top,maxZ)  B+7(+x,top,maxZ)
    CVertex pVertices[32] =
    {
        // Box 0 — Slide   x[-0.030,+0.030]  y[+0.040,+0.160]  z[-0.040,+0.320]
        CVertex(-0.030f,  0.040f, -0.040f,  sR,sG,sB), // 0
        CVertex( 0.030f,  0.040f, -0.040f,  sR,sG,sB), // 1
        CVertex( 0.030f,  0.160f, -0.040f,  sR,sG,sB), // 2
        CVertex(-0.030f,  0.160f, -0.040f,  sR,sG,sB), // 3
        CVertex( 0.030f,  0.040f,  0.320f,  sR,sG,sB), // 4
        CVertex(-0.030f,  0.040f,  0.320f,  sR,sG,sB), // 5
        CVertex(-0.030f,  0.160f,  0.320f,  sR,sG,sB), // 6
        CVertex( 0.030f,  0.160f,  0.320f,  sR,sG,sB), // 7

        // Box 1 — Grip    x[-0.030,+0.030]  y[-0.130,+0.040]  z[-0.100,+0.000]
        CVertex(-0.030f, -0.130f, -0.100f,  gR,gG,gB), // 8
        CVertex( 0.030f, -0.130f, -0.100f,  gR,gG,gB), // 9
        CVertex( 0.030f,  0.040f, -0.100f,  gR,gG,gB), // 10
        CVertex(-0.030f,  0.040f, -0.100f,  gR,gG,gB), // 11
        CVertex( 0.030f, -0.130f,  0.000f,  gR,gG,gB), // 12
        CVertex(-0.030f, -0.130f,  0.000f,  gR,gG,gB), // 13
        CVertex(-0.030f,  0.040f,  0.000f,  gR,gG,gB), // 14
        CVertex( 0.030f,  0.040f,  0.000f,  gR,gG,gB), // 15

        // Box 2 — Trigger Guard  x[-0.030,+0.030]  y[-0.030,+0.040]  z[+0.000,+0.180]
        CVertex(-0.030f, -0.030f,  0.000f,  tR,tG,tB), // 16
        CVertex( 0.030f, -0.030f,  0.000f,  tR,tG,tB), // 17
        CVertex( 0.030f,  0.040f,  0.000f,  tR,tG,tB), // 18
        CVertex(-0.030f,  0.040f,  0.000f,  tR,tG,tB), // 19
        CVertex( 0.030f, -0.030f,  0.180f,  tR,tG,tB), // 20
        CVertex(-0.030f, -0.030f,  0.180f,  tR,tG,tB), // 21
        CVertex(-0.030f,  0.040f,  0.180f,  tR,tG,tB), // 22
        CVertex( 0.030f,  0.040f,  0.180f,  tR,tG,tB), // 23

        // Box 3 — Barrel   x[-0.016,+0.016]  y[+0.060,+0.120]  z[+0.320,+0.440]
        CVertex(-0.016f,  0.060f,  0.320f,  bR,bG,bB), // 24
        CVertex( 0.016f,  0.060f,  0.320f,  bR,bG,bB), // 25
        CVertex( 0.016f,  0.120f,  0.320f,  bR,bG,bB), // 26
        CVertex(-0.016f,  0.120f,  0.320f,  bR,bG,bB), // 27
        CVertex( 0.016f,  0.060f,  0.440f,  bR,bG,bB), // 28
        CVertex(-0.016f,  0.060f,  0.440f,  bR,bG,bB), // 29
        CVertex(-0.016f,  0.120f,  0.440f,  bR,bG,bB), // 30
        CVertex( 0.016f,  0.120f,  0.440f,  bR,bG,bB), // 31
    };

    // CCW winding for DX LH — same pattern as CCubeMesh per box
    UINT pIndices[144] =
    {
        // Box 0 — Slide
         0, 1, 2,   0, 2, 3,    4, 5, 6,   4, 6, 7,
         5, 0, 3,   5, 3, 6,    1, 4, 7,   1, 7, 2,
         3, 2, 7,   3, 7, 6,    5, 4, 1,   5, 1, 0,
        // Box 1 — Grip
         8, 9,10,   8,10,11,   12,13,14,  12,14,15,
        13, 8,11,  13,11,14,    9,12,15,   9,15,10,
        11,10,15,  11,15,14,   13,12, 9,  13, 9, 8,
        // Box 2 — Trigger Guard
        16,17,18,  16,18,19,   20,21,22,  20,22,23,
        21,16,19,  21,19,22,   17,20,23,  17,23,18,
        19,18,23,  19,23,22,   21,20,17,  21,17,16,
        // Box 3 — Barrel
        24,25,26,  24,26,27,   28,29,30,  28,30,31,
        29,24,27,  29,27,30,   25,28,31,  25,31,26,
        27,26,31,  27,31,30,   29,28,25,  29,25,24,
    };

    m_nVertices = 32;
    m_nIndices  = 144;
    m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    m_pd3dVertexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList,
        pVertices, sizeof(CVertex) * m_nVertices,
        D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
        &m_pd3dVertexUploadBuffer);
    m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
    m_d3dVertexBufferView.StrideInBytes  = sizeof(CVertex);
    m_d3dVertexBufferView.SizeInBytes    = sizeof(CVertex) * m_nVertices;

    m_pd3dIndexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList,
        pIndices, sizeof(UINT) * m_nIndices,
        D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER,
        &m_pd3dIndexUploadBuffer);
    m_d3dIndexBufferView.BufferLocation = m_pd3dIndexBuffer->GetGPUVirtualAddress();
    m_d3dIndexBufferView.Format         = DXGI_FORMAT_R32_UINT;
    m_d3dIndexBufferView.SizeInBytes    = sizeof(UINT) * m_nIndices;
}

CGlockMesh::~CGlockMesh() {}
void CGlockMesh::ReleaseUploadBuffers() { CMesh::ReleaseUploadBuffers(); }
