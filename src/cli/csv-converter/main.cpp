
#include <algorithm>
#include <cstring>
#include <fstream>
#include <vector>

#include <range/v3/algorithm/sort.hpp>
#include <range/v3/view/split.hpp>

#include <CLI/CLI.hpp>
#include <spdlog/spdlog.h>

#include "csv.hpp"

using namespace csv;

using f32 = std::float_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using volume = std::pair<float, float>;

struct header {
    u16 magic{0xfeca};
    u16 version{1};
    std::array<char, 64> symbols{};
};

struct sample {
    u32 timestamp;
    f32 open;
    f32 high;
    f32 low;
    f32 close;
    volume vol;
    u32 trade_count;
};

static_assert(sizeof(header) == 68);
static_assert(sizeof(sample) == 32);

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

auto main(int argc, char **argv) -> int {
    CLI::App app("CSV to binary converter for cryptodatadownload.com");

    std::string input_file;
    std::string output_dir{"./"};

    auto input_file_opt = app.add_option("-i,--input,input", input_file, "CSV input file");
    auto output_dir_opt = app.add_option("-o,--output,ouput", output_dir, "Output directory");

    input_file_opt->required()->check(CLI::ExistingFile);
    output_dir_opt->check(CLI::ExistingDirectory);
    CLI11_PARSE(app, argc, argv);

    spdlog::info("input file: {}", input_file);
    spdlog::info("output dir: {}", output_dir);

    CSVReader reader(input_file);

    std::vector<sample> samples;
    header head{};
    auto symbol = reader.begin()->operator[]("symbol").get<std::string>(); // assume same data
    const auto size = symbol.size() > head.symbols.size() ? head.symbols.size() : symbol.size();
    std::memcpy(head.symbols.data(), symbol.c_str(), size);
    std::replace(symbol.begin(), symbol.end(), '/', '2');

    std::ofstream output(output_dir + symbol + ".cmap", std::ios::out | std::ios::binary);

    for (CSVRow &row : reader)
        samples.emplace_back(read(row));

    output.write(reinterpret_cast<const char *>(&head), sizeof(head));

    std::sort(samples.begin(), samples.end(), [](auto &&rhs, auto &&lhs) {
        return rhs.timestamp < lhs.timestamp;
    });

    for (auto &&sample : samples)
        output.write(reinterpret_cast<const char *>(&sample), sizeof(sample));

    spdlog::info("samples count: {}", samples.size());
    spdlog::info("samples size: {:.2f}MiB", samples.size() * sizeof(sample) / 1024.0 / 1024.0);

    return 0;
}
