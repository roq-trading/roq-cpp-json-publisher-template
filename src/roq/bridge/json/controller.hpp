/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include <span>

#include "roq/utils/container.hpp"

#include "roq/io/context.hpp"

#include "roq/io/sys/signal.hpp"
#include "roq/io/sys/timer.hpp"

#include "roq/io/net/tcp/listener.hpp"

#include "roq/client/poller.hpp"

#include "roq/bridge/json/config.hpp"
#include "roq/bridge/json/session.hpp"
#include "roq/bridge/json/settings.hpp"
#include "roq/bridge/json/shared.hpp"

namespace roq {
namespace bridge {
namespace json {

struct Controller final : public io::sys::Signal::Handler,
                          public io::sys::Timer::Handler,
                          public io::net::tcp::Listener::Handler,
                          public Session::Handler,
                          public client::Poller::Handler {
  Controller(Settings const &, Config const &, io::Context &, std::span<std::string_view const> const &);

  Controller(Controller const &) = delete;

  void dispatch();

 protected:
  // io::sys::Signal::Handler

  void operator()(io::sys::Signal::Event const &) override;

  // io::sys::Timer

  void operator()(io::sys::Timer::Event const &) override;

  // io::net::tcp::Listener::Handler

  void operator()(io::net::tcp::Connection::Factory &) override;
  void operator()(io::net::tcp::Connection::Factory &, io::NetworkAddress const &) override;

  // Session::Handler

  void operator()(Session::Disconnect const &) override;

  // client::Handler

  void operator()(Event<Disconnected> const &) override;
  void operator()(Event<ReferenceData> const &) override;
  void operator()(Event<TopOfBook> const &) override;
  void operator()(Event<PositionUpdate> const &) override;

  // utils

  void create_session(io::net::tcp::Connection::Factory &, uint64_t session_id);

  void remove_zombies();

 private:
  Settings const &settings_;
  Shared shared_;
  // io
  io::Context &context_;
  std::unique_ptr<io::sys::Signal> const terminate_;
  std::unique_ptr<io::sys::Signal> const interrupt_;
  std::unique_ptr<io::sys::Timer> const timer_;
  std::unique_ptr<io::net::tcp::Listener> const listener_;
  std::unique_ptr<client::Poller> dispatcher_;
  // sessions
  uint64_t next_session_id_ = {};
  utils::unordered_map<uint64_t, std::unique_ptr<Session>> sessions_;
  utils::unordered_set<uint64_t> zombies_;
  std::chrono::nanoseconds next_heartbeat_ = {};
  std::chrono::nanoseconds next_cleanup_ = {};
};

}  // namespace json
}  // namespace bridge
}  // namespace roq
