#include <CLI/CLI.hpp>
#include <fmt/format.h>
#include <nlohmann/json.hpp>
#include <range/v3/all.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <chrono>
#include <optional>
#include <sstream>
#include <unordered_map>
#include <vector>

#include <networking/downloader.hpp>
#include <types.hpp>

using namespace ranges;
using namespace std::chrono_literals;
using namespace nlohmann;

using strings = std::vector<std::string>;

namespace coingecko {
namespace v3 {

constexpr auto api = "https://api.coingecko.com/api/v3";

struct price {
    double value{};
    double market_cap{};
    double volume_24h{};
    double change_24h{};
};

namespace request {
struct price {
    strings ids; // assets
    strings vs_currencies; // usd
    bool include_market_cap{true};
    bool include_24hr_vol{true};
    bool include_24hr_change{true};
    bool include_last_updated_at{true};
    uint precision{}; // 0 - 18;
};
} // namespace request

namespace response {
using price = std::unordered_map<std::string, std::unordered_map<std::string, price>>;
}

auto ask(const request::price &req) -> std::optional<response::price> {
    if (req.ids.empty()) return {};
    if (req.vs_currencies.empty()) return {};

    constexpr std::array<char, 3> comma = {'%', '2', 'C'};

    const auto ids = req.ids | views::join(comma) | to<std::string>();
    const auto vs = req.vs_currencies | views::join(comma) | to<std::string>();
    const auto params = {
        fmt::format("ids={}", ids),
        fmt::format("vs_currencies={}", vs),
        fmt::format("include_market_cap={}", req.include_market_cap),
        fmt::format("include_24hr_vol={}", req.include_24hr_vol),
        fmt::format("include_24hr_change={}", req.include_24hr_change),
        fmt::format("include_last_updated_at={}", req.include_last_updated_at),
    };

    const auto url_params = params | views::join('&') | to<std::string>();
    const auto url = fmt::format("{}/simple/price?{}", api, url_params);

    const auto response = network::request(url);

    const auto j = json::parse(response.value_or(""));

    response::price ret;

    for (auto it = j.begin(); it != j.end(); ++it) {
        for (auto &&currency : req.vs_currencies) {
            price v;
            v.value = it.value().value<double>(currency, {});
            ret[it.key()].emplace(currency, v);
        }
    }

    return ret;
}

} // namespace v3
} // namespace coingecko

auto main(int argc, char **argv) -> int {
    CLI::App app("portfolio");
    std::vector<std::string> inputs;
    auto input_file_opt = app.add_option("-i,--input,input", inputs, "input json");
    input_file_opt->allow_extra_args()->check(CLI::ExistingFile);
    CLI11_PARSE(app, argc, argv);

    auto console = spdlog::stdout_color_mt("console");
    spdlog::set_pattern("%v");

    coingecko::v3::request::price price;
    price.ids = {"bitcoin", "cardano"};
    price.vs_currencies = {"usd", "btc", "pln"};

    const auto response = coingecko::v3::ask(price).value();

    for (auto [asset, prices] : response)
        for (auto &&[symbol, price] : prices)
            spdlog::info("asset {}: value in {}: {}", asset, symbol, price.value);

    return 0;
}
