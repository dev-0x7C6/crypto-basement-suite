#include <CLI/CLI.hpp>
#include <csv.hpp>
#include <filesystem>
#include <indicators/block_progress_bar.hpp>
#include <indicators/progress_bar.hpp>
#include <indicators/setting.hpp>
#include <simdjson.h>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <chrono>
#include <format>
#include <iterator>
#include <meta>
#include <ranges>
#include <string>
#include <vector>

#include <readers/balances.hpp>
#include <rest/requests.hpp>
#include <types.hpp>

#include "midnight.hpp"

using namespace csv;
using namespace std;
using namespace std::chrono_literals;
using namespace std::chrono;

constexpr auto provider = "https://mainnet.prod.gd.midnighttge.io";

enum class error {
    bad_request,
    json_error
};

auto request(const string &url, const map<string, string> &header = {}) -> expected<string, error> {
    try {
        const auto response = ::curl::request(url, header);
        if (!response) return unexpected(error::bad_request);
        return response.value();
    } catch (...) {
    }

    return unexpected(error::bad_request);
}

template <typename T>
auto deserialize(string json) -> T {
    return simdjson::from(std::move(json));
}

template <typename T>
auto request_json(const string &url, const map<string, string> &header = {}) -> expected<T, error> {
    try {
        const auto ret = request(url, header);
        if (!ret)
            return unexpected(ret.error());

        return deserialize<T>(std::move(ret.value()));
    } catch (...) {
    }

    return unexpected(error::json_error);
}

auto to_year_month_day(const string &input) -> year_month_day {
    try {
        istringstream ss{input};
        sys_seconds tp;
        ss >> parse("%FT%TZ", tp);
        return floor<days>(tp);
    } catch (...) {
    }

    return {};
}

auto short_addr(const string &long_addr) -> string {
    string addr;
    addr += long_addr.substr(0, 10);
    addr += "..";
    addr += long_addr.substr(long_addr.size() - 4, 4);
    return addr;
}

auto valid_entry_address(const readers::midnight_address_entry &entry) {
    return entry.address.starts_with("addr1");
}

auto main(int argc, char **argv) -> int {
    CLI::App app("midnight-airdrop-tracker");
    std::filesystem::path filepath;

    app.add_option("-i,--input", filepath, "csv format <description, id, address>") //
        ->required() //
        ->allow_extra_args() //
        ->check(CLI::ExistingFile); //

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError &e) {
        return -1;
    }

    auto entries = //
        readers::read_midnight_addresses({"/home/dev/workdir/midnight.csv"}) //
        | views::filter(valid_entry_address);

    const auto count = ranges::distance(entries);

    using namespace indicators;

    ProgressBar bar{
        option::BarWidth{50},
        option::Start{"["},
        option::Fill{"="},
        option::Lead{">"},
        option::Remainder{" "},
        option::End{"]"},
        option::ForegroundColor{Color::green},
        option::ShowPercentage{true},
        option::FontStyles{vector<FontStyle>{FontStyle::bold}},
        option::MinProgress{0},
        option::MaxProgress{count}};

    using namespace midnight::airdrop;

    addresses_vec addresses;
    for (auto &&entry : entries) {
        const auto query = format("{}/thaws/{}/schedule", provider, entry.address);

        bar.set_option(option::PostfixText{fmt::format("Checking {}", short_addr(entry.address))});
        bar.tick();

        struct json_thaw_response {
            thaws_vec thaws;
        };

        const auto ret = request_json<json_thaw_response>(query);

        address address;
        address.id = entry.address;
        address.thaws = std::move(ret->thaws);
        addresses.emplace_back(std::move(address));
    }

    ranges::sort(addresses, [](const address &lhs, const address &rhs) {
        const auto lamount = accumulate_redeemable_thaws(lhs.thaws);
        const auto ramount = accumulate_redeemable_thaws(rhs.thaws);
        return lamount > ramount;
    });

    spdlog::set_pattern("%v");

    for (auto &&[index, address] : addresses | ranges::views::enumerate)
        spdlog::info("{}: {} <- redeemable[{}/{}]: {}",
            index,
            short_addr(address.id), //
            count_redeemable_thaws(address.thaws), //
            address.thaws.size(),
            accumulate_redeemable_thaws(address.thaws) / 1'000'000 //
        );

    const auto stats = extract_stats(addresses);
    const auto fmt_total = stats.total / 1'000'000;
    const auto fmt_total_redeemable = stats.total_redeemable / 1'000'000;
    const auto fmt_total_redeemable_percent = stats.total_redeemable / stats.total * 100.0;
    const auto fmt_total_redeemed = stats.total_redeemed / 1'000'000;
    const auto fmt_total_redeemed_percent = stats.total_redeemed / stats.total * 100.0;
    const auto fmt_total_pending = fmt_total - fmt_total_redeemable - fmt_total_redeemed;
    const auto fmt_total_pending_percent = fmt_total_pending / fmt_total * 100.0;

    spdlog::info("total: {}", fmt_total);
    spdlog::info("redeemable: {}, ({:.2f}%)", fmt_total_redeemable, fmt_total_redeemable_percent);
    spdlog::info("redeemed: {}, ({:.2f}%)", fmt_total_redeemed, fmt_total_redeemed_percent);
    spdlog::info("pending: {}, ({:.2f}%)", fmt_total_pending, fmt_total_pending_percent);
    spdlog::info("thaw count: {}/{}", stats.accessible_thaw_count, stats.total_thaw_count);

    return 0;
}
