#pragma once

#include <libcoingecko/v3/categories/list.hpp>
#include <libcoingecko/v3/coins/list.hpp>
#include <libcoingecko/v3/global/global.hpp>
#include <libcoingecko/v3/options.hpp>
#include <libcoingecko/v3/ping.hpp>
#include <libcoingecko/v3/simple/price.hpp>
#include <libcoingecko/v3/simple/supported_vs_currencies.hpp>

namespace coingecko::v3 {

class controller {
public:
    controller() = default;
    controller(const options &opts)
            : opts(opts) {}

    auto ping() -> bool { return ::coingecko::v3::ping(); }
    auto list(const coins::list::settings &s = {}) { return coins::list::query(s, opts); }
    auto price(const simple::price::parameters &s = {}) { return simple::price::query(s, opts); }

private:
    options opts{};
};

} // namespace coingecko::v3
