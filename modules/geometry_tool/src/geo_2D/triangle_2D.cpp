#include "geo_2D/triangle_2D.h"

#include <stdexcept>
#include <cmath>

#include "geo_2D/vector_2D.h"

namespace tools_2D 
{
    tools_2D::triangle::triangle(const tools_2D::point& a, const tools_2D::point& b, const tools_2D::point& c)
    {
        auto cross = tools_2D::vector_2D{a,b}.cross(tools_2D::vector_2D{b,c});

        if (cross > 0) 
                    vertices = {a, b, c};
        else if (cross < 0) 
                    vertices = {a, c, b};
        else 
            throw std::invalid_argument("Triangle vertices are collinear");
    }


    tools_2D::triangle::triangle(const tools_2D::point& p, const tools_2D::segment& e) 
        : vertices{p, e.get_points().first, e.get_points().second }{}
            

    bool 
    tools_2D::triangle::containsVertex(const tools_2D::point& p) const 
    { 
        return std::ranges::find(vertices, p) != vertices.end(); 
    }     

    tools_2D::circle 
    tools_2D::triangle::circum_circle() const
    { 
        tools_2D::vector_2D ab(vertices[0], vertices[1]);
        tools_2D::vector_2D ac(vertices[0], vertices[2]);

        double ab_len2 = ab.length_sq() * 0.5;
        double ac_len2 = ac.length_sq() * 0.5;
        double d       = ab.cross(ac);

        tools_2D::point center(
        vertices[0].get_coordinates().first  + (ab_len2 * ac.get_coordinates().second - ac_len2 * ab.get_coordinates().second) / d,
        vertices[0].get_coordinates().second + (ac_len2 * ab.get_coordinates().first  - ab_len2 * ac.get_coordinates().first ) / d);

        return tools_2D::circle(center, std::sqrt(tools_2D::vector_2D(center, vertices[0]).length_sq())); 
    }

    std::array<segment,3> 
    tools_2D::triangle::get_edges() const
    {
        segment e1(vertices[0],vertices[1]);
        segment e2(vertices[0],vertices[2]);
        segment e3(vertices[1],vertices[2]);
        return std::array{e1, e2, e3};
    }    
}