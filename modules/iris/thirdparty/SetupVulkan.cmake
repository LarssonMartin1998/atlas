find_package(Vulkan REQUIRED)

if(APPLE)
  target_compile_definitions(atlas PUBLIC VK_USE_PLATFORM_MACOS_MVK
                                          VK_ENABLE_BETA_EXTENSIONS)
elseif(UNIX)
  target_compile_definitions(atlas PUBLIC VK_USE_PLATFORM_WAYLAND_KHR)
endif()

if(NOT Vulkan_FOUND)
  message(FATAL_ERROR "Vulkan not found")
else()
  target_include_directories(atlas SYSTEM PUBLIC ${Vulkan_INCLUDE_DIRS})
  target_link_libraries(atlas PRIVATE Vulkan::Vulkan)
endif()
