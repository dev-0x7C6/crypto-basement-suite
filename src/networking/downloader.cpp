#include "downloader.hpp"

#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/cURLpp.hpp>

#include <sstream>

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
