#include "geo_2D/circle_2D.h"
#include "base/dbl_compare.h"
namespace tools_2D 
{
    tools_2D::circle::circle(tools_2D::point center, double radius) 
        : center(center), radius(radius) {}

    bool 
    tools_2D::circle::contains(const tools_2D::point& p) const
    { 
        return dbl_inf(center.distance(p), radius); 
    }
} // namespace tools_2D