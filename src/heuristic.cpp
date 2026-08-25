#include "pyraminx/heuristic.hpp"

#include <algorithm>
#include <array>
#include <memory>
#include <queue>
#include <stdexcept>
#include <string>

namespace pyraminx {
namespace {

using Key = std::uint32_t;

constexpr Key encode(std::array<std::uint8_t, 3> positions) noexcept {
    return static_cast<Key>(positions[0]) |
           (static_cast<Key>(positions[1]) << 6U) |
           (static_cast<Key>(positions[2]) << 12U);
}

constexpr std::array<std::uint8_t, 3> decode(Key key) noexcept {
    return {static_cast<std::uint8_t>(key & 63U),
            static_cast<std::uint8_t>((key >> 6U) & 63U),
            static_cast<std::uint8_t>((key >> 12U) & 63U)};
}

const std::unordered_map<Key, std::uint8_t>& pattern_distances() {
    static const auto distances = [] {
        std::unordered_map<Key, std::uint8_t> result;
        std::queue<Key> frontier;
        const Key goal = encode({0, 21, 42});
        result.emplace(goal, 0);
        frontier.push(goal);

        while (!frontier.empty()) {
            const auto current = frontier.front();
            frontier.pop();
            const auto positions = decode(current);
            const auto distance = result.at(current);
            for (const auto move : kAllMoves) {
                auto next_positions = positions;
                for (auto& position : next_positions) {
                    position = position_after_move(position, move);
                }
                const auto next = encode(next_positions);
                if (result.emplace(next, static_cast<std::uint8_t>(distance + 1U)).second) {
                    frontier.push(next);
                }
            }
        }
        return result;
    }();
    return distances;
}

} // namespace

std::string_view to_string(HeuristicKind kind) noexcept {
    switch (kind) {
        case HeuristicKind::Zero: return "zero";
        case HeuristicKind::Misplaced: return "misplaced";
        case HeuristicKind::PatternDatabase: return "pdb";
    }
    return "unknown";
}

HeuristicKind parse_heuristic(std::string_view name) {
    if (name == "zero") return HeuristicKind::Zero;
    if (name == "misplaced") return HeuristicKind::Misplaced;
    if (name == "pdb" || name == "pattern-database") return HeuristicKind::PatternDatabase;
    throw std::invalid_argument("unknown heuristic '" + std::string(name) + "'");
}

std::uint8_t misplaced_heuristic(const State& state) noexcept {
    std::size_t misplaced = 0;
    for (std::size_t index = 0; index < state.stickers.size(); ++index) {
        misplaced += state.stickers[index] != index ? 1U : 0U;
    }
    // A single outer-layer move changes at most 36 sticker positions.
    return static_cast<std::uint8_t>((misplaced + 35U) / 36U);
}

PatternDatabase::PatternDatabase() : distances_(&pattern_distances()) {}

PatternDatabase::Key PatternDatabase::project(const State& state) noexcept {
    std::array<std::uint8_t, 3> positions{};
    for (std::size_t position = 0; position < state.stickers.size(); ++position) {
        for (std::size_t tracked = 0; tracked < kTracked.size(); ++tracked) {
            if (state.stickers[position] == kTracked[tracked]) {
                positions[tracked] = static_cast<std::uint8_t>(position);
            }
        }
    }
    return encode(positions);
}

std::uint8_t PatternDatabase::operator()(const State& state) const noexcept {
    const auto found = distances_->find(project(state));
    return found == distances_->end() ? 0U : found->second;
}

Heuristic::Heuristic(HeuristicKind kind) : kind_(kind) {
    if (kind_ == HeuristicKind::PatternDatabase) database_.emplace();
}

std::uint8_t Heuristic::operator()(const State& state) const noexcept {
    if (kind_ == HeuristicKind::Zero) return 0;
    const auto misplaced = misplaced_heuristic(state);
    if (kind_ == HeuristicKind::Misplaced) return misplaced;
    return std::max(misplaced, (*database_)(state));
}

std::size_t Heuristic::database_entries() const noexcept {
    return database_ ? database_->entries() : 0U;
}

} // namespace pyraminx
