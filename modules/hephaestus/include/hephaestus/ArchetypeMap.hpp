#pragma once

#include "hephaestus/ComponentSignature.hpp"
#include <memory>
#include <unordered_map>

namespace atlas::hephaestus {
class Archetype;
}

namespace atlas::hephaestus {
using ArchetypePtr = std::unique_ptr<Archetype>;

// ArchetypeMap using ComponentSignature
using ArchetypeMap = std::unordered_map<
    ComponentSignature,
    ArchetypePtr,
    ComponentSignatureHash,
    ComponentSignatureEqual>;
} // namespace atlas::hephaestus
