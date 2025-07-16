#pragma once

#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>
#include <functional>
#include <string_view>
#include <type_traits>

namespace atlas::hephaestus {

// Type-safe wrapper for archetype key using bitmasking
// Supports up to 256 component types (4 * 64) without heap allocations
// This is a hard limit (but is easily adjustable if needed). 256 components should be sufficient
// for most games. Might be a good idea to add this to a generated file via CMake later on and let
// it be controlled from the Game space.
class ArchetypeKey {
  public:
    static constexpr std::size_t STORAGE_SIZE = 4;
    using ValueType = std::uint64_t;
    using StorageType = std::array<ValueType, STORAGE_SIZE>;

    constexpr ArchetypeKey() = default;
    constexpr explicit ArchetypeKey(const StorageType& storage)
        : storage(storage) {}

    constexpr auto operator==(const ArchetypeKey& other) const -> bool {
        return storage == other.storage;
    }

    constexpr auto operator!=(const ArchetypeKey& other) const -> bool {
        return !(*this == other);
    }

    constexpr auto operator<(const ArchetypeKey& other) const -> bool {
        // Lexicographic comparison
        for (std::size_t i = 0; i < STORAGE_SIZE; ++i) {
            if (storage.at(i) != other.storage.at(i)) {
                return storage.at(i) < other.storage.at(i);
            }
        }
        return false; // Equal
    }

    [[nodiscard]] constexpr auto get_storage() const -> const StorageType& {
        return storage;
    }

    [[nodiscard]] constexpr auto has_component(size_t component_id) const -> bool {
        const auto bucket = component_id / 64;
        const auto bit = component_id % 64;
        return bucket < STORAGE_SIZE && (storage.at(bucket) & (1ULL << bit)) != 0;
    }

    constexpr auto add_component(std::size_t component_id) -> ArchetypeKey& {
        const auto bucket = component_id / 64;
        const auto bit = component_id % 64;
        if (bucket < STORAGE_SIZE) {
            storage.at(bucket) |= (1ULL << bit);
        }
        return *this;
    }

    [[nodiscard]] constexpr auto is_subset_of(const ArchetypeKey& other) const -> bool {
        for (std::size_t i = 0; i < STORAGE_SIZE; ++i) {
            if ((storage.at(i) & other.storage.at(i)) != storage.at(i)) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] constexpr auto intersects_with(const ArchetypeKey& other) const -> bool {
        for (std::size_t i = 0; i < STORAGE_SIZE; ++i) {
            if ((storage.at(i) & other.storage.at(i)) != 0) {
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] constexpr auto count_components() const -> std::uint32_t {
        std::uint32_t count = 0;
        for (const auto& bucket : storage) {
            count += static_cast<std::uint32_t>(std::popcount((bucket)));
        }
        return count;
    }

    [[nodiscard]] constexpr auto empty() const -> bool {
        return std::ranges::all_of(storage, [](const ValueType& bucket) {
            return bucket == 0;
        });
    }

  private:
    StorageType storage{};
};

constexpr auto next_id() -> std::size_t {
    static std::size_t id = 0;
    return id++;
}

template <typename T>
constexpr auto counter() -> std::size_t {
    static std::size_t value = next_id();
    return value;
}

// Simple compile-time string hash using FNV-1a algorithm
constexpr auto hash_string(std::string_view str) -> std::uint64_t {
    std::uint64_t hash = 14695981039346656037ULL; // FNV offset basis
    for (char c : str) {
        hash ^= static_cast<std::uint64_t>(c);
        hash *= 1099511628211ULL; // FNV prime
    }
    return hash;
}

template <typename T>
constexpr auto get_component_type_id() -> std::size_t {
    // Normalize the type to remove cv-qualifiers and references
    using NormalizedType = std::remove_cvref_t<T>;

    // Use a type-specific approach that works with normalized types
    // This ensures const Position and Position get the same ID
    const auto hash = counter<NormalizedType>();

    // Ensure we stay within our storage capacity
    return hash % (ArchetypeKey::STORAGE_SIZE * 64);
}

// Hash functor for ArchetypeKey
struct ArchetypeKeyHash {
    constexpr auto operator()(const ArchetypeKey& sig) const -> std::size_t {
        // Combine all storage buckets into a single hash
        std::size_t result = 0;
        const auto& storage = sig.get_storage();
        for (std::size_t i = 0; i < ArchetypeKey::STORAGE_SIZE; ++i) {
            // Use a simple hash combining algorithm
            result ^= std::hash<std::uint64_t>{}(storage.at(i)) + 0x9e3779b9
                      + (result << std::size_t{6}) + (result >> std::size_t{2});
        }
        return result;
    }
};

struct ArchetypeKeyEqual {
    constexpr auto operator()(const ArchetypeKey& lhs, const ArchetypeKey& rhs) const -> bool {
        return lhs == rhs;
    }
};

} // namespace atlas::hephaestus
