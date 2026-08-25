#include "pyraminx/solver.hpp"

#include <algorithm>
#include <functional>
#include <limits>
#include <queue>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace pyraminx {
namespace {

using Clock = std::chrono::steady_clock;

struct Parent {
    State previous;
    Move move{};
};

using Parents = std::unordered_map<State, Parent, StateHash>;

std::vector<Move> reconstruct(const State& goal, const State& initial, const Parents& parents) {
    std::vector<Move> path;
    State current = goal;
    while (!(current == initial)) {
        const auto found = parents.find(current);
        if (found == parents.end()) return {};
        path.push_back(found->second.move);
        current = found->second.previous;
    }
    std::reverse(path.begin(), path.end());
    return path;
}

bool over_limit(const Clock::time_point start, const SolveOptions& options,
                const SolveStats& stats) {
    return stats.nodes_expanded >= options.node_limit ||
           Clock::now() - start >= options.time_limit;
}

void finish_stats(SolveStats& stats, const Clock::time_point start, std::size_t stored_states) {
    stats.elapsed = std::chrono::duration_cast<std::chrono::microseconds>(Clock::now() - start);
    constexpr std::size_t approximate_hash_overhead = sizeof(void*) * 4U;
    stats.estimated_peak_bytes = stored_states *
        (sizeof(State) + sizeof(Parent) + approximate_hash_overhead);
}

SolveResult breadth_first(const State& initial, const SolveOptions& options,
                          const Clock::time_point start) {
    SolveResult result;
    result.optimal = true;
    std::queue<State> frontier;
    std::unordered_set<State, StateHash> visited;
    Parents parents;
    frontier.push(initial);
    visited.insert(initial);

    while (!frontier.empty()) {
        result.stats.peak_frontier = std::max(result.stats.peak_frontier, frontier.size());
        if (over_limit(start, options, result.stats)) {
            result.timed_out = true;
            break;
        }
        const State current = frontier.front();
        frontier.pop();
        ++result.stats.nodes_expanded;
        if (current.is_solved()) {
            result.solved = true;
            result.moves = reconstruct(current, initial, parents);
            break;
        }
        for (const auto move : kAllMoves) {
            ++result.stats.nodes_generated;
            const State next = apply_move(current, move);
            if (visited.insert(next).second) {
                parents.emplace(next, Parent{current, move});
                frontier.push(next);
            }
        }
    }
    finish_stats(result.stats, start, visited.size() + result.stats.peak_frontier);
    return result;
}

struct OpenNode {
    State state;
    std::uint32_t g{};
    std::uint32_t f{};
    bool operator>(const OpenNode& other) const noexcept {
        return f == other.f ? g < other.g : f > other.f;
    }
};

SolveResult a_star(const State& initial, const SolveOptions& options, const Heuristic& heuristic,
                   const Clock::time_point start) {
    SolveResult result;
    result.optimal = true;
    std::priority_queue<OpenNode, std::vector<OpenNode>, std::greater<>> frontier;
    std::unordered_map<State, std::uint32_t, StateHash> best_cost;
    Parents parents;
    frontier.push({initial, 0, heuristic(initial)});
    best_cost.emplace(initial, 0);

    while (!frontier.empty()) {
        result.stats.peak_frontier = std::max(result.stats.peak_frontier, frontier.size());
        if (over_limit(start, options, result.stats)) {
            result.timed_out = true;
            break;
        }
        const OpenNode current = frontier.top();
        frontier.pop();
        const auto known = best_cost.find(current.state);
        if (known == best_cost.end() || known->second != current.g) continue;
        ++result.stats.nodes_expanded;
        if (current.state.is_solved()) {
            result.solved = true;
            result.moves = reconstruct(current.state, initial, parents);
            break;
        }
        for (const auto move : kAllMoves) {
            ++result.stats.nodes_generated;
            const State next = apply_move(current.state, move);
            const auto next_g = current.g + 1U;
            const auto found = best_cost.find(next);
            if (found != best_cost.end() && found->second <= next_g) continue;
            best_cost[next] = next_g;
            parents.insert_or_assign(next, Parent{current.state, move});
            frontier.push({next, next_g, next_g + heuristic(next)});
        }
    }
    finish_stats(result.stats, start, best_cost.size() + result.stats.peak_frontier);
    return result;
}

SolveResult ida_star(const State& initial, const SolveOptions& options, const Heuristic& heuristic,
                     const Clock::time_point start) {
    SolveResult result;
    result.optimal = true;
    int bound = heuristic(initial);
    std::vector<Move> path;
    std::unordered_set<State, StateHash> on_path{initial};
    constexpr int found_solution = -1;
    constexpr int stopped = -2;

    std::function<int(const State&, int, std::optional<Move>)> search =
        [&](const State& current, int depth, std::optional<Move> previous) -> int {
        const int estimate = depth + heuristic(current);
        if (estimate > bound) return estimate;
        if (current.is_solved()) return found_solution;
        if (over_limit(start, options, result.stats)) return stopped;
        ++result.stats.nodes_expanded;
        int next_bound = std::numeric_limits<int>::max();
        for (const auto move : kAllMoves) {
            if (previous && static_cast<std::uint8_t>(*previous) / 2U ==
                                static_cast<std::uint8_t>(move) / 2U) {
                continue;
            }
            ++result.stats.nodes_generated;
            const State next = apply_move(current, move);
            if (!on_path.insert(next).second) continue;
            path.push_back(move);
            result.stats.peak_frontier = std::max(result.stats.peak_frontier, path.size() + 1U);
            const int outcome = search(next, depth + 1, move);
            if (outcome == found_solution) return found_solution;
            path.pop_back();
            on_path.erase(next);
            if (outcome == stopped) return stopped;
            next_bound = std::min(next_bound, outcome);
        }
        return next_bound;
    };

    while (true) {
        const int outcome = search(initial, 0, std::nullopt);
        if (outcome == found_solution) {
            result.solved = true;
            result.moves = path;
            break;
        }
        if (outcome == stopped || outcome == std::numeric_limits<int>::max()) {
            result.timed_out = outcome == stopped;
            break;
        }
        bound = outcome;
    }
    finish_stats(result.stats, start, result.stats.peak_frontier);
    return result;
}

} // namespace

std::string_view to_string(Algorithm algorithm) noexcept {
    switch (algorithm) {
        case Algorithm::BreadthFirst: return "bfs";
        case Algorithm::AStar: return "astar";
        case Algorithm::IDAStar: return "idastar";
    }
    return "unknown";
}

Algorithm parse_algorithm(std::string_view name) {
    if (name == "bfs") return Algorithm::BreadthFirst;
    if (name == "astar" || name == "a*") return Algorithm::AStar;
    if (name == "idastar" || name == "ida*") return Algorithm::IDAStar;
    throw std::invalid_argument("unknown algorithm '" + std::string(name) + "'");
}

SolveResult solve(const State& initial, const SolveOptions& options) {
    const auto start = Clock::now();
    if (!initial.has_valid_stickers()) {
        throw std::invalid_argument("state is not a permutation of the 64 stickers");
    }
    Heuristic heuristic(options.algorithm == Algorithm::BreadthFirst
                            ? HeuristicKind::Zero
                            : options.heuristic);
    SolveResult result;
    if (options.algorithm == Algorithm::BreadthFirst) {
        result = breadth_first(initial, options, start);
    } else if (options.algorithm == Algorithm::AStar) {
        result = a_star(initial, options, heuristic, start);
    } else {
        result = ida_star(initial, options, heuristic, start);
    }
    result.stats.initial_heuristic = heuristic(initial);
    return result;
}

bool verifies_solution(const State& initial, std::span<const Move> solution) noexcept {
    return apply_moves(initial, solution).is_solved();
}

} // namespace pyraminx

