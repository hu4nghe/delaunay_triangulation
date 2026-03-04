// Adapt these to your actual API, types, and headers.
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>
#include <random>
#include <chrono>
#include <iostream>


#include "geo_2D/triangulation_2D.h"
#include "geo_3D/triangulation_3D.h"
#include "geo_3D/surface_mesh_3D.h"

bool satisfies_empty_circle(
    const std::vector<tools_2D::point>     pts,
    const std::vector<tools_2D::triangle>& triangles)
{
    for (auto& triangle : triangles)
    {
        const auto circum = triangle.circum_circle();
        for (const auto point : pts)
        {
            if (triangle.containsVertex(point)) continue;
            if (circum.contains(point)) return false;
        }
    }
    return true;
}

// ---------- Test data ----------
std::vector<tools_2D::point> square_4()
{
    return {
        {0.0, 0.0}, // 0
        {1.0, 0.0}, // 1
        {1.0, 1.0}, // 2
        {0.0, 1.0}  // 3
    };
}

std::vector<tools_2D::point> three_points_triangle()
{
    return {
        {0.0, 0.0}, // 0
        {1.0, 0.0}, // 1
        {0.0, 1.0}  // 2
    };
}

std::vector<tools_2D::point> collinear_4()
{
    return {{0.0, 0.0}, {1.0, 0.0}, {2.0, 0.0}, {3.0, 0.0}};
}

std::vector<tools_2D::point> with_duplicates()
{
    return {
        {0.0, 0.0},
        {1.0, 0.0},
        {1.0, 0.0}, // duplicate
        {0.5, 0.8}};
}

std::vector<tools_2D::point> random_cloud_2d(
    int      n,
    unsigned seed = 42)
{
    std::mt19937                           rng(seed);
    std::uniform_real_distribution<double> dist(-10.0, 10.0);
    std::vector<tools_2D::point>           pts;
    pts.reserve(n);
    for (int i = 0; i < n; ++i) pts.push_back({dist(rng), dist(rng)});
    return pts;
}

std::vector<tools_3D::point> random_cloud_3d(
    int      n,
    unsigned seed = 42)
{
    std::mt19937                           rng(seed);
    std::uniform_real_distribution<double> dist(-10.0, 10.0);
    std::vector<tools_3D::point>           pts;
    pts.reserve(n);
    for (int i = 0; i < n; ++i) pts.emplace_back(dist(rng), dist(rng), dist(rng));
    return pts;
}

// ---------- Tests ----------

TEST_CASE(
    "Basic",
    "[delaunay][basic]")
{
    auto pts       = three_points_triangle();
    auto triangles = tools_2D::boyer_watson_2D(pts);
    REQUIRE(triangles.size() == 1);

    // Empty-circle satisfied trivially with 3 points.
    REQUIRE(satisfies_empty_circle(pts, triangles));
}

TEST_CASE(
    "Square",
    "[delaunay][square]")
{
    auto pts       = square_4();
    auto triangles = tools_2D::boyer_watson_2D(pts);

    // Two triangles expected.
    REQUIRE(triangles.size() == 2);
    REQUIRE(satisfies_empty_circle(pts, triangles));
}

TEST_CASE(
    "Random cloud",
    "[delaunay][random_cloud]")
{
    auto pts       = random_cloud_2d(500, 2025);
    auto triangles = tools_2D::boyer_watson_2D(pts);
    REQUIRE(satisfies_empty_circle(pts, triangles));
}

TEST_CASE(
    "Guibas-Stolfi API",
    "[delaunay][guibas_stolfi]")
{
    auto pts       = random_cloud_2d(500, 2025);
    auto triangles = tools_2D::guibas_stolfi_2D(pts);
    REQUIRE(satisfies_empty_circle(pts, triangles));
}

TEST_CASE(
    "Random cloud Benchmark",
    "[delaunay][benchmark]")
{
    auto pts = random_cloud_2d(500, 2025);
    BENCHMARK("boyer_watson_2D 500 points")
    {
        return tools_2D::boyer_watson_2D(pts);
    };
}

TEST_CASE(
    "Random cloud Benchmark Guibas-Stolfi",
    "[delaunay][benchmark]")
{
    auto pts = random_cloud_2d(500, 2025);
    BENCHMARK("guibas_stolfi_2D 500 points")
    {
        return tools_2D::guibas_stolfi_2D(pts);
    };
}

// ---------- 3D Performance Tests ----------

TEST_CASE(
    "3D Triangulation Performance Test",
    "[performance][3d]")
{
    SECTION("1000 points - Bowyer-Watson 3D")
    {
        auto pts = random_cloud_3d(1000, 2026);
        
        auto start = std::chrono::high_resolution_clock::now();
        auto tetrahedra = tools_3D::bowyer_watson_3D(pts);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        std::cout << "\n[PERF] Bowyer-Watson 3D - 1000 points: " << duration << " ms" 
                  << " (" << tetrahedra.size() << " tetrahedra)" << std::endl;
        
        REQUIRE(!tetrahedra.empty());
    }
    
    SECTION("5000 points - Bowyer-Watson 3D")
    {
        auto pts = random_cloud_3d(5000, 2026);
        
        auto start = std::chrono::high_resolution_clock::now();
        auto tetrahedra = tools_3D::bowyer_watson_3D(pts);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        std::cout << "\n[PERF] Bowyer-Watson 3D - 5000 points: " << duration << " ms" 
                  << " (" << tetrahedra.size() << " tetrahedra)" << std::endl;
        
        REQUIRE(!tetrahedra.empty());
    }
    
    SECTION("10000 points - Bowyer-Watson 3D")
    {
        auto pts = random_cloud_3d(10000, 2026);
        
        auto start = std::chrono::high_resolution_clock::now();
        auto tetrahedra = tools_3D::bowyer_watson_3D(pts);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        std::cout << "\n[PERF] Bowyer-Watson 3D - 10000 points: " << duration << " ms" 
                  << " (" << tetrahedra.size() << " tetrahedra)" << std::endl;
        
        REQUIRE(!tetrahedra.empty());
    }
    
    SECTION("10000 points - Full Pipeline (Triangulation + Surface Extraction)")
    {
        auto pts = random_cloud_3d(10000, 2026);
        
        auto start = std::chrono::high_resolution_clock::now();
        auto tetrahedra = tools_3D::bowyer_watson_3D(pts);
        auto surface = tools_3D::extract_boundary_surface(tetrahedra);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto triangulation_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        std::cout << "\n[PERF] Full Pipeline - 10000 points:" << std::endl;
        std::cout << "  - Triangulation: " << triangulation_duration << " ms (" << tetrahedra.size() << " tetrahedra)" << std::endl;
        std::cout << "  - Surface mesh: " << surface.vertices.size() << " vertices, " 
                  << surface.triangles.size() << " triangles" << std::endl;
        
        REQUIRE(!tetrahedra.empty());
        REQUIRE(!surface.vertices.empty());
    }
}

TEST_CASE(
    "3D Benchmark - Catch2 Benchmark",
    "[benchmark][3d]")
{
    auto pts = random_cloud_3d(1000, 2026);
    
    BENCHMARK("Bowyer-Watson 3D - 1000 points")
    {
        return tools_3D::bowyer_watson_3D(pts);
    };
}

TEST_CASE(
    "2D Performance Test - Different Point Counts",
    "[performance][2d]")
{
    SECTION("100 points - Bowyer-Watson 2D")
    {
        auto pts = random_cloud_2d(100, 2026);
        
        auto start = std::chrono::high_resolution_clock::now();
        auto triangles = tools_2D::boyer_watson_2D(pts);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        std::cout << "\n[PERF] Bowyer-Watson 2D - 100 points: " << duration << " ms (" 
                  << triangles.size() << " triangles)" << std::endl;
        
        REQUIRE(!triangles.empty());
    }
    
    SECTION("1000 points - Bowyer-Watson 2D")
    {
        auto pts = random_cloud_2d(1000, 2026);
        
        auto start = std::chrono::high_resolution_clock::now();
        auto triangles = tools_2D::boyer_watson_2D(pts);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        std::cout << "\n[PERF] Bowyer-Watson 2D - 1000 points: " << duration << " ms (" 
                  << triangles.size() << " triangles)" << std::endl;
        
        REQUIRE(!triangles.empty());
    }
    
    SECTION("10000 points - Bowyer-Watson 2D")
    {
        auto pts = random_cloud_2d(10000, 2026);
        
        auto start = std::chrono::high_resolution_clock::now();
        auto triangles = tools_2D::boyer_watson_2D(pts);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        std::cout << "\n[PERF] Bowyer-Watson 2D - 10000 points: " << duration << " ms (" 
                  << triangles.size() << " triangles)" << std::endl;
        
        REQUIRE(!triangles.empty());
    }
    
    SECTION("10000 points - Guibas-Stolfi 2D")
    {
        auto pts = random_cloud_2d(10000, 2026);
        
        auto start = std::chrono::high_resolution_clock::now();
        auto triangles = tools_2D::guibas_stolfi_2D(pts);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        std::cout << "\n[PERF] Guibas-Stolfi 2D - 10000 points: " << duration << " ms (" 
                  << triangles.size() << " triangles)" << std::endl;
        
        REQUIRE(!triangles.empty());
    }
}
