#pragma once

#include "model.hpp"

#include <vector>
#include <chrono>

using namespace std::chrono_literals;

namespace provider {

struct stub {
    constexpr auto value(const types::time_point t) noexcept -> types::currency {
        return {std::abs(std::sin((t.point % 1000) * 0.0001f) * 1000.0f), t.point};
    }

    constexpr auto range() const noexcept -> types::time_range {
        return {1630948469, 1630970069};
    }

    constexpr auto range_changed(std::function<void()> &&) noexcept {
    }
};

constexpr auto iterate(provider::model auto model, std::function<void(const types::time_point, const types::currency value)> &&callable, const std::chrono::seconds resolution = 1s) noexcept {
    for (auto i = model.range().begin; i <= model.range().end; i += resolution) {
        callable(i, model.value(i));
    }
}

} // namespace provider
