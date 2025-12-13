/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <span>

#include "roq/service.hpp"

namespace roq {
namespace samples {
namespace publisher {
namespace json {

struct Application final : public Service {
  using Service::Service;

 protected:
  int main(args::Parser const &) override;
};

}  // namespace json
}  // namespace publisher
}  // namespace samples
}  // namespace roq
