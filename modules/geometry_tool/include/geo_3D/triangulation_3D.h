#pragma once

#include "geo_3D/point_3D.h"
#include "geo_3D/tetrahedron_3D.h"

#include <vector>

namespace tools_3D
{
    auto bowyer_watson_3D(const std::vector<tools_3D::point>& points)
        -> std::vector<tools_3D::tetrahedron>;

    auto guibas_stolfi_3D(const std::vector<tools_3D::point>& points)
        -> std::vector<tools_3D::tetrahedron>;
}
