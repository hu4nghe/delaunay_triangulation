#include <catch2/catch_test_macros.hpp>

#include <random>
#include <vector>

#include "geo_3D/predicates_3D.h"
#include "geo_3D/surface_mesh_3D.h"
#include "geo_3D/triangulation_3D.h"

namespace
{
    auto random_cloud_3d(
        int      n,
        unsigned seed = 42) -> std::vector<tools_3D::point>
    {
        std::mt19937                           rng(seed);
        std::uniform_real_distribution<double> dist(-10.0, 10.0);

        std::vector<tools_3D::point> pts;
        pts.reserve(n);
        for (int i = 0; i < n; ++i)
            pts.emplace_back(dist(rng), dist(rng), dist(rng));
        return pts;
    }

    auto satisfies_empty_sphere(
        const std::vector<tools_3D::point>&       pts,
        const std::vector<tools_3D::tetrahedron>& tetras) -> bool
    {
        for (const auto& tet : tetras)
        {
            const auto& v = tet.vertices();
            for (const auto& p : pts)
            {
                if (tet.contains_vertex(p)) continue;

                if (tools_3D::in_sphere(v[0], v[1], v[2], v[3], p))
                    return false;
            }
        }
        return true;
    }
} // namespace

TEST_CASE(
    "3D Bowyer-Watson basic tetra",
    "[delaunay][3d][bowyer]")
{
    std::vector<tools_3D::point> pts{
        {0.0, 0.0, 0.0},
        {1.0, 0.0, 0.0},
        {0.0, 1.0, 0.0},
        {0.0, 0.0, 1.0}};

    auto tetras = tools_3D::bowyer_watson_3D(pts);
    REQUIRE_FALSE(tetras.empty());
    REQUIRE(satisfies_empty_sphere(pts, tetras));
}

TEST_CASE(
    "3D Bowyer-Watson random cloud",
    "[delaunay][3d][bowyer]")
{
    auto pts    = random_cloud_3d(40, 2026);
    auto tetras = tools_3D::bowyer_watson_3D(pts);
    REQUIRE_FALSE(tetras.empty());
    REQUIRE(satisfies_empty_sphere(pts, tetras));
}

TEST_CASE(
    "3D Guibas-Stolfi API random cloud",
    "[delaunay][3d][guibas]")
{
    auto pts    = random_cloud_3d(40, 2027);
    auto tetras = tools_3D::guibas_stolfi_3D(pts);
    REQUIRE_FALSE(tetras.empty());
    REQUIRE(satisfies_empty_sphere(pts, tetras));
}

TEST_CASE(
    "3D boundary surface extraction single tetra",
    "[delaunay][3d][surface]")
{
    const tools_3D::tetrahedron single_tetra(
        tools_3D::point(0.0, 0.0, 0.0),
        tools_3D::point(1.0, 0.0, 0.0),
        tools_3D::point(0.0, 1.0, 0.0),
        tools_3D::point(0.0, 0.0, 1.0));

    auto mesh = tools_3D::extract_boundary_surface({single_tetra});
    REQUIRE(mesh.vertices.size() == 4);
    REQUIRE(mesh.triangles.size() == 4);
}

TEST_CASE(
    "3D boundary surface extraction shared face",
    "[delaunay][3d][surface]")
{
    const tools_3D::tetrahedron tetrahedron_a(
        tools_3D::point(0.0, 0.0, 0.0),
        tools_3D::point(1.0, 0.0, 0.0),
        tools_3D::point(0.0, 1.0, 0.0),
        tools_3D::point(0.0, 0.0, 1.0));

    const tools_3D::tetrahedron tetrahedron_b(
        tools_3D::point(0.0, 0.0, 0.0),
        tools_3D::point(1.0, 0.0, 0.0),
        tools_3D::point(0.0, 1.0, 0.0),
        tools_3D::point(0.0, 0.0, -1.0));

    auto mesh =
        tools_3D::extract_boundary_surface({tetrahedron_a, tetrahedron_b});
    REQUIRE(mesh.triangles.size() == 6);
}
