#pragma once

#include "core/IModule.hpp"
#include "core/ITickable.hpp"

namespace atlas::hephaestus {
class IHephaestus : public virtual core::IModule, public core::ITickable {
  public:
    ~IHephaestus() override = default;

    IHephaestus(const IHephaestus &) = delete;
    auto operator=(const IHephaestus &) -> IHephaestus & = delete;

    IHephaestus(IHephaestus &&) = delete;
    auto operator=(IHephaestus &&) -> IHephaestus & = delete;

  protected:
    IHephaestus() = default;
};
} // namespace atlas::hephaestus
