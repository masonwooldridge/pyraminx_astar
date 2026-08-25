#include "pyraminx/solver.hpp"

#include <emscripten/emscripten.h>

#include <chrono>
#include <exception>
#include <sstream>
#include <string>

namespace {

std::string output;

std::string escape_json(const std::string& value) {
    std::string escaped;
    for (const char character : value) {
        if (character == '"' || character == '\\') escaped.push_back('\\');
        escaped.push_back(character);
    }
    return escaped;
}

} // namespace

extern "C" {

EMSCRIPTEN_KEEPALIVE const char* solve_scramble(const char* notation, const char* algorithm,
                                                const char* heuristic) {
    try {
        const auto scramble = pyraminx::parse_moves(notation ? notation : "");
        const auto initial = pyraminx::apply_moves(pyraminx::State::solved(), scramble);
        pyraminx::SolveOptions options;
        options.algorithm = pyraminx::parse_algorithm(algorithm ? algorithm : "astar");
        options.heuristic = pyraminx::parse_heuristic(heuristic ? heuristic : "pdb");
        options.time_limit = std::chrono::seconds(10);
        const auto result = pyraminx::solve(initial, options);
        std::ostringstream json;
        json << "{\"solved\":" << (result.solved ? "true" : "false")
             << ",\"solution\":\"" << escape_json(pyraminx::format_moves(result.moves)) << '"'
             << ",\"length\":" << result.moves.size()
             << ",\"expanded\":" << result.stats.nodes_expanded
             << ",\"generated\":" << result.stats.nodes_generated
             << ",\"peakFrontier\":" << result.stats.peak_frontier
             << ",\"memoryBytes\":" << result.stats.estimated_peak_bytes
             << ",\"milliseconds\":" << static_cast<double>(result.stats.elapsed.count()) / 1000.0
             << '}';
        output = json.str();
    } catch (const std::exception& error) {
        output = "{\"solved\":false,\"error\":\"" + escape_json(error.what()) + "\"}";
    }
    return output.c_str();
}

EMSCRIPTEN_KEEPALIVE const char* state_faces(const char* notation) {
    try {
        const auto state = pyraminx::apply_moves(
            pyraminx::State::solved(), pyraminx::parse_moves(notation ? notation : ""));
        output.clear();
        output.reserve(state.stickers.size());
        for (const auto sticker : state.stickers) output.push_back("RGYB"[sticker / 16U]);
    } catch (const std::exception& error) {
        output = "error:" + std::string(error.what());
    }
    return output.c_str();
}

} // extern "C"
