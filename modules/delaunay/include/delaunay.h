/**
 * @file delaunay.h
 * @author HUANG He (hu4nghe@outlook.com)
 * @brief Delaunay Triangulation Algorithm Library
 * 
 * Provides two main functions:
 * 1. delaunay_triangulate() - Perform Delaunay triangulation on a point set
 * 2. read_and_triangulate() - Read coordinates from CSV file and triangulate
 * 
 * Implementation based on Bowyer-Watson incremental algorithm, guaranteeing all
 * triangles satisfy the Delaunay property (circumcircle property: no point lies
 * inside any triangle's circumcircle).
 * 
 * @version 1.0
 * @date 2026/21/02
 * @copyright Copyright (c) 2025 HUANG He
 * @license MIT
 */

#pragma once

#include "geo_2D/triangle_2D.h"
#include <vector>

/**
 * @brief Perform Delaunay triangulation on a point set
 * 
 * Uses the Bowyer-Watson incremental algorithm to triangulate the input point set.
 * Algorithm steps:
 * 1. Construct a super-triangle containing all points
 * 2. Insert points incrementally, maintaining the Delaunay property:
 *    - Identify triangles whose circumcircles contain the new point ("bad" triangles)
 *    - Extract the boundaries of these triangles
 *    - Construct new triangles from the new point and boundary edges
 * 3. Remove triangles containing any super-triangle vertex
 * 
 * @param[in] points Input point set vector
 *            - Requires minimum of 3 points
 *            - Points > 3 recommended for meaningful triangles
 *            - Collinear points create degenerate triangles (area = 0)
 * 
 * @return std::vector<tools_2D::triangle> 
 *         - Generated triangles all satisfying the Delaunay property
 *         - May return degenerate results if input < 3 points
 * 
 * @complexity
 *   - Time: O(n log n) average case, O(n²) worst case
 *   - Space: O(n) for storing all triangles
 * 
 * @note 
 * - Collinear points in input create degenerate triangles (area = 0)
 *   Consider removing or perturbing collinear points in preprocessing
 * - Floating-point comparisons depend on geometry_tool implementation,
 *   may be affected by precision limits
 * 
 * @example
 * @code
 * std::vector<tools_2D::point> points = {
 *     {0.0, 0.0}, {1.0, 0.0}, {1.0, 1.0}, {0.0, 1.0}
 * };
 * 
 * auto triangles = delaunay_triangulate(points);
 * std::cout << "Generated " << triangles.size() << " triangles\n";
 * 
 * for (const auto& tri : triangles) {
 *     // Process each triangle
 *     auto circum = tri.circum_circle();
 * }
 * @endcode
 * 
 * @see read_and_triangulate() - Read from CSV file and triangulate
 */
auto delaunay_triangulate(const std::vector<tools_2D::point>& points) 
    -> std::vector<tools_2D::triangle>;

/**
 * @brief Read coordinates from CSV file and perform Delaunay triangulation
 * 
 * Convenience function combining CSV parsing and triangulation.
 * Reads coordinates from hardcoded path "points.csv", with format of one point per line (x,y).
 * 
 * @return std::vector<tools_2D::triangle> 
 *         Triangulation result satisfying the Delaunay property
 * 
 * @throw parse_error 
 *        If CSV file format is invalid (not 2 columns, values cannot convert to double, etc.)
 * 
 * @throw std::invalid_argument 
 *        If fewer than 4 points read (triangulation needs minimum 3 points,
 *        but validation logic requires at least 4)
 * 
 * @throw std::filesystem::filesystem_error
 *        If file does not exist or lacks read permissions
 * 
 * @complexity Same as delaunay_triangulate()
 * 
 * @note 
 * - CSV file path is hardcoded as "points.csv", must be in program's working directory
 * - Recommended CSV format (no header):
 *   @code
 *   0.0,0.0
 *   1.0,0.0
 *   1.0,1.0
 *   0.0,1.0
 *   @endcode
 * 
 * @example
 * @code
 * try {
 *     auto triangles = read_and_triangulate();
 *     std::cout << "Successfully generated " << triangles.size() << " triangles\n";
 * } catch (const parse_error& e) {
 *     std::cerr << "CSV parsing failed: " << e.what() << "\n";
 * } catch (const std::invalid_argument& e) {
 *     std::cerr << "Argument error: " << e.what() << "\n";
 * }
 * @endcode
 * 
 * @see delaunay_triangulate() - Direct point set triangulation
 * @see read_csv_coords() - Underlying CSV reading function
 */
auto read_and_triangulate() -> std::vector<tools_2D::triangle>;