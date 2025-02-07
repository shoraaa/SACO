#pragma once

#include "utils.h"
#include "problem_instance.h"


struct RouteIterator {
    const std::vector<uint32_t> &route_;
    size_t position_ = 0;

    uint32_t goto_succ() noexcept {
        position_ = (position_ + 1 < route_.size()) ? position_ + 1 : 0;
        return route_[position_];
    }

    uint32_t goto_pred() noexcept {
        position_ = position_ != 0 ? position_ - 1 : route_.size() - 1;
        return route_[position_];
    }
};


struct Solution {
    std::vector<uint32_t> route_;
    double cost_ = std::numeric_limits<double>::max();
    std::vector<uint32_t> node_indices_;

    Solution() = default;

    Solution(const std::vector<uint32_t> &route, double cost)
        : route_(route),
          cost_(cost),
          node_indices_(route.size(), 0) {
        update_node_indices();
    }

    void update(const std::vector<uint32_t> &route, double cost) {
        route_ = route;
        cost_ = cost;
        update_node_indices();
    }

    void update(const Solution *other) {
        route_ = other->route_;
        cost_ = other->cost_;
        update_node_indices();
    }

    void update_node_indices() {
        for (size_t i = 0; i < route_.size(); ++i) {
            node_indices_[route_[i]] = static_cast<uint32_t>(i);
        }
    }

    void swap_(uint32_t i, uint32_t j) {
        std::swap(node_indices_[route_[i]], node_indices_[route_[j]]);
        std::swap(route_[i], route_[j]);
    }

    void relocate(uint32_t u, uint32_t v) {
        // place v after u
        uint32_t i = node_indices_[u], j = node_indices_[v];
        if (j < i) {
            swap_(i, j);
            while (j < i - 1) {
                swap_(j, j + 1);
                ++j;
            }
        } else {
            while (j > i + 1) {
                swap_(j, j - 1);
                --j;
            }
        }
    }

    void swap_with_next(uint32_t i, const ProblemInstance& problem) {
        // swap route[i] with route[i + 1]
        assert(i + 1 < route_.size());
        uint32_t u = route_[i], v = route_[i + 1];
        cost_ -= problem.get_distance(get_pred(u), u) + problem.get_distance(v, get_succ(v));
        std::swap(node_indices_[u], node_indices_[v]);
        std::swap(route_[i], route_[i + 1]);
        cost_ += problem.get_distance(get_pred(u), u) + problem.get_distance(v, get_succ(v));
    }

    void relocate_rgaco(uint32_t u, uint32_t v, const ProblemInstance& problem) {
        // place v after u
        uint32_t i = node_indices_[u], j = node_indices_[v];
        while (j < i) {
            swap_with_next(j, problem);
            ++j;
        }
        while (j > i + 1) {
            swap_with_next(j - 1, problem);
            --j;
        }
    }

    // We assume that route is undirected
    [[nodiscard]] bool contains_edge(uint32_t edge_head, uint32_t edge_tail) const {
        return get_succ(edge_head) == edge_tail   // same edge
            || get_pred(edge_head) == edge_tail;  // reversed
    }

    [[nodiscard]] uint32_t get_succ(uint32_t node) const {
        auto index = node_indices_[node];
        return route_[(index + 1u < route_.size()) ? index + 1u : 0u];
    }

    [[nodiscard]] uint32_t get_pred(uint32_t node) const {
        auto index = node_indices_[node];
        return route_[(index > 0u) ? index - 1u : route_.size() - 1u];
    }

    RouteIterator get_iterator(uint32_t start_node) {
        return { route_, node_indices_[start_node] };
    }
};


struct Ant : public Solution {
    std::vector<uint32_t> unvisited_;
    Bitmask  visited_bitmask_;
    uint32_t dimension_ = 0;
    uint32_t visited_count_ = 0;
    uint32_t changes_count = 0;

    Ant() : Solution() {}

    Ant(const std::vector<uint32_t> &route, double cost)
        : Solution(route, cost),
          unvisited_(route.size(), static_cast<uint32_t >(route.size())),
          dimension_(static_cast<uint32_t>(route.size())),
          visited_count_(static_cast<uint32_t>(route.size())) {
    }

    void initialize(uint32_t dimension) {
        dimension_ = dimension;
        visited_count_ = 0;

        route_.resize(dimension);
        node_indices_.reserve(dimension);

        unvisited_.resize(dimension);
        std::iota(unvisited_.begin(), unvisited_.end(), 0);

        visited_bitmask_.resize(dimension);
        visited_bitmask_.clear();
    }

    void visit(uint32_t node) {
        assert(!is_visited(node));

        route_[visited_count_++] = node;
        visited_bitmask_.set_bit(node);
    }

    void validate(const ProblemInstance& problem) {
        assert(route_.size() == problem.dimension_);
        assert(cost_ == problem.calculate_route_length(route_));
        visited_bitmask_.clear();

        for (auto u : route_) {
            assert(!is_visited(u));
            visited_bitmask_.set_bit(u);
        }
    }

    [[nodiscard]] bool is_visited(uint32_t node) const {
        return visited_bitmask_.get_bit(node);
    }

    bool try_visit(uint32_t node) {
        if (!is_visited(node)) {
            visit(node);
            return true;
        }
        return false;
    }

    [[nodiscard]] uint32_t get_current_node() const {
        return route_[visited_count_ - 1];
    }

    [[nodiscard]] uint32_t get_unvisited_count() const {
        return dimension_ - visited_count_;
    }

    const std::vector<uint32_t> &get_unvisited_nodes() {
        // Filter out visited nodes from unvisited_ list that
        // now can be invalid
        //
        // This has linear complexity but should not be a problem
        // if this method is not called very often.
        size_t n = unvisited_.size();
        size_t j = 0;
        for (size_t i = 0; i < n; ++i) {
            auto node = unvisited_[i];
            if (!is_visited(node)) {
                unvisited_[j++] = node;
            }
        }
        assert(j == get_unvisited_count());
        unvisited_.resize(j);
        return unvisited_;
    }
};


struct DoubleLinkedListSolution {

};

struct DoubleLinkedListAnt {

};