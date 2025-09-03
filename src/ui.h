#pragma once

#include "XPWidgets.h"
#include "XPStandardWidgets.h"
#include "config.h"
#include "combination_tracker.h"

#include <vector>
#include <string>

class UI {
public:
    UI() = default;
    ~UI() = default;
    
    void initialize(Config* config, CombinationTracker* tracker);
    void show_window();
    void hide_window();
    bool is_window_visible() const;
    
    void update();
    
private:
    Config* config_ = nullptr;
    CombinationTracker* tracker_ = nullptr;
    
    // Main window
    XPWidgetID main_window_ = nullptr;
    
    // Binding list
    XPWidgetID binding_list_ = nullptr;
    XPWidgetID scroll_bar_ = nullptr;
    
    // Current combination display
    XPWidgetID current_combination_label_ = nullptr;
    
    // Recording controls
    XPWidgetID record_button_ = nullptr;
    XPWidgetID clear_button_ = nullptr;
    XPWidgetID command_input_ = nullptr;
    XPWidgetID description_input_ = nullptr;
    XPWidgetID save_button_ = nullptr;
    
    // Edit mode
    XPWidgetID edit_button_ = nullptr;
    XPWidgetID delete_button_ = nullptr;
    
    // State
    bool recording_ = false;
    int selected_binding_index_ = -1;
    int scroll_position_ = 0;
    
    // Constants
    static constexpr int WINDOW_WIDTH = 600;
    static constexpr int WINDOW_HEIGHT = 500;
    static constexpr int MARGIN = 10;
    static constexpr int BUTTON_HEIGHT = 25;
    static constexpr int INPUT_HEIGHT = 25;
    static constexpr int LIST_ITEM_HEIGHT = 30;
    
    void create_widgets();
    void destroy_widgets();
    void update_binding_list();
    void update_current_combination_display();
    void handle_record_button();
    void handle_clear_button();
    void handle_save_button();
    void handle_edit_button();
    void handle_delete_button();
    void handle_binding_selection(int index);
    
    static int widget_callback(XPWidgetMessage message, XPWidgetID widget, intptr_t param1, intptr_t param2);
    int handle_widget_message(XPWidgetMessage message, XPWidgetID widget, intptr_t param1, intptr_t param2);
    
    std::string format_combination(const std::set<int>& combination) const;
    void refresh_ui();
};