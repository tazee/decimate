//
// Decimate class using CGAL Triangulated Surface Mesh Simplification.
//
#include "lxsdk/lxresult.h"
#include "lxsdk/lxvmath.h"
#include <lxsdk/lx_mesh.hpp>
#include <lxsdk/lx_value.hpp>
#include <lxsdk/lxu_math.hpp>
#include <lxsdk/lxu_matrix.hpp>

#include <vector>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <iostream>

#include <CGAL/Simple_cartesian.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Surface_mesh_simplification/edge_collapse.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Edge_count_ratio_stop_predicate.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Count_stop_predicate.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Edge_length_cost.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Midpoint_placement.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/GarlandHeckbert_policies.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/LindstromTurk_cost.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/LindstromTurk_placement.h>
#include <CGAL/Surface_mesh_simplification/Edge_collapse_visitor_base.h>

#include "decimate.hpp"
#include "triangulate.hpp"

//
// Edge Collapse class.
//
typedef CGAL::Simple_cartesian<double>                          Kernel;
typedef Kernel::FT                                              FT;
typedef Kernel::Point_3                                         Point_3;
typedef CGAL::Surface_mesh<Point_3>                             Surface_mesh;
 
namespace SMS = CGAL::Surface_mesh_simplification;
 
typedef SMS::GarlandHeckbert_plane_policies<Surface_mesh, Kernel>                  GHPolicies;
typedef SMS::GarlandHeckbert_probabilistic_plane_policies<Surface_mesh, Kernel>    Prob_plane;
typedef SMS::GarlandHeckbert_triangle_policies<Surface_mesh, Kernel>               Classic_tri;
typedef SMS::GarlandHeckbert_probabilistic_triangle_policies<Surface_mesh, Kernel> Prob_tri;

//
// Convert the internal CDecimate mesh representation to a CGAL Surface_mesh.
//
static void ConvertToCGALMesh(Surface_mesh& out_mesh, std::map<Surface_mesh::Edge_index, bool>& constrained_edges, CDecimate* context)
{
    std::unordered_map<CDecimate::CVerxID, Surface_mesh::Vertex_index> vertex_map;

    for (auto& v : context->m_vertices)
    {
        Point_3 p(v->pos[0], v->pos[1], v->pos[2]);
        Surface_mesh::Vertex_index vi = out_mesh.add_vertex(p);
        vertex_map[v] = vi;
    }

    for (auto& tri : context->m_triangles)
    {
        Surface_mesh::Vertex_index v0 = vertex_map[tri->v0];
        Surface_mesh::Vertex_index v1 = vertex_map[tri->v1];
        Surface_mesh::Vertex_index v2 = vertex_map[tri->v2];
        out_mesh.add_face(v0, v1, v2);
    }

    CLxUser_Edge    uedge;
    uedge.fromMesh(context->m_mesh);

    CLxUser_Polygon upoly0, upoly1;
    upoly0.fromMesh(context->m_mesh);
    upoly1.fromMesh(context->m_mesh);

    printf("Total edges in CGAL mesh: %lu material (%d)\n", static_cast<unsigned long>(out_mesh.number_of_edges()), context->m_preserveMaterial);
    for (auto e : out_mesh.edges())
    {
        // ここでは全てのエッジを非制約エッジとする例
        constrained_edges[e] = false;
        auto he = out_mesh.halfedge(e);
        auto i0 = out_mesh.source(he);
        auto i1 = out_mesh.target(he);
        auto v0 = context->m_vertices[i0]->vrt;
        auto v1 = context->m_vertices[i1]->vrt;
        uedge.SelectEndpoints(v0, v1);
        if (uedge.TestMarks(context->m_mark_lock) == LXe_TRUE)
        {
            constrained_edges[e] = true;
            continue;
        }
        if (context->m_preserveBoundary)
        {
            if (uedge.IsBorder() == LXe_TRUE)
            {
                constrained_edges[e] = true;
                continue;
            }
        }
        if (context->m_preserveMaterial)
        {
            unsigned int count;
            uedge.PolygonCount(&count);
            if (count != 2)
                continue;
            LXtPolygonID pol;
            uedge.PolygonByIndex(0, &pol);
            upoly0.Select(pol);
            uedge.PolygonByIndex(1, &pol);
            upoly1.Select(pol);
            CLxUser_StringTag tag0, tag1;
            tag0.set(upoly0);
            tag1.set(upoly1);
            const char *mat0 = tag0.Value(LXi_PTAG_MATR);
            const char *mat1 = tag1.Value(LXi_PTAG_MATR);
            if (std::string(mat0) != std::string(mat1))
            {
                constrained_edges[e] = true;
                continue;
            }
        }
    }
}

static bool EquivalentPoints(CLxUser_Polygon& poly, std::vector<LXtPointID>& points)
{
    unsigned int npoints;
    poly.VertexCount(&npoints);
    if (npoints != points.size())
        return false;
    for (auto i = 0u; i < npoints; i++)
    {
        LXtPointID pntID;
        poly.VertexByIndex(i, &pntID);
        if (std::find(points.begin(), points.end(), pntID) == points.end())
            return false;
    }
    return true;
}

static void PrintCGALMesh(Surface_mesh& mesh)
{
    std::cout << "Vertices:" << std::endl;
    for (auto v : mesh.vertices())
    {
        Point_3 p = mesh.point(v);
        std::cout << "  Vertex " << v << ": (" << p.x() << ", " << p.y() << ", " << p.z() << ")" << std::endl;
    }

    std::cout << "Faces:" << std::endl;
    for (auto f : mesh.faces())
    {
        std::cout << "  Face " << f << ":";
        for (auto v : CGAL::vertices_around_face(mesh.halfedge(f), mesh))
        {
            std::cout << " " << v;
        }
        std::cout << std::endl;
    }
    for (auto e : mesh.edges())
    {
        auto he = mesh.halfedge(e);
        auto v0 = mesh.source(he);
        auto v1 = mesh.target(he);
        std::cout << "  Edge " << e << ": (" << v0 << " - " << v1 << ")" << std::endl;
    }
}

struct VertexMapVisitor : public SMS::Edge_collapse_visitor_base<Surface_mesh>
{
    // マップ：元の頂点 → 残った／統合された頂点
    std::vector<std::tuple<Surface_mesh::Vertex_index, Surface_mesh::Vertex_index, bool>>& vmap;

    VertexMapVisitor(std::vector<std::tuple<Surface_mesh::Vertex_index, Surface_mesh::Vertex_index, bool>>& _vmap)
      : vmap(_vmap) {}

    // 折りたたまれる直前
#if 0
    void OnCollapsing(const Profile& profile,
                    std::optional<Point> placement)
    {
        auto v0 = profile.v0();
        auto v1 = profile.v1();
        printf("Collapsing edge (%lu) - (%lu)\n", static_cast<unsigned long>(v0), static_cast<unsigned long>(v1));
        // v1 を v0 に統合する（例示：実際にどちらに残るかは policy による）
        //vmap[v1] = v0;
    }
#endif
    // 折りたたみ完了時
    void OnCollapsed(const Profile& profile, Surface_mesh::Vertex_index new_v)
    {
        auto v0 = profile.v0();
        auto v1 = profile.v1();
        bool forward = (new_v == v0);
        vmap.emplace_back(v0, v1, forward);
    }
};

//
// Decimmate the mesh by the given ratio.
//
LxResult CDecimate::DecimateMesh(CLxUser_Mesh& base_mesh)
{
    BuildMesh(base_mesh);

    Surface_mesh surface_mesh;
    std::map<Surface_mesh::Edge_index, bool> constrained_edges;
    ConvertToCGALMesh(surface_mesh, constrained_edges, this);
    //PrintCGALMesh(surface_mesh);

    std::vector<std::tuple<Surface_mesh::Vertex_index, Surface_mesh::Vertex_index, bool>> vertex_map;

    int target_count = static_cast<int>(surface_mesh.number_of_edges());

    if (m_mode == CDecimate::Ratio)
        target_count *= m_ratio;
    else if (m_mode == CDecimate::Count)
        target_count -= m_count;

    //SMS::Edge_count_ratio_stop_predicate<Surface_mesh> stop(m_ratio);
    SMS::Edge_count_stop_predicate<Surface_mesh> stop(target_count);

    // Visitor 登録
    VertexMapVisitor visitor(vertex_map);

    int r = 0;

    if (m_cost == CDecimate::Lindstrom_Turk)
    {
        std::cout << "Using Lindstrom-Turk cost and placement.\n";
        r = SMS::edge_collapse(
            surface_mesh,
            stop,
            CGAL::parameters::visitor(visitor)
                            .get_cost(SMS::LindstromTurk_cost<Surface_mesh>())
                            .get_placement(SMS::LindstromTurk_placement<Surface_mesh>())
                            .edge_is_constrained_map(boost::make_assoc_property_map(constrained_edges))
            );
    }
    else if (m_cost == CDecimate::Garland_Heckbert)
    {
        std::cout << "Using Garland-Heckbert cost and placement.\n";
        GHPolicies policies(surface_mesh);
        r = SMS::edge_collapse(
            surface_mesh,
            stop,
            CGAL::parameters::visitor(visitor)
                            .get_cost(policies.get_cost())
                            .get_placement(policies.get_placement())
                            .edge_is_constrained_map(boost::make_assoc_property_map(constrained_edges))
            );
    }
    else
    {
        std::cout << "Using Edge Length cost and Midpoint placement.\n";
        r = SMS::edge_collapse(
            surface_mesh,
            stop,
            CGAL::parameters::visitor(visitor)
                            .get_cost(SMS::Edge_length_cost<Surface_mesh>())
                            .get_placement(SMS::Midpoint_placement<Surface_mesh>())
                            .edge_is_constrained_map(boost::make_assoc_property_map(constrained_edges))
            );
    }
    std::cout << "\nFinished!\n" << r << " edges removed.\n" << surface_mesh.number_of_edges() << " final edges.\n";
    //PrintCGALMesh(surface_mesh);
    for (const auto& [v0, v1, forward] : vertex_map) {
        CollapseEdge(static_cast<int>(v0), static_cast<int>(v1), forward);
    }
    for (auto v : surface_mesh.vertices())
    {
        Point_3 p = surface_mesh.point(v);
        auto cv = m_vertices[static_cast<size_t>(v)];
        if (cv->collapsed)
            continue;
        cv->new_pos[0] = p.x();
        cv->new_pos[1] = p.y();
        cv->new_pos[2] = p.z();
    }
    for (auto& tri : m_triangles)
    {
        if (tri->deleted)
            continue;
        if (tri->v0 == tri->v1 || tri->v1 == tri->v2 || tri->v2 == tri->v0)
        {
            tri->deleted = true;
        }
    }
    return LXe_OK;
}

LxResult CDecimate::AddPolygon(LXtPolygonID pol)
{
    CFace face;
    m_faces[pol] = face;
    return LXe_OK;
}

LxResult CDecimate::AddTriangle(LXtPolygonID pol, LXtPointID v0, LXtPointID v1, LXtPointID v2)
{
    m_triangles.push_back(std::make_shared<CTriangle>());
    CTriangleID tri = m_triangles.back();
    tri->index = static_cast<int>(m_triangles.size()-1);

    CVerxID dv[3];
    dv[0] = AddVertex(v0, pol, tri);
    dv[1] = AddVertex(v1, pol, tri);
    dv[2] = AddVertex(v2, pol, tri);

    CLxUser_Edge edge;
    edge.fromMesh(m_mesh);

    if (pol)
    {
        CFace& face = m_faces[pol];
        face.tris.push_back(tri);
    }

    tri->v0   = dv[0];
    tri->v1   = dv[1];
    tri->v2   = dv[2];
    tri->pol  = pol;
    tri->updated = false;
    tri->deleted = false;

    AddEdge(dv[0], dv[1], tri);
    AddEdge(dv[1], dv[2], tri);
    AddEdge(dv[2], dv[0], tri);
    return LXe_OK;
}

LxResult CDecimate::AddEdge(CVerxID v0, CVerxID v1, CTriangleID tri)
{
    // Check if the edge already exists
    for (auto& edge : v0->edge)
    {
        if ((edge->v0 == v0 && edge->v1 == v1) || (edge->v0 == v1 && edge->v1 == v0))
        {
            edge->tris.push_back(tri);
            return LXe_OK;
        }
    }

    // Create a new edge
    m_edges.push_back(std::make_shared<CEdge>());
    CEdgeID edge = m_edges.back();
    edge->v0 = v0;
    edge->v1 = v1;
    edge->collapsed = false;
    edge->tris.push_back(tri);

    v0->edge.push_back(edge);
    v1->edge.push_back(edge);
    return LXe_OK;
}

LXtPolygonID CDecimate::TracePolygon(LXtPointID vrt, LXtPolygonID pol, int shift)
{
    CLxUser_Edge edge;
    edge.fromMesh(m_mesh);

    CLxUser_Polygon poly;
    poly.fromMesh(m_mesh);

    LXtPolygonID pol1;
    LXtPointID   vrt1;
    unsigned int npol, nvert;

    m_poly.Select(pol);
    m_poly.VertexCount(&nvert);

    for (auto i = 0u; i < nvert; i++)
    {
        m_poly.VertexByIndex(i, &vrt1);
        if (vrt1 == vrt)
        {
            m_poly.VertexByIndex((i + shift + nvert) % nvert, &vrt1);
            edge.SelectEndpoints(vrt, vrt1);
            edge.PolygonCount(&npol);
            for (auto j = 0u; j < npol; j++)
            {
                edge.PolygonByIndex(j, &pol1);
                poly.Select(pol1);
                if (poly.TestMarks(m_mark_hide) == LXe_TRUE)
                    continue;
                if (poly.TestMarks(m_mark_lock) == LXe_TRUE)
                    continue;
                if (pol1 != pol)
                {
                    return pol1;
                }
            }
            return nullptr;
        }
    }
    return nullptr;
}

// Find and get the CTriangle with the given vrt and pol.
CDecimate::CTriangleID CDecimate::FetchTriangle(LXtPointID vrt, LXtPolygonID pol)
{
    LXtPolygonID pol0 = pol;
    unsigned int npol;

    m_vert.Select(vrt);
    m_vert.PolygonCount(&npol);

    if (m_faces.find(pol) != m_faces.end())
    {
        CFace& face = m_faces[pol];
        for (auto& tri : face.tris)
        {
            if (tri->v0->vrt == vrt || tri->v1->vrt == vrt || tri->v2->vrt == vrt)
                return tri;
        }
    }
    npol--;

    pol = pol0;
    while (npol > 0)
    {
        pol = TracePolygon(vrt, pol, +1);
        if (!pol)
            break;
        if (pol == pol0)
            break;
        if (m_faces.find(pol) != m_faces.end())
        {
            CFace& face = m_faces[pol];
            for (auto& tri : face.tris)
            {
                if (tri->v0->vrt == vrt || tri->v1->vrt == vrt || tri->v2->vrt == vrt)
                    return tri;
            }
        }
        npol--;
    }

    pol = pol0;
    while (npol > 0)
    {
        pol = TracePolygon(vrt, pol, -1);
        if (!pol)
            break;
        if (pol == pol0)
            break;
        if (m_faces.find(pol) != m_faces.end())
        {
            CFace& face = m_faces[pol];
            for (auto& tri : face.tris)
            {
                if (tri->v0->vrt == vrt || tri->v1->vrt == vrt || tri->v2->vrt == vrt)
                    return tri;
            }
        }
        npol--;
    }

    return nullptr;
}

CDecimate::CVerxID CDecimate::FetchVertex(LXtPointID vrt)
{
    m_vert.Select(vrt);
    unsigned count;
    m_vert.PolygonCount(&count);
    for (auto i = 0u; i < count; i++)
    {
        LXtPolygonID pol;
        m_vert.PolygonByIndex(i, &pol);
        CTriangleID ref = FetchTriangle(vrt, pol);
        if (ref)
        {
            if (ref->v0->vrt == vrt)
                return ref->v0;
            if (ref->v1->vrt == vrt)
                return ref->v1;
            if (ref->v2->vrt == vrt)
                return ref->v2;
        }
    }
    return nullptr;
}

CDecimate::CVerxID CDecimate::AddVertex(LXtPointID vrt, LXtPolygonID pol, CTriangleID tri)
{
    if (!pol)
    {
        return FetchVertex(vrt);
    }
    //printf("AddVertex vrt (%p) pol (%p)", vrt, pol);
    CTriangleID ref = FetchTriangle(vrt, pol);
    if (ref)
    {
        CVerxID dv = nullptr;

        if (ref->v0->vrt == vrt)
            dv = ref->v0;
        if (ref->v1->vrt == vrt)
            dv = ref->v1;
        if (ref->v2->vrt == vrt)
            dv = ref->v2;

        if (dv)
        {
            dv->tris.push_back(tri);
            return dv;
        }
    }

    m_vertices.push_back(std::make_shared<CVerx>());
    CVerxID dv = m_vertices.back();

    dv->tris.push_back(tri);
    dv->vrt   = vrt;
    dv->tri   = tri;
    dv->index = static_cast<unsigned>(m_vertices.size()-1);
    dv->marks = LXiMARK_ANY;
    dv->collapsed = false;
    m_vert.Select(vrt);
    LXtFVector pos;
    m_vert.Pos(pos);
    LXx_VCPY(dv->pos, pos);
    LXx_VCPY(dv->new_pos, pos);
    m_vert.Index(&dv->vrt_index);
    return dv;
}

// Visitor to build triangles from polygons
//
class TripleFaceVisitor : public CLxImpl_AbstractVisitor
{
public:
    LxResult Evaluate()
    {
        unsigned nvert;
        m_poly.VertexCount(&nvert);
        if (nvert < 3)
            return LXe_OK;

        m_poly.SetMarks(m_mark_done);

        LXtID4 type;
        m_poly.Type(&type);
        if ((type != LXiPTYP_FACE) && (type != LXiPTYP_PSUB) && (type != LXiPTYP_SUBD))
            return LXe_OK;

        m_context->AddPolygon(m_poly.ID());

        std::vector<LXtPointID> points;

        LXtPointID v0, v1, v2;
        if (nvert == 3)
        {
            m_poly.VertexByIndex(0, &v0);
            m_poly.VertexByIndex(1, &v1);
            m_poly.VertexByIndex(2, &v2);
            m_context->AddTriangle(m_poly.ID(), v0, v1, v2);
        }
        else if (MeshUtil::PolygonFixedVertexList(m_mesh, m_poly, points))
        {
            bool done = false;
            if (nvert > 4)
            {
                LXtVector norm;
                //MeshUtil::VertexListNormal(m_mesh, points, norm);
                m_poly.Normal(norm);
                AxisPlane axisPlane(norm);
                std::vector<std::vector<LXtPointID>> tris;
                CTriangulate ctri(m_mesh);
                LxResult result = LXe_OK;
                result = ctri.ConstraintDelaunay(axisPlane, points, tris);
                if (result == LXe_OK)
                {
                    for (auto& vert : tris)
                    {
                        m_context->AddTriangle(m_poly.ID(), vert[0], vert[1], vert[2]);
                    }
                    done = true;
                }
            }
            if (!done)
            {
                v0 = points[0];
                for (auto i = 1u; i < points.size()-1; i++)
                {
                    v1 = points[i];
                    v2 = points[i+1];
                    m_context->AddTriangle(m_poly.ID(), v0, v1, v2);
                }
            }
        }
        else
        {
            unsigned count;
            m_poly.GenerateTriangles(&count);
            for (auto i = 0u; i < count; i++)
            {
                m_poly.TriangleByIndex(i, &v0, &v1, &v2);
                m_context->AddTriangle(m_poly.ID(), v0, v1, v2);
            }
        }
        return LXe_OK;
    }

    CLxUser_Mesh    m_mesh;
    CLxUser_Polygon m_poly;
    CLxUser_Point   m_vert;
    LXtMarkMode     m_mark_done;
    struct CDecimate*  m_context;
};

// Build internal mesh representation
//
LxResult CDecimate::BuildMesh(CLxUser_Mesh& base_mesh)
{
    CLxUser_MeshService mesh_svc;
    TripleFaceVisitor triFace;

    m_mesh.set(base_mesh);
    m_poly.fromMesh(m_mesh);
    m_vert.fromMesh(m_mesh);
    m_vmap.fromMesh(m_mesh);

    triFace.m_mesh = m_mesh;
    triFace.m_poly.fromMesh(m_mesh);
    triFace.m_vert.fromMesh(m_mesh);
    triFace.m_mark_done = mesh_svc.ClearMode(LXsMARK_USER_0);
    triFace.m_context = this;
    triFace.m_poly.Enum(&triFace, LXiMARK_ANY);
    printf("Build mesh with %zu vertices %zu triangles %zu edges\n", m_vertices.size(), m_triangles.size(), m_edges.size());
    return LXe_OK;
}

// Write internal mesh representation back to mesh
//
LxResult CDecimate::WriteMesh(CLxUser_Mesh& out_mesh)
{
    //printf("Writing mesh with %zu vertices and %zu triangles\n", m_vertices.size(), m_triangles.size());
    std::vector<LXtPointID> point_ids(m_vertices.size());
    m_vert.fromMesh(out_mesh);
    for (auto& v : m_vertices)
    {
        if (v->collapsed)
            point_ids[v->index] = nullptr;
        else
        {
            LXtPointID new_vrt;
            LXtVector pos;
            LXx_VCPY(pos, v->new_pos);
            m_vert.New(pos, &new_vrt);
            point_ids[v->index] = new_vrt;
        }
    }
    m_poly.fromMesh(out_mesh);

    for (auto& tri : m_triangles)
    {
        if (tri->deleted)
            continue;

        unsigned int rev = 0;
        LXtPointID points[3];
        points[0] = point_ids[tri->v0->index];
        points[1] = point_ids[tri->v1->index];
        points[2] = point_ids[tri->v2->index];

        LXtPolygonID new_pol;
        m_poly.New(LXiPTYP_FACE, points, 3, rev, &new_pol);
    }
    return LXe_OK;
}

CDecimate::CEdgeID CDecimate::FetchEdge(CVerxID v0, CVerxID v1)
{
    for (auto& edge : v0->edge)
    {
        if ((edge->v0 == v0 && edge->v1 == v1) || (edge->v0 == v1 && edge->v1 == v0))
        {
            return edge;
        }
    }
    return nullptr;
}


LxResult CDecimate::CollapseEdge(int verx0_index, int verx1_index, bool forward)
{
    CVerxID v0 = m_vertices[verx0_index];
    CVerxID v1 = m_vertices[verx1_index];

    CEdgeID target_edge = FetchEdge(v0, v1);
    if (!target_edge)
        return LXe_FAILED;

    if (forward)
    {
        // Collapse v1 into v0
        for (auto& tri : v1->tris)
        {
            if (tri->v0 == v1)
                tri->v0 = v0;
            if (tri->v1 == v1)
                tri->v1 = v0;
            if (tri->v2 == v1)
                tri->v2 = v0;
            tri->updated = true;
            v0->tris.push_back(tri);
        }
        for (auto& edge : v1->edge)
        {
            if (edge->v0 == v1)
                edge->v0 = v0;
            if (edge->v1 == v1)
                edge->v1 = v0;
            CEdgeID edge1 = FetchEdge(edge->v0, edge->v1);
            if (edge1 != nullptr && edge1 != edge)
                edge->collapsed = true;
            else
                v0->edge.push_back(edge);
        }
        v1->collapsed = true;
    }
    else
    {
        // Collapse v0 into v1
        for (auto& tri : v0->tris)
        {
            if (tri->v0 == v0)
                tri->v0 = v1;
            if (tri->v1 == v0)
                tri->v1 = v1;
            if (tri->v2 == v0)
                tri->v2 = v1;
            tri->updated = true;
            v1->tris.push_back(tri);
        }
        for (auto& edge : v0->edge)
        {
            if (edge->v0 == v0)
                edge->v0 = v1;
            if (edge->v1 == v0)
                edge->v1 = v1;
            CEdgeID edge1 = FetchEdge(edge->v0, edge->v1);
            if (edge1 != nullptr && edge1 != edge)
                edge->collapsed = true;
            else
                v1->edge.push_back(edge);
        }
        v0->collapsed = true;
    }
    target_edge->collapsed = true;
    for (auto& tri : target_edge->tris)
    {
        tri->deleted = true;
        if (forward)
        {
            v1->tris.erase(std::remove(v1->tris.begin(), v1->tris.end(), tri), v1->tris.end());
        }
        else
        {
            v0->tris.erase(std::remove(v0->tris.begin(), v0->tris.end(), tri), v0->tris.end());
        }
        tri->deleted = true;
    }
    return LXe_OK;
}

LxResult CDecimate::ApplyMesh(CLxUser_Mesh& edit_mesh)
{
    m_mesh.set(edit_mesh);
    m_poly.fromMesh(m_mesh);
    m_vert.fromMesh(m_mesh);
    m_vmap.fromMesh(m_mesh);

    printf("ApplyMesh mesh with %zu vertices and %zu triangles\n", m_vertices.size(), m_triangles.size());
    for (auto& v : m_vertices)
    {
        m_vert.Select(v->vrt);
        if (v->collapsed)
            m_vert.Remove();
        else
            m_vert.SetPos(v->new_pos);
    }

    if (m_triple)
    {
        for (auto& tri : m_triangles)
        {
            if (tri->deleted)
                continue;

            unsigned int rev = 0;
            LXtPointID point_ids[3];
            point_ids[0] = tri->v0->vrt;
            point_ids[1] = tri->v1->vrt;
            point_ids[2] = tri->v2->vrt;

            LXtPolygonID new_pol;
            m_poly.NewProto(LXiPTYP_FACE, point_ids, 3, rev, &new_pol);
        }
        for (auto& face : m_faces)
        {
            m_poly.Select(face.first);
            m_poly.Remove();
        }
    }
    else
    {
        for (auto& face : m_faces)
        {
            unsigned int rev = 0;
            std::vector<LXtPointID> points = {};
            GetPointsFromFace(face.second, points, rev);
            m_poly.Select(face.first);
            if (points.size() < 3)
                m_poly.Remove();

            else if (FaceIsUpdated(face.second) == true)
            {
                m_poly.SetMarks(m_mark_done);
                m_poly.SetVertexList(points.data(), static_cast<unsigned>(points.size()), rev);
            }
        }
    }
    return LXe_OK;
}


bool CDecimate::FaceIsUpdated(CDecimate::CFace& face)
{
    for (auto& tri : face.tris)
    {
        if (tri->updated || tri->deleted)
            return true;
    }
    return false;
}


LxResult CDecimate::GetPointsFromFace(CDecimate::CFace& face, std::vector<LXtPointID>& points, unsigned int& rev)
{
    rev = 0;
    points.clear();
    for (auto& tri : face.tris)
    {
        if (tri->deleted)
            continue;
        if (points.empty())
        {
            points.push_back(tri->v0->vrt);
            points.push_back(tri->v1->vrt);
            points.push_back(tri->v2->vrt);
        }
        else
        {
            for (auto i = 0u; i < points.size(); i++)
            {
                LXtPointID vrt = nullptr;
                auto j = (i + 1) % points.size();
                if (points[i] == tri->v1->vrt && points[j] == tri->v0->vrt)
                {
                    vrt = tri->v2->vrt;
                }
                else if (points[i] == tri->v2->vrt && points[j] == tri->v1->vrt)
                {
                    vrt = tri->v0->vrt;
                }
                else if (points[i] == tri->v0->vrt && points[j] == tri->v2->vrt)
                {
                    vrt = tri->v1->vrt;
                }
                if (vrt != nullptr)
                {
                    points.insert(points.begin() + j, vrt);
                    break;
                }
            }
        }
    }
    if (points.size() > 2)
    {
        if (points.front() == points.back())
            points.pop_back();
    }
#if 0
    if (points.size() > 2)
    {
        LXtVector norm0;
        m_poly.Select(face.tris[0]->pol);
        m_poly.Normal(norm0);
        LXtFVector a, b, c, norm1;
        m_vert.Select(points[0]);
        m_vert.Pos(a);
        m_vert.Select(points[1]);
        m_vert.Pos(b);
        m_vert.Select(points.back());
        m_vert.Pos(c);
        LXx_VSUB(b, a);
        LXx_VSUB(c, a);
        LXx_VCROSS(norm1, b, c);
        if (LXx_VDOT(norm0, norm1) < 0.0)
        {
            rev = 1;
        }
    }
#endif
    return LXe_OK;
}

