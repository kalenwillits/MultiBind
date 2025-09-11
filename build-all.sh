#!/bin/bash

set -e

echo "========================================"
echo " MultiBind - Complete Build Script"
echo "========================================"
echo
echo "This script will:"
echo "1. Download X-Plane SDK if needed"
echo "2. Build the plugin"
echo "3. Show installation instructions"
echo
read -p "Press Enter to continue..."

# Step 1: Setup SDK if needed
if [[ ! -f "SDK/CHeaders/XPLM/XPLMPlugin.h" ]]; then
    echo "Step 1: Setting up X-Plane SDK..."
    ./setup-sdk.sh
else
    echo "✅ Step 1: X-Plane SDK already available"
fi

echo
echo "Step 2: Building plugin..."
./build.sh

echo
echo "========================================"
echo "✅ BUILD COMPLETE!"
echo "========================================"
echo
echo "Next steps:"
echo "1. Copy the MultiBind folder to: X-Plane 12/Resources/plugins/"
echo "2. Restart X-Plane"
echo "3. Configure your joystick buttons in X-Plane Settings"
echo "4. Create button combinations in Plugins → MultiBind"
echo
echo "See QUICKSTART.md for detailed usage instructions"
echo