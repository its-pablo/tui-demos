#pragma once

// std library headers
#include <algorithm>
#include <array>
#include <atomic>
#include <cstdlib>
#include <iomanip>
#include <memory>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

// 3rd library headers
// ---- ftxui ----
#include <ftxui/component/component.hpp>

inline std::string float_to_string(float value) {
    std::stringstream stream;
    stream << std::fixed << std::setprecision(2) << value;
    return stream.str();
}

namespace Devices {

enum class Type { Analog, Digital };

const std::array<Type, 2> all_types = {Type::Analog, Type::Digital};

inline std::string type_to_string(Type type) {
    switch (type) {
    case Type::Analog:
        return "Analog";
    case Type::Digital:
        return "Digital";
    default:
        return "Unknown";
    }
}

enum class Modality { In, InOut };

const std::array<Modality, 2> all_modalities = {Modality::In, Modality::InOut};

inline std::string modality_to_string(Modality modality) {
    switch (modality) {
    case Modality::In:
        return "In";
    case Modality::InOut:
        return "InOut";
    default:
        return "Unknown";
    }
}

class Device {

  public:
    static const int interval_s = 1;
    static const int hist_size = 200;

    std::string name;
    unsigned int pin;
    Type type;
    Modality modality;
    // Optional fields for digital devices
    std::optional<bool> is_active_low;
    // Optional fields for analog devices
    std::optional<std::string> units;
    std::optional<std::string> units_abbreviation;
    std::optional<unsigned int> abs_min;
    std::optional<unsigned int> abs_max;
    std::optional<float> rel_min;
    std::optional<float> rel_max;
    std::vector<std::pair<float, float>> warnings;
    std::vector<std::pair<float, float>> cautions;
    std::vector<std::pair<float, float>> optimals;
    // TUI elements
    ftxui::GraphFunction ui_history;
    ftxui::Decorator ui_thresholds;

    Device(std::string name, unsigned int pin)
        : name(name), pin(pin), ui_history([this](int width, int height) {
              return this->get_value_transform(width, height);
          }){};

    void clear_optionals() {
        is_active_low.reset();
        units.reset();
        units_abbreviation.reset();
        abs_min.reset();
        abs_max.reset();
        rel_min.reset();
        rel_max.reset();
        warnings.clear();
        cautions.clear();
        optimals.clear();
        _value_analog.store(0.0f);
        _value_digital.store(0);
    }

    std::string info() const;

    void to_in() { modality = Modality::In; }

    void to_in_out() { modality = Modality::InOut; }

    void to_digital(bool is_active_low = false) {
        clear_optionals();
        type = Type::Digital;
        this->is_active_low = is_active_low;
        _value_digital.store(std::rand() % 2);
    }

    void to_analog(std::optional<std::string> units, unsigned int abs_min,
                   unsigned int abs_max, float rel_min, float rel_max) {
        clear_optionals();
        type = Type::Analog;
        this->units = units;
        if (units.has_value()) {
            if (units.value().find(",") != std::string::npos) {
                this->units_abbreviation =
                    units.value().substr(units.value().find(",") + 1);
            }
        }
        this->abs_min = abs_min;
        this->abs_max = abs_max;
        this->rel_min = rel_min;
        this->rel_max = rel_max;
        _value_analog.store(static_cast<float>(std::rand()) / RAND_MAX);
    }

    ftxui::Component ui_detailed() const;
    ftxui::Component ui_overview() const;
    void set_ui_thresholds();

    // Const getters
    std::string get_name() const { return name; }
    float get_value_analog() const { return _value_analog.load(); }
    unsigned int get_value_digital() const { return _value_digital.load(); }
    std::array<float, hist_size> get_value_analog_hist() const {
        return _value_analog_hist;
    }
    std::array<int, hist_size> get_value_digital_hist() const {
        return _value_digital_hist;
    }
    std::vector<int> get_value_transform(int width, int height) const;
    bool is_warning(float value) const;
    bool is_caution(float value) const;
    bool is_optimal(float value) const;

    void update_value();
    void record_value_to_hist();

  private:
    // Modifiable values
    std::atomic<float> _value_analog;
    std::atomic<int> _value_digital;

    // History of 100 values
    std::array<float, hist_size> _value_analog_hist{0.0f};
    std::array<int, hist_size> _value_digital_hist{0};
    std::vector<int> get_value_analog_transform(int width, int height) const;
    std::vector<int> get_value_digital_transform(int width, int height) const;
    std::vector<std::pair<float, float>> find_uncovered_intervals();
};

void from_toml(std::vector<std::unique_ptr<Device>> &devices,
               const std::string &toml_path);

} // namespace Devices
