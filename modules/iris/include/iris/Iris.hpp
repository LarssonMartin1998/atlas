#pragma once

#include "core/ITickable.hpp"
#include "core/Module.hpp"
#include <cstdint>

class GLFWwindow;

namespace atlas::iris {
class Iris final : public core::Module, public core::ITickable {
  public:
    explicit Iris(core::IEngine& engine);

    auto start() -> void override;
    auto shutdown() -> void override;

    auto tick() -> void override;
    [[nodiscard]] auto get_tick_rate() const -> unsigned override;

  private:
    GLFWwindow* window = nullptr;
    signed long int buffer_size = 0;
    std::uint32_t per_frame_data_buf = 0;
    std::uint32_t vertexShader = 0;
    std::uint32_t fragmentShader = 0;
    std::uint32_t program = 0;
    std::uint32_t vertex_array_object = 0;
};
} // namespace atlas::iris
