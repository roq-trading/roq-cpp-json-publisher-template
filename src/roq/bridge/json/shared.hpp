/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include <string>

#include "roq/utils/container.hpp"

#include "roq/cache/position.hpp"
#include "roq/cache/reference_data.hpp"
#include "roq/cache/top_of_book.hpp"

#include "roq/bridge/json/settings.hpp"

namespace roq {
namespace bridge {
namespace json {

struct Shared final {
  explicit Shared(Settings const &);

  Shared(Shared const &) = delete;

  Settings const &settings;

  // cache

  struct Source final {
    utils::unordered_map<uint64_t, cache::ReferenceData> reference_data;
    utils::unordered_map<uint64_t, cache::TopOfBook> top_of_book;

    struct Account final {
      utils::unordered_map<uint64_t, cache::Position> position;

     protected:
      friend Shared;
      void operator()(Event<PositionUpdate> const &, uint64_t id);
    };
    utils::unordered_map<std::string, Account> accounts;

    template <typename Callback>
    bool get_id(Callback callback, std::string_view const &exchange, std::string_view const &symbol) {
      auto iter_1 = lookup_.find(exchange);
      if (iter_1 == std::end(lookup_))
        return false;
      auto iter_2 = (*iter_1).second.find(symbol);
      if (iter_2 == std::end((*iter_1).second))
        return false;
      callback((*iter_2).second);
      return true;
    }

    template <typename Callback>
    bool get_account(Callback callback, std::string_view const &account) {
      auto iter = accounts.find(account);
      if (iter == std::end(accounts))
        return false;
      callback((*iter).second);
      return true;
    }

   protected:
    friend Shared;

    void operator()(Event<ReferenceData> const &, uint64_t id);
    void operator()(Event<TopOfBook> const &, uint64_t id);

   private:
    uint64_t next_id_ = {};
    utils::unordered_map<std::string, utils::unordered_map<std::string, uint64_t>> lookup_;
  };
  utils::unordered_map<uint8_t, Source> sources;

  template <typename Callback>
  bool get_source(Callback callback, uint8_t source) {
    auto iter = sources.find(source);
    if (iter == std::end(sources))
      return false;
    callback((*iter).second);
    return true;
  }

  template <typename T>
  void operator()(Event<T> const &event) {
    using value_type = std::remove_cvref<T>::type;
    auto &[message_info, value] = event;
    if constexpr (std::is_same<value_type, Disconnected>::value) {
      sources.erase(message_info.source);
    } else {
      auto &source = get_source(message_info.source);
      auto id = get_id(source, value.exchange, value.symbol);
      constexpr bool has_account = requires(T const &t) { t.account; };
      if constexpr (has_account) {
        auto &account = get_account(source, value.account);
        account(event, id);
      } else {
        source(event, id);
      }
    }
  }

 protected:
  Source &get_source(uint8_t source);
  Source::Account &get_account(Source &, std::string_view const &account);
  uint64_t get_id(Source &, std::string_view const &exchange, std::string_view const &symbol);
};

}  // namespace json
}  // namespace bridge
}  // namespace roq
