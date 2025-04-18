#include "iris/Iris.hpp"

// clang-format off
// Important that glad is included before GLFW
#include <glad/gl.h>
#include <GLFW/glfw3.h>
// clang-format on

#include <cassert>
#include <cstdlib>
#include <print>

namespace atlas::iris {
Iris::Iris(core::IEngine& engine) : core::Module(engine) {}

auto Iris::start() -> void {
    std::println("Iris::start()");

    glfwSetErrorCallback([](int error, const char* description) {
        std::println(stderr, "GLFW Error {}: {}", error, description);
    });

    assert(glfwInit() != 0 && "GLFW failed to initialize");
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    constexpr auto xres = 1280;
    constexpr auto yres = 720;
    auto* window = glfwCreateWindow(xres, yres, "Daedalus", nullptr, nullptr);
    if (window == nullptr) {
        std::println(stderr, "Failed to create GLFW window");
        glfwTerminate();
    }

    glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode,
                                  int action, int mods) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    });

    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval(1);
}

auto Iris::shutdown() -> void {
    std::println("Iris::shutdown()");
    glfwTerminate();
}

auto Iris::tick() -> void { std::println("Iris tick"); }
auto Iris::get_tick_rate() const -> unsigned { return 1; }
} // namespace atlas::iris
