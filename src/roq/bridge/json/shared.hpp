/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include <span>
#include <string_view>

#include "roq/bridge/json/config.hpp"
#include "roq/bridge/json/settings.hpp"

namespace roq {
namespace bridge {
namespace json {

struct Shared final {
  Shared(Settings const &, Config const &, std::span<std::string_view const> const &params);

  Shared(Shared const &) = delete;

  Settings const &settings;
  Config const &config;
  std::span<std::string_view const> const &params;
};

}  // namespace json
}  // namespace bridge
}  // namespace roq
