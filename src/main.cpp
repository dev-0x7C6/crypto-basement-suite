#include <model/model.hpp>
#include <model/stub.hpp>
#include <iostream>

auto main(int, char **) -> int {
    currency::data::provider::stub stub{};
    cbs::time fresh = std::chrono::system_clock::now();
    cbs::time last = fresh;
    auto miliseconds_difference = std::chrono::duration_cast<std::chrono::milliseconds>(fresh - last);
    int64_t time_period = 10;
    while(true)
    {
        fresh = std::chrono::system_clock::now();  
        miliseconds_difference = std::chrono::duration_cast<std::chrono::milliseconds>(fresh - last);
        if(miliseconds_difference.count() > time_period)
        {
            std::cout << "Price: " << stub.value(fresh).price << std::endl;
            last = fresh;
        }
    }
    return 0;
}
