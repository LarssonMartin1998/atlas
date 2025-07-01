#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <type_traits>
#include <typeinfo>

namespace atlas::hephaestus {

// Type-safe wrapper for component signature using bitmasking
// Supports up to 256 component types (4 * 64) without heap allocations
class ComponentSignature {
public:
    static constexpr size_t STORAGE_SIZE = 4; // 4 * 64 = 256 components
    using ValueType = std::uint64_t;
    using StorageType = std::array<ValueType, STORAGE_SIZE>;

    constexpr ComponentSignature() = default;
    constexpr explicit ComponentSignature(const StorageType& storage) : storage(storage) {}

    constexpr auto operator==(const ComponentSignature& other) const -> bool {
        return storage == other.storage;
    }

    constexpr auto operator!=(const ComponentSignature& other) const -> bool {
        return !(*this == other);
    }

    constexpr auto operator<(const ComponentSignature& other) const -> bool {
        // Lexicographic comparison
        for (size_t i = 0; i < STORAGE_SIZE; ++i) {
            if (storage[i] != other.storage[i]) {
                return storage[i] < other.storage[i];
            }
        }
        return false; // Equal
    }

    constexpr auto get_storage() const -> const StorageType& {
        return storage;
    }

    // Legacy compatibility - returns first storage element
    constexpr auto get_value() const -> ValueType {
        return storage[0];
    }

    constexpr auto has_component(size_t component_id) const -> bool {
        const size_t bucket = component_id / 64;
        const size_t bit = component_id % 64;
        return bucket < STORAGE_SIZE && (storage[bucket] & (1ULL << bit)) != 0;
    }

    constexpr auto add_component(size_t component_id) -> ComponentSignature& {
        const size_t bucket = component_id / 64;
        const size_t bit = component_id % 64;
        if (bucket < STORAGE_SIZE) {
            storage[bucket] |= (1ULL << bit);
        }
        return *this;
    }

    constexpr auto remove_component(size_t component_id) -> ComponentSignature& {
        const size_t bucket = component_id / 64;
        const size_t bit = component_id % 64;
        if (bucket < STORAGE_SIZE) {
            storage[bucket] &= ~(1ULL << bit);
        }
        return *this;
    }

    constexpr auto is_subset_of(const ComponentSignature& other) const -> bool {
        for (size_t i = 0; i < STORAGE_SIZE; ++i) {
            if ((storage[i] & other.storage[i]) != storage[i]) {
                return false;
            }
        }
        return true;
    }

    constexpr auto intersects_with(const ComponentSignature& other) const -> bool {
        for (size_t i = 0; i < STORAGE_SIZE; ++i) {
            if ((storage[i] & other.storage[i]) != 0) {
                return true;
            }
        }
        return false;
    }

    constexpr auto count_components() const -> int {
        int count = 0;
        for (const auto& bucket : storage) {
            count += __builtin_popcountll(bucket);
        }
        return count;
    }

    constexpr auto empty() const -> bool {
        for (const auto& bucket : storage) {
            if (bucket != 0) {
                return false;
            }
        }
        return true;
    }

private:
    StorageType storage{};
};

// Simple compile-time string hash using FNV-1a algorithm
constexpr auto hash_string(const char* str) -> std::uint64_t {
    std::uint64_t hash = 14695981039346656037ULL; // FNV offset basis
    while (*str) {
        hash ^= static_cast<std::uint64_t>(*str++);
        hash *= 1099511628211ULL; // FNV prime
    }
    return hash;
}

// Get unique component type ID for a given component type
// Now supports up to 256 components (STORAGE_SIZE * 64)
template<typename T>
inline auto get_component_type_id() -> size_t {
    // Normalize the type to remove cv-qualifiers and references
    using NormalizedType = std::remove_cvref_t<T>;
    
    // Use a type-specific approach that works with normalized types
    // This ensures const Position and Position get the same ID
    const auto hash = hash_string(typeid(NormalizedType).name());
    
    // Use modulo to ensure we stay within our storage capacity
    return hash % (ComponentSignature::STORAGE_SIZE * 64);
}

// Get component bitmask for a given component type (legacy compatibility)
template<typename T>
auto get_component_bitmask() -> std::uint64_t {
    const auto id = get_component_type_id<std::remove_cvref_t<T>>();
    if (id < 64) {
        return (1ULL << id);
    }
    return 0; // Component ID is outside first 64 components
}

// Hash functor for ComponentSignature
struct ComponentSignatureHash {
    constexpr auto operator()(const ComponentSignature& sig) const -> std::size_t {
        // Combine all storage buckets into a single hash
        std::size_t result = 0;
        const auto& storage = sig.get_storage();
        for (size_t i = 0; i < ComponentSignature::STORAGE_SIZE; ++i) {
            // Use a simple hash combining algorithm
            result ^= std::hash<std::uint64_t>{}(storage[i]) + 0x9e3779b9 + (result << 6) + (result >> 2);
        }
        return result;
    }
};

// Equality functor for ComponentSignature
struct ComponentSignatureEqual {
    constexpr auto operator()(const ComponentSignature& lhs, const ComponentSignature& rhs) const -> bool {
        return lhs == rhs;
    }
};

} // namespace atlas::hephaestus