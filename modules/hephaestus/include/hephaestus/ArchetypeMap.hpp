#pragma once

#include "hephaestus/ArchetypeKey.hpp"
#include <memory>
#include <unordered_map>

namespace atlas::hephaestus {
class Archetype;
}

namespace atlas::hephaestus {
using ArchetypePtr = std::unique_ptr<Archetype>;

// ArchetypeMap using ArchetypeKey
using ArchetypeMap = std::unordered_map<
    ArchetypeKey,
    ArchetypePtr,
    ArchetypeKeyHash,
    ArchetypeKeyEqual>;
} // namespace atlas::hephaestus
