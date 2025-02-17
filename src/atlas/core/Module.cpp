#include "core/Module.hpp"

namespace atlas::core {
Module::Module(IEngine& engine) : engine(engine) {}

auto Module::get_engine() const -> IEngine& { return engine; }
auto Module::start() -> void {}
auto Module::shutdown() -> void {}
} // namespace atlas::core
