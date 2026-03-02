/**
 * @file segment_2D.h
 * @author HUANG He (he.hu4ng@outlook.com)
 * @brief 2D segment class
 * @version 0.1
 * @date 2025-09-18
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#pragma once

#include "geo_2D/point_2D.h"
#include "point_2D.h"

namespace tools_2D 
{
    class segment
    {
        /// Member ///
    
        std::pair<tools_2D::point, tools_2D::point> endpoints;

    public:

        /**
         * @brief Default Constructor(deleted)
         * We cannot define the neutral element of segement, there for default constructor is not available.
         * 
         */
        segment() = delete;

        /**
         * @brief Construct a new segment object
         * 
         */
        segment(const tools_2D::point&, const tools_2D::point&);

        /**
         * @brief 
         * 
         * @param other Segement to compare
         * @return true 
         * @return false 
         */
        bool operator==(const segment& other) const;

        /**
         * @brief Less than operator (for STL containers)
         * Do not explicitly use this to compare two segments !
         * 
         * @param other Segement to compare
         * @return true 
         * @return false 
         */
        bool operator<(const segment& other) const;

        /**
         * @brief Get the points object
         * 
         * @return std::pair<tools_2D::point, tools_2D::point> 
         */
        auto get_points() const ->std::pair<tools_2D::point, tools_2D::point>;
    };
} // namespace tools_2D

namespace std 
{
    template<>
    struct hash<tools_2D::segment>
    {
        std::size_t operator()(tools_2D::segment const& seg) const noexcept 
        {
            auto [a, b] = seg.get_points();
            auto ha = std::hash<double>{}(a.get_x()) ^ (std::hash<double>{}(a.get_y()) << 1);
            auto hb = std::hash<double>{}(b.get_x()) ^ (std::hash<double>{}(b.get_y()) << 1);
            if (ha < hb) return ha ^ (hb + 0x9e3779b9 + (ha<<6) + (ha>>2));
            else         return hb ^ (ha + 0x9e3779b9 + (hb<<6) + (hb>>2));
        }
    };
} // namespace std