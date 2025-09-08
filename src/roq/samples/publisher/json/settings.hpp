/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include "roq/compat/fmt.hpp"

#include <fmt/format.h>

#include "roq/client.hpp"

#include "roq/samples/publisher/json/flags/flags.hpp"

namespace roq {
namespace samples {
namespace publisher {
namespace json {

struct Settings final : public client::flags::Settings {
  explicit Settings(args::Parser const &);

  flags::Flags flags;
};

}  // namespace json
}  // namespace publisher
}  // namespace samples
}  // namespace roq

template <>
struct fmt::formatter<roq::samples::publisher::json::Settings> {
  constexpr auto parse(format_parse_context &context) { return std::begin(context); }
  auto format(roq::samples::publisher::json::Settings const &value, format_context &context) const {
    using namespace std::literals;
    return fmt::format_to(
        context.out(),
        R"({{)"
        R"(flags={}, )"
        R"(client={})"
        R"(}})"sv,
        value.flags,
        static_cast<roq::client::Settings2 const &>(value));
  }
};
