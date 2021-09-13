#pragma once

#include "indicators/indicators.hpp"

namespace indicator {

struct exponential_moving_average {
    exponential_moving_average(types::indicator_settings settings = {})
            : m_settings(settings) {
        m_settings.frame_size = m_settings.frame_size.value_or(25);
        k_param = 2.0 / static_cast<double>(m_settings.frame_size.value() + 1);
    };

    auto compute_value(const types::time_point t) noexcept -> types::indicator_value {
        if (is_empty(data_set) || data_set.size() < m_settings.frame_size.value())
            return {};

        if (data_set.back().time_stamp < t.point)
            return {};

        std::size_t current_window_fill = 0;
        double current_window_ema_sum = 0.0;

        for (auto data_i = data_set.rbegin(); data_i != data_set.rend(); ++data_i) {
            if (data_i->time_stamp <= t.point) {
                current_window_fill++;
                current_window_ema_sum = data_i->price * k_param + current_window_ema_sum * (1 - k_param);
                if (current_window_fill == m_settings.frame_size.value()) {
                    return {static_cast<types::indicator_value>(current_window_ema_sum)};
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
    double k_param{};
    std::vector<types::currency> data_set;
};

} // namespace indicator
