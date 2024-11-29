#pragma once

#include <typeindex>
#include <vector>

#include "hephaestus/Constants.hpp"

namespace atlas::hephaestus {
struct TypeIndexVectorHash {
    auto
    operator()(const std::vector<std::type_index> &types) const -> std::size_t {
        std::size_t seed = 0;
        constexpr size_t left_shift = 6;
        constexpr size_t right_shift = 2;
        for (const auto &type : types) {
            seed ^= type.hash_code() + golden_ratio_32 + (seed << left_shift) +
                    (seed >> right_shift);
        }
        return seed;
    }
};

struct TypeIndexVectorEqual {
    auto operator()(const std::vector<std::type_index> &lhs,
                    const std::vector<std::type_index> &rhs) const -> bool {
        return lhs == rhs;
    }
};
} // namespace atlas::hephaestus
