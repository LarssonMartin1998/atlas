#pragma once

#include "hephaestus/Hashing.hpp"
#include "hephaestus/ComponentSignature.hpp"
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>
namespace atlas::hephaestus {
class Archetype;
}

namespace atlas::hephaestus {
using ArchetypePtr = std::unique_ptr<Archetype>;

// Legacy ArchetypeMap using std::vector<std::type_index>
using ArchetypeMap = std::unordered_map<
    std::vector<std::type_index>,
    ArchetypePtr,
    TypeIndexVectorHash,
    TypeIndexVectorEqual>;

// Optimized ArchetypeMap using ComponentSignature
using OptimizedArchetypeMap = std::unordered_map<
    ComponentSignature,
    ArchetypePtr,
    ComponentSignatureHash,
    ComponentSignatureEqual>;
} // namespace atlas::hephaestus
