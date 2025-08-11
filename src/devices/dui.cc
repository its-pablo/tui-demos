// std library headers
#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

// 3rd party headers
// ---- ftxui ----
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>

// Local headers
#include "devices.h"
#include "dui.h"

using namespace ftxui;

ftxui::Decorator value_color(const Devices::Device &device, const float value) {
    using namespace Devices;
    using namespace ftxui;

    switch (device.type) {
    case Type::Analog: {
        float value_scaled =
            value * (device.rel_max.value() - device.rel_min.value()) +
            device.rel_min.value();
        std::string value_str = float_to_string(value_scaled);
        std::string units_abbreviation_str =
            device.units_abbreviation.has_value()
                ? " " + device.units_abbreviation.value()
                : "";

        if (device.is_warning(value_scaled)) {
            return color(Color::Red1);
        } else if (device.is_caution(value_scaled)) {
            return color(Color::Yellow1);
        } else if (device.is_optimal(value_scaled)) {
            return color(Color::Green1);
        } else {
            return color(Color::Default);
        }
    }
    case Type::Digital:
    default:
        return color(Color::Default);
    }
}

ftxui::Element value_text(const Devices::Device &device, const float value) {
    using namespace Devices;
    using namespace ftxui;

    switch (device.type) {
    case Type::Analog: {
        float value_scaled =
            value * (device.rel_max.value() - device.rel_min.value()) +
            device.rel_min.value();

        std::string value_str = float_to_string(value_scaled);
        std::string units_abbreviation_str =
            device.units_abbreviation.has_value()
                ? " " + device.units_abbreviation.value()
                : "";

        return hbox(text(value_str) | value_color(device, value),
                    text(units_abbreviation_str)) |
               bold;
    }
    case Type::Digital: {
        std::string value_str = device.is_active_low.value()
                                    ? (value == 0 ? "Active" : "Inactive")
                                    : (value == 0 ? "Inactive" : "Active");

        return text(value_str) | bold;
    }
    default:
        return text("N/A");
    }
}

Component Devices::Device::ui_detailed() const {
    return Renderer([this] {
        std::vector<Element> info;
        info.push_back(text(" Pin:        " + std::to_string(pin) + " "));
        info.push_back(
            text(" Type:       " + Devices::type_to_string(type) + " "));
        info.push_back(text(
            " Modality:   " + Devices::modality_to_string(modality) + " "));
        float value;
        Element y_axis_units;
        switch (type) {
        case Type::Analog: {
            info.push_back(text(
                " Units:      " + (units.has_value() ? units.value() : "N/A") +
                " "));
            info.push_back(
                text(" Absolute min: " +
                     (abs_min.has_value() ? std::to_string(abs_min.value())
                                          : "N/A") +
                     " "));
            info.push_back(
                text(" Absolute max: " +
                     (abs_max.has_value() ? std::to_string(abs_max.value())
                                          : "N/A") +
                     " "));
            info.push_back(
                text(" Relative min: " +
                     (rel_min.has_value() ? float_to_string(rel_min.value())
                                          : "N/A") +
                     " "));
            info.push_back(
                text(" Relative max: " +
                     (rel_max.has_value() ? float_to_string(rel_max.value())
                                          : "N/A") +
                     " "));
            for (const auto &warning : warnings) {
                info.push_back(text(" Warning: [" +
                                    float_to_string(warning.first) + ", " +
                                    float_to_string(warning.second) + "] "));
            }
            for (const auto &caution : cautions) {
                info.push_back(text(" Caution: [" +
                                    float_to_string(caution.first) + ", " +
                                    float_to_string(caution.second) + "] "));
            }
            for (const auto &optimal : optimals) {
                info.push_back(text(" Optimal: [" +
                                    float_to_string(optimal.first) + ", " +
                                    float_to_string(optimal.second) + "] "));
            }
            value = get_value_analog();
            std::string units = (units_abbreviation.has_value()
                                     ? " " + units_abbreviation.value()
                                     : "");
            std::string rel_min_str = float_to_string(rel_min.value()) + units;
            std::string rel_max_str = float_to_string(rel_max.value()) + units;
            std::string rel_half_str =
                float_to_string((rel_max.value() + rel_min.value()) / 2) +
                units;
            std::string rel_75_str =
                float_to_string(3 * (rel_max.value() + rel_min.value()) / 4) +
                units;
            std::string rel_25_str =
                float_to_string((rel_max.value() + rel_min.value()) / 4) +
                units;
            y_axis_units = vbox(text(rel_max_str), filler(), text(rel_75_str),
                                filler(), text(rel_half_str), filler(),
                                text(rel_25_str), filler(), text(rel_min_str));
            break;
        }
        case Type::Digital: {
            info.push_back(
                text(" Active Low: " +
                     (is_active_low.has_value()
                          ? (is_active_low.value() ? std::string("Yes")
                                                   : std::string("No"))
                          : "N/A") +
                     " "));
            value = get_value_digital();
            y_axis_units = vbox(text("High"), filler(), text("Low"));
            break;
        }
        }
        std::vector<std::vector<Element>> groupings;
        for (int i = 0; i < info.size(); i++) {
            if (i % 4 == 0) {
                std::vector<Element> grouping;
                groupings.push_back(grouping);
            }
            groupings.at(i / 4).push_back(info.at(i));
        }
        std::vector<Element> vboxes;
        for (auto &grouping : groupings) {
            vboxes.push_back(vbox(grouping));
            vboxes.push_back(separator());
        }
        vboxes.pop_back();
        auto graph_element =
            hbox({graph(ui_history) | color(Color::Default),
                  separatorHeavy() | ui_thresholds, y_axis_units}) |
            flex;
        return vbox({window(text(" Info ") | bold, hbox(vboxes)),
                     window(text(" History ") | bold, graph_element)}) |
               flex;
    });
}

Component Devices::Device::ui_overview() const {
    return Renderer([this](bool focused) {
        Element element;
        switch (type) {
        case Type::Analog: {
            float value = get_value_analog();
            std::string min_str = float_to_string(rel_min.value());
            std::string max_str = float_to_string(rel_max.value());
            element =
                window(text(" " + name + " "),
                       hbox({
                           hbox({text("Value: "), value_text(*this, value)}) |
                               size(WIDTH, EQUAL, 18),
                           separator(),
                           separator(),
                           text(min_str) | hcenter | size(WIDTH, EQUAL, 8),
                           separator(),
                           gauge(value) | value_color(*this, value),
                           separator(),
                           text(max_str) | hcenter | size(WIDTH, EQUAL, 8),
                       }));
            break;
        }
        case Type::Digital: {
            int value = get_value_digital();
            std::string state = is_active_low.value()
                                    ? (value == 0 ? "Active" : "Inactive")
                                    : (value == 0 ? "Inactive" : "Active");
            element =
                window(text(" " + name + " "),
                       hbox({
                           hbox({text("State: "), value_text(*this, value)}) |
                               size(WIDTH, EQUAL, 18),
                           separator(),
                           separator(),
                           text("Low") | hcenter | size(WIDTH, EQUAL, 8),
                           separator(),
                           gauge(value),
                           separator(),
                           text("High") | hcenter | size(WIDTH, EQUAL, 8),
                       }));
            break;
        }
        default: {
            element = window(text(" " + name + " "),
                             text("This device does not have a type."));
            break;
        }
        }
        return (focused) ? element | focus | inverted : element;
    });
}

Devices::UI::DetailsView::DetailsView(
    const std::vector<std::unique_ptr<Devices::Device>> &devices,
    std::mutex &hist_lock)
    : _devices(devices), _hist_lock(hist_lock) {
    for (auto &device : devices) {
        _menu_width = std::max(_menu_width,
                               static_cast<int>(device->get_name().length()));
        _tabs.push_back(device->get_name());
        _tab_views.push_back(device->ui_detailed());
    }
    _menu_width += 3;
    _menu_width = std::min(_menu_width, 50);
    _tab_toggle = Menu(&_tabs, &_tab_selected, MenuOption::Vertical());
    _tab_container = Container::Tab(_tab_views, &_tab_selected);
    _container = Container::Horizontal({
        _tab_toggle,
        _tab_container,
    });
    _renderer = Renderer(_container, [this] {
        const std::lock_guard<std::mutex> lg(_hist_lock);
        return hbox({
                   _tab_toggle->Render() | size(WIDTH, EQUAL, _menu_width) |
                       vscroll_indicator | yframe,
                   separator(),
                   _tab_container->Render(),
               }) |
               flex;
    });
}

Devices::UI::OverviewView::OverviewView(
    const std::vector<std::unique_ptr<Devices::Device>> &devices,
    std::mutex &hist_lock)
    : _devices(devices), _hist_lock(hist_lock) {
    for (const auto &device : _devices) {
        _device_views.push_back(device->ui_overview());
    }
    _container = Container::Vertical({_device_views}, &_device_selected);
    _renderer = Renderer(_container, [&] {
        const std::lock_guard<std::mutex> lg(_hist_lock);
        return _container->Render() | vscroll_indicator | yframe;
    });
}

Devices::UI::MainView::MainView(
    const std::vector<std::unique_ptr<Devices::Device>> &devices,
    std::mutex &hist_lock)
    : _overview_view(OverviewView(devices, hist_lock)),
      _details_view(DetailsView(devices, hist_lock)) {
    // Set up the main view components
    _tab_toggle = Toggle(&_tabs, &_tab_selected);
    _tab_container = Container::Tab(
        {
            _overview_view.get_renderer(),
            _details_view.get_renderer(),
            Renderer([] { return text("Schedule content") | center; }),
            Renderer([] { return text("Device config content") | center; }),
        },
        &_tab_selected);
    _container = Container::Vertical({
        _tab_toggle,
        _tab_container,
    });
    _renderer = Renderer(_container, [this] {
        return vbox({
                   _tab_toggle->Render(),
                   separator(),
                   _tab_container->Render(),
               }) |
               border;
    });
}

void Devices::UI::run(std::vector<std::unique_ptr<Devices::Device>> &devices) {
    std::mutex hist_lock;
    auto screen = ScreenInteractive::Fullscreen();
    screen.TrackMouse(false);
    std::atomic<bool> run = true;
    std::thread refresh_ui([&]() {
        while (run) {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(0.05s);
            screen.Post(Event::Custom);
        }
    });
    std::thread update_values([&]() mutable {
        while (run) {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(0.05s);
            for (auto &device : devices) {
                device->update_value();
            }
        }
    });
    std::thread record_to_hist([&]() {
        while (run) {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(0.25s);
            const std::lock_guard<std::mutex> lg(hist_lock);
            for (auto &device : devices) {
                device->record_value_to_hist();
            }
        }
    });
    auto catch_exit = ftxui::CatchEvent([&](Event event) {
        if (event == Event::Character('q')) {
            screen.ExitLoopClosure()();
            return true;
        }
        return false;
    });
    MainView main_view(devices, hist_lock);
    screen.Loop(main_view.get_renderer() | catch_exit);
    run = false;
    refresh_ui.join();
    update_values.join();
    record_to_hist.join();
}
