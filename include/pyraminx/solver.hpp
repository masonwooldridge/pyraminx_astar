#pragma once

#include "pyraminx/heuristic.hpp"

#include <chrono>
#include <cstddef>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

namespace pyraminx {

enum class Algorithm { BreadthFirst, AStar, IDAStar };

struct SolveOptions {
    Algorithm algorithm{Algorithm::AStar};
    HeuristicKind heuristic{HeuristicKind::PatternDatabase};
    std::chrono::milliseconds time_limit{10'000};
    std::size_t node_limit{2'000'000};
};

struct SolveStats {
    std::size_t nodes_expanded{};
    std::size_t nodes_generated{};
    std::size_t peak_frontier{};
    std::size_t estimated_peak_bytes{};
    std::chrono::microseconds elapsed{};
    std::uint8_t initial_heuristic{};
};

struct SolveResult {
    bool solved{};
    bool optimal{};
    bool timed_out{};
    std::vector<Move> moves;
    SolveStats stats;
};

[[nodiscard]] std::string_view to_string(Algorithm algorithm) noexcept;
[[nodiscard]] Algorithm parse_algorithm(std::string_view name);
[[nodiscard]] SolveResult solve(const State& initial, const SolveOptions& options = {});
[[nodiscard]] bool verifies_solution(const State& initial, std::span<const Move> solution) noexcept;

} // namespace pyraminx

