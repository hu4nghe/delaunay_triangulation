#include "geo_3D/predicates_3D.h"

#include <array>

namespace
{
    auto det3(
        double a11,
        double a12,
        double a13,
        double a21,
        double a22,
        double a23,
        double a31,
        double a32,
        double a33) -> double
    {
        return a11 * (a22 * a33 - a23 * a32) - a12 * (a21 * a33 - a23 * a31) +
               a13 * (a21 * a32 - a22 * a31);
    }

    auto det4(
        const std::array<
            std::array<
                double,
                4>,
            4>& m) -> double
    {
        const double m00 = m[0][0], m01 = m[0][1], m02 = m[0][2], m03 = m[0][3];
        const double m10 = m[1][0], m11 = m[1][1], m12 = m[1][2], m13 = m[1][3];
        const double m20 = m[2][0], m21 = m[2][1], m22 = m[2][2], m23 = m[2][3];
        const double m30 = m[3][0], m31 = m[3][1], m32 = m[3][2], m33 = m[3][3];

        return m00 * det3(m11, m12, m13, m21, m22, m23, m31, m32, m33) -
               m01 * det3(m10, m12, m13, m20, m22, m23, m30, m32, m33) +
               m02 * det3(m10, m11, m13, m20, m21, m23, m30, m31, m33) -
               m03 * det3(m10, m11, m12, m20, m21, m22, m30, m31, m32);
    }
} // namespace

namespace tools_3D
{
    auto orient3d(
        const tools_3D::point& a,
        const tools_3D::point& b,
        const tools_3D::point& c,
        const tools_3D::point& d) -> double
    {
        const double adx = a.get_x() - d.get_x();
        const double ady = a.get_y() - d.get_y();
        const double adz = a.get_z() - d.get_z();

        const double bdx = b.get_x() - d.get_x();
        const double bdy = b.get_y() - d.get_y();
        const double bdz = b.get_z() - d.get_z();

        const double cdx = c.get_x() - d.get_x();
        const double cdy = c.get_y() - d.get_y();
        const double cdz = c.get_z() - d.get_z();

        return adx * (bdy * cdz - bdz * cdy) - ady * (bdx * cdz - bdz * cdx) +
               adz * (bdx * cdy - bdy * cdx);
    }

    auto in_sphere(
        const tools_3D::point& a,
        const tools_3D::point& b,
        const tools_3D::point& c,
        const tools_3D::point& d,
        const tools_3D::point& e) -> bool
    {
        auto row = [&](const tools_3D::point& p) -> std::array<double, 4>
        {
            const double x = p.get_x() - e.get_x();
            const double y = p.get_y() - e.get_y();
            const double z = p.get_z() - e.get_z();
            return {x, y, z, x * x + y * y + z * z};
        };

        const std::array<std::array<double, 4>, 4> m{
            row(a),
            row(b),
            row(c),
            row(d)};
        const double det = det4(m);
        const double o   = orient3d(a, b, c, d);

        if (o > 0.0) return det > geometric_epsilon_3D;
        return det < -geometric_epsilon_3D;
    }
} // namespace tools_3D
