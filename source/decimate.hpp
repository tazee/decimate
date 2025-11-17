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
#include "cmesh.hpp"

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

    // source mesh context
    CMesh m_cmesh;

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
    }

    //
    // Collapse edges
    //
    LxResult DecimateMesh (CLxUser_Mesh& base_mesh);
};