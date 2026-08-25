#include "pyraminx/solver.hpp"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>

namespace {

struct Row {
    std::size_t depth{};
    pyraminx::Algorithm algorithm{};
    pyraminx::HeuristicKind heuristic{};
    std::size_t solved{};
    std::vector<std::size_t> lengths;
    std::vector<std::size_t> expanded;
    std::vector<double> milliseconds;
    std::vector<std::size_t> bytes;
    std::vector<double> accuracy;
};

template <typename T>
double median(std::vector<T> values) {
    if (values.empty()) return 0.0;
    std::sort(values.begin(), values.end());
    const auto middle = values.size() / 2U;
    if (values.size() % 2U) return static_cast<double>(values[middle]);
    return (static_cast<double>(values[middle - 1U]) + static_cast<double>(values[middle])) / 2.0;
}

} // namespace

int main(int argc, char** argv) {
    std::size_t samples = 3;
    std::size_t maximum_depth = 4;
    if (argc > 1) samples = std::stoull(argv[1]);
    if (argc > 2) maximum_depth = std::stoull(argv[2]);

    std::mt19937 random(0xC0FFEEU);
    std::vector<Row> rows;
    for (std::size_t depth = 1; depth <= maximum_depth; ++depth) {
        std::vector<pyraminx::State> cases;
        for (std::size_t sample = 0; sample < samples; ++sample) {
            cases.push_back(pyraminx::apply_moves(pyraminx::State::solved(),
                                                  pyraminx::make_scramble(depth, random)));
        }
        for (const auto algorithm : {pyraminx::Algorithm::BreadthFirst,
                                     pyraminx::Algorithm::AStar,
                                     pyraminx::Algorithm::IDAStar}) {
            Row row;
            row.depth = depth;
            row.algorithm = algorithm;
            row.heuristic = algorithm == pyraminx::Algorithm::BreadthFirst
                                ? pyraminx::HeuristicKind::Zero
                                : pyraminx::HeuristicKind::PatternDatabase;
            for (const auto& state : cases) {
                pyraminx::SolveOptions options;
                options.algorithm = algorithm;
                options.heuristic = row.heuristic;
                options.time_limit = std::chrono::seconds(10);
                const auto result = pyraminx::solve(state, options);
                if (!result.solved) continue;
                ++row.solved;
                row.lengths.push_back(result.moves.size());
                row.expanded.push_back(result.stats.nodes_expanded);
                row.milliseconds.push_back(static_cast<double>(result.stats.elapsed.count()) / 1000.0);
                row.bytes.push_back(result.stats.estimated_peak_bytes);
                row.accuracy.push_back(result.moves.empty() ? 1.0
                    : static_cast<double>(result.stats.initial_heuristic) / static_cast<double>(result.moves.size()));
            }
            rows.push_back(std::move(row));
        }
    }

    std::cout << "| Scramble depth | Solver | Heuristic | Solved | Median solution | Median expanded | Median time (ms) | Est. peak (KiB) | Heuristic accuracy |\n"
                 "|---:|---|---|---:|---:|---:|---:|---:|---:|\n";
    for (const auto& row : rows) {
        std::cout << "| " << row.depth << " | " << pyraminx::to_string(row.algorithm)
                  << " | " << pyraminx::to_string(row.heuristic) << " | " << row.solved << '/' << samples
                  << " | " << median(row.lengths) << " | " << median(row.expanded)
                  << " | " << std::fixed << std::setprecision(3) << median(row.milliseconds)
                  << " | " << std::setprecision(1) << median(row.bytes) / 1024.0
                  << " | " << std::setprecision(0) << median(row.accuracy) * 100.0 << "% |\n";
    }
    return EXIT_SUCCESS;
}

