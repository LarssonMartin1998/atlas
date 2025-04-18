add_library(glad STATIC)
target_include_directories(glad PUBLIC include/glad)
target_sources(glad PRIVATE src/glad/gl.c src/glad/vulkan.c)
if(MINGW)
  target_sources(glad PRIVATE src/glad/wgl.c)
endif()

target_link_libraries(atlas PRIVATE glad)
