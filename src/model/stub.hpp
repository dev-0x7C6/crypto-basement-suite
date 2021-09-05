#include "model.hpp"

namespace currency::data::provider {

struct stub {
    auto value(const cbs::time t) const noexcept {
        currency_details ret{};
        ret.price = std::sin(std::chrono::duration_cast<std::chrono::milliseconds>(t.time_since_epoch()).count()/1000.0);
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
