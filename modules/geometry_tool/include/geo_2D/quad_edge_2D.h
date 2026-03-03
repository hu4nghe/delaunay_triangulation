#pragma once

#include <array>
#include <memory>
#include <vector>

namespace tools_2D::detail
{
    class quad_edge_mesh
    {
    public:
        struct edge
        {
            edge* rot{};
            edge* next{};
            int origin = -1;
            bool alive = false;

            auto sym() const -> edge*;
            auto inv_rot() const -> edge*;

            auto o_next() const -> edge*;
            auto o_prev() const -> edge*;
            auto l_next() const -> edge*;
            auto r_prev() const -> edge*;

            auto dest() const -> int;
        };

        quad_edge_mesh() = default;

        auto make_edge(int from, int to) -> edge*;
        void splice(edge* a, edge* b);
        auto connect(edge* a, edge* b) -> edge*;
        void delete_edge(edge* e);

        auto primal_edges() const -> const std::vector<edge*>&;

    private:
        struct quad_edge_block
        {
            std::array<edge, 4> e;
        };

        std::vector<std::unique_ptr<quad_edge_block>> storage_;
        std::vector<edge*> primal_edges_;
    };
}
