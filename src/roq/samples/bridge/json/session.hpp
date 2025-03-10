/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <memory>

#include "roq/io/context.hpp"

#include "roq/io/net/tcp/connection.hpp"

#include "roq/web/rest/server.hpp"

#include "roq/samples/bridge/json/shared.hpp"

namespace roq {
namespace samples {
namespace bridge {
namespace json {

struct Session final : public web::rest::Server::Handler {
  struct Disconnect final {
    uint64_t session_id = {};
  };

  struct Handler {
    virtual void operator()(Disconnect const &) = 0;
  };

  Session(Handler &, Shared &, io::Context &, io::net::tcp::Connection::Factory &, uint64_t session_id);

  Session(Session const &) = delete;

  void ping(std::chrono::nanoseconds now);

 protected:
  // web::rest::Server::Handler

  void operator()(web::rest::Server::Disconnected const &) override;
  void operator()(web::rest::Server::Request const &) override;
  void operator()(web::rest::Server::Text const &) override;
  void operator()(web::rest::Server::Binary const &) override;

  // utils

  enum class State {
    CONNECTED,
    READY,
    ZOMBIE,
  };

  void operator()(State);

  void disconnect();

  void send_response(web::rest::Server::Request const &, web::http::Status, std::string_view const &body = {});

  template <typename Callback>
  bool get_source(Callback, std::string_view const &source, std::string_view const &exchange, std::string_view const &symbol) const;

  template <typename Callback>
  bool get_source_and_account(
      Callback, std::string_view const &source, std::string_view const &exchange, std::string_view const &symbol, std::string_view const &account) const;

  // process

  void process_request(web::rest::Server::Request const &);

  void process_reference_data(
      web::rest::Server::Request const &, std::string_view const &source, std::string_view const &exchange, std::string_view const &symbol);
  void process_top_of_book(
      web::rest::Server::Request const &, std::string_view const &source, std::string_view const &exchange, std::string_view const &symbol);
  void process_position(
      web::rest::Server::Request const &,
      std::string_view const &source,
      std::string_view const &exchange,
      std::string_view const &symbol,
      std::string_view const &account);

 private:
  Handler &handler_;
  Shared &shared_;
  io::Context &context_;
  uint64_t const session_id_;
  std::unique_ptr<web::rest::Server> const server_;
  std::chrono::nanoseconds last_refresh_ = {};
  State state_ = {};
};

}  // namespace json
}  // namespace bridge
}  // namespace samples
}  // namespace roq
