/**
 * @file point_2D.h
 * @author HUANG He (he.hu4ng@outlook.com)
 * @brief 2D point class
 * @version 0.1
 * @date 2025-09-18
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#pragma once

#include <utility>

namespace tools_2D
{
    class point
    {
        //Object counter that assign a unique ID for every point object.
        static inline std::size_t next_ID{1};

        // Point ID
        std::size_t ID;

        // 2D Coordinates
        double      x;
        double      y;

    public :

        /// Constructors ///

        /**
         * @brief Default constructor
         * Construct a new point object by default at (0, 0)
         * 
         */
        point();

        /**
         * @brief Constructor (by cooridnates)
         * Construct a new point object with 2D coordinate.
         * 
         * @param x Abscissa 
         * @param y Ordinate 
         */
        point(double x, double y);

        /**
         * @brief Copy constructor
         * The copy constructor only copies coordinate and will generate a new ID for new point.
         * 
         * @param other Another point
         */
        point(const point& other);

        /**
         * @brief Move constructor
         * The new point will take the old point's ID and the old one will not be usable anymore(moved).
         * 
         * @param other Another point to move
         */
        point(point&& other) noexcept;

        /**
         * @brief Copy assignment operator
         * See copy constructor.
         * 
         * @param other Another point
         * @return point& New point
         */
        point& operator=(const point& other); 
        /**
         * @brief Move assignment operator
         * See move constructor.
         * 
         * @param other Another point
         * @return point& New point 
         */
        point& operator=(point&& other) noexcept;

        /// Comparaison operators ///

        /**
         * @brief Equal-to operator 
         * 
         * @param other Another point to compare
         * @return true Two points have the same coordinate.
         * @return false Two points do not have the same coordinate.
         */
        bool operator==(const point& other) const;

        /**
         * @brief Not-equal-to operator
         * 
         * @param other Another point to compare
         * @return true Two points do not have the same coordinate.
         * @return false Two points have the same coordinate.
         */
        bool operator!=(const point& other) const;

        /**
         * @brief Less than operator 
         * For STL containers, you should not use this to compare two points !
         * 
         * @param other Point to compare
         * @return true If this->x < other.x, if x are equal, check if this->y < other.y.
         * @return false 
         */
        bool operator<(const point& other) const;

        /**
         * @brief Addition operator
         * 
         * @param other Point to add
         * @return point A point at (x1 + x2, y1 + y2).
         */
        point operator+(const point& other) const;

        /**
         * @brief Substraction operator
         * 
         * @param other Point to subtracte
         * @return point A point at (x1 - x2, y1 - y2).
         */
        point operator-(const point& other) const;

        /**
         * @brief Multiplication operator
         * 
         * @param t Multiplier
         * @return point A point at (x × t, y × t).
         */
        point operator*(double t) const;
        
        /// Member functions ///

        /**
         * @brief Distance function
         * 
         * @param other Point to calculate distance.
         * @return double Distance between another point and this.
         */
        double distance(const point& other) const;

        /**
         * @brief Slope function
         * 
         * @param other Point to calculate slope
         * @return double Slope of line defined by input point and this.
         */
        double slope(const point& other) const;

        /// Getters ///

        /**
         * @brief ID getter.
         * 
         * @return const int ID
         */
        const std::size_t get_ID() const;

        /**
         * @brief Get the coordinates object
         * 
         * @return const std::pair<double, double> 
         */
        const std::pair<double, double> get_coordinates() const;

        /**
         * @brief Get the x value
         * 
         * @return const auto x
         */
        const double get_x() const;

        /**
         * @brief Get the y value
         * 
         * @return const auto y
         */
        const double get_y() const;

        /// Setter ///

        /**
         * @brief Set the coordinate object
         * 
         * @param new_x New abscissa 
         * @param new_y New ordinate
         */
        void set_coordinate(double new_x, double new_y);
    };

} // namespace tools_2D