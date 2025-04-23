#include "iris/Iris.hpp"

// clang-format off
#include <glad/gl.h>
#include <GLFW/glfw3.h>
// clang-format on
#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include <cassert>
#include <cstdlib>
#include <print>

static const char* const shaderCodeVertex = R"(
#version 450 core
layout(std140, binding = 0) uniform PerFrameData
{
	uniform mat4 MVP;
	uniform int isWireframe;
};
layout (location=0) out vec3 color;
const vec3 pos[8] = vec3[8](
	vec3(-1.0,-1.0, 1.0),
	vec3( 1.0,-1.0, 1.0),
	vec3( 1.0, 1.0, 1.0),
	vec3(-1.0, 1.0, 1.0),

	vec3(-1.0,-1.0,-1.0),
	vec3( 1.0,-1.0,-1.0),
	vec3( 1.0, 1.0,-1.0),
	vec3(-1.0, 1.0,-1.0)
);
const vec3 col[8] = vec3[8](
	vec3( 1.0, 0.0, 0.0),
	vec3( 0.0, 1.0, 0.0),
	vec3( 0.0, 0.0, 1.0),
	vec3( 1.0, 1.0, 0.0),

	vec3( 1.0, 1.0, 0.0),
	vec3( 0.0, 0.0, 1.0),
	vec3( 0.0, 1.0, 0.0),
	vec3( 1.0, 0.0, 0.0)
);
const int indices[36] = int[36](
	// front
	0, 1, 2, 2, 3, 0,
	// right
	1, 5, 6, 6, 2, 1,
	// back
	7, 6, 5, 5, 4, 7,
	// left
	4, 0, 3, 3, 7, 4,
	// bottom
	4, 5, 1, 1, 0, 4,
	// top
	3, 2, 6, 6, 7, 3
);
void main()
{
	int idx = indices[gl_VertexID];
	gl_Position = MVP * vec4(pos[idx], 1.0);
	color = isWireframe > 0 ? vec3(0.0) : col[idx];
}
)";

static const char* const shaderCodeFragment = R"(
#version 450 core
layout (location=0) in vec3 color;
layout (location=0) out vec4 out_FragColor;
void main()
{
	out_FragColor = vec4(0.32, 0.4, 0.1, 1.0);
	// out_FragColor = vec4(color, 1.0);
};
)";

struct PerFrameData {
    glm::mat4 model_view_projection{1.0F};
    int is_wireframe{0};
};

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
        auto is_ok = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &is_ok);
        if (!is_ok) {
            auto len = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
            std::string log(len, '\0');
            glGetShaderInfoLog(shader, len, nullptr, log.data());
            std::println(stderr, "{} compile failed:\n{}", name, log);
            std::exit(1);
        }
    };

    auto checkProgram = [&](GLuint prog) {
        auto is_ok = 0;
        glGetProgramiv(prog, GL_LINK_STATUS, &is_ok);
        if (!is_ok) {
            auto len = 0;
            glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
            std::string log(len, '\0');
            glGetProgramInfoLog(prog, len, nullptr, log.data());
            std::println(stderr, "Program link failed:\n{}", log);
            std::exit(1);
        }
    };

    buffer_size = sizeof(PerFrameData);
    glCreateBuffers(1, &per_frame_data_buf);
    glNamedBufferStorage(per_frame_data_buf, buffer_size, nullptr,
                         GL_DYNAMIC_STORAGE_BIT);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, per_frame_data_buf, 0, buffer_size);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-1.0F, -1.0F);

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
    auto width = 0;
    auto height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    glClearColor(1.0F, 1.0F, 1.0F, 1.0F);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const auto ratio = static_cast<float>(width) / static_cast<float>(height);
    const auto one = glm::vec3(1.0F);
    const auto identity = glm::mat4(1.0F);
    const auto time = static_cast<float>(glfwGetTime());
    const auto rotation = glm::vec3(0.0F, 0.0F, -3.5F);
    const auto matrix =
        glm::rotate(glm::translate(identity, rotation), time, one);
    const auto perspective = glm::perspective(45.0F, ratio, 0.1F, 1000.0F);

    PerFrameData per_frame_data{
        .model_view_projection = perspective * matrix,
        .is_wireframe = 0,
    };

    glNamedBufferSubData(per_frame_data_buf, 0, buffer_size, &per_frame_data);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    constexpr auto num_cube_vertices = 36;
    glDrawArrays(GL_TRIANGLES, 0, num_cube_vertices);

    per_frame_data.is_wireframe = 1;
    glNamedBufferSubData(per_frame_data_buf, 0, buffer_size, &per_frame_data);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawArrays(GL_TRIANGLES, 0, num_cube_vertices);

    glfwSwapBuffers(window);

    glfwPollEvents();
}

auto Iris::get_tick_rate() const -> unsigned { return 1; }
} // namespace atlas::iris
