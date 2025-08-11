#pragma once

#include <taskflow/taskflow.hpp>

#include "hephaestus/ArchetypeMap.hpp"

namespace atlas::core {
class IEngine;
} // namespace atlas::core

namespace atlas::hephaestus {
class SystemBase {
  public:
    virtual ~SystemBase() = 0;

    SystemBase(const SystemBase&) = delete;
    auto operator=(const SystemBase&) -> SystemBase& = delete;

    SystemBase(SystemBase&&) = delete;
    auto operator=(SystemBase&&) -> SystemBase& = delete;

    virtual auto set_concurrent_systems(std::size_t estimate) -> void = 0;
    virtual auto execute(const core::IEngine& engine, tf::Subflow& subflow) -> void = 0;
    virtual auto cache_affected_archetypes(const ArchetypeMap& archetypes) -> void = 0;
    virtual auto create_query() -> void = 0;

  protected:
    SystemBase() = default;
};

inline SystemBase::~SystemBase() = default;
} // namespace atlas::hephaestus
