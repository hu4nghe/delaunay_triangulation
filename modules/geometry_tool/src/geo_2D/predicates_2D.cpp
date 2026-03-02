#include "geo_2D/predicates_2D.h"

namespace tools_2D
{
    auto orient2d(const tools_2D::point& a, const tools_2D::point& b, const tools_2D::point& c) -> double
    {
        return (b.get_x() - a.get_x()) * (c.get_y() - a.get_y())
             - (b.get_y() - a.get_y()) * (c.get_x() - a.get_x());
    }

    auto in_circle(
        const tools_2D::point& a,
        const tools_2D::point& b,
        const tools_2D::point& c,
        const tools_2D::point& d
    ) -> bool
    {
        const double ax = a.get_x() - d.get_x();
        const double ay = a.get_y() - d.get_y();
        const double bx = b.get_x() - d.get_x();
        const double by = b.get_y() - d.get_y();
        const double cx = c.get_x() - d.get_x();
        const double cy = c.get_y() - d.get_y();

        const double det =
            (ax * ax + ay * ay) * (bx * cy - by * cx)
          - (bx * bx + by * by) * (ax * cy - ay * cx)
          + (cx * cx + cy * cy) * (ax * by - ay * bx);

        const double o = orient2d(a, b, c);
        if (o > 0.0)
            return det > geometric_epsilon;
        return det < -geometric_epsilon;
    }
}
