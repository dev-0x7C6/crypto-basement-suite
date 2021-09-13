#pragma once

#include "indicators.hpp"

namespace indicator {

struct moving_average {
    moving_average(types::indicator_settings settings = {})
            : m_settings(settings) {
        m_settings.frame_size = m_settings.frame_size.value_or(25);
    }

    auto compute_value(const types::time_point t) noexcept -> types::indicator_value {
        if (is_empty(data_set) || data_set.size() < m_settings.frame_size)
            return {};

        if (data_set.back().time_stamp < t)
            return {};

        std::size_t current_window_fill = 0;
        float current_window_sum = 0.0;
        for (auto data_i = data_set.rbegin(); data_i != data_set.rend(); ++data_i) {
            if (data_i->time_stamp <= t) {
                current_window_fill++;
                current_window_sum += data_i->price;
                if (current_window_fill == m_settings.frame_size) {
                    return {current_window_sum / m_settings.frame_size.value()};
                }
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
