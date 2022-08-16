
#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <optional>
#include <vector>

#include <fmt/format.h>

#include <range/v3/algorithm/sort.hpp>
#include <range/v3/view/split.hpp>

#include <CLI/CLI.hpp>
#include <spdlog/spdlog.h>

#include <csv.hpp>
#include <common.hpp>

using namespace csv;

auto fix(const std::vector<sample> &v, statistics &stats) -> std::vector<sample> {
    std::vector<sample> ret;
    const auto diff = v[1].timestamp - v[0].timestamp;

    s64 correction{};
    spdlog::info("samples by: {}s", diff);

    for (auto i = 0ull; i < v.size(); ++i) {
        if (v[i].timestamp % diff) {
            spdlog::warn("unaligned data at: {}, skipping", v[i].timestamp);
            stats.unaligned++;
            continue;
        }

        if (v[i].trade_count == 0) {
            spdlog::warn("tradecount 0 at: {}, skipping", v[i].timestamp);
            stats.empty++;
        }

        const s64 timestamp = v[i].timestamp - correction;
        const s64 expected = diff * i + v.front().timestamp;
        const auto skew = timestamp - expected;

        if (skew) {
            spdlog::warn("detected data skew at: {}, missing: {} samples", v[i].timestamp, std::abs(skew / diff));
            stats.missing += std::abs(skew / diff);
            for (auto j = expected; j < timestamp; j += diff) {
                auto s = v[i];
                s.timestamp = j;
                s.trade_count = 0;
                ret.emplace_back(s);
            }
            correction += skew;
            continue;
        }

        ret.emplace_back(v[i]);
    }

    return ret;
}

namespace csv {
struct scheme {
    std::pair<std::string, std::string> pair;
    std::string timestamp;
    std::string open;
    std::string close;
    std::string high;
    std::string low;
    std::string trade_count;
    std::pair<std::string, std::string> volume;
    int divide_timestamp{1};
};
} // namespace csv

auto read(const CSVRow &row, const csv::scheme &data) -> sample {
    sample ret{};

    if (!data.timestamp.empty())
        ret.timestamp = row[data.timestamp].get<u64>() / data.divide_timestamp;

    if (!data.close.empty())
        ret.close = row[data.close].get<float>();

    if (!data.high.empty())
        ret.high = row[data.high].get<float>();

    if (!data.low.empty())
        ret.low = row[data.low].get<float>();

    if (!data.open.empty())
        ret.open = row[data.open].get<float>();

    if (!data.volume.first.empty())
        ret.vol.first = row[data.volume.first].get<float>();

    if (!data.volume.second.empty())
        ret.vol.second = row[data.volume.second].get<float>();

    if (!data.trade_count.empty())
        ret.trade_count = row[data.trade_count].get<u64>();

    return ret;
}

/*
 * CryptoDataDownload example
 * --pair BTC/USDT
 * --timestamp-div 1000
 * --timestamp unix
 * --open open
 * --close close
 * --low low
 * --high high
 * --volume "Volume BTC,Volume USDT"
 * --input Binance_BTCUSDT_2019_minute.csv
 * --input Binance_BTCUSDT_2020_minute.csv
 * --input Binance_BTCUSDT_2021_minute.csv
 * --input Binance_BTCUSDT_2022_minute.csv
 * --output btc2usdt.cmap
 *
 * or:
 * --crypto-data-download
 * --input Binance_BTCUSDT_2019_minute.csv
 * --output btc2usdt.cmap
 */

auto make_crypto_data_download_scheme(const std::pair<std::string, std::string> &symbol) -> csv::scheme {
    auto volume = [](const std::string &symbol) {
        return fmt::format("Volume {}", symbol);
    };

    return {
        .pair = symbol,
        .timestamp = "unix",
        .open = "open",
        .close = "close",
        .high = "high",
        .low = "low",
        .trade_count = "tradecount",
        .volume = {volume(symbol.first), volume(symbol.second)},
        .divide_timestamp = 1000,
    };
}

auto main(int argc, char **argv) -> int {
    CLI::App app("CSV to binary converter for cryptodatadownload.com");

    std::vector<std::string> inputs;
    std::string output_file;
    csv::scheme scheme{};
    auto crypto_data_download{false};

    auto input_file_opt = app.add_option("-i,--input,input", inputs, "CSV input file");
    auto output_dir_opt = app.add_option("-o,--output,ouput", output_file, "Output directory");
    auto symbol_opt = app.add_option("-p,--pair,pair", scheme.pair, "Pair symbol i.e. BTC/USDT");
    auto scheme_cdd = app.add_flag("--crypto-data-download", crypto_data_download, "Crypto data download scheme");
    auto timestamp_opt = app.add_option("--timestamp,timestamp", scheme.timestamp, "CSV column for 'timestamp' column");
    auto timestamp_div_opt = app.add_option("--timestamp-div", scheme.divide_timestamp, "Divide 'timestamp' by X");
    auto open_opt = app.add_option("--open,open", scheme.open, "CSV column for 'open' price");
    auto close_opt = app.add_option("--close,close", scheme.close, "CSV column for 'close' price");
    auto high_opt = app.add_option("--high,high", scheme.high, "CSV column for 'high' price");
    auto low_opt = app.add_option("--low,low", scheme.low, "CSV column for 'low' price");
    auto volume_opt = app.add_option("--volume,volume", scheme.volume, "CSV column pair for volume data (separated by ',')");

    symbol_opt->required(true)->expected(1)->delimiter('/');
    volume_opt->expected(1)->delimiter(',');

    input_file_opt->allow_extra_args()->required()->check(CLI::ExistingFile);
    output_dir_opt->required();
    CLI11_PARSE(app, argc, argv);

    if (crypto_data_download)
        scheme = make_crypto_data_download_scheme(scheme.pair);

    const auto pair = scheme.pair.first + "/" + scheme.pair.second;

    std::vector<sample> samples;
    statistics stats{};
    header head{};
    std::memcpy(head.symbols.data(), pair.data(), pair.size());

    for (auto &&input : inputs) {
        spdlog::info("parsing {} ...", input);
        CSVReader reader(input);

        for (CSVRow &row : reader)
            samples.emplace_back(read(row, scheme));
    }

    std::ofstream output(output_file, std::ios::out | std::ios::binary);

    std::sort(samples.begin(), samples.end(), [](auto &&lhs, auto &&rhs) {
        return lhs.timestamp < rhs.timestamp;
    });

    stats.clones = samples.size();
    samples.erase(std::unique(samples.begin(), samples.end()), samples.end());
    stats.clones = stats.clones - samples.size();

    samples = fix(samples, stats);

    output.write(reinterpret_cast<const char *>(&head), sizeof(head));

    for (auto &&sample : samples) {
        const auto fs = to_fs(sample);
        output.write(reinterpret_cast<const char *>(&fs), sizeof(fs));
    }

    spdlog::info("samples count: {}", samples.size());
    spdlog::info("samples size: {:.2f}MiB", samples.size() * sizeof(sample) / 1024.0 / 1024.0);
    spdlog::info("sample clone count: {}", stats.clones);
    spdlog::info("sample empty count: {}", stats.empty);
    spdlog::info("sample missing: {}", stats.missing);
    spdlog::info("sample unaligned: {}", stats.unaligned);
    return 0;
}
