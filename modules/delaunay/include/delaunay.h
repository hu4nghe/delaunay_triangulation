/**
 * @file delaunay.h
 * @author HUANG He (hu4nghe@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2025-09-26
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "geo_2D/triangle_2D.h"
#include <vector>

auto read_and_triangulate() -> std::vector<tools_2D::triangle>;
auto delaunay_triangulate(const std::vector<tools_2D::point>& points) -> std::vector<tools_2D::triangle>;