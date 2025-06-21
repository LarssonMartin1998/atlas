#pragma once

#include "hephaestus/Hashing.hpp"
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>
namespace atlas::hephaestus {
class Archetype;
}

namespace atlas::hephaestus {
using ArchetypePtr = std::unique_ptr<Archetype>;
using ArchetypeMap = std::unordered_map<
    std::vector<std::type_index>,
    ArchetypePtr,
    TypeIndexVectorHash,
    TypeIndexVectorEqual>;
} // namespace atlas::hephaestus
