#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include <csv.hpp>

namespace readers {

struct midnight_address_entry {
    std::string description;
    std::string id;
    std::string address;
};

auto read_midnight_addresses(const std::vector<std::filesystem::path> &files) -> std::vector<midnight_address_entry> {
    using namespace ::std;
    using namespace ::csv;

    vector<midnight_address_entry> ret;

    for (auto &&file : files) {
        CSVReader reader(file.string());
        for (auto &&row : reader)
            if (row.size() >= 3)
                ret.emplace_back(midnight_address_entry{
                    .description = row[0].get(),
                    .id = row[1].get(),
                    .address = row[2].get(),
                });
    }

    return ret;
}

} // namespace readers
