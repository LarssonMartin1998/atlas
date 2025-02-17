#pragma once

namespace atlas::hephaestus {
class IComponent {
  public:
    virtual ~IComponent() = default;

    IComponent(const IComponent&) = default;
    auto operator=(const IComponent&) -> IComponent& = default;

    IComponent(IComponent&&) = delete;
    auto operator=(IComponent&&) -> IComponent& = delete;

  protected:
    IComponent() = default;
};
} // namespace atlas::hephaestus
