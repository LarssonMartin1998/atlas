#pragma once

namespace atlas::hephaestus {
// This class should not be copied, we would enforce this by deleting the copy
// constructor. However, that would it so that inherited classes are no longer
// aggregates, which would dissallow aggregate initialization.
//
// A workaround to get around this limitation is the use of RValueArg concept
// which can be found in hephaestus/Concepts.hpp. We require that when
// constructing components in the component storage in the Archetype.
struct Component {};
} // namespace atlas::hephaestus
