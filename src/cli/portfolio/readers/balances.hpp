#pragma once

#include <string>
#include <vector>

#include <csv.hpp>

namespace readers {

auto balances_from_csv(const std::vector<std::string> &files) -> std::vector<std::pair<std::string, double>> {
    using namespace ::std;
    using namespace ::csv;

    vector<pair<string, double>> ret;
    for (auto &&file : files) {
        // logger->info("reading data from {}", file);
        CSVReader reader(file);

        for (auto &&row : reader)
            ret.emplace_back(make_pair(row[0].get(), row[1].get<double>()));
    }

    std::ranges::sort(ret, [](auto &&lhs, auto &&rhs) {
        return lhs.first < rhs.first;
    });

    return ret;
}

} // namespace readers
