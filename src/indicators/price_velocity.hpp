#pragma once

#include "indicators.hpp"

#include <ranges>

namespace indicator {
//https://www.investopedia.com/articles/technical/081501.asp
struct price_velocity {
    price_velocity(types::indicator_settings settings = {})
            : m_settings(settings) {
        m_settings.frame_size = m_settings.frame_size.value_or(25);
    }

    auto compute_value(const types::time_point t) noexcept -> types::indicator_value {
        if (is_empty(data_set) || data_set.size() < m_settings.frame_size)
            return {};
        // if time stamp is outside of data set
        if (data_set.back().time_stamp < t)
            return {};

        auto current_price = data_set.back();
        std::size_t current_window_fill = 0;
        for (auto &&sample : data_set | std::views::reverse) {
            if (t.after(sample.time_stamp))
                break;
            // save the ,,current'' price for the searched time t
            if (current_window_fill == 0){
                current_price = sample;
            }
            current_window_fill++;
            if (current_window_fill == m_settings.frame_size) {
                return {current_price.price - sample.price};
            }
        }
        return {};
    }

    // this function is too similar for every indicators, going to make it a free function
    auto load_data(types::currency data) noexcept -> void {
        if (is_empty(data_set)) {
            data_set.push_back(data);
            return;
        }

        if (data_set.back().time_stamp <= data.time_stamp) {
            data_set.push_back(data);
            return;
        }

        for (auto data_i = data_set.rbegin(); data_i != data_set.rend(); ++data_i) {
            if (data_i->time_stamp <= data.time_stamp) {
                if (data_i->time_stamp == data.time_stamp) {
                    data_i->price = data.price;
                    return;
                } else {
                    data_set.insert(data_i.base() + 1, data);
                    return;
                }
            }
        }
    }

private:
    types::indicator_settings m_settings;
    std::vector<types::currency> data_set;
};

} // namespace indicator