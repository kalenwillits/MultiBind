#include "ui.h"
#include "XPLMUtilities.h"
#include "XPLMDisplay.h"
#include "input_validation.h"
#include "constants.h"

#include <sstream>
#include <algorithm>
#include <cstring>

UI::~UI()
{
    destroy_widgets();
}

void UI::initialize(Config* config, CombinationTracker* tracker)
{
    _config = config;
    _tracker = tracker;
}

void UI::show_window()
{
    if (!_main_window) {
        create_widgets();
        if (!verify_widget_creation()) {
            destroy_widgets();
            return;
        }
    }
    
    if (_main_window) {
        XPShowWidget(_main_window);
        
        // Update aircraft display on show
        if (_config) {
            std::string aircraft_name = _config->get_aircraft_id();
            if (!aircraft_name.empty()) {
                update_aircraft_display(aircraft_name);
            }
        }
        
        refresh_ui();
    }
}

void UI::hide_window()
{
    if (_main_window) {
        XPHideWidget(_main_window);
    }
}

bool UI::is_window_visible() const
{
    return _main_window && XPIsWidgetVisible(_main_window);
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
    _main_window = XPCreateWidget(left, top, right, bottom,
                                 1, "Multibind Configuration", 1,
                                 nullptr, xpWidgetClass_MainWindow);
    
    XPSetWidgetProperty(_main_window, xpProperty_MainWindowHasCloseBoxes, 1);
    XPSetWidgetProperty(_main_window, xpProperty_Refcon, (intptr_t)this);
    XPAddWidgetCallback(_main_window, widget_callback);
    
    int y = top - 40;
    
    // Current aircraft label
    _aircraft_label = XPCreateWidget(left + MARGIN, y, left + WINDOW_WIDTH - MARGIN, y - 20,
                                   1, "Current Aircraft: Loading...", 0, _main_window,
                                   xpWidgetClass_Caption);
    y -= 30;
    
    // Status message label
    _status_message_label = XPCreateWidget(left + MARGIN, y, left + WINDOW_WIDTH - MARGIN, y - 20,
                                         1, "", 0, _main_window,
                                         xpWidgetClass_Caption);
    y -= 25;
    
    // Current combination display
    _current_combination_label = XPCreateWidget(left + MARGIN, y, left + WINDOW_WIDTH - MARGIN, y - 20,
                                              1, "Current Combination: None", 0, _main_window,
                                              xpWidgetClass_Caption);
    y -= 30;
    
    // Recording controls
    _record_button = XPCreateWidget(left + MARGIN, y, left + MARGIN + 80, y - BUTTON_HEIGHT,
                                   1, "Record", 0, _main_window,
                                   xpWidgetClass_Button);
    XPAddWidgetCallback(_record_button, widget_callback);
    
    _clear_button = XPCreateWidget(left + MARGIN + 90, y, left + MARGIN + 170, y - BUTTON_HEIGHT,
                                  1, "Clear", 0, _main_window,
                                  xpWidgetClass_Button);
    XPAddWidgetCallback(_clear_button, widget_callback);
    y -= 35;
    
    // Command input
    XPCreateWidget(left + MARGIN, y, left + MARGIN + 100, y - 20,
                  1, "Target Command:", 0, _main_window,
                  xpWidgetClass_Caption);
    
    _command_input = XPCreateWidget(left + MARGIN + 110, y, left + WINDOW_WIDTH - MARGIN, y - INPUT_HEIGHT,
                                   1, "", 0, _main_window,
                                   xpWidgetClass_TextField);
    XPSetWidgetProperty(_command_input, xpProperty_TextFieldType, xpTextEntryField);
    y -= 35;
    
    // Description input
    XPCreateWidget(left + MARGIN, y, left + MARGIN + 100, y - 20,
                  1, "Description:", 0, _main_window,
                  xpWidgetClass_Caption);
    
    _description_input = XPCreateWidget(left + MARGIN + 110, y, left + WINDOW_WIDTH - MARGIN, y - INPUT_HEIGHT,
                                       1, "", 0, _main_window,
                                       xpWidgetClass_TextField);
    XPSetWidgetProperty(_description_input, xpProperty_TextFieldType, xpTextEntryField);
    y -= 35;
    
    // Action buttons
    _save_button = XPCreateWidget(left + MARGIN, y, left + MARGIN + 80, y - BUTTON_HEIGHT,
                                 1, "Save", 0, _main_window,
                                 xpWidgetClass_Button);
    XPAddWidgetCallback(_save_button, widget_callback);
    y -= 35;
    
    // Selection controls
    XPCreateWidget(left + MARGIN, y, left + MARGIN + 150, y - 20,
                  1, "Select binding # to edit/delete:", 0, _main_window,
                  xpWidgetClass_Caption);
    
    _selection_input = XPCreateWidget(left + MARGIN + 160, y, left + MARGIN + 200, y - INPUT_HEIGHT,
                                     1, "", 0, _main_window,
                                     xpWidgetClass_TextField);
    XPSetWidgetProperty(_selection_input, xpProperty_TextFieldType, xpTextEntryField);
    
    _edit_button = XPCreateWidget(left + MARGIN + 210, y, left + MARGIN + 260, y - BUTTON_HEIGHT,
                                 1, "Edit", 0, _main_window,
                                 xpWidgetClass_Button);
    XPAddWidgetCallback(_edit_button, widget_callback);
    
    _delete_button = XPCreateWidget(left + MARGIN + 270, y, left + MARGIN + 330, y - BUTTON_HEIGHT,
                                   1, "Delete", 0, _main_window,
                                   xpWidgetClass_Button);
    XPAddWidgetCallback(_delete_button, widget_callback);
    
    // Confirmation buttons (initially hidden)
    _confirm_button = XPCreateWidget(left + MARGIN + 340, y, left + MARGIN + 400, y - BUTTON_HEIGHT,
                                    1, "Confirm", 0, _main_window,
                                    xpWidgetClass_Button);
    XPAddWidgetCallback(_confirm_button, widget_callback);
    XPHideWidget(_confirm_button);
    
    _cancel_button = XPCreateWidget(left + MARGIN + 410, y, left + MARGIN + 470, y - BUTTON_HEIGHT,
                                   1, "Cancel", 0, _main_window,
                                   xpWidgetClass_Button);
    XPAddWidgetCallback(_cancel_button, widget_callback);
    XPHideWidget(_cancel_button);
    y -= 40;
    
    // Binding list
    XPCreateWidget(left + MARGIN, y, left + WINDOW_WIDTH - MARGIN, y - 20,
                  1, "Current Bindings:", 0, _main_window,
                  xpWidgetClass_Caption);
    y -= 25;
    
    _binding_list = XPCreateWidget(left + MARGIN, y, left + WINDOW_WIDTH - MARGIN - 20, bottom + MARGIN,
                                  1, "", 0, _main_window,
                                  xpWidgetClass_TextField);
    XPAddWidgetCallback(_binding_list, widget_callback);
    
    _scroll_bar = XPCreateWidget(left + WINDOW_WIDTH - MARGIN - 20, y, left + WINDOW_WIDTH - MARGIN, bottom + MARGIN,
                                1, "", 0, _main_window,
                                xpWidgetClass_ScrollBar);
}

void UI::destroy_widgets()
{
    if (_main_window) {
        XPDestroyWidget(_main_window, 1);
        _main_window = nullptr;
        _aircraft_label = nullptr;
        _status_message_label = nullptr;
        _binding_list = nullptr;
        _current_combination_label = nullptr;
        _record_button = nullptr;
        _clear_button = nullptr;
        _command_input = nullptr;
        _description_input = nullptr;
        _save_button = nullptr;
        _selection_input = nullptr;
        _edit_button = nullptr;
        _delete_button = nullptr;
        _confirm_button = nullptr;
        _cancel_button = nullptr;
        _scroll_bar = nullptr;
    }
}

void UI::update_binding_list()
{
    if (!_config || !_binding_list) return;
    
    const auto& bindings = _config->get_bindings();
    
    if (bindings.empty()) {
        XPSetWidgetDescriptor(_binding_list, "No bindings configured yet.\n\nTo create a binding:\n1. Click 'Record'\n2. Press your button combination\n3. Enter X-Plane command\n4. Click 'Save'");
        return;
    }
    
    // Build the list content with improved formatting
    std::string list_content;
    
    for (size_t i = 0; i < bindings.size(); ++i) {
        std::string combo_str = format_combination(bindings[i].button_combination);
        std::string description = bindings[i].description.empty() ? "No description" : bindings[i].description;
        
        // Format: [Index] Buttons -> Command (Description)
        list_content += std::to_string(i + 1) + ". " + combo_str + " -> " + 
                       bindings[i].target_command + "\n   (" + description + ")\n\n";
        
        // Limit display to prevent UI overflow (about 10-15 bindings visible)
        if (list_content.length() > 1500) {
            list_content += "... (" + std::to_string(bindings.size() - i - 1) + " more bindings)\n";
            break;
        }
    }
    
    XPSetWidgetDescriptor(_binding_list, list_content.c_str());
}

void UI::update_current_combination_display()
{
    if (!_current_combination_label || !_tracker) {
        return;
    }
    
    const auto& pressed = _tracker->get_currently_pressed_buttons();
    std::string display_text = "Current Combination: ";
    
    if (pressed.empty()) {
        display_text += "None";
    } else {
        display_text += format_combination(pressed);
    }
    
    XPSetWidgetDescriptor(_current_combination_label, display_text.c_str());
}

void UI::update_aircraft_display(const std::string& aircraft_name)
{
    if (!_aircraft_label) return;
    
    std::string display_text = "Current Aircraft: " + aircraft_name;
    XPSetWidgetDescriptor(_aircraft_label, display_text.c_str());
}

void UI::show_status_message(const std::string& message, bool is_error)
{
    if (!_status_message_label) return;
    
    std::string display_text = is_error ? "❌ " : "✅ ";
    display_text += message;
    
    XPSetWidgetDescriptor(_status_message_label, display_text.c_str());
}

void UI::clear_status_message()
{
    if (_status_message_label) {
        XPSetWidgetDescriptor(_status_message_label, "");
    }
}

void UI::handle_record_button()
{
    if (!_tracker) return;
    
    if (_recording) {
        _tracker->stop_recording();
        _recording = false;
        XPSetWidgetDescriptor(_record_button, "Record");
        show_status_message("Recording stopped");
    } else {
        _tracker->start_recording();
        _recording = true;
        XPSetWidgetDescriptor(_record_button, "Stop");
        show_status_message("Recording started - press your button combination");
    }
}

void UI::handle_clear_button()
{
    if (!_tracker) return;
    
    _tracker->clear_currently_pressed();
    if (_recording) {
        _tracker->stop_recording();
        _tracker->start_recording(); // Restart recording with empty combination
    }
    
    // Clear input fields
    if (_command_input) XPSetWidgetDescriptor(_command_input, "");
    if (_description_input) XPSetWidgetDescriptor(_description_input, "");
    
    clear_status_message();
    show_status_message("Inputs cleared");
}

void UI::handle_save_button()
{
    if (!_config || !_tracker) return;
    
    // Get command and description from input fields
    char command_text[256] = "";
    char description_text[256] = "";
    
    if (_command_input) {
        XPGetWidgetDescriptor(_command_input, command_text, sizeof(command_text));
    }
    if (_description_input) {
        XPGetWidgetDescriptor(_description_input, description_text, sizeof(description_text));
    }
    
    // Get recorded combination
    const auto& combination = _recording ? _tracker->get_recorded_combination() : _tracker->get_currently_pressed_buttons();
    
    // Validation
    if (combination.empty()) {
        show_status_message("Error: No button combination specified", true);
        return;
    }
    
    if (strlen(command_text) == 0) {
        show_status_message("Error: No X-Plane command specified", true);
        return;
    }
    
    // Basic command validation (should start with common X-Plane prefixes)
    std::string cmd_str(command_text);
    if (cmd_str.find("sim/") != 0 && cmd_str.find("laminar/") != 0 && 
        cmd_str.find("custom/") != 0 && cmd_str.find("multibind/") != 0) {
        show_status_message("Warning: Command should typically start with sim/ or laminar/", false);
    }
    
    // Create and save binding
    MultibindBinding binding(combination, std::string(command_text), std::string(description_text));
    
    if (_selected_binding_index >= 0) {
        // Update existing binding
        _config->update_binding(_selected_binding_index, binding);
        _selected_binding_index = -1;
        show_status_message("Binding updated successfully");
    } else {
        // Add new binding
        _config->add_binding(binding);
        show_status_message("New binding added successfully");
    }
    
    // Save to file and update tracker
    if (!_config->save_config()) {
        show_status_message("Error: Failed to save configuration file", true);
        return;
    }
    
    _tracker->set_bindings(_config->get_bindings());
    
    // Clear inputs and stop recording
    handle_clear_button();
    if (_recording) {
        handle_record_button();
    }
    
    refresh_ui();
}

void UI::handle_edit_button()
{
    if (!_config || !_selection_input) {
        show_status_message("Error: Cannot access selection input", true);
        return;
    }
    
    // Get selection from input field
    using namespace multibind::constants;
    
    char selection_text[SELECTION_TEXT_BUFFER_SIZE] = "";
    XPGetWidgetDescriptor(_selection_input, selection_text, sizeof(selection_text));
    
    const auto& bindings = _config->get_bindings();
    auto selection_opt = multibind::validation::parse_selection_number(
        selection_text, 1, static_cast<int>(bindings.size()));
    
    if (!selection_opt) {
        if (strlen(selection_text) == 0) {
            show_status_message("Error: Enter a binding number to edit", true);
        } else {
            show_status_message("Error: Invalid binding number. Enter a number between 1-" + 
                              std::to_string(bindings.size()), true);
        }
        return;
    }
    
    int selection_num = *selection_opt;
    
    // Convert from 1-based to 0-based index
    _selected_binding_index = selection_num - 1;
    const auto& binding = bindings[_selected_binding_index];
    
    // Populate input fields with existing binding data
    if (_command_input) {
        XPSetWidgetDescriptor(_command_input, binding.target_command.c_str());
    }
    if (_description_input) {
        XPSetWidgetDescriptor(_description_input, binding.description.c_str());
    }
    
    // Show the combination (but user will need to re-record it)
    std::string combo_str = format_combination(binding.button_combination);
    show_status_message("Editing binding #" + std::to_string(selection_num) + " (" + combo_str + ") - modify and Save");
}

void UI::handle_delete_button()
{
    if (!_config || !_selection_input) {
        show_status_message("Error: Cannot access selection input", true);
        return;
    }
    
    // If already confirming, ignore additional delete clicks
    if (_confirming_delete) {
        return;
    }
    
    // Get selection from input field
    using namespace multibind::constants;
    
    char selection_text[SELECTION_TEXT_BUFFER_SIZE] = "";
    XPGetWidgetDescriptor(_selection_input, selection_text, sizeof(selection_text));
    
    const auto& bindings = _config->get_bindings();
    auto selection_opt = multibind::validation::parse_selection_number(
        selection_text, 1, static_cast<int>(bindings.size()));
    
    if (!selection_opt) {
        if (strlen(selection_text) == 0) {
            show_status_message("Error: Enter a binding number to delete", true);
        } else {
            show_status_message("Error: Invalid binding number. Enter a number between 1-" + 
                              std::to_string(bindings.size()), true);
        }
        return;
    }
    
    int selection_num = *selection_opt;
    
    // Convert from 1-based to 0-based index and enter confirmation mode
    _delete_candidate_index = selection_num - 1;
    _confirming_delete = true;
    
    // Show confirmation message
    const auto& binding = bindings[_delete_candidate_index];
    std::string combo_str = format_combination(binding.button_combination);
    show_status_message("Delete binding #" + std::to_string(selection_num) + " (" + combo_str + ")? Click Confirm or Cancel");
    
    update_button_visibility();
}

void UI::handle_confirm_button()
{
    if (!_config || !_confirming_delete || _delete_candidate_index < 0) {
        return;
    }
    
    const auto& bindings = _config->get_bindings();
    if (_delete_candidate_index >= static_cast<int>(bindings.size())) {
        show_status_message("Error: Selected binding no longer exists", true);
        _confirming_delete = false;
        update_button_visibility();
        return;
    }
    
    // Perform the actual deletion
    int display_num = _delete_candidate_index + 1; // For user feedback
    _config->remove_binding(_delete_candidate_index);
    
    if (!_config->save_config()) {
        show_status_message("Error: Failed to save after deletion", true);
        _confirming_delete = false;
        update_button_visibility();
        return;
    }
    
    _tracker->set_bindings(_config->get_bindings());
    
    // Reset state
    _confirming_delete = false;
    _delete_candidate_index = -1;
    _selected_binding_index = -1;
    
    // Clear the selection input
    if (_selection_input) {
        XPSetWidgetDescriptor(_selection_input, "");
    }
    
    update_button_visibility();
    refresh_ui();
    show_status_message("Binding #" + std::to_string(display_num) + " deleted successfully");
}

void UI::handle_cancel_button()
{
    if (!_confirming_delete) {
        return;
    }
    
    // Cancel deletion and reset state
    _confirming_delete = false;
    _delete_candidate_index = -1;
    
    show_status_message("Deletion cancelled");
    update_button_visibility();
}

void UI::update_button_visibility()
{
    if (!_confirm_button || !_cancel_button) return;
    
    if (_confirming_delete) {
        XPShowWidget(_confirm_button);
        XPShowWidget(_cancel_button);
    } else {
        XPHideWidget(_confirm_button);
        XPHideWidget(_cancel_button);
    }
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
    // Get the UI instance from the widget's refcon
    UI* ui_instance = nullptr;
    
    // Find the main window first
    XPWidgetID main_window = widget;
    while (main_window && XPGetParentWidget(main_window)) {
        main_window = XPGetParentWidget(main_window);
    }
    
    if (main_window) {
        ui_instance = (UI*)XPGetWidgetProperty(main_window, xpProperty_Refcon, nullptr);
    }
    
    if (!ui_instance) {
        return 0;
    }
    
    return ui_instance->handle_widget_message(message, widget, param1, param2);
}

int UI::handle_widget_message(XPWidgetMessage message, XPWidgetID widget, intptr_t param1, intptr_t param2)
{
    switch (message) {
        case xpMessage_CloseButtonPushed:
            if (widget == _main_window) {
                hide_window();
                return 1;
            }
            break;
            
        case xpMsg_PushButtonPressed:
            if (widget == _record_button) {
                handle_record_button();
                return 1;
            } else if (widget == _clear_button) {
                handle_clear_button();
                return 1;
            } else if (widget == _save_button) {
                handle_save_button();
                return 1;
            } else if (widget == _edit_button) {
                handle_edit_button();
                return 1;
            } else if (widget == _delete_button) {
                handle_delete_button();
                return 1;
            } else if (widget == _confirm_button) {
                handle_confirm_button();
                return 1;
            } else if (widget == _cancel_button) {
                handle_cancel_button();
                return 1;
            }
            break;
            
        case xpMsg_TextFieldChanged:
            // Handle binding list selection (simplified approach)
            if (widget == _binding_list) {
                // For now, we'll use a simple approach - in a full implementation,
                // we'd need to track mouse clicks and calculate which binding was selected
                show_status_message("Click Edit or Delete buttons to modify a binding");
                return 1;
            }
            break;
    }
    
    return 0;
}

bool UI::verify_widget_creation()
{
    // Check that critical widgets were created successfully
    if (!_main_window) {
        XPLMDebugString("Multibind: ERROR - Failed to create main window\n");
        return false;
    }
    
    if (!_aircraft_label || !_status_message_label || !_current_combination_label) {
        XPLMDebugString("Multibind: ERROR - Failed to create status labels\n");
        return false;
    }
    
    if (!_record_button || !_clear_button) {
        XPLMDebugString("Multibind: ERROR - Failed to create recording buttons\n");
        return false;
    }
    
    if (!_command_input || !_description_input || !_save_button) {
        XPLMDebugString("Multibind: ERROR - Failed to create input fields\n");
        return false;
    }
    
    if (!_selection_input || !_edit_button || !_delete_button) {
        XPLMDebugString("Multibind: ERROR - Failed to create editing controls\n");
        return false;
    }
    
    return true;
}