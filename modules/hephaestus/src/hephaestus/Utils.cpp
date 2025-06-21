#include "hephaestus/Utils.hpp"

namespace atlas::hephaestus {
auto compare_signatures(
    const std::vector<std::type_index>& lhs,
    const std::vector<std::type_index>& rhs
) -> bool {
    if (lhs.size() != rhs.size()) {
        return false;
    }

    // No need to sort, the signatures should always be sorted from creation.
    return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

auto are_signatures_overlapping(
    const std::vector<std::type_index>& lhs,
    const std::vector<std::type_index>& rhs
) -> bool {
    size_t lhs_idx = 0;
    size_t rhs_idx = 0;

    while (lhs_idx < lhs.size() && rhs_idx < rhs.size()) {
        if (lhs[lhs_idx] == rhs[rhs_idx]) {
            return true;
        }

        if (lhs[lhs_idx] < rhs[rhs_idx]) {
            ++lhs_idx;
        } else {
            ++rhs_idx;
        }
    }

    return false;
}
} // namespace atlas::hephaestus
