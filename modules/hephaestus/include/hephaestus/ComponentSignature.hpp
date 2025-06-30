#pragma once

#include <cstdint>
#include <type_traits>

namespace atlas::hephaestus {

// Type-safe wrapper for component signature using bitmasking
class ComponentSignature {
public:
    using ValueType = std::uint64_t;

    constexpr ComponentSignature() = default;
    constexpr explicit ComponentSignature(ValueType value) : value_(value) {}

    constexpr auto operator==(const ComponentSignature& other) const -> bool {
        return value_ == other.value_;
    }

    constexpr auto operator!=(const ComponentSignature& other) const -> bool {
        return !(*this == other);
    }

    constexpr auto operator<(const ComponentSignature& other) const -> bool {
        return value_ < other.value_;
    }

    constexpr auto get_value() const -> ValueType {
        return value_;
    }

    constexpr auto has_component(ValueType component_bit) const -> bool {
        return (value_ & component_bit) != 0;
    }

    constexpr auto add_component(ValueType component_bit) -> ComponentSignature& {
        value_ |= component_bit;
        return *this;
    }

    constexpr auto remove_component(ValueType component_bit) -> ComponentSignature& {
        value_ &= ~component_bit;
        return *this;
    }

    constexpr auto is_subset_of(const ComponentSignature& other) const -> bool {
        return (value_ & other.value_) == value_;
    }

    constexpr auto intersects_with(const ComponentSignature& other) const -> bool {
        return (value_ & other.value_) != 0;
    }

    constexpr auto count_components() const -> int {
        return __builtin_popcountll(value_);
    }

private:
    ValueType value_ = 0;
};

// Component type ID generator using compile-time hash of type name
namespace detail {
    // Simple compile-time string hash using FNV-1a algorithm
    constexpr auto hash_string(const char* str) -> std::uint64_t {
        std::uint64_t hash = 14695981039346656037ULL; // FNV offset basis
        while (*str) {
            hash ^= static_cast<std::uint64_t>(*str++);
            hash *= 1099511628211ULL; // FNV prime
        }
        return hash;
    }
    
    // Extract position of first set bit (effectively log2 for powers of 2)
    constexpr auto get_bit_position(std::uint64_t hash) -> std::uint8_t {
        // Use a subset of hash bits to get a position in 0-63 range
        return static_cast<std::uint8_t>(hash % 64);
    }
}

// Get unique component type ID for a given component type
template<typename T>
constexpr auto get_component_type_id() -> std::uint8_t {
    constexpr auto hash = detail::hash_string(__PRETTY_FUNCTION__);
    return detail::get_bit_position(hash);
}

// Get component bitmask for a given component type
template<typename T>
constexpr auto get_component_bitmask() -> std::uint64_t {
    constexpr auto id = get_component_type_id<std::remove_cvref_t<T>>();
    return (1ULL << id);
}

// Hash functor for ComponentSignature
struct ComponentSignatureHash {
    constexpr auto operator()(const ComponentSignature& sig) const -> std::size_t {
        return static_cast<std::size_t>(sig.get_value());
    }
};

// Equality functor for ComponentSignature
struct ComponentSignatureEqual {
    constexpr auto operator()(const ComponentSignature& lhs, const ComponentSignature& rhs) const -> bool {
        return lhs == rhs;
    }
};

} // namespace atlas::hephaestus