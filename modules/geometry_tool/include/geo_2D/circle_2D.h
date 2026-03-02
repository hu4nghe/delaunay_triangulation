 /**
 * @file circle_2D.h
 * @author HUANG He (he.hu4ng@outlook.com)
 * @brief 2D Circle class
 * @version 0.1
 * @date 2025-09-18
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#pragma once

#include "geo_2D/point_2D.h"

namespace tools_2D
{
    class circle
    {
        //Members 
        tools_2D::point center;
        double          radius;

    public:

        /// Constructors ///

        /**
         * @brief Default constructor
         * 
         */
        circle() = default; 
        
        /**
         * @brief Construct a new circle object.
         * 
         * @param center center of circle
         * @param radius radius of circle
         */
        circle(tools_2D::point center, double radius);

        /// Member functions ///

        /**
         * @brief Check if a point is inside the circle.
         * 
         * @param p Point to check
         * @return true 
         * @return false 
         */
        bool contains(const tools_2D::point& p) const;
    };
} // namespace tools_2D
