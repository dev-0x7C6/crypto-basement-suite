
#include <algorithm>
#include <cstring>
#include <fstream>
#include <optional>
#include <vector>

#include <range/v3/algorithm/sort.hpp>
#include <range/v3/view/split.hpp>

#include <CLI/CLI.hpp>
#include <spdlog/spdlog.h>

#include <common.hpp>

auto main(int argc, char **argv) -> int {
    CLI::App app("cmap merge tool");

    std::vector<std::string> inputs;
    std::string output_file;

    auto input_file_opt = app.add_option("-i,--input,input", inputs, "cmap input files");
    auto output_file_opt = app.add_option("-o,--output,ouput", output_file, "cmap output file");

    input_file_opt->allow_extra_args()->check(CLI::ExistingFile);
    output_file_opt->required();
    CLI11_PARSE(app, argc, argv);

    spdlog::info("output file: {}", output_file);

    std::vector<sample> samples;
    std::vector<header> headers;

    for (auto &&input : inputs) {
        spdlog::info("reading header: {}", input);
        headers.emplace_back([&input]() {
            std::ifstream file(input, std::ios::in | std::ios::binary);
            header head;
            file.read(reinterpret_cast<char *>(&head), sizeof(header));
            return head;
        }());
    }

    const auto valid = std::all_of(headers.begin(), headers.end(), [&headers](auto &&sample) {
        return sample == headers.front();
    });

    spdlog::info("all headers are same: {}", valid);

    for (auto &&input : inputs) {
        std::ifstream file(input, std::ios::in | std::ios::binary);
        file.seekg(sizeof(header), std::ios::beg);

        auto cnt{0ull};
        while (!file.eof()) {
            sample s;
            file.read(reinterpret_cast<char *>(&s), sizeof(s));
            if (file.gcount() == 0)
                break;
            samples.emplace_back(s);
            cnt++;
        }
        spdlog::info("file {}, provided samples: {}", input, cnt);
        spdlog::info("size: {}", sizeof(sample) * cnt);
    }

    spdlog::info("all samples: {}", samples.size());
    std::sort(samples.begin(), samples.end());
    samples.erase(std::unique(samples.begin(), samples.end()), samples.end());
    spdlog::info("unique samples: {}", samples.size());

    const header h = headers.front();
    std::ofstream file(output_file, std::ios::out | std::ios::binary | std::ios::trunc);
    file.write(reinterpret_cast<const char *>(&h), sizeof(header));
    file.write(reinterpret_cast<const char *>(samples.data()), samples.size() * sizeof(sample));

    return 0;
}
