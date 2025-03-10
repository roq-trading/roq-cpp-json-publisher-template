/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include <fmt/core.h>

#include "roq/client/config.hpp"

#include "roq/samples/bridge/json/settings.hpp"

namespace roq {
namespace samples {
namespace bridge {
namespace json {

struct Config final : public client::Config {
  explicit Config(Settings const &);

  Config(Config &&) = default;
  Config(Config const &) = delete;

 protected:
  Settings const &settings_;
  void dispatch(Handler &) const override;
};

}  // namespace json
}  // namespace bridge
}  // namespace samples
}  // namespace roq

template <>
struct fmt::formatter<roq::samples::bridge::json::Config> {
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(roq::samples::bridge::json::Config const &, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(}})"sv);
  }
};
