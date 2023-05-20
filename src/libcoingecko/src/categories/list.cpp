#include "libcoingecko/v3/categories/list.hpp"
#include "api.hpp"

#include <fmt/format.h>
#include <range/v3/all.hpp>

using namespace ranges;
using namespace coingecko::v3::coins::categories;

namespace coingecko::v3::coins::categories {

auto list(const options &opts) -> std::expected<categories, error> {
    const auto json = request("coins/categories/list", opts);
    if (!json) return std::unexpected(json.error());

    std::vector<category> ret;
    for (auto object : json.value()) {
        struct category category;
        set(object, "category_id", category.id);
        set(object, "name", category.name);
        ret.emplace_back(std::move(category));
    }

    return ret;
}

} // namespace coingecko::v3::coins::categories
