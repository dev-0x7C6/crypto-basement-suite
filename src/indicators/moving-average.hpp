#pragma once

#include "indicators.hpp"

namespace indicator {

struct moving_average {
    //its bad, made it just for testing
    inline static const std::string Option_WindowSize = "window_size";

    moving_average() {
        window_size = 0;
        data_set = std::make_unique<std::vector<types::currency>>();
    };

    auto compute_value(const types::time_point t) noexcept -> types::indicator_value {
        if (data_set->size() == 0 || data_set->size() < window_size)
            return {static_cast<types::indicator_value>(0)};
        if (data_set->back().time_stamp < t.point)
            return {static_cast<types::indicator_value>(0)};
        std::size_t current_window_fill = 0;
        double current_window_sum = 0;
        for (auto data_i = data_set->rbegin(); data_i != data_set->rend(); ++data_i) {
            if (data_i->time_stamp <= t.point) {
                current_window_fill++;
                current_window_sum += data_i->price;
                if (current_window_fill == window_size) {
                    return {static_cast<types::indicator_value>(current_window_sum / window_size)};
                }
            }
        }
        return {static_cast<types::indicator_value>(0)};
    }

    // its bad need to think about it later
    /*
    void find_configuration_option = [](std::tuple<std::string, std::string> option) {
         if (std::get<0>(option) == Option_WindowSize)
            window_size = std::get<1>(option);
     };
    */
    auto configure(std::list<std::tuple<std::string, std::string>> options_list) noexcept -> void {
        //std::for_each(options_list.begin(), options_list.end(), find_configuration_option);
        window_size = 25;
    }

    // this function is too similar for every indicators, going to make it a free function
    auto load_data(types::currency data) noexcept -> void {
        if (data_set->size() != 0) {
            if (data_set->back().time_stamp <= data.time_stamp) {
                data_set->push_back(data);
                return;
            }
            for (auto data_i = data_set->rbegin(); data_i != data_set->rend(); ++data_i) {
                if (data_i->time_stamp <= data.time_stamp) {
                    if (data_i->time_stamp == data.time_stamp) {
                        data_i->price = data.price;
                        return;
                    } else {
                        data_set->insert(data_i.base() + 1, data);
                        return;
                    }
                }
            }
        } else {
            data_set->push_back(data);
            return;
        }
    }

private:
    std::size_t window_size;
    std::unique_ptr<std::vector<types::currency>> data_set;
};

} // namespace indicator
