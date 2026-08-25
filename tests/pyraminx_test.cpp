#include "pyraminx/solver.hpp"

#include <cstdlib>
#include <functional>
#include <iostream>
#include <random>
#include <string>
#include <unordered_set>
#include <vector>

namespace {

int failures = 0;

void check(bool condition, const char* expression, const char* test, int line) {
    if (!condition) {
        ++failures;
        std::cerr << "[FAIL] " << test << ':' << line << " — " << expression << '\n';
    }
}

#define CHECK(expression) check((expression), #expression, __func__, __LINE__)

void move_inverse_restores_state() {
    const auto solved = pyraminx::State::solved();
    for (const auto move : pyraminx::kAllMoves) {
        CHECK(pyraminx::apply_move(pyraminx::apply_move(solved, move), pyraminx::inverse(move)) == solved);
    }
}

void rotations_have_order_three() {
    const auto solved = pyraminx::State::solved();
    for (std::size_t index = 0; index < pyraminx::kAllMoves.size(); index += 2) {
        auto state = solved;
        for (int turn = 0; turn < 3; ++turn) state = pyraminx::apply_move(state, pyraminx::kAllMoves[index]);
        CHECK(state == solved);
    }
}

void generated_scrambles_are_valid() {
    std::mt19937 random(42);
    for (std::size_t depth = 0; depth < 50; ++depth) {
        const auto moves = pyraminx::make_scramble(depth, random);
        CHECK(moves.size() == depth);
        CHECK(pyraminx::apply_moves(pyraminx::State::solved(), moves).has_valid_stickers());
    }
}

void hashes_are_consistent() {
    std::mt19937 random(7);
    const auto scramble = pyraminx::make_scramble(8, random);
    const auto state = pyraminx::apply_moves(pyraminx::State::solved(), scramble);
    pyraminx::StateHash hash;
    CHECK(hash(state) == hash(state));
    std::unordered_set<pyraminx::State, pyraminx::StateHash> states;
    states.insert(state);
    states.insert(state);
    CHECK(states.size() == 1);
}

void every_solver_returns_a_valid_solution() {
    const auto scramble = pyraminx::parse_moves("U1 L2 R3'");
    const auto initial = pyraminx::apply_moves(pyraminx::State::solved(), scramble);
    for (const auto algorithm : {pyraminx::Algorithm::BreadthFirst,
                                 pyraminx::Algorithm::AStar,
                                 pyraminx::Algorithm::IDAStar}) {
        pyraminx::SolveOptions options;
        options.algorithm = algorithm;
        options.time_limit = std::chrono::seconds(20);
        const auto result = pyraminx::solve(initial, options);
        CHECK(result.solved);
        CHECK(pyraminx::verifies_solution(initial, result.moves));
    }
}

void informed_solvers_match_bfs_and_pdb_is_admissible() {
    const std::vector<std::string> cases{"U1", "U2 L1", "R3 B2' U1"};
    for (const auto& notation : cases) {
        const auto initial = pyraminx::apply_moves(pyraminx::State::solved(), pyraminx::parse_moves(notation));
        pyraminx::SolveOptions baseline_options;
        baseline_options.algorithm = pyraminx::Algorithm::BreadthFirst;
        baseline_options.time_limit = std::chrono::seconds(20);
        const auto baseline = pyraminx::solve(initial, baseline_options);
        CHECK(baseline.solved);

        pyraminx::Heuristic heuristic(pyraminx::HeuristicKind::PatternDatabase);
        CHECK(heuristic(initial) <= baseline.moves.size());

        for (const auto algorithm : {pyraminx::Algorithm::AStar, pyraminx::Algorithm::IDAStar}) {
            auto options = baseline_options;
            options.algorithm = algorithm;
            options.heuristic = pyraminx::HeuristicKind::PatternDatabase;
            const auto result = pyraminx::solve(initial, options);
            CHECK(result.solved);
            CHECK(result.moves.size() == baseline.moves.size());
        }
    }
}

} // namespace

int main() {
    const std::vector<std::pair<const char*, std::function<void()>>> tests{
        {"move/inverse", move_inverse_restores_state},
        {"rotation order", rotations_have_order_three},
        {"scramble validity", generated_scrambles_are_valid},
        {"state hashing", hashes_are_consistent},
        {"solution verification", every_solver_returns_a_valid_solution},
        {"optimality and admissibility", informed_solvers_match_bfs_and_pdb_is_admissible},
    };
    for (const auto& [name, test] : tests) {
        test();
        if (failures == 0) std::cout << "[PASS] " << name << '\n';
    }
    if (failures != 0) {
        std::cerr << failures << " assertion(s) failed\n";
        return EXIT_FAILURE;
    }
    std::cout << tests.size() << " test groups passed\n";
    return EXIT_SUCCESS;
}

