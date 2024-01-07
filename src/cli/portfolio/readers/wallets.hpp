#pragma once

#include <string>
#include <vector>

#include <csv.hpp>

namespace readers {

auto wallets_from_csv(const std::vector<std::string> &files) -> std::vector<std::pair<std::string, std::string>> {
    using namespace ::std;
    using namespace ::csv;

    vector<pair<string, string>> ret;
    for (auto &&file : files) {
        // logger->info("reading data from {}", file);
        CSVReader reader(file);
        for (auto &&row : reader)
            ret.emplace_back(make_pair(row[0].get(), row[1].get()));
    }

    return ret;
}

} // namespace readers
