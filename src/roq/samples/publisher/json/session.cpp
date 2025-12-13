/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/samples/publisher/json/session.hpp"

#include <magic_enum/magic_enum_format.hpp>

#include <cassert>

#include "roq/logging.hpp"

#include "roq/clock.hpp"

#include "roq/utils/enum.hpp"
#include "roq/utils/update.hpp"

#include "roq/codec/type.hpp"

using namespace std::literals;

namespace roq {
namespace samples {
namespace publisher {
namespace json {

// === CONSTANTS ===

namespace {
auto const TIMEOUT = 5s;

auto const CACHE_CONTROL_NO_STORE = "no-store"sv;
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
      SOURCE,
      EXCHANGE,
      SYMBOL,
      ACCOUNT,
    };
    for (auto &[key, value] : request.query) {
      auto key_2 = utils::parse_enum<Key>(key);
      switch (key_2) {
        case Key::SOURCE:
          source = value;
          break;
        case Key::EXCHANGE:
          exchange = value;
          break;
        case Key::SYMBOL:
          symbol = value;
          break;
        case Key::ACCOUNT:
          account = value;
          break;
      };
    }
  }

  std::string_view source;
  std::string_view account;
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
    log::info("[{}] Upgrading to websocket..."sv, session_id_);
    (*server_).upgrade(request);
    state_ = State::READY;
  } else {
    process_request(request);
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

void Session::send_response(web::rest::Server::Request const &request, web::http::Status status, std::string_view const &body) {
  auto connection = [&]() -> web::http::Connection {
    if (status == web::http::Status::OK && request.headers.connection.has(web::http::Connection::KEEP_ALIVE)) {
      return web::http::Connection::KEEP_ALIVE;
    }
    return web::http::Connection::CLOSE;
  }();
  auto content_type = [&]() -> web::http::ContentType {
    if (std::empty(body)) {
      return {};
    }
    return web::http::ContentType::APPLICATION_JSON;
  }();
  auto response = web::rest::Server::Response{
      .status = status,
      .connection = connection,
      .sec_websocket_accept = {},
      .cache_control = CACHE_CONTROL_NO_STORE,
      .content_type = content_type,
      .body = body,
  };
  (*server_).send(response);
}

template <typename Callback>
bool Session::get_source(Callback callback, std::string_view const &source, std::string_view const &exchange, std::string_view const &symbol) const {
  auto result = false;
  auto helper = [&](auto &source) {
    auto helper_2 = [&](auto id) {
      result = true;
      callback(source, id);
    };
    source.get_id(helper_2, exchange, symbol);
  };
  shared_.get_source(helper, source);
  return result;
}

template <typename Callback>
bool Session::get_source_and_account(
    Callback callback, std::string_view const &source, std::string_view const &exchange, std::string_view const &symbol, std::string_view const &account)
    const {
  auto result = false;
  auto helper = [&](auto &source, auto id) {
    auto helper_2 = [&](auto &account) {
      result = true;
      callback(source, account, id);
    };
    source.get_account(helper_2, account);
  };
  get_source(helper, source, exchange, symbol);
  return result;
}

// process

void Session::process_request(web::rest::Server::Request const &request) {
  auto path = request.path;
  log::warn("DBUG path=[{}]"sv, fmt::join(path, ", "sv));
  if (std::empty(path)) {
    return;
  }
  enum class Type {
    REFERENCE_DATA,
    TOP_OF_BOOK,
    POSITION,
  };
  auto type = utils::parse_enum<Type>(path[0]);
  log::warn("DEBUG type={}"sv, type);
  Query query{request};
  switch (type) {
    using enum Type;
    case REFERENCE_DATA:
      process_reference_data(request, query.source, query.exchange, query.symbol);
      break;
    case TOP_OF_BOOK:
      process_top_of_book(request, query.source, query.exchange, query.symbol);
      break;
    case POSITION:
      process_position(request, query.source, query.exchange, query.symbol, query.account);
      break;
  }
}

void Session::process_reference_data(
    web::rest::Server::Request const &request, std::string_view const &source, std::string_view const &exchange, std::string_view const &symbol) {
  auto helper = [&](auto &source, auto id) {
    auto iter = source.reference_data.find(id);
    if (iter == std::end(source.reference_data)) {
      return;  // note! not found
    }
    auto &reference_data = (*iter).second;
    auto body = fmt::format(
        R"({{)"
        R"("description":"{}",)"
        R"("tick_size":{})"
        R"(}})"sv,
        reference_data.description,
        reference_data.tick_size);
    send_response(request, web::http::Status::OK, body);
  };
  get_source(helper, source, exchange, symbol);
}

void Session::process_top_of_book(
    web::rest::Server::Request const &request, std::string_view const &source, std::string_view const &exchange, std::string_view const &symbol) {
  auto helper = [&](auto &source, auto id) {
    auto iter = source.top_of_book.find(id);
    if (iter == std::end(source.top_of_book)) {
      return;  // note! not found
    }
    auto &top_of_book = (*iter).second;
    auto body = fmt::format(
        R"({{)"
        R"("bid_price":{},)"
        R"("ask_price":{})"
        R"(}})"sv,
        top_of_book.layer.bid_price,
        top_of_book.layer.ask_price);
    log::warn(R"(DEBUG body="{}")"sv, body);
    send_response(request, web::http::Status::OK, body);
  };
  get_source(helper, source, exchange, symbol);
}

void Session::process_position(
    [[maybe_unused]] web::rest::Server::Request const &request,
    std::string_view const &source,
    std::string_view const &exchange,
    std::string_view const &symbol,
    std::string_view const &account) {
  auto helper = [&]([[maybe_unused]] auto &source, auto &account, auto id) {
    auto iter = account.position.find(id);
    if (iter == std::end(account.position)) {
      return;  // not found
    }
    auto &position = (*iter).second;
    auto body = fmt::format(
        R"({{)"
        R"("long_quantity":{},)"
        R"("short_quantity":{})"
        R"(}})"sv,
        position.long_quantity,
        position.short_quantity);
    log::warn(R"(DEBUG body="{}")"sv, body);
    send_response(request, web::http::Status::OK, body);
  };
  get_source_and_account(helper, source, exchange, symbol, account);
}

}  // namespace json
}  // namespace publisher
}  // namespace samples
}  // namespace roq
