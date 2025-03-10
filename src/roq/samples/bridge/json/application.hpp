/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include <span>

#include "roq/service.hpp"

namespace roq {
namespace samples {
namespace bridge {
namespace json {

struct Application final : public roq::Service {
  using roq::Service::Service;

 protected:
  int main(args::Parser const &) override;
};

}  // namespace json
}  // namespace bridge
}  // namespace samples
}  // namespace roq
