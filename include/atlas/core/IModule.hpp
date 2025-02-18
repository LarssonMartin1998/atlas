#pragma once

#include "core/IEngineHandle.hpp"
#include "core/IStartShutdown.hpp"

namespace atlas::core {
class IEngine;
} // namespace atlas::core

namespace atlas::core {
class IModule : public IStartShutdown, public IEngineHandle {
  public:
    ~IModule() override = default;

    IModule(const IModule&) = delete;
    auto operator=(const IModule&) -> IModule& = delete;

    IModule(IModule&&) = delete;
    auto operator=(IModule&&) -> IModule& = delete;

  protected:
    IModule() = default;
};
} // namespace atlas::core
