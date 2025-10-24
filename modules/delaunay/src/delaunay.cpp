/**
 * @file delaunay.cpp
 * @author HUANG He (he.hu4ng@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2025-09-25
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "csv_parser.h"
#include "delaunay.h"

#include <ranges>
#include <functional>
#include <set>

auto super_triangle(const std::vector<tools_2D::point>& points)
{
    auto [min_x, max_x] = std::ranges::minmax(points | std::views::transform(std::mem_fn(&tools_2D::point::get_x)));
    auto [min_y, max_y] = std::ranges::minmax(points | std::views::transform(std::mem_fn(&tools_2D::point::get_y)));

    double cx = (min_x + max_x) / 2.0;
    double cy = (min_y + max_y) / 2.0;
    double L  = std::max(max_x - min_x, max_y - min_y);

    double alpha = 20.0;
    double S = std::max(1.0, alpha * L);

    tools_2D::point bl(cx - S/2.0, cy - S/2.0); // bottom-left
    tools_2D::point br(cx + S/2.0, cy - S/2.0); // bottom-right
    tools_2D::point tl(cx - S/2.0, cy + S/2.0); // top-left
    tools_2D::point tr(cx + S/2.0, cy + S/2.0); // top-right

    tools_2D::point top_mid(cx, cy + S/2.0);

    return std::tuple(bl, br, top_mid);
}

auto delaunay_triangulate(const std::vector<tools_2D::point>& points) -> std::vector<tools_2D::triangle>
{    
    // Construct a super tools_2D::triangle at first.
    auto [p1, p2, p3] = super_triangle(points);
    std::vector<tools_2D::triangle> all_triangles{tools_2D::triangle{p1, p2, p3}};

    for (const auto& p : points) 
    {
        std::set<tools_2D::segment> polygon;
        std::erase_if(
            all_triangles, 
            [&](const tools_2D::triangle& tri)
            {
                if (tri.circum_circle().contains(p))
                {
                    for (const auto& edge : tri.get_edges()) 
                        if (!polygon.erase(edge)) 
                            polygon.insert(edge);
                    return true;
                }
                return false;             
            });
                         
        //construct new triangles with current tools_2D::point and segments.                                
        for (const auto& edge : polygon)
            all_triangles.emplace_back(p, edge);
            
    }

    // Remove triangles that contain any of the super tools_2D::triangle vertices.
    std::erase_if(
        all_triangles,
        [&](const tools_2D::triangle& tri) 
        {
            for (const auto& p : {p1, p2, p3})
                if(tri.containsVertex(p)) 
                    return true;
            return false;
        });
                                        
    return all_triangles;
}

auto read_and_triangulate() -> std::vector<tools_2D::triangle>
{
    auto coords = read_csv_coords("points.csv");
    if (coords.size() <= 3) 
        throw std::invalid_argument("At least 4 points are required for triangulation.");

    std::vector<tools_2D::point> points;
    for(const auto [x, y] : coords)
        points.emplace_back(x, y);

    return delaunay_triangulate(points);   
}