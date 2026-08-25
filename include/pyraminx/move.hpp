#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string_view>

namespace pyraminx {

// Four axes times four independently rotating layers, in both directions.
enum class Move : std::uint8_t {
    U1, U1Prime, L1, L1Prime, R1, R1Prime, B1, B1Prime,
    U2, U2Prime, L2, L2Prime, R2, R2Prime, B2, B2Prime,
    U3, U3Prime, L3, L3Prime, R3, R3Prime, B3, B3Prime,
    U4, U4Prime, L4, L4Prime, R4, R4Prime, B4, B4Prime,
};

inline constexpr std::array<Move, 32> kAllMoves{
    Move::U1, Move::U1Prime, Move::L1, Move::L1Prime,
    Move::R1, Move::R1Prime, Move::B1, Move::B1Prime,
    Move::U2, Move::U2Prime, Move::L2, Move::L2Prime,
    Move::R2, Move::R2Prime, Move::B2, Move::B2Prime,
    Move::U3, Move::U3Prime, Move::L3, Move::L3Prime,
    Move::R3, Move::R3Prime, Move::B3, Move::B3Prime,
    Move::U4, Move::U4Prime, Move::L4, Move::L4Prime,
    Move::R4, Move::R4Prime, Move::B4, Move::B4Prime,
};

[[nodiscard]] constexpr Move inverse(Move move) noexcept {
    return static_cast<Move>(static_cast<std::uint8_t>(move) ^ 1U);
}

[[nodiscard]] std::string_view to_string(Move move) noexcept;
[[nodiscard]] std::optional<Move> parse_move(std::string_view token) noexcept;

} // namespace pyraminx

