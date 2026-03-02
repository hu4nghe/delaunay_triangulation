/**
 * @file triangle.h
 * @author HUANG He (he.hu4ng@outlook.com)
 * @brief 2D triangle class
 * @version 0.1
 * @date 2025-09-18
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#pragma once 

#include <array>

#include "geo_2D/segment_2D.h"
#include "geo_2D/circle_2D.h"

namespace tools_2D
{
    class triangle
    {
        std::array<tools_2D::point,3> vertices;
    public:

        /**
         * @brief Default Constructor(deleted)
         * We cannot define the neutral element of triangle, there for default constructor is not available.
         * 
         */
        triangle() = delete;

        /**
         * @brief Constructor with three points.
         * 
         * @param a vertex 1
         * @param b vertex 2
         * @param c vertex 3
         */
        triangle(const tools_2D::point& a, const tools_2D::point& b, const tools_2D::point& c);

        /**
         * @brief Constructor with a points and a segement.
         * 
         * @param p vertex 1
         * @param e segment of vertex 2 and 3
         */
        triangle(const tools_2D::point& p, const tools_2D::segment& e);

        /**
         * @brief To check if current triangle contains a vertex.
         * 
         * @param p 
         * @return true If triangle contains vertex
         * @return false If triangle does not contain vertex
         */
        bool containsVertex(const tools_2D::point& p) const;

        /**
         * @brief Generate a circum circle of triangle.
         * 
         * @return tools_2D::circle the circum circle of triangle
         */
        tools_2D::circle circum_circle() const;
        
        /**
         * @brief Return 3 edges of triangle
         * 
         * @return std::array<segment,3> array that contains 3 edges.
         */
        std::array<segment,3> get_edges() const;
    };
} // namespace tools_2D