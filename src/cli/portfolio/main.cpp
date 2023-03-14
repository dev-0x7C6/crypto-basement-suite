#include <CLI/CLI.hpp>
#include <fmt/format.h>
#include <nlohmann/json.hpp>
#include <range/v3/view/join.hpp>
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

template <typename T>
auto get(const json &j, const std::string &key) -> std::optional<T> {
    if (!j.contains(key)) return {};
    return j[key].get<T>();
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
    const auto json = json::parse(response.value_or(""));

    if (json.contains("status") && json["status"].contains("error_code"))
        return {};

    response::price ret;

    for (auto it = json.begin(); it != json.end(); ++it) {
        const auto &key = it.key();
        const auto &value = it.value();
        for (auto &&currency : req.vs_currencies) {
            price valuation;
            valuation.value = get<double>(value, currency).value_or(0);
            valuation.change_24h = get<double>(value, fmt::format("{}_24h_change", currency)).value_or(0);
            valuation.market_cap = get<double>(value, fmt::format("{}_market_cap", currency)).value_or(0);
            valuation.volume_24h = get<double>(value, fmt::format("{}_24h_vol", currency)).value_or(0);
            ret[key].emplace(currency, valuation);
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
    price.ids = {"bitcoin", "cardano", "polkadot", "cosmos"};
    price.vs_currencies = {"usd", "btc", "pln"};

    const auto response = coingecko::v3::ask(price);
    if (!response) return 0;

    // test input
    std::vector<std::pair<std::string, double>> input{
        {"bitcoin", 0.12},
        {"bitcoin", 0.24},
        {"bitcoin", 2.01},
        {"cardano", 1203.50},
        {"cardano", 1929.29},
        {"cardano", 2384.03},
        {"cosmos", 15.22},
        {"cosmos", 20.21},
        {"cosmos", 41.98},
        {"polkadot", 129.23},
        {"polkadot", 21.02},
        {"polkadot", 99.99},
    };

    std::map<std::string, double> portfolio;
    for (auto [symbol, balance] : input)
        portfolio[symbol] += balance;

    std::map<std::string, double> total;

    for (auto &&[portfolio_asset, portfolio_ballance] : portfolio) {
        spdlog::info("+ {} [{:f}]", portfolio_asset, portfolio_ballance);
        for (auto &&[asset, prices] : response.value())
            if (portfolio_asset == asset)
                for (auto &&[currency, valuation] : prices) {
                    const auto value = portfolio_ballance * valuation.value;
                    spdlog::info(" -> /{}: {:f}", currency, value);
                    total[currency] += value;
                }
        spdlog::info("");
    }

    spdlog::info("+ total");
    for (auto &&[currency, valuation] : total)
        spdlog::info(" -> /{}: {:f}", currency, valuation);

    return 0;
}
