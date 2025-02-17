#include "core/Game.hpp"

#include <cassert>

namespace atlas::core {
auto Game::get_engine() const -> IEngine& {
    assert(engine && "Engine is not set, call set_engine() first.");
    return *engine;
}

auto Game::set_engine(IEngine& engine_ref) -> void { engine = &engine_ref; }
} // namespace atlas::core
