/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/bridge/json/session.hpp"

#include <magic_enum/magic_enum_format.hpp>

#include <cassert>

#include "roq/logging.hpp"

#include "roq/clock.hpp"

#include "roq/utils/enum.hpp"
#include "roq/utils/update.hpp"

#include "roq/codec/type.hpp"

using namespace std::literals;

namespace roq {
namespace bridge {
namespace json {

// === CONSTANTS ===

namespace {
auto const TIMEOUT = 5s;
}  // namespace

// === HELPERS ===

namespace {
auto create_server(auto &handler, auto &settings, auto &factory) {
  auto config = web::rest::Server::Config{
      .decode_buffer_size = {},
      .encode_buffer_size = {},
      .connection = {},
      .request_timeout = {},
      .server = {},  // XXX FIXME TODO
      .access_control_allow_origin = "*"sv,
      .url_prefix = settings.flags.url_prefix,
  };
  return web::rest::Server::create(handler, factory, config);
}

struct Query final {
  explicit Query(web::rest::Server::Request const &request) {
    enum class Key {
      EXCHANGE,
      SYMBOL,
    };
    for (auto &[key, value] : request.query) {
      auto key_2 = utils::parse_enum<Key>(key);
      switch (key_2) {
        case Key::EXCHANGE:
          exchange = value;
          break;
        case Key::SYMBOL:
          symbol = value;
          break;
      };
    }
  }

  std::string_view exchange;
  std::string_view symbol;
};
}  // namespace

// === IMPLEMENTATION ===

Session::Session(Handler &handler, Shared &shared, io::Context &context, io::net::tcp::Connection::Factory &factory, uint64_t session_id)
    : handler_{handler}, shared_{shared}, context_{context}, session_id_{session_id}, server_{create_server(*this, shared_.settings, factory)},
      last_refresh_{clock::get_system()} {
  assert(last_refresh_.count());
}

void Session::ping(std::chrono::nanoseconds now) {
  switch (state_) {
    using enum State;
    case CONNECTED:
      if ((last_refresh_ + TIMEOUT) < now) {
        log::info("[{}] Detected timeout"sv, session_id_);
        disconnect();
      }
      break;
    case READY:
      // note! web framework will manage ping/pong and timeout
      break;
    case ZOMBIE:
      return;
  }
}

// web::rest::Server::Handler

void Session::operator()(web::rest::Server::Disconnected const &) {
  log::info("[{}] Disconnected"sv, session_id_);
  last_refresh_ = clock::get_system();
  disconnect();
}

void Session::operator()(web::rest::Server::Request const &request) {
  log::info("[{}] Request"sv, session_id_);
  last_refresh_ = clock::get_system();
  if (request.headers.connection.has(web::http::Connection::UPGRADE)) {
    check_upgrade(request);
    log::info("[{}] Upgrading to websocket..."sv, session_id_);
    (*server_).upgrade(request);
    state_ = State::READY;
  } else {
    check_request(request);
  }
}

void Session::operator()(web::rest::Server::Text const &) {
  log::debug("[{}] Text"sv, session_id_);
  last_refresh_ = clock::get_system();
  switch (state_) {
    using enum State;
    case CONNECTED:
      disconnect();
      break;
    case READY:
      assert(false);  // XXX FIXME TODO
      break;
    case ZOMBIE:
      break;
  }
}

void Session::operator()(web::rest::Server::Binary const &) {
  log::debug("[{}] Binary"sv, session_id_);
  last_refresh_ = clock::get_system();
  switch (state_) {
    using enum State;
    case CONNECTED:
      disconnect();
      break;
    case READY:
      assert(false);  // XXX FIXME TODO
      break;
    case ZOMBIE:
      break;
  }
}

// utils

void Session::operator()(State state) {
  if (utils::update(state_, state)) {
    log::info("[{}] state={}"sv, session_id_, state_);
  }
}

void Session::check_upgrade(web::rest::Server::Request const &request) {
  Query query{request};
  if (std::empty(query.exchange))
    throw RuntimeError{"Unexpected: missing 'exchange' (query param)"sv};
  if (std::empty(query.symbol))
    throw RuntimeError{"Unexpected: missing 'symbol' (query param)"sv};
}

void Session::check_request(web::rest::Server::Request const &request) {
  auto path = request.path;
  log::warn("DBUG path=[{}]"sv, fmt::join(path, ", "sv));
  if (std::empty(path))
    return;
  enum class Type {
    REFERENCE_DATA,
    TOP_OF_BOOK,
  };
  auto type = utils::parse_enum<Type>(path[0]);
  log::warn("DEBUG type={}"sv, type);
  Query query{request};
  if (std::empty(query.exchange))
    throw RuntimeError{"Unexpected: missing 'exchange' (query param)"sv};
  if (std::empty(query.symbol))
    throw RuntimeError{"Unexpected: missing 'symbol' (query param)"sv};
  log::warn(R"(DEBUG exchange="{}", symbol="{}")"sv, query.exchange, query.symbol);
}

void Session::disconnect() {
  auto helper = [&]() {
    (*this)(State::ZOMBIE);
    auto disconnect = Disconnect{
        .session_id = session_id_,
    };
    handler_(disconnect);
  };
  switch (state_) {
    using enum State;
    case CONNECTED:
      helper();
      break;
    case READY:
      (*server_).close();
      helper();
      break;
    case ZOMBIE:
      break;
  }
}

}  // namespace json
}  // namespace bridge
}  // namespace roq
