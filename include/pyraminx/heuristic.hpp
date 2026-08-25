#pragma once

#include "pyraminx/state.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <unordered_map>

namespace pyraminx {

enum class HeuristicKind { Zero, Misplaced, PatternDatabase };

[[nodiscard]] std::string_view to_string(HeuristicKind kind) noexcept;
[[nodiscard]] HeuristicKind parse_heuristic(std::string_view name);
[[nodiscard]] std::uint8_t misplaced_heuristic(const State& state) noexcept;

class PatternDatabase {
public:
    PatternDatabase();

    [[nodiscard]] std::uint8_t operator()(const State& state) const noexcept;
    [[nodiscard]] std::size_t entries() const noexcept { return distances_->size(); }

private:
    using Key = std::uint32_t;
    static constexpr std::array<std::uint8_t, 3> kTracked{0, 21, 42};
    const std::unordered_map<Key, std::uint8_t>* distances_{};

    [[nodiscard]] static Key project(const State& state) noexcept;
};

class Heuristic {
public:
    explicit Heuristic(HeuristicKind kind = HeuristicKind::PatternDatabase);
    [[nodiscard]] std::uint8_t operator()(const State& state) const noexcept;
    [[nodiscard]] std::size_t database_entries() const noexcept;

private:
    HeuristicKind kind_;
    std::optional<PatternDatabase> database_;
};

} // namespace pyraminx
