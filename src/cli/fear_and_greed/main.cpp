#include <nlohmann/json.hpp>
#include <range/v3/all.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <chrono>
#include <optional>
#include <sstream>
#include <vector>

#include <rest/requests.hpp>
#include <types.hpp>

using namespace ranges;
using namespace std::chrono_literals;
using namespace nlohmann;

namespace fear_and_greed {

struct info {
    std::string value;
    std::string classification;
    std::string timestamp;
    std::string to_next_update;
};

auto info_from_json(const json &v) noexcept {
    info ret{};
    ret.value = v.value("value", ret.value);
    ret.classification = v.value("value_classification", ret.classification);
    ret.timestamp = v.value("timestamp", ret.timestamp);
    ret.to_next_update = v.value("time_until_update", ret.to_next_update);
    return ret;
}

auto to_info(const json &v) noexcept -> std::vector<info> {
    if (!v.contains("data")) return {};

    std::vector<info> ret;
    for (const auto &node : v["data"])
        ret.emplace_back(info_from_json(node));

    return ret;
}
} // namespace fear_and_greed

auto main(int argc, char **argv) -> int {
    auto console = spdlog::stdout_color_mt("console");

    const auto content = curl::request("https://api.alternative.me/fng/?limit=32").value_or("");
    const auto data = json::parse(content);
    const auto table = fear_and_greed::to_info(data);

    spdlog::set_pattern("%v");

    for (auto &&entry : table)
        spdlog::info("{}, {}: {}", entry.timestamp, entry.classification, entry.value);

    return 0;
}
