#include "geo_2D/point_2D.h"
#include "base/dbl_compare.h"

namespace tools_2D
{
    tools_2D::point::point()                       : x(0)      , y(0)      , ID(next_ID++) {}
    tools_2D::point::point(double x, double y)     : x(x)      , y(y)      , ID(next_ID++) {}
    tools_2D::point::point(const point& other)     : x(other.x), y(other.y), ID(next_ID++) {}
    tools_2D::point::point(point&& other) noexcept : x(other.x), y(other.y), ID(std::exchange(other.ID, 0)) {}
        
    point& 
    tools_2D::point::operator=(const point& other)     
    {
        if (this != &other)
        {
            x = other.x; 
            y = other.y; 
            ID = next_ID++; // Assign new ID for new point.
        } 
        return *this; 
    }

    point& 
    tools_2D::point::operator=(point&& other) 
    noexcept 
    {
        if (this != &other)
        {
            x = other.x; 
            y = other.y; 
            ID = other.ID; // New point takes over old's ID
            other.ID = 0; // Original point is moved.
        } 
        return *this; 
    } 

    bool
    tools_2D::point::operator==(const point& other)
    const 
    { 
        return tools_2D::dbl_eq(x, other.x) && tools_2D::dbl_eq(y, other.y); 
    }

    bool
    tools_2D::point::operator!=(const point& other)   
    const 
    { 
        return !(*this == other); 
    }
    bool
    tools_2D::point::operator<(const point& other) 
    const 
    { 
        return tools_2D::dbl_eq(x, other.x) ? tools_2D::dbl_inf(y, other.y) : tools_2D::dbl_inf(x, other.x); 
    }
    
    point 
    tools_2D::point::operator+(const point& other) 
    const 
    { 
        return {x + other.x, y + other.y}; 
    }
    
    point 
    tools_2D::point::operator-(const point& other) 
    const 
    { 
        return {x - other.x, y - other.y}; 
    }
    
    point 
    tools_2D::point::operator*(double t) 
    const 
    { 
        return {x * t, y * t}; 
    }

    double
    tools_2D::point::distance(const point& other)  
    const 
    { 
        return std::hypot(x - other.x, y - other.y); 
    }
    
    double
    tools_2D::point::slope(const point& other)   
    const 
    { 
        return tools_2D::dbl_eq(x, other.x) ? std::numeric_limits<double>::infinity() : (y - other.y) / (x - other.x); 
    }
    
    const std::size_t
    tools_2D::point::get_ID() 
    const 
    { 
        return ID;
    }
    
    const std::pair<double, double>
    tools_2D::point::get_coordinates() 
    const 
    { 
        return {x, y}; 
    }

    const double 
    tools_2D::point::get_x() const
    {
        return x;
    }

    const double 
    tools_2D::point::get_y() const
    {
        return y;
    }
    
    void
    tools_2D::point::set_coordinate(double new_x, double new_y) 
    { 
        x = new_x; y = new_y; 
    }           
} // namespace tools_2D