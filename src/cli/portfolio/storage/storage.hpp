#pragma once

#include <common/share.hpp>
#include <libcoingecko/v3/simple/price.hpp>

namespace storage {

auto save(const portfolio &portfolio, const coingecko::v3::simple::price::prices &summary) -> void;

}
