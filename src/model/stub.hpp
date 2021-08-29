#include "model.hpp"

namespace currency::data::provider {

struct stub {
    constexpr auto value(const cbs::time t) const noexcept {
        currency_details ret{};
        ret.price = std::sin(t.time_since_epoch().count());
        return ret;
    }

    constexpr auto range() const noexcept {
        time_range range{};
        return range;
    }

    constexpr auto range_changed(std::function<void()> &&) noexcept {
    }
};

}
