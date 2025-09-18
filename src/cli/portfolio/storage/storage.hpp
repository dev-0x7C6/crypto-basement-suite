#pragma once

#include <common/share.hpp>
#include <libcoingecko/v3/simple/price.hpp>

#include <filesystem>

namespace storage {

auto save(const portfolio &portfolio, const coingecko::v3::simple::price::prices &summary) -> bool;

}
