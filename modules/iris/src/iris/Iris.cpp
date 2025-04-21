#include "iris/Iris.hpp"

// clang-format off
// Important that glad is included before GLFW
#include <glad/gl.h>
#include <GLFW/glfw3.h>
// clang-format on

#include <cassert>
#include <cstdlib>
#include <print>

static const char* const shaderCodeVertex = R"(
#version 410 core
layout (location=0) out vec3 color;
const vec2 pos[3] = vec2[3](
	vec2(-0.6, -0.4),
	vec2( 0.6, -0.4),
	vec2( 0.0,  0.6)
);
const vec3 col[3] = vec3[3](
	vec3( 1.0, 0.0, 0.0 ),
	vec3( 0.0, 1.0, 0.0 ),
	vec3( 0.0, 0.0, 1.0 )
);
void main()
{
	gl_Position = vec4(pos[gl_VertexID], 0.0, 1.0);
	color = col[gl_VertexID];
}
)";

static const char* const shaderCodeFragment = R"(
#version 410 core
layout (location=0) in vec3 color;
layout (location=0) out vec4 out_FragColor;
void main()
{
	out_FragColor = vec4(color, 1.0);
}
)";

namespace atlas::iris {
Iris::Iris(core::IEngine& engine) : core::Module(engine) {}

auto Iris::start() -> void {
    std::println("Iris::start()");

    glfwSetErrorCallback([](int error, const char* description) {
        std::println(stderr, "GLFW Error {}: {}", error, description);
    });

    assert(glfwInit() != 0 && "GLFW failed to initialize");
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

    constexpr auto xres = 1280;
    constexpr auto yres = 720;
    window = glfwCreateWindow(xres, yres, "Daedalus", nullptr, nullptr);
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

    glGenVertexArrays(1, &vertex_array_object);
    glBindVertexArray(vertex_array_object);

    auto checkShader = [&](GLuint shader, char const* name) {
        GLint ok = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
        if (!ok) {
            GLint len = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
            std::string log(len, '\0');
            glGetShaderInfoLog(shader, len, nullptr, log.data());
            std::println(stderr, "{} compile failed:\n{}", name, log);
            std::exit(1);
        }
    };

    auto checkProgram = [&](GLuint prog) {
        GLint ok = 0;
        glGetProgramiv(prog, GL_LINK_STATUS, &ok);
        if (!ok) {
            GLint len = 0;
            glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
            std::string log(len, '\0');
            glGetProgramInfoLog(prog, len, nullptr, log.data());
            std::println(stderr, "Program link failed:\n{}", log);
            std::exit(1);
        }
    };
    const auto vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &shaderCodeVertex, nullptr);
    glCompileShader(vertexShader);
    const auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &shaderCodeFragment, nullptr);
    glCompileShader(fragmentShader);
    const auto program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    // …after glCompileShader(shaderVertex):
    checkShader(vertexShader, "Vertex shader");
    // …after glCompileShader(fragmentShader):
    checkShader(fragmentShader, "Fragment shader");
    // …after glLinkProgram(program):
    checkProgram(program);

    glUseProgram(program);
    glfwPollEvents();
}

auto Iris::shutdown() -> void {
    std::println("Iris::shutdown()");

    glDeleteProgram(program);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glDeleteVertexArrays(1, &vertex_array_object);

    glfwDestroyWindow(window);
    glfwTerminate();
}

auto Iris::tick() -> void {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    glfwSwapBuffers(window);

    glfwPollEvents();
}

auto Iris::get_tick_rate() const -> unsigned { return 1; }
} // namespace atlas::iris
