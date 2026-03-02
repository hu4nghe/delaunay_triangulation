#pragma once

#include "geo_2D/point_2D.h"

namespace tools_2D
{
    class vector_2D 
    {
        tools_2D::point cord;
    public :
        
        /**
         * @brief Default constructor
         *
         */
        vector_2D()                             = default;

        /**
         * @brief Copy constructor
         * 
         */
        vector_2D(const vector_2D &)            = default;

        /**
         * @brief Move constructor
         * 
         */
        vector_2D(vector_2D &&)                 = default;

        /**
         * @brief Copy assignment operator
         * 
         * @return vector_2D& value to copy
         */
        vector_2D &operator=(const vector_2D &) = default;

        /**
         * @brief Move assignment operator
         * 
         * @return vector_2D& value to move
         */
        vector_2D &operator=(vector_2D &&)      = default;
        
        /**
         * @brief Vector from origin -> p.
         * 
         * @param p ending point
         */
        vector_2D(const tools_2D::point& p);

        /**
         * @brief Vector from p1 -> p2.
         * 
         * @param p1 starting point
         * @param p2 ending point
         */
        vector_2D(const tools_2D::point& p1, const tools_2D::point& p2);

        /**
         * @brief Addition operator.
         * 
         * @param other vector to add
         * @return vector_2D sum of two vector
         */
        vector_2D operator+(const vector_2D &other) const;

        /**
         * @brief Substraction operator.
         * 
         * @param other vector to substracte
         * @return vector_2D diff of two vector
         */
        vector_2D operator-(const vector_2D& other) const;

        /**
         * @brief Constant mutiplication.
         * 
         * @param t scalar
         * @return vector_2D result vector
         */
        vector_2D operator*(double t) const;

        /**
         * @brief Dot product.
         * 
         * @param other vector to dot
         * @return double dot result equals to |a||b|cos θ
         */
        double dot(const vector_2D& other) const;

        /**
         * @brief Cross product.
         * 
         * @param other vector to cross
         * @return double cross result equals to |a||b|sin θ 
         */
        double cross(const vector_2D& other) const;

        /**
         * @brief Length of vector in square.
         * 
         * @return double Length in square
         */
        double length_sq() const;

        /**
         * @brief Get the vector coord.
         * 
         * @return std::pair<double, double> coord of vector
         */
        auto get_coordinates() const -> std::pair<double, double>; 
    };
}