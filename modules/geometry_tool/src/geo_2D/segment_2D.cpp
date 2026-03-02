#include "geo_2D/segment_2D.h"

tools_2D::segment::segment(const tools_2D::point& a, const tools_2D::point& b) 
    : endpoints{a < b ? a : b, a < b ? b : a} {}

bool
tools_2D::segment::operator==(const segment& other) const 
{ 
    return endpoints == other.endpoints; 
}

bool 
tools_2D::segment::operator<(const segment& other) const 
{ 
    return endpoints < other.endpoints; 
}

std::pair<tools_2D::point, tools_2D::point>
tools_2D::segment::get_points() const 
{ 
    return endpoints; 
}