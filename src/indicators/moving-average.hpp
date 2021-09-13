#pragma once

#include "indicators.hpp"

namespace indicator {

struct moving_average {
    moving_average(types::indicator_settings settings = {})
            : m_settings(settings) {
        m_settings.frame_size = m_settings.frame_size.value_or(25);
    }

    auto compute_value(const types::time_point t) noexcept -> types::indicator_value {
        if (data_set.size() == 0 || data_set.size() < m_settings.frame_size)
            return {static_cast<types::indicator_value>(0)};
        if (data_set.back().time_stamp < t.point)
            return {static_cast<types::indicator_value>(0)};
        std::size_t current_window_fill = 0;
        double current_window_sum = 0;
        for (auto data_i = data_set.rbegin(); data_i != data_set.rend(); ++data_i) {
            if (data_i->time_stamp <= t.point) {
                current_window_fill++;
                current_window_sum += data_i->price;
                if (current_window_fill == m_settings.frame_size) {
                    return {static_cast<types::indicator_value>(current_window_sum / m_settings.frame_size.value())};
                }
            }
        }
        return {static_cast<types::indicator_value>(0)};
    }

    // this function is too similar for every indicators, going to make it a free function
    auto load_data(types::currency data) noexcept -> void {
        if (data_set.size() != 0) {
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
        } else {
            data_set.push_back(data);
            return;
        }
    }

private:
    types::indicator_settings m_settings;
    std::vector<types::currency> data_set;
};

} // namespace indicator
