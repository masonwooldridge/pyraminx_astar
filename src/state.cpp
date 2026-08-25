#include "pyraminx/state.hpp"

#include <algorithm>
#include <array>
#include <iomanip>
#include <numeric>
#include <sstream>
#include <stdexcept>

namespace pyraminx {
namespace {

using Cycle = std::array<std::uint8_t, 3>;
using Cycles = std::vector<Cycle>;

const std::array<Cycles, 16> kClockwiseCycles{{
    {{{0, 47, 25}}},
    {{{9, 16, 57}}},
    {{{15, 63, 32}}},
    {{{31, 41, 48}}},
    {{{1, 40, 27}}, {{2, 46, 26}}, {{3, 45, 20}}},
    {{{4, 19, 59}}, {{10, 18, 58}}, {{11, 17, 52}}},
    {{{8, 61, 35}}, {{13, 62, 34}}, {{14, 56, 33}}},
    {{{24, 36, 49}}, {{29, 42, 50}}, {{30, 43, 51}}},
    {{{4, 35, 29}}, {{5, 39, 28}}, {{6, 38, 22}}, {{7, 44, 21}}, {{8, 43, 17}}},
    {{{1, 49, 24}}, {{5, 53, 23}}, {{6, 54, 22}}, {{12, 60, 21}}, {{13, 61, 20}}},
    {{{3, 36, 59}}, {{6, 38, 54}}, {{7, 37, 60}}, {{11, 40, 51}}, {{12, 39, 55}}},
    {{{19, 56, 45}}, {{22, 54, 38}}, {{23, 55, 44}}, {{27, 52, 33}}, {{28, 53, 37}}},
    {{{9, 32, 31}}, {{10, 34, 30}}, {{11, 33, 24}}, {{12, 37, 23}},
     {{13, 36, 19}}, {{14, 42, 18}}, {{15, 41, 16}}, {{48, 57, 63}},
     {{49, 59, 56}}, {{50, 58, 62}}, {{51, 52, 61}}, {{53, 60, 55}}},
    {{{0, 31, 63}}, {{2, 30, 62}}, {{3, 29, 56}}, {{7, 28, 55}},
     {{8, 27, 51}}, {{14, 26, 50}}, {{15, 25, 48}}, {{32, 47, 41}},
     {{33, 40, 43}}, {{34, 46, 42}}, {{35, 45, 36}}, {{37, 39, 44}}},
    {{{0, 57, 41}}, {{1, 52, 43}}, {{2, 58, 42}}, {{4, 49, 45}},
     {{5, 53, 44}}, {{9, 48, 47}}, {{10, 50, 46}}, {{16, 31, 25}},
     {{17, 24, 27}}, {{18, 30, 26}}, {{19, 29, 20}}, {{21, 23, 28}}},
    {{{0, 9, 15}}, {{1, 11, 8}}, {{2, 10, 14}}, {{3, 4, 13}},
     {{5, 12, 7}}, {{16, 63, 47}}, {{17, 61, 40}}, {{18, 62, 46}},
     {{20, 59, 35}}, {{21, 60, 39}}, {{25, 57, 32}}, {{26, 58, 34}}},
}};

constexpr std::array<std::string_view, 32> kMoveNames{
    "U1", "U1'", "L1", "L1'", "R1", "R1'", "B1", "B1'",
    "U2", "U2'", "L2", "L2'", "R2", "R2'", "B2", "B2'",
    "U3", "U3'", "L3", "L3'", "R3", "R3'", "B3", "B3'",
    "U4", "U4'", "L4", "L4'", "R4", "R4'", "B4", "B4'",
};

constexpr std::array<char, 4> kFaceLetters{'R', 'G', 'Y', 'B'};
constexpr std::array<std::string_view, 4> kAnsi{"\033[91m", "\033[92m", "\033[93m", "\033[94m"};

std::string sticker_text(std::uint8_t sticker, bool color) {
    const auto face = static_cast<std::size_t>(sticker / 16U);
    if (!color) {
        return std::string(1, kFaceLetters[face]);
    }
    return std::string(kAnsi[face]) + kFaceLetters[face] + "\033[0m";
}

} // namespace

State State::solved() noexcept {
    State state;
    std::iota(state.stickers.begin(), state.stickers.end(), std::uint8_t{0});
    return state;
}

bool State::is_solved() const noexcept { return *this == solved(); }

bool State::has_valid_stickers() const noexcept {
    std::array<bool, kStickerCount> seen{};
    for (const auto sticker : stickers) {
        if (sticker >= kStickerCount || seen[sticker]) {
            return false;
        }
        seen[sticker] = true;
    }
    return true;
}

std::size_t StateHash::operator()(const State& state) const noexcept {
    std::size_t hash = sizeof(std::size_t) == 8 ? 1469598103934665603ULL : 2166136261U;
    constexpr std::size_t prime64 = 1099511628211ULL;
    constexpr std::size_t prime32 = 16777619U;
    for (const auto sticker : state.stickers) {
        hash ^= sticker;
        hash *= sizeof(std::size_t) == 8 ? prime64 : prime32;
    }
    return hash;
}

State apply_move(const State& state, Move move) noexcept {
    State result = state;
    const auto value = static_cast<std::uint8_t>(move);
    const bool clockwise = (value % 2U) == 0U;
    const auto& cycles = kClockwiseCycles[value / 2U];
    for (const auto& cycle : cycles) {
        const auto [a, b, c] = cycle;
        if (clockwise) {
            result.stickers[a] = state.stickers[b];
            result.stickers[b] = state.stickers[c];
            result.stickers[c] = state.stickers[a];
        } else {
            result.stickers[a] = state.stickers[c];
            result.stickers[b] = state.stickers[a];
            result.stickers[c] = state.stickers[b];
        }
    }
    return result;
}

std::uint8_t position_after_move(std::uint8_t position, Move move) noexcept {
    const auto value = static_cast<std::uint8_t>(move);
    const bool clockwise = (value % 2U) == 0U;
    for (const auto& cycle : kClockwiseCycles[value / 2U]) {
        const auto [a, b, c] = cycle;
        if (clockwise) {
            if (position == a) return c;
            if (position == b) return a;
            if (position == c) return b;
        } else {
            if (position == a) return b;
            if (position == b) return c;
            if (position == c) return a;
        }
    }
    return position;
}

State apply_moves(State state, std::span<const Move> moves) noexcept {
    for (const auto move : moves) {
        state = apply_move(state, move);
    }
    return state;
}

std::string_view to_string(Move move) noexcept {
    return kMoveNames[static_cast<std::uint8_t>(move)];
}

std::optional<Move> parse_move(std::string_view token) noexcept {
    for (std::size_t index = 0; index < kMoveNames.size(); ++index) {
        if (kMoveNames[index] == token) {
            return static_cast<Move>(index);
        }
    }
    return std::nullopt;
}

std::vector<Move> parse_moves(const std::string& notation) {
    std::istringstream input(notation);
    std::vector<Move> moves;
    std::string token;
    while (input >> token) {
        const auto move = parse_move(token);
        if (!move) {
            throw std::invalid_argument("unknown move '" + token + "'");
        }
        moves.push_back(*move);
    }
    return moves;
}

std::string format_moves(std::span<const Move> moves) {
    std::ostringstream output;
    for (std::size_t index = 0; index < moves.size(); ++index) {
        if (index != 0) {
            output << ' ';
        }
        output << to_string(moves[index]);
    }
    return output.str();
}

std::vector<Move> make_scramble(std::size_t length, std::mt19937& random) {
    std::uniform_int_distribution<std::uint8_t> distribution(0, 31);
    std::vector<Move> moves;
    moves.reserve(length);
    while (moves.size() < length) {
        const auto candidate = static_cast<Move>(distribution(random));
        if (!moves.empty()) {
            const auto previous = static_cast<std::uint8_t>(moves.back());
            const auto next = static_cast<std::uint8_t>(candidate);
            if (previous / 2U == next / 2U) {
                continue;
            }
        }
        moves.push_back(candidate);
    }
    return moves;
}

std::string render_net(const State& state, bool ansi_color) {
    std::ostringstream output;
    for (std::size_t face = 0; face < 4; ++face) {
        output << "Face " << kFaceLetters[face] << ":\n";
        std::size_t offset = face * 16;
        for (std::size_t row = 0; row < 4; ++row) {
            output << std::string((3U - row) * 2U, ' ');
            const auto width = 2U * row + 1U;
            for (std::size_t column = 0; column < width; ++column) {
                output << sticker_text(state.stickers[offset++], ansi_color) << ' ';
            }
            output << '\n';
        }
    }
    return output.str();
}

} // namespace pyraminx
