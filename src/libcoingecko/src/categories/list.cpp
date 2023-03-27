#include "libcoingecko/v3/categories/list.hpp"
#include "api.hpp"

#include <fmt/format.h>
#include <range/v3/all.hpp>

using namespace ranges;
using namespace coingecko::v3::coins::categories;

namespace coingecko::v3::coins::categories {

auto list() -> std::vector<category> {
    const auto url = fmt::format("{}/coins/categories/list", api);
    const auto json = request(url);
    if (json.empty()) return {};

    std::vector<category> ret;
    for (auto object : json) {
        struct category category;
        category.id = get<std::string>(object, "category_id").value_or("");
        category.name = get<std::string>(object, "name").value_or("");
        ret.emplace_back(std::move(category));
    }

    return ret;
}

} // namespace coingecko::v3::coins::categories
