#pragma once

namespace atlas::core {
class IStartShutdown {
  public:
    virtual ~IStartShutdown() = default;

    IStartShutdown(const IStartShutdown &) = delete;
    auto operator=(const IStartShutdown &) -> IStartShutdown & = delete;

    IStartShutdown(IStartShutdown &&) = delete;
    auto operator=(IStartShutdown &&) -> IStartShutdown & = delete;

    virtual void start() = 0;
    virtual void shutdown() = 0;

  protected:
    IStartShutdown() = default;
};
} // namespace atlas::core
