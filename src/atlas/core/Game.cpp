#include "core/Game.hpp"

#include <cassert>

namespace atlas::core {
auto Game::get_engine() const -> IEngine & {
    const auto shared = engine.lock();
    assert(shared);
    return *shared;
}

auto Game::set_engine(std::weak_ptr<IEngine> engine_ptr) -> void {
    engine = engine_ptr;
}
} // namespace atlas::core
