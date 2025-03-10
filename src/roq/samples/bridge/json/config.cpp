/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/samples/bridge/json/config.hpp"

#include "roq/samples/bridge/json/settings.hpp"

using namespace std::literals;

namespace roq {
namespace samples {
namespace bridge {
namespace json {

// === IMPLEMENTATION ===

Config::Config(Settings const &settings) : settings_{settings} {
}

void Config::dispatch(Handler &handler) const {
  handler(client::Account{
      .regex = ".*"sv,
  });
  handler(client::Symbol{
      .regex = ".*"sv,
      .exchange = "deribit"sv,
  });
  handler(client::Symbol{
      .regex = ".*"sv,
      .exchange = {},
  });
}

}  // namespace json
}  // namespace bridge
}  // namespace samples
}  // namespace roq
