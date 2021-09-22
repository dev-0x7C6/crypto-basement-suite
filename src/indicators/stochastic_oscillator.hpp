#pragma once

#include "indicators.hpp"
#include <limits>
#include <ranges>
#include <iostream>

namespace indicator {
// https://www.investopedia.com/terms/s/stochasticoscillator.asp
struct stochastic_oscillator {
    stochastic_oscillator(types::indicator_settings settings = {})
            : m_settings(settings) {
        m_settings.frame_size = m_settings.frame_size.value_or(5);
    }

    auto compute_value(const types::time_point t) noexcept -> types::indicator_value {
        if (is_empty(data_set) || data_set.size() < m_settings.frame_size)
            return {};
        // if time stamp is outside of data set
        if (data_set.back().time_stamp < t)
            return {};

        auto current_price = data_set.back();
        std::size_t current_window_fill = 0;
        bool frame_will_not_be_exceded = false;
        float period_maximum_price = 0;
        float period_minimum_price = std::numeric_limits<float>::max();
        for (auto sample = data_set.rbegin(); sample != data_set.rend(); ++sample) {
            if (t.after(sample->time_stamp))
                break;
            // if current point is closer than window length then indicator value should not be computed
            if(!frame_will_not_be_exceded) {
                if(data_set.rend() - sample < m_settings.frame_size.value_or(0)) {
                    return {};
                }
                else {
                    current_price.price = sample->price;
                    frame_will_not_be_exceded = true;
                }
            }
            if(period_maximum_price < sample->price)
                period_maximum_price = sample->price;
            if(period_minimum_price > sample->price)
                period_minimum_price = sample->price;
            if(current_window_fill == m_settings.frame_size.value_or(0))
            {
                if (period_maximum_price - period_minimum_price != 0 && m_settings.frame_size.value_or(0) > 0)
                    return {static_cast<float>(100.0*((current_price.price-period_minimum_price)/(period_maximum_price - period_minimum_price)))};
                else
                    return {100.0};
            }
            current_window_fill++;
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
