#include "model.hpp"

namespace currency::data::provider {

struct stub {
    constexpr auto value(const cbs::time t) const noexcept {
        currency_details ret{};
        ret.price = std::sin(t.timestamp);
        return ret;
    }

    constexpr auto range() const noexcept {
        time_range range{};
        range.begin = 1630948469;
        range.end = 1630948499;
        return range;
    }

    constexpr auto range_changed(std::function<void()> &&) noexcept {
    }
};

constexpr auto iterate(currency::data::provider::model auto model, const std::chrono::seconds resolution, std::function<void(const cbs::time t)> &&callable) noexcept {
    for (auto i = model.range().begin; i <= model.range().end; i += resolution) {
        callable(i);
    }
}

} // namespace currency::data::provider
