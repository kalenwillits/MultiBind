# Multibind

An X-Plane 12 plugin that enables and manages multi button
keybinds to invoke x-Plane commands


# Features
- Allows multi-button keybind assignments
- Import & Export profiles for each aircraft
- Multiplatform (Windows, Mac, & Linux)
- A new `Multibind` plugin menu

# User flows


On start
```
Open up XPlane 12 and load an aircraft
Multibind plugin looks for a `multibind/{aircraft-identifer}.csv` file
if the multibind directory does not exist, create one
if the multibind file does not exist, create one
if the Multibind file does exist, parse and load it.
Multibind creates 999 custom command button assignments, if they don't already exist
```

Create a new binding
```
User navigates to Plugins->Multibind
The Multibind window opens
The Multibind window displays the `multibind/{aircraft-identifer}.csv` file as a table that can be scrolled through
Multibind now detects if any of the multibind custom commands are pressed
user presses multibind_001
The multibind plugin window tracks and displays which multibind commands have been pressed
The user restarts the multibind tracker by clicking the [clear] button
user presses multibind_001
and a text field appears
User pressed multibind_003
User types into a text field within the multibind plugin window the dataref command of which they want to create a new binding for (sim/actuators/starters engaged)
User clicks 'Save'
Multibind appends the new binding mapping to the internal bindings hashmap
The multibind hashmap is written to the Multibind/{aircraft-identifier}.csv file.
Now when the user holds multibind_001 and multibind_003 buttons in any order, the custom dataref command is invoked, and the aircrafts starter is engaged. 
```

Save bindings
```
Within the multibind plugin window,
User clicks 'Save'
Multibinds are updated in the new binding mapping to the internal bindings hashmap
The bindings hashmap is serialized and saved to `multibind/{aircraft-identifer}.csv, overwrite if already exists` 
```

Edit an existing binding
```
Within the multibind plugin window,
User scrolls to the desired keybind and clicks it
A delete button appears
An edit button appears
The user clicks the edit button,
The existing button presses appear
a text field appears populated with the existing dataref command
The user clicks the [clear] button
The multibind_xxx button disappear
The user presses new multibind buttons
The user clicks Save
```


# Resources
[X Plane SDK docs home](https://developer.x-plane.com/sdk/)
[X Plane SDK docuemnts](https://developer.x-plane.com/sdk/plugin-sdk-documents/)
[X Plane SDK Downloads](https://developer.x-plane.com/sdk/plugin-sdk-downloads/)
[X Plane SDK Sample Code](https://developer.x-plane.com/sdk/plugin-sdk-sample-code/)
[X Plane forums](https://forums.x-plane.org/)
[FlyWithLua -- A complex plugin](https://github.com/X-Friese/FlyWithLua)
[Example of how to detect key presses](https://developer.x-plane.com/code-sample/keysniffer/)

# Research hints:
Creating a custom command
```
 Multibind_000= XPLMCreateCommand("multibind/000", "Multibind 000");
```
https://developer.x-plane.com/code-sample/custom-command-with-custom-dataref/


# Style
- Minimal simple code that is easy to read
- The simpler the UI the better

# Requirements 
- The result must include complete feature code and,
- a decision log
- A README.md going over the code structure
- Details on compiling
- Install instructions
