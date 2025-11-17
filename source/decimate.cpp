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
    CMesh& cmesh = context->m_cmesh;

    std::unordered_map<CVerxID, Surface_mesh::Vertex_index> vertex_map;

    for (auto& v : cmesh.m_vertices)
    {
        Point_3 p(v->pos[0], v->pos[1], v->pos[2]);
        Surface_mesh::Vertex_index vi = out_mesh.add_vertex(p);
        vertex_map[v] = vi;
    }

    for (auto& tri : cmesh.m_triangles)
    {
        Surface_mesh::Vertex_index v0 = vertex_map[tri->v0];
        Surface_mesh::Vertex_index v1 = vertex_map[tri->v1];
        Surface_mesh::Vertex_index v2 = vertex_map[tri->v2];
        out_mesh.add_face(v0, v1, v2);
    }

    CLxUser_Edge    uedge;
    uedge.fromMesh(cmesh.m_mesh);

    CLxUser_Polygon upoly0, upoly1;
    upoly0.fromMesh(cmesh.m_mesh);
    upoly1.fromMesh(cmesh.m_mesh);

    printf("Total edges in CGAL mesh: %lu material (%d)\n", static_cast<unsigned long>(out_mesh.number_of_edges()), context->m_preserveMaterial);
    for (auto e : out_mesh.edges())
    {
        // ここでは全てのエッジを非制約エッジとする例
        constrained_edges[e] = false;
        auto he = out_mesh.halfedge(e);
        auto i0 = out_mesh.source(he);
        auto i1 = out_mesh.target(he);
        auto v0 = cmesh.m_vertices[i0]->vrt;
        auto v1 = cmesh.m_vertices[i1]->vrt;
        uedge.SelectEndpoints(v0, v1);
        if (uedge.TestMarks(cmesh.m_mark_lock) == LXe_TRUE)
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
    m_cmesh.BuildMesh(base_mesh);

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
        m_cmesh.CollapseEdge(static_cast<int>(v0), static_cast<int>(v1), forward);
    }
    for (auto v : surface_mesh.vertices())
    {
        Point_3 p = surface_mesh.point(v);
        auto cv = m_cmesh.m_vertices[static_cast<size_t>(v)];
        if (cv->collapsed)
            continue;
        cv->new_pos[0] = p.x();
        cv->new_pos[1] = p.y();
        cv->new_pos[2] = p.z();
    }
    for (auto& tri : m_cmesh.m_triangles)
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
