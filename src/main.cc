// std library headers
#include <atomic>
#include <cmath>
#include <ctime>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <thread>

// 3rd party headers
// ---- cxxopts ----
#include <cxxopts.hpp>
// ---- tomlplusplus ----
#include <toml++/toml.hpp>
// ---- ftxui ----
#include <ftxui/component/captured_mouse.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/mouse.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/screen/string.hpp>

// Local headers
#include "devices.h"
#include "dui.h"

// Example run: ./build_and_run.sh -h
void hello_world() { std::cout << "Hello, World!" << std::endl; }

// Example run: ./build_and_run.sh -t test_files/basic.toml
int toml_demo(const std::string &toml_path) {
    // Return signal
    int ret = 0;

    // TOML lib, by https://github.com/marzer/tomlplusplus
    // See API docs https://marzer.github.io/tomlplusplus/
    std::cout << "Parsing TOML from: " << toml_path << std::endl;

    try {
        auto config = toml::parse_file(toml_path);
        config.for_each([](auto &key, auto &value) {
            std::cout << "Found element with key = " << key
                      << ", and value =" << value << std::endl;
        });
    } catch (const toml::parse_error &err) {
        std::cout << "Failed to parse TOML!" << std::endl << err << std::endl;
        ret = 0;
    }

    return ret;
}

// Example run: ./build_and_run.sh -d test_files/garden_config.toml
int devices_parser(const std::string &toml_path) {
    // Return signal
    int ret = 0;

    // TOML lib, by https://github.com/marzer/tomlplusplus
    // See API docs https://marzer.github.io/tomlplusplus/
    std::cout << "Parsing devices from TOML config: " << toml_path << std::endl;

    std::vector<std::unique_ptr<Devices::Device>> devices;
    Devices::from_toml(devices, toml_path);

    if (devices.size() == 0) {
        std::cout << "No devices found in the TOML file." << std::endl;
        return 1; // Return error if no devices found
    }

    std::cout << "Found " << devices.size() << " devices." << std::endl;

    for (auto &device : devices) {
        std::cout << device->info();
    }

    return ret;
}

// Example run: ./build_and_run.sh -f 0
void ftxui_demo() {
    // Demo as seen here https://github.com/ArthurSonzogni/ftxui-starter

    using namespace ftxui;

    auto summary = [&] {
        auto content = vbox({
            hbox({text(L"- done:   "), text(L"3") | bold}) |
                color(Color::Green),
            hbox({text(L"- active: "), text(L"2") | bold}) |
                color(Color::RedLight),
            hbox({text(L"- queue:  "), text(L"9") | bold}) | color(Color::Red),
        });
        return window(text(L" Summary "), content);
    };

    auto document = //
        vbox({
            hbox({
                summary(),
                summary(),
                summary() | flex,
            }),
            summary(),
            summary(),
        });

    // Limit the size of the document to 80 char.
    // document = document | size(WIDTH, LESS_THAN, 80);

    auto screen = Screen::Create(Dimension::Full(), Dimension::Fit(document));
    Render(screen, document);

    std::cout << screen.ToString() << '\0' << std::endl;
}

// Example run: ./build_and_run.sh -f 1
void ftxui_demo_renderer() {
    // Demo as seen here
    // https://github.com/ArthurSonzogni/FTXUI/blob/main/examples/component/renderer.cpp

    using namespace ftxui;
    auto screen = ScreenInteractive::Fullscreen();

    // A Renderer() is a component using a lambda function as a parameter to
    // render itself.

    // 1. Example of focusable renderer:
    auto renderer_focusable = Renderer([](bool focused) {
        if (focused) {
            return text("FOCUSABLE RENDERER()") | center | bold | border;
        } else {
            return text(" Focusable renderer() ") | center | border;
        }
    });

    // 2. Example of a non focusable renderer.
    auto renderer_non_focusable =
        Renderer([&] { return text("~~~~~ None Focusable renderer() ~~~~~"); });

    // 3. Renderer can wrap other components to redefine their Render()
    // function.
    auto button = Button("Wrapped quit button", screen.ExitLoopClosure());
    auto renderer_wrap = Renderer(button, [&] {
        if (button->Focused()) {
            return button->Render() | bold | color(Color::Red);
        } else {
            return button->Render();
        }
    });

    // Let's renderer everyone:
    screen.Loop(Container::Vertical({
        renderer_focusable,
        renderer_non_focusable,
        renderer_wrap,
    }));
}

ftxui::Component DummyWindowContent() {
    using namespace ftxui;

    class Impl : public ComponentBase {
      private:
        bool checked[3] = {false, false, false};
        float slider = 50.0f;

      public:
        Impl() {
            Add(Container::Vertical({
                Checkbox("Check me", &checked[0]),
                Checkbox("Check me", &checked[1]),
                Checkbox("Check me", &checked[2]),
                Slider("Slider", &slider, 0.0f, 100.0f),
            }));
        }
    };

    return Make<Impl>();
}

// Example run: ./build_and_run.sh -f 2
void ftxui_demo_window() {
    // Demo as seen here
    // https://github.com/ArthurSonzogni/FTXUI/blob/main/examples/component/window.cpp

    using namespace ftxui;

    int window_1_left = 20;
    int window_1_top = 5;
    int window_1_width = 40;
    int window_1_height = 20;

    auto window_1 = Window({
        .inner = DummyWindowContent(),
        .title = "First window",
        .left = &window_1_left,
        .top = &window_1_top,
        .width = &window_1_width,
        .height = &window_1_height,
    });

    auto window_2 = Window({
        .inner = DummyWindowContent(),
        .title = "My window",
        .left = 40,
        .top = 20,
    });

    auto window_3 = Window({
        .inner = DummyWindowContent(),
        .title = "My window",
        .left = 60,
        .top = 30,
    });

    auto window_4 = Window({
        .inner = DummyWindowContent(),
    });

    auto window_5 = Window({});

    auto window_container = Container::Stacked({
        window_1,
        window_2,
        window_3,
        window_4,
        window_5,
    });

    auto display_win_1 = Renderer([&] {
        return text("window_1: " + std::to_string(window_1_width) + "x" +
                    std::to_string(window_1_height) + " + " +
                    std::to_string(window_1_left) + "," +
                    std::to_string(window_1_top));
    });

    auto layout = Container::Vertical({
        display_win_1,
        window_container,
    });

    auto screen = ScreenInteractive::Fullscreen();
    screen.Loop(layout);
}

ftxui::Component MainComponent(std::function<void()> show_modal,
                               std::function<void()> exit) {
    using namespace ftxui;

    auto button_style = ButtonOption::Animated();

    auto component = Container::Vertical({
        Button("Show modal", show_modal, button_style),
        Button("Quit", exit, button_style),
    });

    component |= Renderer([&](Element inner) {
        return vbox({
                   text("Main component"),
                   separator(),
                   inner,
               }) |
               size(WIDTH, GREATER_THAN, 15) | size(HEIGHT, GREATER_THAN, 15) |
               border | center;
    });

    return component;
}

ftxui::Component ModalComponent(std::function<void()> do_nothing,
                                std::function<void()> hide_modal) {
    using namespace ftxui;

    auto component = Container::Vertical({
        Button("Do nothing", do_nothing),
        Button("Quit modal", hide_modal),
    });

    component |= Renderer([&](Element inner) {
        return vbox({
                   text("Modal component"),
                   separator(),
                   inner,
               }) |
               size(WIDTH, GREATER_THAN, 30) | border;
    });

    return component;
}

// Example run: ./build_and_run.sh -f 3
void ftxui_demo_dialog() {
    // Demo as seen here
    // https://github.com/ArthurSonzogni/FTXUI/blob/main/examples/component/modal_dialog.cpp

    using namespace ftxui;

    auto screen = ScreenInteractive::TerminalOutput();

    // State of the application
    bool modal_shown = false;

    // Show actions modifying the state
    auto show_modal = [&] { modal_shown = true; };
    auto hide_modal = [&] { modal_shown = false; };
    auto exit = screen.ExitLoopClosure();
    auto do_nothing = [&] {};

    // Instantiate the main and modal components
    auto main_component = MainComponent(show_modal, exit);
    auto modal_component = ModalComponent(do_nothing, hide_modal);

    // Use the 'Modal' function to use together the main component and its modal
    // window. The |modal_show| boolean controls twhether the modal is shown or
    // not.
    main_component |= Modal(modal_component, &modal_shown);

    screen.Loop(main_component);
}

// Example run: ./build_and_run.sh -f 5
void ftxui_demo_menu() {
    // Demo as seen here
    // https://github.com/ArthurSonzogni/FTXUI/blob/main/examples/component/menu_in_frame_horizontal.cpp

    using namespace ftxui;

    std::vector<std::string> entries;
    int selected = 0;

    for (int i = 0; i < 10; ++i) {
        entries.push_back(std::to_string(i));
    }

    auto radiobox = Menu(&entries, &selected, MenuOption::Horizontal());
    auto renderer = Renderer(radiobox, [&] {
        return radiobox->Render() | hscroll_indicator | frame;
    });

    auto screen = ScreenInteractive::FitComponent();
    screen.Loop(renderer);
}

// Example run: ./build_and_run.sh -f 6
void ftxui_demo_split() {
    // Demo as seen here
    // https://github.com/ArthurSonzogni/FTXUI/blob/main/examples/component/resizable_split.cpp

    using namespace ftxui;

    auto screen = ScreenInteractive::Fullscreen();

    auto middle = Renderer([] { return text("Middle") | center; });
    auto left = Renderer([] { return text("Left") | center; });
    auto right = Renderer([] { return text("Right") | center; });
    auto top = Renderer([] { return text("Top") | center; });
    auto bottom = Renderer([] { return text("Bottom") | center; });

    int left_size = 20;
    int right_size = 20;
    int top_size = 10;
    int bottom_size = 10;

    auto container = middle;
    container = ResizableSplitLeft(left, container, &left_size);
    container = ResizableSplitRight(right, container, &right_size);
    container = ResizableSplitTop(top, container, &top_size);
    container = ResizableSplitBottom(bottom, container, &bottom_size);

    auto renderer =
        Renderer(container, [&] { return container->Render() | border; });

    screen.Loop(renderer);
}

// Example run: ./build_and_run.sh -f 7
void ftxui_demo_tabs() {
    // Demo as seen here
    // https://github.com/ArthurSonzogni/FTXUI/blob/main/examples/component/tab_horizontal.cpp

    using namespace ftxui;

    std::vector<std::string> tabs = {"Overview", "Schedule", "Settings"};

    int tab_selected = 0;
    auto tab_toggle = Toggle(&tabs, &tab_selected);
    auto tab_container = Container::Tab(
        {
            Renderer([] { return text("Overview content") | center; }),
            Renderer([] { return text("Schedule content") | center; }),
            Renderer([] { return text("Settings content") | center; }),
        },
        &tab_selected);

    auto container = Container::Vertical({
        tab_toggle,
        tab_container,
    });

    auto renderer = Renderer(container, [&] {
        return vbox({
                   tab_toggle->Render(),
                   separator(),
                   tab_container->Render(),
               }) |
               border;
    });

    auto screen = ScreenInteractive::Fullscreen();
    screen.Loop(renderer);
}

// Example run: ./build_and_run.sh -f 8
void ftxui_demo_toggle() {
    // Demo as seen here
    // https://github.com/ArthurSonzogni/FTXUI/blob/main/examples/component/toggle.cpp

    using namespace ftxui;

    std::vector<std::string> t1_entries = {"On", "Off"};
    std::vector<std::string> t2_entries = {"Enabled", "Disabled"};
    std::vector<std::string> t3_entries = {"$10", "$0"};
    std::vector<std::string> t4_entries = {"Nothing", "One element",
                                           "Several elements"};

    int t1_selected = 0;
    int t2_selected = 0;
    int t3_selected = 0;
    int t4_selected = 0;

    Component t1 = Toggle(&t1_entries, &t1_selected);
    Component t2 = Toggle(&t2_entries, &t2_selected);
    Component t3 = Toggle(&t3_entries, &t3_selected);
    Component t4 = Toggle(&t4_entries, &t4_selected);

    auto container = Container::Vertical({t1, t2, t3, t4});

    auto renderer = Renderer(container, [&] {
        return vbox({
                   text("Toggle components:"),
                   separator(),
                   hbox(text(" * Poweroff on startup       : "), t1->Render()),
                   hbox(text(" * Out of process            : "), t2->Render()),
                   hbox(text(" * Price of the information  : "), t3->Render()),
                   hbox(text(" * Number of elements        : "), t4->Render()),
               }) |
               border;
    });

    auto screen = ScreenInteractive::TerminalOutput();
    screen.Loop(renderer);
}

// Example run: ./build_and_run.sh -f 9
void ftxui_demo_graph() {
    // Demo as seen here (HTOP)
    // https://github.com/ArthurSonzogni/FTXUI/blob/main/examples/component/homescreen.cpp

    using namespace ftxui;

    int shift = 0;
    auto my_graph = [&shift](int width, int height) {
        std::vector<int> output(width);
        for (int i = 0; i < width; ++i) {
            float v = 0.5f;
            v += 0.1f * std::sin((i + shift) * 0.1f);
            v += 0.2f * std::sin((i + shift + 10) * 0.15f);
            v += 0.1f * std::sin((i + shift) * 0.03f);
            v *= height;
            output[i] = (int)v;
        }
        return output;
    };

    auto utilization = hbox({
                           vbox({
                               text("100 "),
                               filler(),
                               text("50 "),
                               filler(),
                               text("0 "),
                           }),
                           separator(),
                           graph(std::ref(my_graph)) | color(Color::Blue),
                       }) |
                       flex;

    auto renderer = Renderer([&utilization] {
        return window(text(L" Utilization [%] "), utilization);
    });
    auto screen = ScreenInteractive::Fullscreen();

    std::atomic<bool> refresh_ui_continue = true;
    std::thread refresh_ui([&] {
        while (refresh_ui_continue) {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(0.05s);
            screen.Post([&] { shift++; });
            screen.Post(Event::Custom);
        }
    });

    screen.Loop(renderer);
    refresh_ui_continue = false;
    refresh_ui.join();
}

ftxui::Component DummyWindowContentWithImpl() {
    using namespace ftxui;

    class Impl : public ComponentBase {
      private:
        float scroll_x = 0.1f;
        float scroll_y = 0.1f;

      public:
        Impl() {
            auto content = Renderer([=] {
                const std::string lorem =
                    "Lorem ipsum dolor sit amet, consectetur adipiscing elit, "
                    "sed "
                    "do eiusmod tempor incididunt ut labore et dolore magna "
                    "aliqua. Ut enim ad minim veniam, quis nostrud "
                    "exercitation "
                    "ullamco laboris nisi ut aliquip ex ea commodo consequat. "
                    "Duis "
                    "aute irure dolor in reprehenderit in voluptate velit esse "
                    "cillum dolore eu fugiat nulla pariatur. Excepteur sint "
                    "occaecat cupidatat non proident, sunt in culpa qui "
                    "officia "
                    "deserunt mollit anim id est laborum.";
                return vbox({
                    text(lorem.substr(0, -1)),
                    text(lorem.substr(5, -1)),
                    text(""),
                    text(lorem.substr(10, -1)),
                    text(lorem.substr(15, -1)),
                    text(""),
                    text(lorem.substr(20, -1)),
                    text(lorem.substr(25, -1)),
                    text(""),
                    text(lorem.substr(30, -1)),
                    text(lorem.substr(35, -1)),
                    text(""),
                    text(lorem.substr(40, -1)),
                    text(lorem.substr(45, -1)),
                    text(""),
                    text(lorem.substr(50, -1)),
                    text(lorem.substr(55, -1)),
                    text(""),
                    text(lorem.substr(60, -1)),
                    text(lorem.substr(65, -1)),
                    text(""),
                    text(lorem.substr(70, -1)),
                    text(lorem.substr(75, -1)),
                    text(""),
                    text(lorem.substr(80, -1)),
                    text(lorem.substr(85, -1)),
                    text(""),
                    text(lorem.substr(90, -1)),
                    text(lorem.substr(95, -1)),
                    text(""),
                    text(lorem.substr(100, -1)),
                    text(lorem.substr(105, -1)),
                    text(""),
                    text(lorem.substr(110, -1)),
                    text(lorem.substr(115, -1)),
                    text(""),
                    text(lorem.substr(120, -1)),
                    text(lorem.substr(125, -1)),
                    text(""),
                    text(lorem.substr(130, -1)),
                    text(lorem.substr(135, -1)),
                    text(""),
                    text(lorem.substr(140, -1)),
                });
            });

            auto scrollable_content = Renderer(content, [&, content] {
                return content->Render() |
                       focusPositionRelative(scroll_x, scroll_y) | frame | flex;
            });

            SliderOption<float> option_x;
            option_x.value = &scroll_x;
            option_x.min = 0.0f;
            option_x.max = 1.0f;
            option_x.increment = 0.01f;
            option_x.direction = Direction::Right;
            option_x.color_active = Color::Blue;
            option_x.color_inactive = Color::BlueLight;
            auto scrollbar_x = Slider(option_x);

            SliderOption<float> option_y;
            option_y.value = &scroll_y;
            option_y.min = 0.0f;
            option_y.max = 1.0f;
            option_y.increment = 0.01f;
            option_y.direction = Direction::Down;
            option_y.color_active = Color::Yellow;
            option_y.color_inactive = Color::YellowLight;
            auto scrollbar_y = Slider(option_y);

            Add(Container::Vertical({
                Container::Horizontal({
                    scrollable_content,
                    scrollbar_y,
                }) | flex,
                Container::Horizontal({
                    scrollbar_x,
                    Renderer([] { return text(L"x"); }),
                }),
            }));
        };
    };
    return Make<Impl>();
}

// Example run: ./build_and_run.sh -f 10
void ftxui_demo_scrollable() {
    // Demo as seen here (HTOP)
    // https://github.com/ArthurSonzogni/FTXUI/blob/main/examples/component/scrollbar.cpp

    using namespace ftxui;

    auto window_1 = Window({
        .inner = DummyWindowContentWithImpl(),
        .title = "First window",
        .width = 80,
        .height = 30,
    });

    auto window_2 = Window({
        .inner = DummyWindowContentWithImpl(),
        .title = "Second window",
        .left = 40,
        .top = 20,
        .width = 80,
        .height = 30,
    });

    auto window_container = Container::Stacked({
        window_1,
        window_2,
    });

    auto screen = ScreenInteractive::Fullscreen();
    screen.Loop(window_container);
}

// Example run: ./build_and_run.sh -f 11
void ftxui_demo_gradient() {
    // Demo as seen here
    // https://github.com/ArthurSonzogni/FTXUI/blob/main/examples/component/linear_gradient_gallery.cpp

    using namespace ftxui;

    auto screen = ScreenInteractive::Fullscreen();

    float angle = 180.0f;
    float start = 0.0f;
    float end = 1.0f;

    std::string slider_angle_text;
    std::string slider_start_text;
    std::string slider_end_text;

    auto slider_angle = Slider(&slider_angle_text, &angle, 0.0f, 360.0f);
    auto slider_start = Slider(&slider_start_text, &start, 0.0f, 1.0f, 0.05f);
    auto slider_end = Slider(&slider_end_text, &end, 0.0f, 1.0f, 0.05f);

    auto layout = Container::Vertical({
        slider_angle,
        slider_start,
        slider_end,
    });

    auto renderer = Renderer(layout, [&] {
        slider_angle_text = "angle = " + std::to_string(angle) + "°";
        slider_start_text = "start = " + std::to_string(int(start * 100)) + "%";
        slider_end_text = "end   = " + std::to_string(int(end * 100)) + "%";

        auto background = text("Gradient") | center |
                          bgcolor(LinearGradient()
                                      .Angle(angle)
                                      .Stop(Color::Blue, start)
                                      .Stop(Color::Red, end));
        return vbox({
                   background | flex,
                   layout->Render(),
               }) |
               flex;
    });

    screen.Loop(renderer);
}

// Example run: ./build_and_run.sh -f 12
void ftxui_demo_static_gradient() {
    using namespace ftxui;

    auto screen = ScreenInteractive::Fullscreen();

    auto renderer = Renderer([&] {
        auto background = text("Gradient") | center |
                          bgcolor(LinearGradient()
                                      .Angle(90)
                                      .Stop(Color::Red1, 0.00)
                                      .Stop(Color::Red1, 0.05)
                                      .Stop(Color::Yellow1, 0.05)
                                      .Stop(Color::Yellow1, 0.15)
                                      .Stop(Color::Default, 0.15)
                                      .Stop(Color::Default, 0.47)
                                      .Stop(Color::Green1, 0.47)
                                      .Stop(Color::Green1, 0.53)
                                      .Stop(Color::Default, 0.53)
                                      .Stop(Color::Default, 0.85)
                                      .Stop(Color::Yellow1, 0.85)
                                      .Stop(Color::Yellow1, 0.95)
                                      .Stop(Color::Red1, 0.95)
                                      .Stop(Color::Red1, 1.0));
        return background | flex;
    });

    screen.Loop(renderer);
}

// Example run: ./build_and_run.sh -f 13
void ftxui_demo_maybe() {
    // Demo as seen here:
    // https://github.com/ArthurSonzogni/FTXUI/blob/main/examples/component/maybe.cpp

    using namespace ftxui;

    std::vector<std::string> entries = {
        "entry 1",
        "entry 2",
        "entry 3",
    };
    int menu_1_selected = 0;
    int menu_2_selected = 0;

    bool menu_1_show = false;
    bool menu_2_show = false;

    auto layout = Container::Vertical({
        Checkbox("Show menu_1", &menu_1_show),
        Radiobox(&entries, &menu_1_selected) | border | Maybe(&menu_1_show),
        Checkbox("Show menu_2", &menu_2_show),
        Radiobox(&entries, &menu_2_selected) | border | Maybe(&menu_2_show),

        Renderer([] {
            return text("You found the secret combinaison!") |
                   color(Color::Red);
        }) |
            Maybe([&] { return menu_1_selected == 1 && menu_2_selected == 2; }),
    });

    auto screen = ScreenInteractive::TerminalOutput();
    screen.Loop(layout);
}

int main(int argc, char *argv[]) {
    // Argument parser, by https://github.com/jarro2783/cxxopts
    cxxopts::Options options("C++ tests!",
                             "A simple command line tool I can add a bunch of "
                             "simple examples to as I learn more about C++.");
    options.add_options()("w,world", "Hello World!", cxxopts::value<bool>())(
        "t,toml", "TOML parse example, takes TOML file path as input.",
        cxxopts::value<std::string>())("d,devices",
                                       "Parse a device config TOML file.",
                                       cxxopts::value<std::string>())(
        "f,ftxui", "Sample ftxui usage.", cxxopts::value<int>())(
        "u,ui", "Run device UI.", cxxopts::value<std::string>());
    auto result = options.parse(argc, argv);

    // Return signal, by default assume happy 0
    int ret = 0;

    // Handle hello world option
    if (result.count("world") > 0) {
        hello_world();
    }

    // Handle toml option
    if (result.count("toml") > 0) {
        std::string toml_file = result["toml"].as<std::string>();
        ret = toml_demo(toml_file);
    }

    // Handle devices config TOML
    if (result.count("devices") > 0) {
        std::string toml_file = result["devices"].as<std::string>();
        ret = devices_parser(toml_file);
    }

    // Handle ftxui option
    if (result.count("ftxui") > 0) {
        switch (result["ftxui"].as<int>()) {
        case 0:
            ftxui_demo();
            break;
        case 1:
            ftxui_demo_renderer();
            break;
        case 2:
            ftxui_demo_window();
            break;
        case 3:
            ftxui_demo_dialog();
            break;
        case 4: {
            // Example run: ./build_and_run.sh -f 4
            // Demo as seen here (adapted to be vertical)
            // https://github.com/ArthurSonzogni/FTXUI/blob/main/examples/component/slider.cpp
            using namespace ftxui;

            auto screen = ScreenInteractive::Fullscreen();
            int value = 50;
            auto slider_options =
                SliderOption<int>({.value = &value,
                                   .min = 0,
                                   .max = 100,
                                   .increment = 1,
                                   .direction = Direction::Up});
            auto slider = Slider(slider_options);
            screen.Loop(slider);
            break;
        }
        case 5:
            ftxui_demo_menu();
            break;
        case 6:
            ftxui_demo_split();
            break;
        case 7:
            ftxui_demo_tabs();
            break;
        case 8:
            ftxui_demo_toggle();
            break;
        case 9:
            ftxui_demo_graph();
            break;
        case 10:
            ftxui_demo_scrollable();
            break;
        case 11:
            ftxui_demo_gradient();
            break;
        case 12:
            ftxui_demo_static_gradient();
            break;
        case 13:
            ftxui_demo_maybe();
            break;
        default:
            std::cout << "Unknown ftxui demo option, skipping." << std::endl;
            break;
        }
    }

    // Handle device UI option
    if (result.count("ui") > 0) {
        std::srand(std::time(0));
        std::string toml_file = result["ui"].as<std::string>();
        std::vector<std::unique_ptr<Devices::Device>> devices;
        Devices::from_toml(devices, toml_file);
        if (devices.size() > 0) {
            Devices::UI::run(devices);
        } else {
            std::cout << "No devices found in the TOML file." << std::endl;
            ret = 1; // Return error if no devices found
        }
    }

    return ret;
}
