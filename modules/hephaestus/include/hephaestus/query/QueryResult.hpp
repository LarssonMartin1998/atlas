#pragma once

#include <ranges>

namespace atlas::hephaestus {
template <typename Range> class QueryResult final {
  public:
    explicit QueryResult(Range range) : range{std::move(range)} {}

    auto begin() { return std::ranges::begin(range); }
    auto end() { return std::ranges::end(range); }

  private:
    Range range;
};
} // namespace atlas::hephaestus
