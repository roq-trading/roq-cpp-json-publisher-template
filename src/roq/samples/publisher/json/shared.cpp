/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/samples/publisher/json/shared.hpp"

#include "roq/logging.hpp"

#include "roq/utils/update.hpp"

using namespace std::literals;

namespace roq {
namespace samples {
namespace publisher {
namespace json {

// === HELPERS ===

namespace {
template <typename R>
void apply(R &result, auto &event, auto id) {
  auto iter = result.find(id);
  if (iter == std::end(result)) {
    iter = result.try_emplace(id).first;
  }
  std::ignore = (*iter).second(event);
}
}  // namespace

// === IMPLEMENTATION ===

namespace cache {
bool ReferenceData::operator()(roq::ReferenceData const &value) {
  auto result = false;
  result |= utils::update(description, value.description);
  result |= utils::update(tick_size, value.tick_size);
  return result;
}

bool TopOfBook::operator()(roq::TopOfBook const &value) {
  auto result = false;
  result |= utils::update(layer, value.layer);
  return result;
}

bool Position::operator()(roq::PositionUpdate const &value) {
  auto result = false;
  result |= utils::update(long_quantity, value.long_quantity);
  result |= utils::update(short_quantity, value.short_quantity);
  return result;
}
}  // namespace cache

Shared::Shared(Settings const &settings) : settings{settings} {
}

void Shared::create_source(uint8_t source, std::string_view const &source_name) {
  auto iter_1 = sources_.find(source);
  if (iter_1 != std::end(sources_)) {
    log::fatal("Unexpected"sv);
  }
  auto iter_2 = lookup_.find(source_name);
  if (iter_2 != std::end(lookup_)) {
    log::fatal("Unexpected"sv);
  }
  lookup_.try_emplace(source_name, source);
  sources_.try_emplace(source);
}

void Shared::remove_source(uint8_t source, std::string_view const &source_name) {
  auto iter = lookup_.find(source_name);
  if (iter != std::end(lookup_)) {
    if ((*iter).second != source) {
      log::fatal("Unexpected"sv);
    }
  }
  lookup_.erase(iter);
  sources_.erase(source);
}

Shared::Source &Shared::get_source(uint8_t source) {
  auto iter = sources_.find(source);
  if (iter == std::end(sources_)) [[unlikely]] {
    log::fatal("Unexpected"sv);
  }
  iter = sources_.try_emplace(source).first;
  return (*iter).second;
}

Shared::Source::Account &Shared::get_account(Source &source, std::string_view const &account) {
  auto iter = source.accounts_.find(account);
  if (iter == std::end(source.accounts_)) [[unlikely]] {
    iter = source.accounts_.try_emplace(account).first;
  }
  return (*iter).second;
}

uint64_t Shared::get_id(Source &source, std::string_view const &exchange, std::string_view const &symbol) {
  auto &tmp = source.lookup_[exchange];
  auto iter = tmp.find(symbol);
  if (iter == std::end(tmp)) [[unlikely]] {
    iter = tmp.try_emplace(symbol, ++source.next_id_).first;
  }
  return (*iter).second;
}

void Shared::Source::operator()(Event<ReferenceData> const &event, uint64_t id) {
  apply(reference_data, event, id);
}

void Shared::Source::operator()(Event<TopOfBook> const &event, uint64_t id) {
  apply(top_of_book, event, id);
}

void Shared::Source::Account::operator()(Event<PositionUpdate> const &event, uint64_t id) {
  apply(position, event, id);
}

}  // namespace json
}  // namespace publisher
}  // namespace samples
}  // namespace roq
