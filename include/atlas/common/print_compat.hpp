#pragma once

#include <format>
#include <iostream>

// Compatibility for std::println until full C++23 support is available
namespace std {
    template<typename... Args>
    inline void println(const std::format_string<Args...> fmt, Args&&... args) {
        std::cout << std::format(fmt, std::forward<Args>(args)...) << std::endl;
    }
    
    inline void println(const char* str) {
        std::cout << str << std::endl;
    }
    
    inline void println(const std::string& str) {
        std::cout << str << std::endl;
    }
}