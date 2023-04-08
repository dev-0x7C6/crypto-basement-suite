#include "libcoingecko/v3/categories/list.hpp"
#include "api.hpp"

#include <fmt/format.h>
#include <range/v3/all.hpp>

using namespace ranges;
using namespace coingecko::v3::coins::categories;

namespace coingecko::v3::coins::categories {

auto list(const options &opts) -> std::vector<category> {
    const auto json = request("coins/categories/list", opts);
    if (json.empty()) return {};

    std::vector<category> ret;
    for (auto object : json) {
        struct category category;
        set(object, "category_id", category.id);
        set(object, "name", category.name);
        ret.emplace_back(std::move(category));
    }

    return ret;
}

} // namespace coingecko::v3::coins::categories
