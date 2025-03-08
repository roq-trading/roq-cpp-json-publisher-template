/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <memory>

#include "roq/io/context.hpp"

#include "roq/io/net/tcp/connection.hpp"

#include "roq/web/rest/server.hpp"

#include "roq/bridge/json/shared.hpp"

namespace roq {
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

  void check_upgrade(web::rest::Server::Request const &);
  void check_request(web::rest::Server::Request const &);

  void disconnect();

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
}  // namespace roq
