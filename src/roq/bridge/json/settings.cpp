/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/bridge/json/settings.hpp"

#include "roq/logging.hpp"

#include "roq/client/flags/settings.hpp"

using namespace std::literals;

namespace roq {
namespace bridge {
namespace json {

Settings::Settings(args::Parser const &args) : roq::client::flags::Settings{args}, flags{flags::Flags::create()} {
  log::info("settings={}"sv, *this);
}

}  // namespace json
}  // namespace bridge
}  // namespace roq
