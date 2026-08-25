#include "pyraminx/solver.hpp"

#include <chrono>
#include <cstdlib>
#include <exception>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>

namespace {

void usage(std::ostream& output) {
    output << "Pyraminx search laboratory\n\n"
              "Usage: pyraminx-cli [options]\n"
              "  --scramble \"U4 R3' L2\"  Use an explicit scramble\n"
              "  --random N                 Generate N moves (default: 4)\n"
              "  --seed N                   Random seed (default: 2025)\n"
              "  --algorithm bfs|astar|idastar\n"
              "  --heuristic zero|misplaced|pdb\n"
              "  --time-limit MS            Stop after this many milliseconds\n"
              "  --node-limit N             Stop after this many expansions\n"
              "  --no-color                 Disable ANSI colors\n"
              "  --help                     Show this message\n";
}

std::string require_value(int& index, int argc, char** argv) {
    if (++index >= argc) throw std::invalid_argument("missing value after " + std::string(argv[index - 1]));
    return argv[index];
}

} // namespace

int main(int argc, char** argv) {
    try {
        pyraminx::SolveOptions options;
        std::size_t random_length = 4;
        std::uint32_t seed = 2025;
        std::string notation;
        bool explicit_scramble = false;
        bool color = true;

        for (int index = 1; index < argc; ++index) {
            const std::string argument = argv[index];
            if (argument == "--help" || argument == "-h") {
                usage(std::cout);
                return EXIT_SUCCESS;
            } else if (argument == "--scramble") {
                notation = require_value(index, argc, argv);
                explicit_scramble = true;
            } else if (argument == "--random") {
                random_length = std::stoull(require_value(index, argc, argv));
            } else if (argument == "--seed") {
                seed = static_cast<std::uint32_t>(std::stoul(require_value(index, argc, argv)));
            } else if (argument == "--algorithm") {
                options.algorithm = pyraminx::parse_algorithm(require_value(index, argc, argv));
            } else if (argument == "--heuristic") {
                options.heuristic = pyraminx::parse_heuristic(require_value(index, argc, argv));
            } else if (argument == "--time-limit") {
                options.time_limit = std::chrono::milliseconds(std::stoll(require_value(index, argc, argv)));
            } else if (argument == "--node-limit") {
                options.node_limit = std::stoull(require_value(index, argc, argv));
            } else if (argument == "--no-color") {
                color = false;
            } else {
                throw std::invalid_argument("unknown option '" + argument + "'");
            }
        }

        std::mt19937 random(seed);
        const auto scramble = explicit_scramble ? pyraminx::parse_moves(notation)
                                                : pyraminx::make_scramble(random_length, random);
        const auto initial = pyraminx::apply_moves(pyraminx::State::solved(), scramble);

        std::cout << "Scramble:  " << pyraminx::format_moves(scramble) << "\n"
                  << "Algorithm: " << pyraminx::to_string(options.algorithm) << "\n"
                  << "Heuristic: " << (options.algorithm == pyraminx::Algorithm::BreadthFirst
                                            ? "none"
                                            : pyraminx::to_string(options.heuristic)) << "\n\n"
                  << pyraminx::render_net(initial, color) << '\n';

        const auto result = pyraminx::solve(initial, options);
        if (!result.solved) {
            std::cout << "No solution within the configured resource limit.\n";
            return EXIT_FAILURE;
        }

        const double milliseconds = static_cast<double>(result.stats.elapsed.count()) / 1000.0;
        std::cout << "Solution:  " << pyraminx::format_moves(result.moves) << "\n"
                  << "Length:    " << result.moves.size() << " (optimal: "
                  << (result.optimal ? "yes" : "no") << ")\n"
                  << "Expanded:  " << result.stats.nodes_expanded << " nodes\n"
                  << "Generated: " << result.stats.nodes_generated << " nodes\n"
                  << "Frontier:  " << result.stats.peak_frontier << " states peak\n"
                  << "Memory:    " << result.stats.estimated_peak_bytes / 1024 << " KiB estimated peak\n"
                  << "Time:      " << std::fixed << std::setprecision(3) << milliseconds << " ms\n"
                  << "Verified:  " << (pyraminx::verifies_solution(initial, result.moves) ? "yes" : "NO") << "\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << "\n\n";
        usage(std::cerr);
        return EXIT_FAILURE;
    }
}

