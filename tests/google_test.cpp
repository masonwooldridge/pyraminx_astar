#include "pyraminx/solver.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <random>
#include <unordered_set>

TEST(Moves, InverseRestoresEveryState) {
    std::mt19937 random(17);
    const auto state = pyraminx::apply_moves(
        pyraminx::State::solved(), pyraminx::make_scramble(12, random));
    for (const auto move : pyraminx::kAllMoves) {
        EXPECT_EQ(pyraminx::apply_move(pyraminx::apply_move(state, move),
                                       pyraminx::inverse(move)),
                  state)
            << pyraminx::to_string(move);
    }
}

TEST(Moves, EveryRotationHasOrderThree) {
    const auto solved = pyraminx::State::solved();
    for (std::size_t index = 0; index < pyraminx::kAllMoves.size(); index += 2) {
        auto state = solved;
        for (int turn = 0; turn < 3; ++turn) {
            state = pyraminx::apply_move(state, pyraminx::kAllMoves[index]);
        }
        EXPECT_EQ(state, solved) << pyraminx::to_string(pyraminx::kAllMoves[index]);
    }
}

TEST(State, ScramblesAreValidAndHashable) {
    std::mt19937 random(42);
    std::unordered_set<pyraminx::State, pyraminx::StateHash> states;
    for (std::size_t depth = 0; depth < 30; ++depth) {
        const auto state = pyraminx::apply_moves(
            pyraminx::State::solved(), pyraminx::make_scramble(depth, random));
        EXPECT_TRUE(state.has_valid_stickers());
        states.insert(state);
    }
    EXPECT_GT(states.size(), 20U);
}

TEST(Solvers, ReturnedSolutionsVerifyAndMatchBfs) {
    const auto initial = pyraminx::apply_moves(
        pyraminx::State::solved(), pyraminx::parse_moves("U1 L2 R3'"));
    pyraminx::SolveOptions options;
    options.algorithm = pyraminx::Algorithm::BreadthFirst;
    options.time_limit = std::chrono::seconds(20);
    const auto baseline = pyraminx::solve(initial, options);
    ASSERT_TRUE(baseline.solved);

    for (const auto algorithm : {pyraminx::Algorithm::AStar,
                                 pyraminx::Algorithm::IDAStar}) {
        options.algorithm = algorithm;
        options.heuristic = pyraminx::HeuristicKind::PatternDatabase;
        const auto result = pyraminx::solve(initial, options);
        EXPECT_TRUE(result.solved);
        EXPECT_TRUE(pyraminx::verifies_solution(initial, result.moves));
        EXPECT_EQ(result.moves.size(), baseline.moves.size());
    }
}

TEST(Heuristic, PatternDatabaseIsAdmissibleOnExactCases) {
    pyraminx::Heuristic heuristic(pyraminx::HeuristicKind::PatternDatabase);
    EXPECT_EQ(heuristic.database_entries(), 5'184U);
    for (const auto& notation : {"U1", "U2 L1", "R3 B2' U1"}) {
        const auto initial = pyraminx::apply_moves(
            pyraminx::State::solved(), pyraminx::parse_moves(notation));
        pyraminx::SolveOptions options;
        options.algorithm = pyraminx::Algorithm::BreadthFirst;
        const auto exact = pyraminx::solve(initial, options);
        ASSERT_TRUE(exact.solved);
        EXPECT_LE(heuristic(initial), exact.moves.size());
    }
}

