#pragma once

#include <common/share.hpp>

#include <filesystem>

namespace json {

bool save(const portfolio &portfolio, const std::filesystem::path &path);

}
