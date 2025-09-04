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
    ~UI();
    
    // Prevent copying to avoid widget ownership issues
    UI(const UI&) = delete;
    UI& operator=(const UI&) = delete;
    
    void initialize(Config* config, CombinationTracker* tracker);
    void show_window();
    void hide_window();
    bool is_window_visible() const;
    
    void update();
    void update_aircraft_display(const std::string& aircraft_name);
    
private:
    Config* _config = nullptr;
    CombinationTracker* _tracker = nullptr;
    
    // Main window
    XPWidgetID _main_window = nullptr;
    
    // Status and info displays
    XPWidgetID _aircraft_label = nullptr;
    XPWidgetID _status_message_label = nullptr;
    XPWidgetID _current_combination_label = nullptr;
    
    // Binding list
    XPWidgetID _binding_list = nullptr;
    XPWidgetID _scroll_bar = nullptr;
    
    // Recording controls
    XPWidgetID _record_button = nullptr;
    XPWidgetID _clear_button = nullptr;
    XPWidgetID _command_input = nullptr;
    XPWidgetID _description_input = nullptr;
    XPWidgetID _save_button = nullptr;
    
    // Edit mode
    XPWidgetID _selection_input = nullptr;
    XPWidgetID _edit_button = nullptr;
    XPWidgetID _delete_button = nullptr;
    XPWidgetID _confirm_button = nullptr;
    XPWidgetID _cancel_button = nullptr;
    
    // State
    bool _recording = false;
    int _selected_binding_index = -1;
    int _scroll_position = 0;
    bool _confirming_delete = false;
    int _delete_candidate_index = -1;
    
    // Constants
    static constexpr int WINDOW_WIDTH = 600;
    static constexpr int WINDOW_HEIGHT = 500;
    static constexpr int MARGIN = 10;
    static constexpr int BUTTON_HEIGHT = 25;
    static constexpr int INPUT_HEIGHT = 25;
    static constexpr int LIST_ITEM_HEIGHT = 30;
    
    void create_widgets();
    void destroy_widgets();
    bool verify_widget_creation();
    void update_binding_list();
    void update_current_combination_display();
    void show_status_message(const std::string& message, bool is_error = false);
    void clear_status_message();
    void handle_record_button();
    void handle_clear_button();
    void handle_save_button();
    void handle_edit_button();
    void handle_delete_button();
    void handle_confirm_button();
    void handle_cancel_button();
    void handle_binding_selection(int index);
    void update_button_visibility();
    
    static int widget_callback(XPWidgetMessage message, XPWidgetID widget, intptr_t param1, intptr_t param2);
    int handle_widget_message(XPWidgetMessage message, XPWidgetID widget, intptr_t param1, intptr_t param2);
    
    std::string format_combination(const std::set<int>& combination) const;
    void refresh_ui();
};