find_package(PkgConfig REQUIRED)
pkg_check_modules(STB REQUIRED stb)

target_include_directories(atlas PUBLIC ${STB_INCLUDE_DIRS})
