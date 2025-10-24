// Adapt these to your actual API, types, and headers.
#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <random>

#include "delaunay.h"

bool satisfies_empty_circle(const std::vector<tools_2D::point> pts, const std::vector<tools_2D::triangle>& triangles) 
{
    for (auto& triangle : triangles) 
    {
        const auto circum = triangle.circum_circle();
        for(const auto point : pts)
        {
            if(triangle.containsVertex(point))
                continue;
            if(circum.contains(point))
                return false;
        }
    }
    return true;
}

// ---------- Test data ----------
std::vector<tools_2D::point> square_4() 
{
    return 
    {
        {0.0, 0.0}, // 0
        {1.0, 0.0}, // 1
        {1.0, 1.0}, // 2
        {0.0, 1.0}  // 3
    };
}

std::vector<tools_2D::point> three_points_triangle() 
{
    return 
    {
        {0.0, 0.0}, // 0
        {1.0, 0.0}, // 1
        {0.0, 1.0}  // 2
    };
}

std::vector<tools_2D::point> collinear_4() 
{
    return 
    {
        {0.0, 0.0},
        {1.0, 0.0},
        {2.0, 0.0},
        {3.0, 0.0}
    };
}

std::vector<tools_2D::point> with_duplicates() 
{
    return 
    {
        {0.0, 0.0},
        {1.0, 0.0},
        {1.0, 0.0}, // duplicate
        {0.5, 0.8}
    };
}

std::vector<tools_2D::point> random_cloud(int n, unsigned seed = 42) 
{
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> dist(-10.0, 10.0);
    std::vector<tools_2D::point> pts; 
    pts.reserve(n);
    for (int i = 0; i < n; ++i) 
        pts.push_back({dist(rng), dist(rng)});
    return pts;
}

// ---------- Tests ----------

TEST_CASE("Basic", "[delaunay][basic]") 
{
    auto pts = three_points_triangle();
    auto triangles = delaunay_triangulate(pts);
    REQUIRE(triangles.size() == 1);

    // Empty-circle satisfied trivially with 3 points.
    REQUIRE(satisfies_empty_circle(pts, triangles));
}

TEST_CASE("Square", "[delaunay][square]") 
{
    auto pts = square_4();
    auto triangles = delaunay_triangulate(pts);

    // Two triangles expected.
    REQUIRE(triangles.size() == 2);
    REQUIRE(satisfies_empty_circle(pts, triangles));
}

TEST_CASE("Random cloud", "[delaunay][random_cloud]") 
{
    auto pts = random_cloud(50000, 2025);
    auto triangles = delaunay_triangulate(pts);
    REQUIRE(satisfies_empty_circle(pts, triangles));
}

TEST_CASE("Random cloud Benchmark", "[delaunay][benchmark]") 
{
    auto pts = random_cloud(500, 2025);
    BENCHMARK("delaunay_triangulate 500 points") 
    {
        return delaunay_triangulate(pts);
    };
}