
#include <algorithm>
#include <cstring>
#include <fstream>
#include <vector>
#include <cmath>

#include <range/v3/algorithm/sort.hpp>
#include <range/v3/view/split.hpp>

#include <CLI/CLI.hpp>
#include <spdlog/spdlog.h>

#include <csv.hpp>
#include <common.hpp>

using namespace csv;

auto read(const CSVRow &row) -> sample {
    const auto get_u64 = [&row](const std::size_t index) noexcept {
        return row[index].get<u64>();
    };

    const auto get_u32 = [&row](const std::size_t index) noexcept {
        return row[index].get<u32>();
    };

    const auto get_float = [&row](const std::size_t index) noexcept {
        return row[index].get<float>();
    };

    return {
        .timestamp = static_cast<u32>(get_u64(0) / 1000),
        .open = get_float(3),
        .high = get_float(4),
        .low = get_float(5),
        .close = get_float(6),
        .vol = {get_float(7), get_float(8)},
        .trade_count = get_u32(9)};
}

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

auto main(int argc, char **argv) -> int {
    CLI::App app("CSV to binary converter for cryptodatadownload.com");

    std::vector<std::string> inputs;
    std::string output_file;

    auto input_file_opt = app.add_option("-i,--input,input", inputs, "CSV input file");
    auto output_dir_opt = app.add_option("-o,--output,ouput", output_file, "Output directory");

    input_file_opt->allow_extra_args()->required()->check(CLI::ExistingFile);
    output_dir_opt->required();
    CLI11_PARSE(app, argc, argv);

    std::vector<sample> samples;
    statistics stats{};
    header head{};

    for (auto &&input : inputs) {
        spdlog::info("parsing: {}", input);
        CSVReader reader(input);

        header head{};
        auto symbol = reader.begin()->operator[]("symbol").get<std::string>(); // assume same data
        const auto size = symbol.size() > head.symbols.size() ? head.symbols.size() : symbol.size();
        std::memcpy(head.symbols.data(), symbol.c_str(), size);
        std::replace(symbol.begin(), symbol.end(), '/', '2');

        for (CSVRow &row : reader)
            samples.emplace_back(read(row));
    }

    std::ofstream output(output_file, std::ios::out | std::ios::binary);

    std::sort(samples.begin(), samples.end(), [](auto &&rhs, auto &&lhs) {
        return rhs.timestamp < lhs.timestamp;
    });

    stats.clones = samples.size();
    samples.erase(std::unique(samples.begin(), samples.end()), samples.end());
    stats.clones = stats.clones - samples.size();

    samples = fix(samples, stats);

    output.write(reinterpret_cast<const char *>(&head), sizeof(head));
    output.write(reinterpret_cast<const char *>(samples.data()), samples.size() * sizeof(sample));

    spdlog::info("samples count: {}", samples.size());
    spdlog::info("samples size: {:.2f}MiB", samples.size() * sizeof(sample) / 1024.0 / 1024.0);
    spdlog::info("sample clone count: {}", stats.clones);
    spdlog::info("sample empty count: {}", stats.empty);
    spdlog::info("sample missing: {}", stats.missing);
    spdlog::info("sample unaligned: {}", stats.unaligned);

    return 0;
}
