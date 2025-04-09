
#include <algorithm>
#include <cstring>
#include <fstream>
#include <vector>

#include <CLI/CLI.hpp>
#include <spdlog/spdlog.h>

#include <common.hpp>

struct cm {
    header head{};
    std::vector<sample> samples;
};

namespace cmap {
namespace memory {

struct data {
    header head{};
    std::vector<sample> samples;
};

auto read(const std::string &path) -> data {
    data ret;
    std::ifstream file(path, std::ios::in | std::ios::binary);
    file.read(reinterpret_cast<char *>(&ret.head), sizeof(ret.head));

    while (!file.eof()) {
        sample s;
        file.read(reinterpret_cast<char *>(&s), sizeof(s));
        if (file.gcount() == 0)
            break;
        ret.samples.emplace_back(s);
    }

    return ret;
}

auto store(const std::string &path, const header &head, const std::vector<sample> &samples) -> void {
    std::ofstream file(path, std::ios::out | std::ios::binary | std::ios::trunc);
    file.write(reinterpret_cast<const char *>(&head), sizeof(head));
    file.write(reinterpret_cast<const char *>(samples.data()), samples.size() * sizeof(sample));
}
} // namespace memory
} // namespace cmap

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

    std::vector<cmap::memory::data> cmaps;
    u64 sizes{};

    for (auto &&input : inputs) {
        cmaps.emplace_back(cmap::memory::read(input));
        const auto size = cmaps.end()->samples.size();

        spdlog::info("file {}, provided samples: {}", input, size);
        spdlog::info("size: {}", sizeof(sample) * size);
    }

    const auto head = cmaps.front().head;
    const auto valid = std::all_of(cmaps.begin(), cmaps.end(), [&](auto &&v) {
        return head == v.head;
    });

    spdlog::info("all headers are same: {}", valid);

    samples.reserve(sizes);

    for (auto &&cmap : cmaps)
        samples.insert(samples.end(), cmap.samples.begin(), cmap.samples.end());

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
