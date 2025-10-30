//
// Decimate class using CGAL Triangulated Surface Mesh Simplification.
//
#pragma once

#include <lxsdk/lx_log.hpp>
#include <lxsdk/lx_mesh.hpp>
#include <lxsdk/lx_value.hpp>
#include <lxsdk/lxu_math.hpp>
#include <lxsdk/lxvmath.h>
#include <lxsdk/lxu_matrix.hpp>
#include <lxsdk/lxu_quaternion.hpp>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/segment.hpp>

#include <vector>
#include <unordered_set>

#include "util.hpp"

struct CDecimate
{
    enum ReductionMode : int
    {
        Ratio = 0,
        Count = 1,
    };

    enum CostStrategy : int
    {
        Edge_Length = 0,
        Lindstrom_Turk = 1,
        Garland_Heckbert = 2,
    };

    struct CVerx;
    struct CEdge;
    struct CTriangle;
    struct CFace;

    typedef std::shared_ptr<CVerx>     CVerxID;
    typedef std::shared_ptr<CEdge>     CEdgeID;
    typedef std::shared_ptr<CTriangle> CTriangleID;

    struct CVerx
    {
        CTriangleID                 tri;
        LXtPointID                  vrt;
        unsigned                    vrt_index;
        unsigned                    index;
        LXtVector                   pos;        // vertex position
        LXtVector                   new_pos;       // new vertex position
        void*                       userData;   // any working data
        LXtMarkMode                 marks;      // marks for working
        std::vector<CEdgeID>        edge;       // connecting edges
        std::vector<CTriangleID>    tris;       // connecting triangles
        bool                        collapsed;  // vertex collapsed flag 
    };

    struct CEdge
    {
        CVerxID                     v0, v1;  // vertex 1,2
        std::vector<CTriangleID>    tris;       // connecting triangles
        bool                        collapsed;  // edge collapsed flag
    };

    struct CTriangle
    {
        LXtPolygonID                pol;
        int                         pol_index;
        unsigned                    index;
        CVerxID                     v0, v1, v2;
        CEdgeID                     edge;      // an edge
        bool                        deleted;   // triangle deleted flag
        bool                        updated;   // triangle updated flag
    };

    struct CFace
    {
        std::vector<CTriangleID>    tris = {};  // triangles of the face
    };

    std::vector<CEdgeID>     m_edges;
    std::vector<CVerxID>     m_vertices;
    std::vector<CTriangleID> m_triangles;

    std::unordered_map<LXtPolygonID, CFace> m_faces;

    CLxUser_Mesh        m_mesh;
    CLxUser_Polygon     m_poly;
    CLxUser_Point       m_vert;
    CLxUser_MeshMap     m_vmap;
    CLxUser_PolygonEdit m_poledit;
    CLxUser_LogService  s_log;
    CLxUser_MeshService s_mesh;

    LXtMarkMode m_pick;
    LXtMarkMode m_mark_done;
    LXtMarkMode m_mark_seam;
    LXtMarkMode m_mark_hide;
    LXtMarkMode m_mark_lock;

    double m_ratio;     // Reduce by ratio of total polygons
    int    m_count;     // Number of polygons to reduce
    int    m_mode;      // Reduction mode
    int    m_cost;
    int    m_preserveBoundary;
    int    m_preserveMaterial;
    int    m_triple;

    CDecimate()
    {
        m_mode  = CDecimate::Ratio;
        m_ratio = 1.0;
        m_count = 0;
        m_triple = 0;

        CLxUser_MeshService mesh_svc;
        m_pick      = mesh_svc.SetMode(LXsMARK_SELECT);
        m_mark_done = mesh_svc.SetMode(LXsMARK_USER_0);
        m_mark_seam = mesh_svc.SetMode(LXsMARK_USER_1);
        m_mark_hide = mesh_svc.SetMode(LXsMARK_HIDE);
        m_mark_lock = mesh_svc.SetMode(LXsMARK_LOCK);
    }

    //
    // Collapse edges
    //
    LxResult DecimateMesh (CLxUser_Mesh& base_mesh);

    //
    // Build internal mesh representation
    //
    LxResult        BuildMesh(CLxUser_Mesh& base_mesh);
    LXtPolygonID    TracePolygon(LXtPointID vrt, LXtPolygonID pol, int shift);
    CTriangleID     FetchTriangle(LXtPointID vrt, LXtPolygonID pol);
    CVerxID         FetchVertex(LXtPointID vrt);
    LxResult        AddTriangle(LXtPolygonID pol, LXtPointID v0, LXtPointID v1, LXtPointID v2);
    LxResult        AddPolygon(LXtPolygonID pol);
    CVerxID         AddVertex(LXtPointID vrt, LXtPolygonID pol, CTriangleID tri);
    LxResult        AddEdge(CVerxID v0, CVerxID v1, CTriangleID tri);
    LxResult        CollapseEdge(int verx0_index, int verx1_index, bool forward);
    CEdgeID         FetchEdge(CVerxID v0, CVerxID v1);
    bool            FaceIsUpdated(CFace& face);
    LxResult        GetPointsFromFace(CFace& face, std::vector<LXtPointID>& points, unsigned int& rev);
    LxResult        ApplyMesh(CLxUser_Mesh& edit_mesh);
    LxResult        WriteMesh(CLxUser_Mesh& out_mesh);
};