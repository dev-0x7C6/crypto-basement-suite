#include "storage.hpp"

#include <fstream>
#include <filesystem>

#include <nlohmann/json.hpp>

using namespace nlohmann;
using namespace std;

auto storage::save(const portfolio &portfolio, const coingecko::v3::simple::price::prices &summary) -> void {
    auto save_portfolio_snapshot = [](const ::portfolio &portfolio, const coingecko::v3::simple::price::prices &summary) {
        json portfolio_json_array = json::array();
        map<string, double> total;

        for (auto &&[asset, ballance] : portfolio) {
            json json_valuation_array = json::array();
            const auto &prices = summary.at(asset);

            for (auto &&[currency, valuation] : prices) {
                const auto value = ballance * valuation.value;
                total[currency] += value;
                json_valuation_array.emplace_back(json{
                    {"currency", currency},
                    {"valuation", value},
                });
            }

            portfolio_json_array.emplace_back(json{
                {"asset", asset},
                {"balance", ballance},
                {"valuation", std::move(json_valuation_array)},
            });
        }

        const auto home_dir_path = getenv("HOME");
        const auto timestamp_epoch = std::chrono::system_clock::now().time_since_epoch();
        const auto timestamp_ms = std::chrono::duration_cast<std::chrono::milliseconds>(timestamp_epoch).count();

        json total_valuation_array = json::array();
        for (auto &&[currency, valuation] : total)
            total_valuation_array.emplace_back(json{
                {"currency", currency},
                {"valuation", valuation},
            });

        const auto portfolio_json_dump = json{
            {"timestamp", timestamp_ms},
            {"portfolio", portfolio_json_array},
            {"total", total_valuation_array},
        };

        std::error_code ec{};
        std::filesystem::create_directories(std::format("{}/.cache/crypto", home_dir_path), ec);
        std::ofstream file(std::format("{}/.cache/crypto/dump_{}.json", home_dir_path, timestamp_ms));
        file << portfolio_json_dump.dump(4);
    };
}
