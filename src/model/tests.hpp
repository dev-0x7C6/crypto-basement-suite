#pragma once

#include "stub.hpp"

namespace provider::tests {

consteval auto test_concept(provider::model auto impl) { return true; }
consteval auto test_concept(auto impl) { return false; }

static_assert(test_concept(provider::stub{}));

} // namespace provider::tests
