#pragma once

#include <expected>
#include <string>

namespace blockfrost::v0 {

enum class error {
    bad_request = 400,
    unauthorized = 401,
    forbidden = 403,
    not_found = 404,
    too_many_requests = 429,
    generic_error = 1000,
    invalid_parameter = 1001,
    required_parameter_missing = 1002,
    api_key_missing = 1003,
    api_key_invalid = 1004,
    rate_limit_exceeded = 1005,
    service_unavailable = 1006,
    parse_error = 1007
};

struct options {
    std::string provider{"https://cardano-mainnet.blockfrost.io/api/v0"};
    std::string key;
};

} // namespace blockfrost::v0
