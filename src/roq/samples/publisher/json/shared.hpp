/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include <string>

#include "roq/utils/container.hpp"

#include "roq/samples/publisher/json/settings.hpp"

namespace roq {
namespace samples {
namespace publisher {
namespace json {

namespace cache {
struct ReferenceData final {
  bool operator()(roq::ReferenceData const &);
  std::string description;
  double tick_size = NaN;
};
struct TopOfBook final {
  bool operator()(roq::TopOfBook const &);
  Layer layer;
};
struct Position final {
  bool operator()(roq::PositionUpdate const &);
  double long_quantity = NaN;
  double short_quantity = NaN;
};
}  // namespace cache

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

    template <typename Callback>
    bool get_id(Callback callback, std::string_view const &exchange, std::string_view const &symbol) {
      auto iter_1 = lookup_.find(exchange);
      if (iter_1 == std::end(lookup_)) {
        return false;
      }
      auto iter_2 = (*iter_1).second.find(symbol);
      if (iter_2 == std::end((*iter_1).second)) {
        return false;
      }
      callback((*iter_2).second);
      return true;
    }

    template <typename Callback>
    bool get_account(Callback callback, std::string_view const &account) {
      auto iter = accounts_.find(account);
      if (iter == std::end(accounts_)) {
        return false;
      }
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
    utils::unordered_map<std::string, Account> accounts_;
  };

  template <typename Callback>
  bool get_source(Callback callback, std::string_view const &source) {
    auto iter_1 = lookup_.find(source);
    if (iter_1 == std::end(lookup_)) {
      return false;
    }
    auto iter_2 = sources_.find((*iter_1).second);
    if (iter_2 == std::end(sources_)) {
      return false;
    }
    callback((*iter_2).second);
    return true;
  }

  template <typename T>
  void operator()(Event<T> const &event) {
    using value_type = std::remove_cvref_t<T>;
    auto &[message_info, value] = event;
    if constexpr (std::is_same_v<value_type, Connected>) {
      create_source(message_info.source, message_info.source_name);
    } else if constexpr (std::is_same_v<value_type, Disconnected>) {
      remove_source(message_info.source, message_info.source_name);
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
  void create_source(uint8_t source, std::string_view const &source_name);
  void remove_source(uint8_t source, std::string_view const &source_name);
  Source &get_source(uint8_t source);
  Source::Account &get_account(Source &, std::string_view const &account);
  uint64_t get_id(Source &, std::string_view const &exchange, std::string_view const &symbol);

 private:
  utils::unordered_map<uint8_t, Source> sources_;
  utils::unordered_map<std::string, uint8_t> lookup_;
};

}  // namespace json
}  // namespace publisher
}  // namespace samples
}  // namespace roq
