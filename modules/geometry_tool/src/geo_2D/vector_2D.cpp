#include "geo_2D/vector_2D.h"

namespace tools_2D 
{
    tools_2D::vector_2D::vector_2D(const tools_2D::point& p) 
        : cord(p) {}
    tools_2D::vector_2D::vector_2D(const tools_2D::point& p1, const tools_2D::point& p2) 
        : cord(p2 - p1) {}

    vector_2D 
    tools_2D::vector_2D::operator+(const vector_2D &other) const 
    { 
        return {cord + other.cord}; 
    }

    vector_2D 
    tools_2D::vector_2D::operator-(const vector_2D& other) const 
    { 
        return {cord - other.cord}; 
    }

    vector_2D 
    tools_2D::vector_2D::operator*(double t) const 
    { 
        return {cord * t }; 
    }

    double 
    tools_2D::vector_2D::dot(const vector_2D& other) const 
    { 
        const auto& [x, y] = cord.get_coordinates();
        const auto& [other_x, other_y] = other.cord.get_coordinates();
        return x * other_x + y * other_y;
            
    }

    double 
    tools_2D::vector_2D::cross(const vector_2D& other) const 
    {
        const auto& [x, y] = cord.get_coordinates();
        const auto& [other_x, other_y] = other.cord.get_coordinates();
        return x * other_y - y * other_x; 
    }

    double 
    tools_2D::vector_2D::length_sq() const 
    {
        const auto& [x, y] = cord.get_coordinates();
        return x * x + y * y; 
    }

    std::pair<double, double>
    tools_2D::vector_2D::get_coordinates() const 
    { 
        return cord.get_coordinates(); 
    }

}