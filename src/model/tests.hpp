#include <stub.hpp>

namespace currency::data::provider::tests {

using model = currency::data::provider::model;

consteval auto test_concept(model auto impl) { return true; }
consteval auto test_concept(auto impl) { return false; }

static_assert(test_concept(stub{}));

}
