/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/bridge/client/config.hpp"

#include "roq/bridge/client/settings.hpp"

namespace roq {
namespace bridge {
namespace json {

// === IMPLEMENTATION ===

Config::Config(Settings const &settings) : settings_{settings} {
}

void Config::dispatch(Handler &) const {
  /*
  // settings
  handler(client::Settings{
      .order_cancel_policy = OrderCancelPolicy::MANAGED_ORDERS,
      .order_management = {},
  });
  // accounts
  handler(client::Account{
      .regex = settings_.account,
  });
  // symbols
  handler(client::Symbol{
      .regex = settings_.symbol,
      .exchange = settings_.exchange,
  });
  // currencies
  handler(client::Symbol{
      .regex = settings_.currencies,
      .exchange = {},
  });
  */
}

}  // namespace json
}  // namespace bridge
}  // namespace roq
