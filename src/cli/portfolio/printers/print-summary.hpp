#pragma once

#include <common/configuration.hpp>
#include <common/share.hpp>
#include <spdlog/spdlog.h>

namespace printers {

void summary(
    const portfolio &total,
    const configuration &config,
    std::shared_ptr<spdlog::logger> logger);

}
