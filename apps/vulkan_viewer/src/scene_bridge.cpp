#include "scene_bridge.h"

#include "geo_3D/surface_mesh_3D.h"
#include "geo_3D/triangulation_3D.h"

#include <algorithm>
#include <random>

namespace
{
    auto random_cloud_3d(
        int      n,
        unsigned seed) -> std::vector<tools_3D::point>
    {
        std::mt19937                           rng(seed);
        std::uniform_real_distribution<double> dist(-10.0, 10.0);

        std::vector<tools_3D::point> pts;
        pts.reserve(static_cast<std::size_t>(n));
        for (int i = 0; i < n; ++i) pts.emplace_back(dist(rng), dist(rng), dist(rng));
        return pts;
    }
} // namespace

SceneBridge::SceneBridge()
{
    rebuild();
}

void SceneBridge::setAlgorithm(Algorithm algorithm)
{
    algorithm_ = algorithm;
}

void SceneBridge::setPointCount(int pointCount)
{
    if (pointCount < 8) pointCount = 8;
    if (pointCount > 2000) pointCount = 2000;
    point_count_ = pointCount;
}

void SceneBridge::reseed()
{
    ++seed_;
}

void SceneBridge::rebuild()
{
    const auto points = random_cloud_3d(point_count_, seed_);

    std::vector<tools_3D::tetrahedron> tetrahedra;
    if (algorithm_ == Algorithm::BowyerWatson)
        tetrahedra = tools_3D::bowyer_watson_3D(points);
    else tetrahedra = tools_3D::guibas_stolfi_3D(points);

    const auto surface = tools_3D::extract_boundary_surface(tetrahedra);

    mesh_.positions.clear();
    mesh_.indices.clear();

    if (surface.vertices.empty() || surface.triangles.empty()) return;

    double min_x = surface.vertices.front().get_x();
    double max_x = min_x;
    double min_y = surface.vertices.front().get_y();
    double max_y = min_y;
    double min_z = surface.vertices.front().get_z();
    double max_z = min_z;

    for (const auto& p : surface.vertices)
    {
        min_x = std::min(min_x, p.get_x());
        max_x = std::max(max_x, p.get_x());
        min_y = std::min(min_y, p.get_y());
        max_y = std::max(max_y, p.get_y());
        min_z = std::min(min_z, p.get_z());
        max_z = std::max(max_z, p.get_z());
    }

    const double cx   = (min_x + max_x) * 0.5;
    const double cy   = (min_y + max_y) * 0.5;
    const double cz   = (min_z + max_z) * 0.5;
    const double span = std::max({max_x - min_x, max_y - min_y, max_z - min_z});
    const double inv  = (span > 0.0) ? (1.8 / span) : 1.0;

    mesh_.positions.reserve(surface.vertices.size());
    for (const auto& p : surface.vertices)
    {
        mesh_.positions.emplace_back(
            static_cast<float>((p.get_x() - cx) * inv),
            static_cast<float>((p.get_y() - cy) * inv),
            static_cast<float>((p.get_z() - cz) * inv));
    }

    mesh_.indices.reserve(surface.triangles.size() * 3);
    for (const auto& tri : surface.triangles)
    {
        mesh_.indices.push_back(tri[0]);
        mesh_.indices.push_back(tri[1]);
        mesh_.indices.push_back(tri[2]);
    }
}

auto SceneBridge::mesh() const -> const MeshData&
{
    return mesh_;
}

auto SceneBridge::pointCount() const -> int
{
    return point_count_;
}

auto SceneBridge::algorithm() const -> Algorithm
{
    return algorithm_;
}
