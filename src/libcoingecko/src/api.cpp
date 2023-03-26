#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/cURLpp.hpp>
#include <nlohmann/json.hpp>

#include <sstream>

using namespace nlohmann;

namespace {

namespace network {
auto request(const std::string &url) -> std::optional<std::string> {
    using namespace curlpp;
    using namespace curlpp::options;
    try {
        Cleanup cleanup;
        Easy request;
        request.setOpt<Url>(url);

        std::stringstream stream;
        stream << request;
        return stream.str();
    }

    catch (RuntimeError &e) {
        std::cout << e.what() << std::endl;
        return {};
    }

    catch (LogicError &e) {
        std::cout << e.what() << std::endl;
        return {};
    }

    return {};
}
} // namespace network

namespace network::json {

auto request(const std::string &url) -> nlohmann::json {
    const auto response = network::request(url);
    if (!response) return {};

    return ::json::parse(response.value());
}
} // namespace network::json

} // namespace

namespace coingecko::v3 {

auto request(const std::string &url) -> nlohmann::json {
    const auto json = network::json::request(url);
    if (json.empty()) return {};

    if (json.contains("status") && json["status"].contains("error_code"))
        return {};

    return json;
}
} // namespace coingecko::v3
