#pragma once

#include <expected>
#include <map>

#include <spdlog/spdlog.h>

#include "common/configuration.hpp"
#include "helpers/threading.hpp"

using shared_logger = std::shared_ptr<spdlog::logger>;

namespace chain {

enum class error {
    unable_to_query,
    data_unavailable,
};

using results = std::expected<std::map<std::string, double>, error>;
using callback = std::function<task<results>(const shared_logger &, const std::string &, const configuration &)>;
} // namespace chain
