
#include "cardano-registry-scanner.hpp"

#include <fstream>
#include <string>

auto read_file_to_string(const std::filesystem::path &filepath) -> std::optional<std::string> {
    std::ifstream file(filepath, std::ios::in | std::ios::binary);

    if (!file.is_open())
        return {};

    std::stringstream buffer;
    buffer << file.rdbuf();

    return buffer.str();
}

namespace cardano::registry {

auto scan(const std::filesystem::path &path) -> token::database {
    if (!std::filesystem::exists(path))
        return {};

    token::database db;

    for (auto &&entry : std::filesystem::recursive_directory_iterator(path)) {
        if (!entry.is_regular_file()) continue;
        if (entry.path().extension() != ".json") continue;

        const auto content = read_file_to_string(entry.path());
        if (content->empty()) continue;

        const auto data = nlohmann::json::parse(content.value(), nullptr, false);

        if (!data.contains("subject")) continue;
        auto contract = data.at("subject").get<std::string>();

        token::details token{};

        try {
            token.divisor = std::pow(10, data.at("decimals").at("value").get<int>());
        } catch (const nlohmann::json::out_of_range &e) {
        }

        db[contract] = token;
        contract.resize(56);
        db[contract] = token;
    }

    return db;
}

} // namespace cardano::registry
