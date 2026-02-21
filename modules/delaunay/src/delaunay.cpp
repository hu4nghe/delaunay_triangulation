/**
 * @file delaunay.cpp
 * @author HUANG He (he.hu4ng@outlook.com)
 * @brief Bowyer-Watson Algorithm Implementation for Delaunay Triangulation
 * 
 * Algorithm Overview:
 * 1. Construct a super-triangle - A large triangle containing all input points
 * 2. Incremental insertion - For each point, perform the following:
 *    a. Find all "bad" triangles whose circumcircles contain the point
 *    b. Extract the boundaries of these triangles
 *    c. Construct new triangles from the new point and boundary edges
 * 3. Cleanup - Remove all triangles containing any super-triangle vertex
 * 
 * Complexity: O(n log n) average, O(n²) worst case
 * 
 * @version 1.0
 * @date 2026/21/02
 * @copyright Copyright (c) 2025 HUANG He
 * @license MIT
 * 
 * References:
 * - Bowyer, A. "Computing Dirichlet tessellations" The Computer Journal (1981)
 * - Watson, D.F. "Computing the n-dimensional Delaunay tessellation" (1981)
 */

#include "csv_parser.h"
#include "delaunay.h"

#include <ranges>
#include <functional>
#include <set>

/**
 * @brief Construct a super-triangle containing all input points
 * 
 * The super-triangle must satisfy two conditions:
 * 1. Enclose all input points
 * 2. Be large enough that its circumcircle will never contain non-input points
 * 
 * Strategy employed:
 * - Compute the bounding box of the point set
 * - Scale size to alpha * max(width, height), where alpha=20
 * - Use an isosceles triangle to ensure numerical stability
 * 
 * @param[in] points Input point set (used to compute bounds)
 * 
 * @return std::tuple<point, point, point> 
 *         Three vertices of the super-triangle (bottom-left, bottom-right, top-middle)
 * 
 * @note Using an isosceles triangle instead of equilateral avoids numerical precision issues
 */
auto super_triangle(const std::vector<tools_2D::point>& points)
{
    // Use C++20 ranges to find min/max of x and y coordinates
    auto [min_x, max_x] = std::ranges::minmax(
        points | std::views::transform(std::mem_fn(&tools_2D::point::get_x))
    );
    auto [min_y, max_y] = std::ranges::minmax(
        points | std::views::transform(std::mem_fn(&tools_2D::point::get_y))
    );

    // Compute center and size of point set
    double cx = (min_x + max_x) / 2.0;
    double cy = (min_y + max_y) / 2.0;
    double L  = std::max(max_x - min_x, max_y - min_y);

    // Super-triangle side length = alpha * L, where alpha=20 is empirical
    // Ensure minimum size of 1.0 to handle degenerate cases
    double alpha = 20.0;
    double S = std::max(1.0, alpha * L);

    // Construct isosceles triangle: base at bottom, vertex at top
    tools_2D::point bl(cx - S/2.0, cy - S/2.0); // bottom-left
    tools_2D::point br(cx + S/2.0, cy - S/2.0); // bottom-right
    tools_2D::point top_mid(cx, cy + S/2.0);    // top-middle

    return std::tuple(bl, br, top_mid);
}

/**
 * @brief Bowyer-Watson Incremental Triangulation Algorithm Implementation
 * 
 * Algorithm Flow:
 * 
 * Phase 1: Initialization
 *   - Create super-triangle
 *   - Add it to the triangle list
 * 
 * Phase 2: Incremental Point Insertion
 *   For each new point p:
 *     a. Identify all "bad" triangles whose circumcircles contain p
 *     b. Extract boundary edges of these triangles:
 *        - If edge is shared by two "bad" triangles, remove it (internal edge)
 *        - Otherwise, keep it (boundary edge)
 *     c. Construct new triangles from point p and each boundary edge
 * 
 * Phase 3: Cleanup
 *   - Remove all triangles containing any super-triangle vertex
 *   - These are triangles adjacent to the super-triangle, not in Delaunay result
 * 
 * @param[in] points Input point set
 *            - Should contain >= 3 non-degenerate points
 *            - Collinear points create degenerate triangles (area = 0)
 * 
 * @return std::vector<tools_2D::triangle> 
 *         All output triangles satisfy the Delaunay property
 *         i.e., each triangle's circumcircle contains no other input points
 * 
 * @complexity
 *   - Time: O(n log n) average, O(n²) worst case
 *     * Worst case occurs with certain special point distributions (e.g., grids)
 *   - Space: O(n) for triangle list and auxiliary sets
 * 
 * @note
 *   - Why std::set for storing boundary edges:
 *     * Fast lookup and deletion (O(log n) per edge)
 *     * Automatic deduplication (shared edges by two "bad" triangles are eliminated)
 *   - std::erase_if used for in-place removal of "bad" triangles (C++20 standard library)
 */
auto delaunay_triangulate(const std::vector<tools_2D::point>& points) 
    -> std::vector<tools_2D::triangle>
{    
    // ============ Phase 1: Initialization ============
    // Create super-triangle and initialize triangle list
    auto [p1, p2, p3] = super_triangle(points);
    std::vector<tools_2D::triangle> all_triangles{tools_2D::triangle{p1, p2, p3}};

    // ============ Phase 2: Incremental Insertion ============
    // Perform one insertion operation for each point
    for (const auto& p : points) 
    {
        // Store all edges needed for constructing triangles for current point
        // std::set auto-deduplicates and provides efficient lookup/deletion
        std::set<tools_2D::segment> polygon;
        
        // Remove all "bad" triangles (circumcircles contain point p)
        std::erase_if(
            all_triangles, 
            [&](const tools_2D::triangle& tri)
            {
                // Check if point p is inside the circumcircle of triangle tri
                if (tri.circum_circle().contains(p))
                {
                    // This is a "bad" triangle that needs to be deleted
                    // Save all its edges as boundaries for constructing new triangles
                    for (const auto& edge : tri.get_edges()) 
                    {
                        // Smart add/remove logic for edges:
                        // - If edge already in polygon, it's shared by two "bad" triangles
                        //   (internal edge), don't keep -> erase returns 1, execute polygon.erase
                        // - If edge not in polygon, it's new, need to keep -> erase returns 0,
                        //   execute polygon.insert
                        if (!polygon.erase(edge)) 
                            polygon.insert(edge);
                    }
                    return true;  // Mark triangle for removal
                }
                return false;  // Keep this triangle
            });
        
        // Construct new triangles from new point and all boundary edges
        // This ensures all newly added triangles have p as a vertex
        for (const auto& edge : polygon)
            all_triangles.emplace_back(p, edge);
    }

    // ============ Phase 3: Cleanup ============
    // Remove all triangles containing any super-triangle vertex
    // These triangles are adjacent to the super-triangle, not in Delaunay result
    std::erase_if(
        all_triangles,
        [&](const tools_2D::triangle& tri) 
        {
            // Check if triangle contains any super-triangle vertex
            for (const auto& vertex : {p1, p2, p3})
                if(tri.containsVertex(vertex)) 
                    return true;  // Mark for removal
            return false;
        });
                                        
    return all_triangles;
}

/**
 * @brief Read coordinates from CSV file and perform Delaunay triangulation
 * 
 * This is a convenience function wrapping the complete workflow of CSV reading
 * and triangulation.
 * 
 * Steps:
 * 1. Read coordinate pairs (x, y) from hardcoded path "points.csv"
 * 2. Validate at least 4 points (> 3 for triangle, plus validation logic)
 * 3. Convert coordinate pairs to tools_2D::point objects
 * 4. Call delaunay_triangulate() to perform triangulation
 * 
 * @return std::vector<tools_2D::triangle> 
 *         Validated Delaunay triangulation result
 * 
 * @throw parse_error 
 *        CSV file format error (from csv_parser)
 *        - Column count not 2
 *        - Values cannot convert to double
 * 
 * @throw std::invalid_argument 
 *        Fewer than 4 points read
 *        Message: At least 4 points are required for triangulation
 * 
 * @throw std::filesystem_error
 *        File does not exist or lacks read permission (from std::filesystem)
 * 
 * @note
 *   - CSV path is hardcoded as "points.csv"
 *   - Can be modified via environment variable or config file (future improvement)
 *   - File should be located in program's working directory
 */
auto read_and_triangulate() -> std::vector<tools_2D::triangle>
{
    // Read coordinates from CSV file
    // read_csv_coords returns std::vector<std::pair<double, double>>
    auto coords = read_csv_coords("points.csv");
    
    // Point count validation: require at least 4 points
    // (3 points can form one triangle, but we need more for meaningful triangulation)
    if (coords.size() <= 3) 
        throw std::invalid_argument("At least 4 points are required for triangulation.");

    // Convert coordinate pairs to point objects
    std::vector<tools_2D::point> points;
    points.reserve(coords.size());  // Pre-allocate to avoid reallocation
    
    for(const auto [x, y] : coords)
        points.emplace_back(x, y);

    // Perform triangulation and return result
    return delaunay_triangulate(points);   
}