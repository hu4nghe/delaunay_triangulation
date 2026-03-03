#include "geo_2D/quad_edge_2D.h"

#include <utility>

namespace tools_2D::detail
{
    auto quad_edge_mesh::edge::sym() const -> edge*
    {
        return rot->rot;
    }

    auto quad_edge_mesh::edge::inv_rot() const -> edge*
    {
        return rot->rot->rot;
    }

    auto quad_edge_mesh::edge::o_next() const -> edge*
    {
        return next;
    }

    auto quad_edge_mesh::edge::o_prev() const -> edge*
    {
        return rot->next->rot;
    }

    auto quad_edge_mesh::edge::l_next() const -> edge*
    {
        return inv_rot()->o_next()->rot;
    }

    auto quad_edge_mesh::edge::r_prev() const -> edge*
    {
        return sym()->o_next();
    }

    auto quad_edge_mesh::edge::dest() const -> int
    {
        return sym()->origin;
    }

    auto quad_edge_mesh::make_edge(
        int from,
        int to) -> edge*
    {
        auto q = std::make_unique<quad_edge_block>();

        q->e[0].rot = &q->e[1];
        q->e[1].rot = &q->e[2];
        q->e[2].rot = &q->e[3];
        q->e[3].rot = &q->e[0];

        q->e[0].next = &q->e[0];
        q->e[1].next = &q->e[3];
        q->e[2].next = &q->e[2];
        q->e[3].next = &q->e[1];

        q->e[0].origin = from;
        q->e[2].origin = to;

        q->e[0].alive = true;
        q->e[2].alive = true;

        storage_.push_back(std::move(q));
        edge* e = &storage_.back()->e[0];
        primal_edges_.push_back(e);
        return e;
    }

    void quad_edge_mesh::splice(
        edge* a,
        edge* b)
    {
        edge* alpha = a->o_next()->rot;
        edge* beta  = b->o_next()->rot;
        std::swap(a->next, b->next);
        std::swap(alpha->next, beta->next);
    }

    auto quad_edge_mesh::connect(
        edge* a,
        edge* b) -> edge*
    {
        edge* e = make_edge(a->dest(), b->origin);
        splice(e, a->l_next());
        splice(e->sym(), b);
        return e;
    }

    void quad_edge_mesh::delete_edge(edge* e)
    {
        splice(e, e->o_prev());
        splice(e->sym(), e->sym()->o_prev());
        e->alive        = false;
        e->sym()->alive = false;
    }

    auto quad_edge_mesh::primal_edges() const -> const std::vector<edge*>&
    {
        return primal_edges_;
    }
} // namespace tools_2D::detail
