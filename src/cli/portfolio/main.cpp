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

template <typename T>
auto get(const json &j, const std::string &key) -> std::optional<T> {
    try {
        return j[key].get<T>();
    } catch (...) {};
    return {};
}

namespace network::json {
nlohmann::json request(const std::string &url) {
    const auto response = network::request(url);
    if (!response) return {};

    return ::json::parse(response.value());
}
} // namespace network::json

namespace coingecko::v3 {

constexpr auto api = "https://api.coingecko.com/api/v3";

nlohmann::json request(const std::string &url) {
    const auto json = network::json::request(url);
    if (json.empty()) return {};

    if (json.contains("status") && json["status"].contains("error_code"))
        return {};

    return json;
}

namespace ping {
auto ask() -> bool {
    const auto url = fmt::format("{}/ping", api);
    const auto json = request(url);
    return !json.empty() && json.contains("gecko_says");
}
} // namespace ping

namespace simple::supported_vs_currencies {

auto ask() -> std::vector<std::string> {
    const auto url = fmt::format("{}/simple/supported_vs_currencies", api);
    const auto json = request(url);
    if (json.empty()) return {};

    std::vector<std::string> ret;
    for (auto &&item : json)
        ret.emplace_back(item);

    return ret;
}
} // namespace simple::supported_vs_currencies

namespace simple::price {

struct price {
    double value{};
    double market_cap{};
    double volume_24h{};
    double change_24h{};
};

struct options {
    strings ids; // assets
    strings vs_currencies; // usd
    bool include_market_cap{true};
    bool include_24hr_vol{true};
    bool include_24hr_change{true};
    bool include_last_updated_at{true};
    uint precision{}; // 0 - 18;
};

using summary = std::unordered_map<std::string, std::unordered_map<std::string, price>>;

auto ask(const options &opts = {}) -> summary {
    if (opts.ids.empty()) return {};
    if (opts.vs_currencies.empty()) return {};

    constexpr std::array<char, 3> comma = {'%', '2', 'C'};

    const auto ids = opts.ids | views::join(comma) | to<std::string>();
    const auto vs = opts.vs_currencies | views::join(comma) | to<std::string>();
    const auto params = {
        fmt::format("ids={}", ids),
        fmt::format("vs_currencies={}", vs),
        fmt::format("include_market_cap={}", opts.include_market_cap),
        fmt::format("include_24hr_vol={}", opts.include_24hr_vol),
        fmt::format("include_24hr_change={}", opts.include_24hr_change),
        fmt::format("include_last_updated_at={}", opts.include_last_updated_at),
    };

    const auto url_params = params | views::join('&') | to<std::string>();
    const auto url = fmt::format("{}/simple/price?{}", api, url_params);
    const auto json = request(url);
    if (json.empty()) return {};

    summary ret;

    for (auto &&[key, value] : json.items()) {
        for (auto &&currency : opts.vs_currencies) {
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
} // namespace simple::price

} // namespace coingecko::v3

auto main(int argc, char **argv) -> int {
    CLI::App app("portfolio");
    std::vector<std::string> inputs;
    auto input_file_opt = app.add_option("-i,--input,input", inputs, "input json");
    input_file_opt->allow_extra_args()->check(CLI::ExistingFile);
    CLI11_PARSE(app, argc, argv);

    auto console = spdlog::stdout_color_mt("console");
    spdlog::set_pattern("%v");

    const auto summary = coingecko::v3::simple::price::ask({
        .ids = {"bitcoin", "cardano", "polkadot", "cosmos", "avalanche-2", "near", "algorand", "solana"},
        .vs_currencies = {"usd", "btc", "pln", "sats"},
    });

    if (summary.empty()) {
        spdlog::error("invalid coingecko data");
        return 1;
    }

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
        for (auto &&[asset, prices] : summary)
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
        spdlog::info(" -> /{}: {:.2f}", currency, valuation);

    return 0;
}
