#pragma once

#include <glm/vec3.hpp>

#include <cstdint>
#include <vector>

class SceneBridge
{
    public:

    enum class Algorithm
    {
        BowyerWatson,
        GuibasStolfi
    };

    struct MeshData
    {
        std::vector<glm::vec3>     positions;
        std::vector<std::uint32_t> indices;
    };

    SceneBridge();

    void setAlgorithm(Algorithm algorithm);
    void setPointCount(int pointCount);
    void reseed();
    void rebuild();

    auto mesh() const -> const MeshData&;
    auto pointCount() const -> int;
    auto algorithm() const -> Algorithm;

    private:

    Algorithm algorithm_{Algorithm::BowyerWatson};
    int       point_count_{120};
    unsigned  seed_{2026};
    MeshData  mesh_;
};
