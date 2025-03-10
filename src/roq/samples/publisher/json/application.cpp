/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/samples/publisher/json/application.hpp"

#include "roq/logging.hpp"

#include "roq/io/engine/context_factory.hpp"

#include "roq/samples/publisher/json/config.hpp"
#include "roq/samples/publisher/json/controller.hpp"
#include "roq/samples/publisher/json/settings.hpp"

using namespace std::literals;

namespace roq {
namespace samples {
namespace publisher {
namespace json {

// === IMPLEMENTATION ===

int Application::main(args::Parser const &args) {
  auto params = args.params();
  if (std::empty(params))
    log::fatal("Expected arguments"sv);
  Settings settings{args};
  log::info("settings={}"sv, settings);
  Config config{settings};
  log::info("config={}"sv, config);
  auto context = io::engine::ContextFactory::create();
  log::info("Starting the publisher"sv);
  Controller{settings, config, *context, params}.dispatch();
  return EXIT_SUCCESS;
}

}  // namespace json
}  // namespace publisher
}  // namespace samples
}  // namespace roq
