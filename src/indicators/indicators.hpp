#pragma once

#include <model/model.hpp>

#include <memory>
#include <list>

namespace indicator {

template <typename type>
concept indicator_model = requires(type object) {
    { object.compute_value(types::time_point{}) } -> std::same_as<types::indicator_value>;
    { object.configure(std::list<std::tuple<std::string, std::string>>()) } -> std::same_as<void>;
    { object.load_data(types::currency{}) } -> std::same_as<void>;
};

struct price_velocity {
    // http://thepatternsite.com/velocity.html
};

// https://starofmysore.com/top-crypto-trading-technical-indicators-to-use-on-primexbt/
struct bollinger_bands {
};

struct relative_strength_index {
};

} // namespace indicator
