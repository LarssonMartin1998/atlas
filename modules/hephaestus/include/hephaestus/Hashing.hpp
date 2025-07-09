#pragma once

#include <typeindex>
#include <vector>

#include "hephaestus/Constants.hpp"

namespace atlas::hephaestus {
struct TypeIndexVectorHash {
    auto operator()(const std::vector<std::type_index>& types) const -> std::size_t {
        std::size_t seed = 0;
        constexpr std::size_t LEFT_SHIFT = 6;
        constexpr std::size_t RIGHT_SHIFT = 2;
        for (const auto& type : types) {
            seed ^= type.hash_code() + GOLDEN_RATIO_32 + (seed << LEFT_SHIFT)
                    + (seed >> RIGHT_SHIFT);
        }
        return seed;
    }
};

struct TypeIndexVectorEqual {
    auto operator()(
        const std::vector<std::type_index>& lhs,
        const std::vector<std::type_index>& rhs
    ) const -> bool {
        return lhs == rhs;
    }
};
} // namespace atlas::hephaestus
