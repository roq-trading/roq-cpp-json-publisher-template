/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/bridge/client/shared.hpp"

using namespace std::literals;

namespace roq {
namespace bridge {
namespace json {

// === HELPERS ===

namespace {}  // namespace

// === IMPLEMENTATION ===

Shared::Shared(Settings const &settings, Config const &config, std::span<std::string_view const> const &params)
    : settings{settings}, config{config}, params{params} {
}

}  // namespace json
}  // namespace bridge
}  // namespace roq
