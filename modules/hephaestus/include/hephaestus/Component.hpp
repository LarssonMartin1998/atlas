#pragma once

#include <cstdint>

namespace atlas::hephaestus {
// This class should not be copied, we would enforce this by deleting the copy
// constructor. However, that would it so that inherited classes are no longer
// aggregates, which would dissallow aggregate initialization.
//
// A workaround to get around this limitation is the use of RValueArg concept
// which can be found in hephaestus/Concepts.hpp. We require that when
// constructing components in the component storage in the Archetype.
template <typename Derived> class Component {
  public:
    [[nodiscard]] auto static get_version() -> const std::uint64_t& {
        return version_counter;
    }

  private:
    friend class Archetype;
    static std::uint64_t version_counter;
};

template <typename Derived>
std::uint64_t Component<Derived>::version_counter = 0;
} // namespace atlas::hephaestus
