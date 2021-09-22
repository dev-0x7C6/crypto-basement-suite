#pragma once

#include "indicators.hpp"

#include <ranges>
#include <iostream>

namespace indicator {
//https://www.investopedia.com/terms/p/pricerateofchange.asp
// additional parameters:
// https://en.wikipedia.org/wiki/Relative_strength_index
// https://www.macroption.com/rsi-calculation/
struct relative_strength_index {
    relative_strength_index(types::indicator_settings settings = {})
            : m_settings(settings) {
        m_settings.frame_size = m_settings.frame_size.value_or(14);
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
        float ratio_of_change = 0;
        float previous_gain_sum = 0;
        float previous_loss_sum = 0;
        for (auto sample = data_set.rbegin(); sample != data_set.rend(); ++sample) {
            if (t.after(sample->time_stamp))
                break;
            // if current point is closer then indicator value should not be computed
            // because we need to substract prtevious point with current to get the rate of change,
            // we need one data point earlier than the frame size, so that why +1
            if(!frame_will_not_be_exceded) {
                if(data_set.rend() - sample < m_settings.frame_size.value_or(0) + 1) {
                    return {};
                }
                else {
                    frame_will_not_be_exceded = true;
                }
            }
            // first value of RSI indicator value need to be computed differently
            ratio_of_change = ((sample->price - (sample + 1)->price)/(sample + 1)->price);
            if(ratio_of_change >= 0)
                previous_gain_sum += ratio_of_change;
            else
                previous_loss_sum += -ratio_of_change;
            if(current_window_fill == m_settings.frame_size.value_or(0))
            {
                if (previous_loss_sum > 0 && m_settings.frame_size.value_or(0) > 0)
                    return {static_cast<float>(100.0-(100.0/(1.0+((previous_gain_sum/static_cast<float>(m_settings.frame_size.value()))/
                            (previous_loss_sum/static_cast<float>(m_settings.frame_size.value()))))))};
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
