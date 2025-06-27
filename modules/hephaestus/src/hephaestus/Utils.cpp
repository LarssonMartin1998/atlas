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

auto are_access_signatures_overlapping(
    const std::vector<ComponentAccess>& lhs,
    const std::vector<ComponentAccess>& rhs
) -> bool {
    size_t lhs_idx = 0;
    size_t rhs_idx = 0;

    while (lhs_idx < lhs.size() && rhs_idx < rhs.size()) {
        const auto& lhs_access = lhs[lhs_idx];
        const auto& rhs_access = rhs[rhs_idx];

        if (lhs_access.type == rhs_access.type) {
            // Same component type - check if there's a conflict
            // Conflict only occurs if at least one access is non-const (write access)
            if (!lhs_access.is_const || !rhs_access.is_const) {
                return true;
            }
            // Both are const (read-only), no conflict
            ++lhs_idx;
            ++rhs_idx; 
        } else if (lhs_access.type < rhs_access.type) {
            ++lhs_idx;
        } else {
            ++rhs_idx;
        }
    }

    return false;
}
} // namespace atlas::hephaestus
