#!/bin/bash
# BTZ - JUCE Framework Setup Script
# Automates JUCE installation for BTZ plugin development

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo "========================================"
echo "BTZ - JUCE Framework Setup"
echo "========================================"
echo ""

# Detect platform
PLATFORM=$(uname -s)
echo -e "${BLUE}Platform detected: $PLATFORM${NC}"
echo ""

# Check dependencies
echo "Checking dependencies..."

# CMake
if ! command -v cmake &> /dev/null; then
    echo -e "${RED}ERROR: CMake not found!${NC}"
    echo "Install: sudo apt-get install cmake (Linux) or brew install cmake (macOS)"
    exit 1
else
    CMAKE_VERSION=$(cmake --version | head -1)
    echo -e "${GREEN}✓ CMake found:${NC} $CMAKE_VERSION"
fi

# C++ Compiler
if ! command -v g++ &> /dev/null && ! command -v clang++ &> /dev/null; then
    echo -e "${RED}ERROR: C++ compiler not found!${NC}"
    echo "Install: sudo apt-get install build-essential (Linux) or xcode-select --install (macOS)"
    exit 1
else
    if command -v g++ &> /dev/null; then
        CXX_VERSION=$(g++ --version | head -1)
        echo -e "${GREEN}✓ g++ found:${NC} $CXX_VERSION"
    else
        CXX_VERSION=$(clang++ --version | head -1)
        echo -e "${GREEN}✓ clang++ found:${NC} $CXX_VERSION"
    fi
fi

# Git
if ! command -v git &> /dev/null; then
    echo -e "${RED}ERROR: Git not found!${NC}"
    echo "Install: sudo apt-get install git (Linux) or brew install git (macOS)"
    exit 1
else
    GIT_VERSION=$(git --version)
    echo -e "${GREEN}✓ Git found:${NC} $GIT_VERSION"
fi

echo ""
echo "========================================"
echo "JUCE Framework Installation"
echo "========================================"
echo ""

# Check if JUCE already exists
if [ -d "JUCE" ]; then
    echo -e "${YELLOW}WARNING: JUCE directory already exists${NC}"
    read -p "Remove and reinstall? (y/N): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        echo "Removing existing JUCE..."
        rm -rf JUCE
    else
        echo "Keeping existing JUCE installation"
        exit 0
    fi
fi

echo "Cloning JUCE framework..."
echo "Repository: https://github.com/juce-framework/JUCE.git"
echo ""

# Clone JUCE as Git submodule (recommended for reproducible builds)
read -p "Install as Git submodule (recommended)? (Y/n): " -n 1 -r
echo

if [[ $REPLY =~ ^[Nn]$ ]]; then
    # Clone directly (not recommended but simpler)
    echo "Cloning JUCE (standalone)..."
    git clone --depth 1 --branch 7.0.12 https://github.com/juce-framework/JUCE.git

    echo ""
    echo -e "${YELLOW}NOTE: JUCE cloned as standalone directory${NC}"
    echo -e "${YELLOW}For reproducible builds, consider using Git submodules${NC}"
else
    # Add as submodule (recommended)
    echo "Adding JUCE as Git submodule..."

    # Check if we're in a git repo
    if ! git rev-parse --git-dir > /dev/null 2>&1; then
        echo -e "${RED}ERROR: Not in a Git repository!${NC}"
        echo "Run 'git init' first or clone directly"
        exit 1
    fi

    # Add submodule
    git submodule add https://github.com/juce-framework/JUCE.git JUCE
    git submodule update --init --recursive

    # Pin to specific version for reproducibility
    cd JUCE
    git checkout 7.0.12
    cd ..

    echo ""
    echo -e "${GREEN}✓ JUCE added as Git submodule (pinned to v7.0.12)${NC}"
    echo ""
    echo "To commit this change:"
    echo "  git add .gitmodules JUCE"
    echo "  git commit -m 'Add JUCE v7.0.12 as submodule'"
fi

# Verify JUCE installation
if [ -d "JUCE/modules" ]; then
    echo ""
    echo -e "${GREEN}✓ JUCE installation successful!${NC}"
    echo ""
    echo "JUCE modules found:"
    ls JUCE/modules | head -10
    echo "  ... (and more)"
else
    echo -e "${RED}ERROR: JUCE installation failed!${NC}"
    exit 1
fi

echo ""
echo "========================================"
echo "Build System Configuration"
echo "========================================"
echo ""

# Platform-specific dependencies
if [ "$PLATFORM" = "Linux" ]; then
    echo "Linux detected - checking for required development packages..."

    MISSING_PACKAGES=()

    # Check for ALSA
    if ! dpkg -l | grep -q libasound2-dev; then
        MISSING_PACKAGES+=("libasound2-dev")
    fi

    # Check for X11
    if ! dpkg -l | grep -q libx11-dev; then
        MISSING_PACKAGES+=("libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev")
    fi

    # Check for Freetype
    if ! dpkg -l | grep -q libfreetype6-dev; then
        MISSING_PACKAGES+=("libfreetype6-dev")
    fi

    # Check for Webkit
    if ! dpkg -l | grep -q libwebkit2gtk-4.0-dev; then
        MISSING_PACKAGES+=("libwebkit2gtk-4.0-dev")
    fi

    if [ ${#MISSING_PACKAGES[@]} -gt 0 ]; then
        echo -e "${YELLOW}Missing packages detected:${NC}"
        for pkg in "${MISSING_PACKAGES[@]}"; do
            echo "  - $pkg"
        done
        echo ""
        echo "Install with:"
        echo "  sudo apt-get install ${MISSING_PACKAGES[*]}"
    else
        echo -e "${GREEN}✓ All required packages installed${NC}"
    fi
fi

echo ""
echo "========================================"
echo "Setup Complete!"
echo "========================================"
echo ""
echo "Next steps:"
echo ""
echo "1. Build the plugin:"
echo "   cd BTZ_JUCE"
echo "   mkdir -p build && cd build"
echo "   cmake .. -DCMAKE_BUILD_TYPE=Release"
echo "   cmake --build . --config Release -j\$(nproc)"
echo ""
echo "2. Run validation:"
echo "   cd .."
echo "   ./scripts/run_pluginval.sh"
echo ""
echo "3. Test in your DAW:"
if [ "$PLATFORM" = "Darwin" ]; then
    echo "   VST3: ~/Library/Audio/Plug-Ins/VST3/BTZ.vst3"
    echo "   AU:   ~/Library/Audio/Plug-Ins/Components/BTZ.component"
elif [ "$PLATFORM" = "Linux" ]; then
    echo "   VST3: ~/.vst3/BTZ.vst3"
else
    echo "   VST3: C:\\Program Files\\Common Files\\VST3\\BTZ.vst3"
fi
echo ""
echo -e "${GREEN}Happy coding!${NC}"
