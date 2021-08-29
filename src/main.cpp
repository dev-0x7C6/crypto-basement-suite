#include <model/model.hpp>
#include <model/stub.hpp>

auto main(int, char **) -> int {
    currency::data::provider::stub stub{};
    return 0;
}
