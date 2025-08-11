#pragma once

// Standard library headers
#include <mutex>
#include <vector>

// ---- ftxui ----
#include <ftxui/component/component.hpp>

// Local headers
#include "devices.h"

namespace Devices {

namespace UI {

using namespace ftxui;

class DetailsView {
  public:
    DetailsView(const std::vector<std::unique_ptr<Devices::Device>> &devices,
                std::mutex &hist_lock);
    Component get_renderer() { return _renderer; };

  private:
    const std::vector<std::unique_ptr<Devices::Device>> &_devices;
    Component _renderer;

    // Variables for the tab view
    int _tab_selected = 0;
    int _menu_width = 0;
    std::vector<std::string> _tabs;
    std::vector<Component> _tab_views;
    Component _tab_toggle;
    Component _tab_container;
    Component _container;

    // Devices history lock
    std::mutex &_hist_lock;
};

class OverviewView {
  public:
    OverviewView(const std::vector<std::unique_ptr<Devices::Device>> &devices,
                 std::mutex &hist_lock);
    Component get_renderer() { return _renderer; };

  private:
    const std::vector<std::unique_ptr<Devices::Device>> &_devices;
    Component _renderer;

    // Variable for focused devices
    int _device_selected = 0;
    std::vector<Component> _device_views;
    Component _container;

    // Devices history lock
    std::mutex &_hist_lock;
};

class MainView {
  public:
    MainView(const std::vector<std::unique_ptr<Devices::Device>> &devices,
             std::mutex &hist_lock);
    Component get_renderer() { return _renderer; };

  private:
    Component _renderer;

    // Variables for the tab view
    int _tab_selected = 0;
    const std::vector<std::string> _tabs = {" Overview ", " Devices ",
                                            " Schedule ", " Device Config "};
    Component _tab_toggle;
    Component _tab_container;
    Component _container;

    // Overview view
    OverviewView _overview_view;
    DetailsView _details_view;
};

void run(std::vector<std::unique_ptr<Devices::Device>> &devices);

} // namespace UI

} // namespace Devices
