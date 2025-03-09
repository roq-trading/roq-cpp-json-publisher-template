/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/bridge/json/shared.hpp"

using namespace std::literals;

namespace roq {
namespace bridge {
namespace json {

// === HELPERS ===

namespace {
template <typename R>
void apply(R &result, auto &event, auto id) {
  auto iter = result.find(id);
  if (iter == std::end(result))
    iter = result.try_emplace(id).first;
  std::ignore = (*iter).second(event);
}
}  // namespace

// === IMPLEMENTATION ===

Shared::Shared(Settings const &settings) : settings{settings} {
}

Shared::Source &Shared::get_source(uint8_t source) {
  auto iter = sources.find(source);
  if (iter == std::end(sources)) [[unlikely]]
    iter = sources.try_emplace(source).first;
  return (*iter).second;
}

Shared::Source::Account &Shared::get_account(Source &source, std::string_view const &account) {
  auto iter = source.accounts.find(account);
  if (iter == std::end(source.accounts)) [[unlikely]]
    iter = source.accounts.try_emplace(account).first;
  return (*iter).second;
}

uint64_t Shared::get_id(Source &source, std::string_view const &exchange, std::string_view const &symbol) {
  auto &tmp = source.lookup_[exchange];
  auto iter = tmp.find(symbol);
  if (iter == std::end(tmp)) [[unlikely]]
    iter = tmp.try_emplace(symbol, ++source.next_id_).first;
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
}  // namespace bridge
}  // namespace roq
