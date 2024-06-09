#include "atlas.hpp"

#include <iostream>

#include "raylib.h"

namespace atlas {
auto lib_entry() -> void {
    constexpr int screen_width = 800;
    constexpr int screen_height = 600;
    InitWindow(screen_width, screen_height, "Atlas");
    std::cout << "Hello from Atlas!\n";
}
} // namespace atlas
