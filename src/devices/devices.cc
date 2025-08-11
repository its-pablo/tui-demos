// std library headers
#include <algorithm>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

// 3rd party headers
// ---- tomlplusplus ----
#include <toml++/toml.hpp>

// Local headers
#include "devices.h"

using namespace Devices;

std::string Device::info() const {
    std::ostringstream oss;
    oss << "------------------------" << std::endl;
    oss << "Device Name: " << name << std::endl;
    oss << "Pin: " << pin << std::endl;
    oss << "Type: " << type_to_string(type) << std::endl;
    oss << "Modality: " << modality_to_string(modality) << std::endl;
    if (type == Type::Digital) {
        if (is_active_low.has_value()) {
            oss << "Active Low: " << (is_active_low.value() ? "Yes" : "No")
                << std::endl;
        }
    } else if (type == Type::Analog) {
        if (units.has_value()) {
            oss << "Units: " << units.value() << std::endl;
        }
        if (abs_min.has_value()) {
            oss << "Absolute Min: " << abs_min.value() << std::endl;
        }
        if (abs_max.has_value()) {
            oss << "Absolute Max: " << abs_max.value() << std::endl;
        }
        if (rel_min.has_value()) {
            oss << "Relative Min: " << rel_min.value() << std::endl;
        }
        if (rel_max.has_value()) {
            oss << "Relative Max: " << rel_max.value() << std::endl;
        }
        for (const auto &warning : warnings) {
            oss << "Warning: [" + float_to_string(warning.first) + ", " +
                       float_to_string(warning.second) + "]"
                << std::endl;
        }
        for (const auto &caution : cautions) {
            oss << "Caution: [" + float_to_string(caution.first) + ", " +
                       float_to_string(caution.second) + "]"
                << std::endl;
        }
        for (const auto &optimal : optimals) {
            oss << "Optimal: [" + float_to_string(optimal.first) + ", " +
                       float_to_string(optimal.second) + "]"
                << std::endl;
        }
    }
    oss << "------------------------" << std::endl;
    return oss.str();
}

void Device::update_value() {
    float rand = static_cast<float>(std::rand()) / RAND_MAX;
    switch (type) {
    case Type::Analog: {
        if (rand <= 0.4) {
            float val = _value_analog - 0.005;
            if (val <= 0.0f) {
                val = 0.01f; // Ensure value does not go below 0
            }
            _value_analog.store(val);
        } else if (rand >= 0.6) {
            float val = _value_analog + 0.005;
            if (val >= 1.0f) {
                val = 0.99f; // Ensure value does not go above 1
            }
            _value_analog.store(val);
        }
        break;
    }
    case Type::Digital: {
        if (rand >= 0.99) {
            _value_digital.store((_value_digital == 1) ? 0 : 1);
        }
        break;
    }
    default:
        break;
    }
}

void Device::record_value_to_hist() {
    std::rotate(_value_analog_hist.begin(), _value_analog_hist.begin() + 1,
                _value_analog_hist.end());
    std::rotate(_value_digital_hist.begin(), _value_digital_hist.begin() + 1,
                _value_digital_hist.end());
    _value_analog_hist[hist_size - 1] = _value_analog.load();
    _value_digital_hist[hist_size - 1] = _value_digital.load();
}

std::vector<int> Device::get_value_analog_transform(int width,
                                                    int height) const {
    auto vals = get_value_analog_hist();
    std::vector<int> transform;
    for (int i = 0; i < width; i++) {
        int index = vals.size() * i / width;
        transform.push_back(vals[index] * height);
    }
    return transform;
}

std::vector<int> Device::get_value_digital_transform(int width,
                                                     int height) const {
    auto vals = get_value_digital_hist();
    std::vector<int> transform;
    for (int i = 0; i < width; i++) {
        int index = vals.size() * i / width;
        transform.push_back(vals[index] * height);
    }
    return transform;
}

std::vector<int> Device::get_value_transform(int width, int height) const {
    switch (type) {
    case Type::Analog:
        return get_value_analog_transform(width, height);
    case Type::Digital:
        return get_value_digital_transform(width, height);
    }
    return {};
}

bool in_intervals(float value,
                  const std::vector<std::pair<float, float>> &intervals) {
    bool in_interval = false;
    for (const auto interval : intervals) {
        if (value >= interval.first && value < +interval.second) {
            in_interval = true;
            break;
        }
    }
    return in_interval;
}

bool Device::is_warning(float value) const {
    return in_intervals(value, warnings);
}

bool Device::is_caution(float value) const {
    return in_intervals(value, cautions);
}

bool Device::is_optimal(float value) const {
    return in_intervals(value, optimals);
}

void parse_pairs(toml::v3::table *table, const std::string key,
                 std::vector<std::pair<float, float>> &intervals) {
    if (table->get(key) && table->get(key)->as_array() &&
        table->get(key)->as_array()->is_array_of_tables()) {
        auto toml_intervals = table->get(key)->as_array();
        for (auto &&interval : *toml_intervals) {
            auto min = interval.as_table()->get("min")->value_or<float>(0.0);
            auto max = interval.as_table()->get("max")->value_or<float>(0.0);
            intervals.push_back(std::make_pair(min, max));
        }
    }
}

std::vector<std::pair<float, float>> Device::find_uncovered_intervals() {
    std::vector<std::pair<float, float>> intervals;
    intervals.insert(intervals.end(), warnings.begin(), warnings.end());
    intervals.insert(intervals.end(), cautions.begin(), cautions.end());
    intervals.insert(intervals.end(), optimals.begin(), optimals.end());
    std::vector<std::pair<float, float>> result;
    if (intervals.empty()) {
        result.push_back({rel_min.value(), rel_max.value()});
        return result;
    }
    // Step 1: Sort intervals by start
    std::vector<std::pair<float, float>> sorted = intervals;
    std::sort(sorted.begin(), sorted.end());

    // Step 2: Merge overlapping intervals
    std::vector<std::pair<float, float>> merged;
    float cur_start = sorted[0].first;
    float cur_end = sorted[0].second;
    for (size_t i = 1; i < sorted.size(); ++i) {
        if (sorted[i].first <= cur_end) {
            cur_end = std::max(cur_end, sorted[i].second);
        } else {
            merged.push_back({cur_start, cur_end});
            cur_start = sorted[i].first;
            cur_end = sorted[i].second;
        }
    }
    merged.push_back({cur_start, cur_end});

    // Step 3: Find gaps between merged intervals
    float prev_end = rel_min.value();
    for (const auto &interval : merged) {
        if (interval.first > prev_end) {
            result.push_back({prev_end, interval.first});
        }
        prev_end = std::max(prev_end, interval.second);
    }
    if (prev_end < rel_max.value()) {
        result.push_back({prev_end, rel_max.value()});
    }
    return result;
}

void Device::set_ui_thresholds() {
    using namespace ftxui;

    ui_thresholds = color(Color::Default);

    if (type == Type::Analog) {
        auto uncovered = find_uncovered_intervals();
        std::map<std::pair<float, float>, Color> intervals_to_color;
        auto add_to_map =
            [](std::map<std::pair<float, float>, Color> &intervals_to_color,
               const std::vector<std::pair<float, float>> &intervals,
               Color color) {
                for (auto &interval : intervals) {
                    intervals_to_color[interval] = color;
                }
            };
        add_to_map(intervals_to_color, warnings, Color::Red1);
        add_to_map(intervals_to_color, cautions, Color::Yellow1);
        add_to_map(intervals_to_color, optimals, Color::Green1);
        add_to_map(intervals_to_color, uncovered, Color::Default);
        LinearGradient lg = LinearGradient().Angle(270);
        auto set_stops = [this](LinearGradient &lg,
                                const std::map<std::pair<float, float>, Color>
                                    &intervals_to_color) {
            auto normalize = [this](float value) {
                return (value - rel_min.value()) /
                       (rel_max.value() - rel_min.value());
            };
            for (const auto &[interval, color] : intervals_to_color) {
                auto min = normalize(interval.first);
                auto max = normalize(interval.second);
                lg = lg.Stop(color, min).Stop(color, max);
            }
        };
        set_stops(lg, intervals_to_color);
        ui_thresholds = color(lg);
    }
}

void Devices::from_toml(std::vector<std::unique_ptr<Device>> &devices,
                        const std::string &toml_path) {
    try {
        auto config = toml::parse_file(toml_path);

        for (auto type : all_types) {
            for (auto modality : all_modalities) {
                std::string type_str = type_to_string(type);
                std::string modality_str = modality_to_string(modality);

                // Process devices of this type and modality
                if (config["Devices"][type_str][modality_str]
                        .is_array_of_tables()) {
                    for (auto &&device :
                         *config["Devices"][type_str][modality_str]
                              .as_array()) {
                        // Get the device table and initialize device
                        auto d_table = device.as_table();
                        std::string name =
                            d_table->get("name")->value<std::string>().value();
                        unsigned int pin =
                            d_table->get("pin")->value<unsigned int>().value();
                        auto &dev = devices.emplace_back(
                            std::make_unique<Device>(name, pin));

                        // Set modality
                        switch (modality) {
                        case Modality::In:
                            dev->to_in();
                            break;
                        case Modality::InOut:
                            dev->to_in_out();
                            break;
                        }

                        // Handle type-specific fields
                        switch (type) {
                        case Type::Digital: {
                            bool is_active_low = d_table->get("is_active_low")
                                                     ->value<bool>()
                                                     .value();
                            dev->to_digital(is_active_low);
                            break;
                        }
                        case Type::Analog: {
                            std::optional<std::string> units =
                                d_table->get("units")->value<std::string>();
                            unsigned int abs_min = d_table->get("abs_min")
                                                       ->value<unsigned int>()
                                                       .value();
                            unsigned int abs_max = d_table->get("abs_max")
                                                       ->value<unsigned int>()
                                                       .value();
                            float rel_min =
                                d_table->get("rel_min")->value<float>().value();
                            float rel_max =
                                d_table->get("rel_max")->value<float>().value();
                            dev->to_analog(units, abs_min, abs_max, rel_min,
                                           rel_max);
                            parse_pairs(d_table, "Warnings", dev->warnings);
                            parse_pairs(d_table, "Cautions", dev->cautions);
                            parse_pairs(d_table, "Optimals", dev->optimals);
                            break;
                        }
                        }
                        dev->set_ui_thresholds();
                    }
                }
            }
        }
    } catch (const toml::parse_error &err) {
        std::cerr << "Failed to parse TOML: " << err.what() << std::endl;
    } catch (const std::out_of_range &err) {
        std::cerr << "Failed to parse TOML: " << err.what() << std::endl;
    } catch (const std::bad_optional_access &err) {
        std::cerr << "Failed to parse TOML: " << err.what() << std::endl;
    }
}
