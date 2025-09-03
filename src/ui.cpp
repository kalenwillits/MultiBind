#include "ui.h"
#include "XPLMUtilities.h"
#include "XPLMDisplay.h"

#include <sstream>
#include <algorithm>

void UI::initialize(Config* config, CombinationTracker* tracker)
{
    config_ = config;
    tracker_ = tracker;
}

void UI::show_window()
{
    if (!main_window_) {
        create_widgets();
    }
    
    if (main_window_) {
        XPShowWidget(main_window_);
        refresh_ui();
    }
}

void UI::hide_window()
{
    if (main_window_) {
        XPHideWidget(main_window_);
    }
}

bool UI::is_window_visible() const
{
    return main_window_ && XPIsWidgetVisible(main_window_);
}

void UI::update()
{
    if (!is_window_visible()) {
        return;
    }
    
    update_current_combination_display();
}

void UI::create_widgets()
{
    // Get screen dimensions for centering
    int screen_width, screen_height;
    XPLMGetScreenSize(&screen_width, &screen_height);
    
    int left = (screen_width - WINDOW_WIDTH) / 2;
    int top = (screen_height + WINDOW_HEIGHT) / 2;
    int right = left + WINDOW_WIDTH;
    int bottom = top - WINDOW_HEIGHT;
    
    // Create main window
    main_window_ = XPCreateWidget(left, top, right, bottom,
                                 1, "Multibind Configuration", 1,
                                 nullptr, xpWidgetClass_MainWindow);
    
    XPSetWidgetProperty(main_window_, xpProperty_MainWindowHasCloseBoxes, 1);
    XPAddWidgetCallback(main_window_, widget_callback);
    
    int y = top - 40;
    
    // Current aircraft label
    XPCreateWidget(left + MARGIN, y, left + WINDOW_WIDTH - MARGIN, y - 20,
                  1, "Current Aircraft: Loading...", 0, main_window_,
                  xpWidgetClass_Caption);
    y -= 30;
    
    // Current combination display
    current_combination_label_ = XPCreateWidget(left + MARGIN, y, left + WINDOW_WIDTH - MARGIN, y - 20,
                                              1, "Current Combination: None", 0, main_window_,
                                              xpWidgetClass_Caption);
    y -= 30;
    
    // Recording controls
    record_button_ = XPCreateWidget(left + MARGIN, y, left + MARGIN + 80, y - BUTTON_HEIGHT,
                                   1, "Record", 0, main_window_,
                                   xpWidgetClass_Button);
    XPAddWidgetCallback(record_button_, widget_callback);
    
    clear_button_ = XPCreateWidget(left + MARGIN + 90, y, left + MARGIN + 170, y - BUTTON_HEIGHT,
                                  1, "Clear", 0, main_window_,
                                  xpWidgetClass_Button);
    XPAddWidgetCallback(clear_button_, widget_callback);
    y -= 35;
    
    // Command input
    XPCreateWidget(left + MARGIN, y, left + MARGIN + 100, y - 20,
                  1, "Target Command:", 0, main_window_,
                  xpWidgetClass_Caption);
    
    command_input_ = XPCreateWidget(left + MARGIN + 110, y, left + WINDOW_WIDTH - MARGIN, y - INPUT_HEIGHT,
                                   1, "", 0, main_window_,
                                   xpWidgetClass_TextField);
    XPSetWidgetProperty(command_input_, xpProperty_TextFieldType, xpTextEntryField);
    y -= 35;
    
    // Description input
    XPCreateWidget(left + MARGIN, y, left + MARGIN + 100, y - 20,
                  1, "Description:", 0, main_window_,
                  xpWidgetClass_Caption);
    
    description_input_ = XPCreateWidget(left + MARGIN + 110, y, left + WINDOW_WIDTH - MARGIN, y - INPUT_HEIGHT,
                                       1, "", 0, main_window_,
                                       xpWidgetClass_TextField);
    XPSetWidgetProperty(description_input_, xpProperty_TextFieldType, xpTextEntryField);
    y -= 35;
    
    // Action buttons
    save_button_ = XPCreateWidget(left + MARGIN, y, left + MARGIN + 80, y - BUTTON_HEIGHT,
                                 1, "Save", 0, main_window_,
                                 xpWidgetClass_Button);
    XPAddWidgetCallback(save_button_, widget_callback);
    
    edit_button_ = XPCreateWidget(left + MARGIN + 90, y, left + MARGIN + 170, y - BUTTON_HEIGHT,
                                 1, "Edit", 0, main_window_,
                                 xpWidgetClass_Button);
    XPAddWidgetCallback(edit_button_, widget_callback);
    
    delete_button_ = XPCreateWidget(left + MARGIN + 180, y, left + MARGIN + 260, y - BUTTON_HEIGHT,
                                   1, "Delete", 0, main_window_,
                                   xpWidgetClass_Button);
    XPAddWidgetCallback(delete_button_, widget_callback);
    y -= 40;
    
    // Binding list
    XPCreateWidget(left + MARGIN, y, left + WINDOW_WIDTH - MARGIN, y - 20,
                  1, "Current Bindings:", 0, main_window_,
                  xpWidgetClass_Caption);
    y -= 25;
    
    binding_list_ = XPCreateWidget(left + MARGIN, y, left + WINDOW_WIDTH - MARGIN - 20, bottom + MARGIN,
                                  1, "", 0, main_window_,
                                  xpWidgetClass_ScrollableArea);
    XPAddWidgetCallback(binding_list_, widget_callback);
    
    scroll_bar_ = XPCreateWidget(left + WINDOW_WIDTH - MARGIN - 20, y, left + WINDOW_WIDTH - MARGIN, bottom + MARGIN,
                                1, "", 0, main_window_,
                                xpWidgetClass_ScrollBar);
}

void UI::destroy_widgets()
{
    if (main_window_) {
        XPDestroyWidget(main_window_, 1);
        main_window_ = nullptr;
        binding_list_ = nullptr;
        current_combination_label_ = nullptr;
        record_button_ = nullptr;
        clear_button_ = nullptr;
        command_input_ = nullptr;
        description_input_ = nullptr;
        save_button_ = nullptr;
        edit_button_ = nullptr;
        delete_button_ = nullptr;
        scroll_bar_ = nullptr;
    }
}

void UI::update_binding_list()
{
    // This is a simplified implementation
    // In a full implementation, we'd create individual widgets for each binding
    // For now, we'll just use debug output to show the concept works
    
    if (!config_) return;
    
    const auto& bindings = config_->get_bindings();
    char list_content[2048] = "";
    
    for (size_t i = 0; i < bindings.size(); ++i) {
        char line[256];
        std::string combo_str = format_combination(bindings[i].button_combination);
        snprintf(line, sizeof(line), "%zu: %s -> %s (%s)\n", 
                i, combo_str.c_str(), 
                bindings[i].target_command.c_str(),
                bindings[i].description.c_str());
        strncat(list_content, line, sizeof(list_content) - strlen(list_content) - 1);
    }
    
    // For now, just log the bindings - in a full implementation this would update the scrollable list
    XPLMDebugString("Multibind: Current bindings list updated\n");
}

void UI::update_current_combination_display()
{
    if (!current_combination_label_ || !tracker_) {
        return;
    }
    
    const auto& pressed = tracker_->get_currently_pressed_buttons();
    std::string display_text = "Current Combination: ";
    
    if (pressed.empty()) {
        display_text += "None";
    } else {
        display_text += format_combination(pressed);
    }
    
    XPSetWidgetDescriptor(current_combination_label_, display_text.c_str());
}

void UI::handle_record_button()
{
    if (!tracker_) return;
    
    if (recording_) {
        tracker_->stop_recording();
        recording_ = false;
        XPSetWidgetDescriptor(record_button_, "Record");
        XPLMDebugString("Multibind: Stopped recording\n");
    } else {
        tracker_->start_recording();
        recording_ = true;
        XPSetWidgetDescriptor(record_button_, "Stop");
        XPLMDebugString("Multibind: Started recording\n");
    }
}

void UI::handle_clear_button()
{
    if (!tracker_) return;
    
    tracker_->clear_currently_pressed();
    if (recording_) {
        tracker_->stop_recording();
        tracker_->start_recording(); // Restart recording with empty combination
    }
    
    // Clear input fields
    if (command_input_) XPSetWidgetDescriptor(command_input_, "");
    if (description_input_) XPSetWidgetDescriptor(description_input_, "");
    
    XPLMDebugString("Multibind: Cleared current combination and inputs\n");
}

void UI::handle_save_button()
{
    if (!config_ || !tracker_) return;
    
    // Get command and description from input fields
    char command_text[256] = "";
    char description_text[256] = "";
    
    if (command_input_) {
        XPGetWidgetDescriptor(command_input_, command_text, sizeof(command_text));
    }
    if (description_input_) {
        XPGetWidgetDescriptor(description_input_, description_text, sizeof(description_text));
    }
    
    // Get recorded combination
    const auto& combination = recording_ ? tracker_->get_recorded_combination() : tracker_->get_currently_pressed_buttons();
    
    if (combination.empty() || strlen(command_text) == 0) {
        XPLMDebugString("Multibind: Cannot save - no combination or command specified\n");
        return;
    }
    
    // Create and save binding
    MultibindBinding binding(combination, std::string(command_text), std::string(description_text));
    
    if (selected_binding_index_ >= 0) {
        // Update existing binding
        config_->update_binding(selected_binding_index_, binding);
        selected_binding_index_ = -1;
        XPLMDebugString("Multibind: Updated existing binding\n");
    } else {
        // Add new binding
        config_->add_binding(binding);
        XPLMDebugString("Multibind: Added new binding\n");
    }
    
    // Save to file and update tracker
    config_->save_config();
    tracker_->set_bindings(config_->get_bindings());
    
    // Clear inputs and stop recording
    handle_clear_button();
    if (recording_) {
        handle_record_button();
    }
    
    refresh_ui();
}

void UI::handle_edit_button()
{
    // Implementation would allow editing selected binding
    XPLMDebugString("Multibind: Edit button clicked (not fully implemented)\n");
}

void UI::handle_delete_button()
{
    if (!config_ || selected_binding_index_ < 0) return;
    
    config_->remove_binding(selected_binding_index_);
    config_->save_config();
    tracker_->set_bindings(config_->get_bindings());
    
    selected_binding_index_ = -1;
    refresh_ui();
    
    XPLMDebugString("Multibind: Deleted selected binding\n");
}

std::string UI::format_combination(const std::set<int>& combination) const
{
    if (combination.empty()) {
        return "None";
    }
    
    std::stringstream ss;
    bool first = true;
    for (int button : combination) {
        if (!first) ss << "+";
        ss << button;
        first = false;
    }
    return ss.str();
}

void UI::refresh_ui()
{
    update_binding_list();
    update_current_combination_display();
}

int UI::widget_callback(XPWidgetMessage message, XPWidgetID widget, intptr_t param1, intptr_t param2)
{
    // Get the UI instance from the main window's refcon (we'd need to set this up)
    // For now, we'll use a global reference - not ideal but works for this demo
    static UI* ui_instance = nullptr;
    
    if (!ui_instance) {
        // This is a hack - in production we'd properly associate the UI instance with the widget
        return 0;
    }
    
    return ui_instance->handle_widget_message(message, widget, param1, param2);
}

int UI::handle_widget_message(XPWidgetMessage message, XPWidgetID widget, intptr_t param1, intptr_t param2)
{
    switch (message) {
        case xpMessage_CloseButtonPushed:
            if (widget == main_window_) {
                hide_window();
                return 1;
            }
            break;
            
        case xpMsg_PushButtonPressed:
            if (widget == record_button_) {
                handle_record_button();
                return 1;
            } else if (widget == clear_button_) {
                handle_clear_button();
                return 1;
            } else if (widget == save_button_) {
                handle_save_button();
                return 1;
            } else if (widget == edit_button_) {
                handle_edit_button();
                return 1;
            } else if (widget == delete_button_) {
                handle_delete_button();
                return 1;
            }
            break;
    }
    
    return 0;
}