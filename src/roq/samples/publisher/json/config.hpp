/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include "roq/compat/fmt.hpp"

#include <fmt/format.h>

#include "roq/client/config.hpp"

#include "roq/samples/publisher/json/settings.hpp"

namespace roq {
namespace samples {
namespace publisher {
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
}  // namespace publisher
}  // namespace samples
}  // namespace roq

template <>
struct fmt::formatter<roq::samples::publisher::json::Config> {
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(roq::samples::publisher::json::Config const &, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(}})"sv);
  }
};
