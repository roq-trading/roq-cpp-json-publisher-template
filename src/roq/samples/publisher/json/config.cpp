/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/samples/publisher/json/config.hpp"

#include "roq/samples/publisher/json/settings.hpp"

using namespace std::literals;

namespace roq {
namespace samples {
namespace publisher {
namespace json {

// === IMPLEMENTATION ===

Config::Config(Settings const &settings) : settings_{settings} {
}

void Config::dispatch(Handler &handler) const {
  handler(
      client::Account{
          .regex = ".*"sv,
      });
  handler(
      client::Symbol{
          .regex = ".*"sv,
          .exchange = "deribit"sv,
      });
  handler(
      client::Symbol{
          .regex = ".*"sv,
          .exchange = {},
      });
}

}  // namespace json
}  // namespace publisher
}  // namespace samples
}  // namespace roq
