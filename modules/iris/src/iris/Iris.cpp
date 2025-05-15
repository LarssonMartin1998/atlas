#include "iris/Iris.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

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
};
layout (location=0) out vec2 uv;
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
const vec2 texture_coordinates[4] = vec2[4](
    vec2(0.0, 0.0), // bottom-left
    vec2(1.0, 0.0), // bottom-right
    vec2(1.0, 1.0), // top-right
    vec2(0.0, 1.0)  // top-left
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
    int vertexID = gl_VertexID;
    int idx = indices[vertexID];
    gl_Position = MVP * vec4(pos[idx], 1.0);
    
    // Calculate which face we're on (0-5)
    int faceID = vertexID / 6;
    // Calculate which vertex we are within the face (0-5)
    int faceVertex = vertexID % 6;
    
    // Map the 6 vertices of each face (two triangles) to the 4 corners of a UV square
    int uvIdx;
    if (faceVertex == 0) uvIdx = 0;      // Bottom-left
    else if (faceVertex == 1) uvIdx = 1;  // Bottom-right
    else if (faceVertex == 2) uvIdx = 2;  // Top-right
    else if (faceVertex == 3) uvIdx = 2;  // Top-right (repeated)
    else if (faceVertex == 4) uvIdx = 3;  // Top-left
    else uvIdx = 0;                      // Bottom-left (repeated)
    
    uv = texture_coordinates[uvIdx];
}
)";

static const char* const shaderCodeFragment = R"(
#version 450 core
layout (location=0) in vec2 uv;
layout (location=0) out vec4 out_FragColor;
uniform sampler2D tex;
void main()
{
	out_FragColor = texture(tex, uv);
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

    auto texture = create_texture("data/ch2_sample3_STB.jpg");

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

    constexpr glm::vec3 clear_color(1.0F, 0.5F, 0.7F);
    glClearColor(clear_color[0], clear_color[1], clear_color[2], 1.0F);
    constexpr auto unsigned_color_buffer_bit =
        static_cast<unsigned>(GL_COLOR_BUFFER_BIT);
    constexpr auto unsigned_depth_buffer_bit =
        static_cast<unsigned>(GL_DEPTH_BUFFER_BIT);
    glClear(unsigned_color_buffer_bit | unsigned_depth_buffer_bit);

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
auto Iris::create_texture(std::string_view path) const -> Texture {
    int channels = 0;
    Texture texture;
    const std::uint8_t* image =
        stbi_load(path.data(), &texture.width, &texture.height, &channels, 3);

    glCreateTextures(GL_TEXTURE_2D, 1, &texture.id);
    glTextureParameteri(texture.id, GL_TEXTURE_MAX_LEVEL, 0);
    glTextureParameteri(texture.id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(texture.id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureStorage2D(texture.id, 1, GL_RGB8, texture.width, texture.height);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTextureSubImage2D(texture.id, 0, 0, 0, texture.width, texture.height,
                        GL_RGB, GL_UNSIGNED_BYTE, image);
    glBindTextures(0, 1, &texture.id);
    stbi_image_free((void*)image);
    return texture;
}
} // namespace atlas::iris
