#include "core/Module.hpp"

namespace atlas::core {
Module::Module(IEngine& engine)
    : engine(engine) {}

auto Module::get_engine() const -> IEngine& {
    return engine;
}
auto Module::pre_start() -> void {}
auto Module::start() -> void {}
auto Module::post_start() -> void {}
auto Module::pre_shutdown() -> void {}
auto Module::shutdown() -> void {}
auto Module::post_shutdown() -> void {}
} // namespace atlas::core
