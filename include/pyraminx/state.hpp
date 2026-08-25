#pragma once

#include "pyraminx/move.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <random>
#include <span>
#include <string>
#include <vector>

namespace pyraminx {

inline constexpr std::size_t kStickerCount = 64;

// Sticker IDs make the state compact, immutable-by-convention, and exactly hashable.
// A sticker's solved face is sticker_id / 16.
struct State {
    std::array<std::uint8_t, kStickerCount> stickers{};

    [[nodiscard]] static State solved() noexcept;
    [[nodiscard]] bool is_solved() const noexcept;
    [[nodiscard]] bool has_valid_stickers() const noexcept;
    bool operator==(const State&) const = default;
};

struct StateHash {
    [[nodiscard]] std::size_t operator()(const State& state) const noexcept;
};

[[nodiscard]] State apply_move(const State& state, Move move) noexcept;
[[nodiscard]] std::uint8_t position_after_move(std::uint8_t position, Move move) noexcept;
[[nodiscard]] State apply_moves(State state, std::span<const Move> moves) noexcept;
[[nodiscard]] std::vector<Move> parse_moves(const std::string& notation);
[[nodiscard]] std::string format_moves(std::span<const Move> moves);
[[nodiscard]] std::vector<Move> make_scramble(std::size_t length, std::mt19937& random);
[[nodiscard]] std::string render_net(const State& state, bool ansi_color = true);

} // namespace pyraminx
