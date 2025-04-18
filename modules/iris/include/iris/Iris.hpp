#pragma once

#include "core/ITickable.hpp"
#include "core/Module.hpp"

namespace atlas::iris {
class Iris final : public core::Module, public core::ITickable {
  public:
    explicit Iris(core::IEngine& engine);

    auto start() -> void override;
    auto shutdown() -> void override;

    auto tick() -> void override;
    [[nodiscard]] auto get_tick_rate() const -> unsigned override;
};
} // namespace atlas::iris
