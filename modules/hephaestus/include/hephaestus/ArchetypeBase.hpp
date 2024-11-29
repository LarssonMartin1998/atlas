#pragma once

namespace atlas::hephaestus {
class ArchetypeBase {
  public:
    virtual ~ArchetypeBase() = 0;

    ArchetypeBase(const ArchetypeBase &) = default;
    auto operator=(const ArchetypeBase &) -> ArchetypeBase & = default;

    ArchetypeBase(ArchetypeBase &&) = delete;
    auto operator=(ArchetypeBase &&) -> ArchetypeBase & = delete;

  protected:
    ArchetypeBase() = default;
};

inline ArchetypeBase::~ArchetypeBase() = default;
} // namespace atlas::hephaestus
