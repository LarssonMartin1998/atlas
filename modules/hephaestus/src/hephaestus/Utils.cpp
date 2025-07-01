#include "hephaestus/Utils.hpp"

namespace atlas::hephaestus {
auto are_dependencies_overlapping(
    const std::vector<SystemDependencies>& lhs,
    const std::vector<SystemDependencies>& rhs
) -> bool {
    size_t lhs_idx = 0;
    size_t rhs_idx = 0;

    while (lhs_idx < lhs.size() && rhs_idx < rhs.size()) {
        const auto& lhs_access = lhs[lhs_idx];
        const auto& rhs_access = rhs[rhs_idx];

        if (lhs_access.type == rhs_access.type) {
            // Same component type - check if there's a conflict
            // Conflict only occurs if at least one access is non-const (write access)
            if (!lhs_access.is_read_only || !rhs_access.is_read_only) {
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
