#!/bin/bash

# Constants
EXECUTABLE_NAME="krypt0n"
BUILD_DIR="build"

check_dependencies() {
    echo "Checking dependencies..."

    # Check for CMake
    if ! command -v cmake &> /dev/null; then
        echo "CMake is not installed. Please install CMake first."
        return 1
    fi

    # Check for essential libs using pkg-config
    REQUIRED_PKGS=("glfw3" "glew" "x11" "libevdev")

    for pkg in "${REQUIRED_PKGS[@]}"; do
        if ! pkg-config --exists "$pkg"; then
            echo "$pkg development files not found. Please install $pkg."
            return 1
        fi
    done

    # Check for root privileges (required for uinput access)
    if [ "$EUID" -ne 0 ]; then
        echo "Root privileges are required to run this application."
        echo "Please run this script with sudo."
        return 1
    fi

    echo "All dependencies are satisfied!"
    return 0
}

clean_build() {
    echo "Cleaning the build directory..."
    if [ -d "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR"
        echo "Build directory cleaned!"
    else
        echo "Build directory does not exist."
    fi
}

configure_and_build() {
    echo "Starting build process..."

    # Clean previous build if it exists
    clean_build

    # Create build directory
    echo "Creating build directory..."
    mkdir -p "$BUILD_DIR"

    # Configure with CMake
    echo "Running CMake configuration..."
    if ! cmake -B "$BUILD_DIR"; then
        echo "CMake configuration failed!"
        return 1
    fi

    # Build the project
    echo "Building the project..."
    if ! cmake --build "$BUILD_DIR" -j$(nproc); then
        echo "Build failed!"
        return 1
    fi

    echo "Build completed successfully!"
    return 0
}

run_program() {
    if [ -f "$BUILD_DIR/bin/$EXECUTABLE_NAME" ]; then
        echo "Running $EXECUTABLE_NAME..."
        cd "$BUILD_DIR/bin" || return 1
        ./"$EXECUTABLE_NAME"
        cd - > /dev/null || return 1
    else
        echo "Error: Executable not found at $BUILD_DIR/bin/$EXECUTABLE_NAME"
        echo "Please build the project first."
        return 1
    fi
}

quick_build_and_run() {
    echo "=== Quick build and run ==="

    if ! check_dependencies; then
        echo "Please install required dependencies before proceeding."
        return 1
    fi

    if ! configure_and_build; then
        echo "Build process failed."
        return 1
    fi

    run_program
}

show_menu() {
    echo -e "\n===== Marvel Game Assistant Tool ====="
    echo "1) Check Dependencies"
    echo "2) Build Project"
    echo "3) Quick Build and Run"
    echo "4) Run Program"
    echo "0) Exit"
    echo -n "Enter your choice: "
}

while true; do
    show_menu
    read -r choice

    case $choice in
        1) check_dependencies ;;
        2) configure_and_build ;;\
        3) quick_build_and_run ;;
        4) run_program ;;
        0)
            echo "Exiting."
            exit 0
            ;;
        *)
            echo "Invalid choice. Please try again."
            ;;
    esac
done