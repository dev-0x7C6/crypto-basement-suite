#include "model.hpp"

#include <vector>

namespace currency::data::provider {

struct stub {
    constexpr auto value(const cbs::time t) noexcept {
        currency_details ret{};
        ret.price = std::abs(sin(m_sin_steps += 0.001) * 1000);
        return ret;
    }

    constexpr auto range() const noexcept {
        time_range range{};
        range.begin = 1630948469;
        range.end = 1630958499;
        return range;
    }

    constexpr auto range_changed(std::function<void()> &&) noexcept {
    }

private:
    float m_sin_steps{};
};

constexpr auto iterate(currency::data::provider::model auto model, const std::chrono::seconds resolution, std::function<void(const cbs::time t, currency_details details)> &&callable) noexcept {
    for (auto i = model.range().begin; i <= model.range().end; i += resolution) {
        callable(i, model.value(i));
    }
}

} // namespace currency::data::provider
