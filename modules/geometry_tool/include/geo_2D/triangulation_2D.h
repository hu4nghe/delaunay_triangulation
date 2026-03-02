#pragma once

#include "geo_2D/point_2D.h"
#include "geo_2D/triangle_2D.h"

#include <vector>

namespace tools_2D
{
    auto boyer_watson_2D(const std::vector<tools_2D::point>& points)
        -> std::vector<tools_2D::triangle>;

    auto guibas_stolfi_2D(const std::vector<tools_2D::point>& points)
        -> std::vector<tools_2D::triangle>;

    auto read_and_triangulate() -> std::vector<tools_2D::triangle>;
}
