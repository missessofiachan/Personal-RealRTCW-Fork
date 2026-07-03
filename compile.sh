#!/usr/bin/env bash

# Exit immediately if a command exits with a non-zero status
set -e

# Get the directory of this script
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

# Color codes for pretty output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[0;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Target Steam directory
TARGET_DIR="/run/media/system/NVME_GAME_1/SteamLibrary/steamapps/common/RealRTCW"

# 1. Parse arguments
CLEAN_BUILD=false
RUN_AFTER_BUILD=false
RUN_ARGS=()

while [[ $# -gt 0 ]]; do
    case "$1" in
        -c|--clean)
            CLEAN_BUILD=true
            shift
            ;;
        -r|--run)
            RUN_AFTER_BUILD=true
            shift
            # Collect all remaining arguments to pass to the game
            while [[ $# -gt 0 ]]; do
                RUN_ARGS+=("$1")
                shift
            done
            ;;
        -h|--help)
            echo "Usage: $0 [options] [--game-args...]"
            echo ""
            echo "Options:"
            echo "  -c, --clean     Perform a clean build by running 'make clean' first."
            echo "  -r, --run       Launch RealRTCW via the native Steam launcher script after a successful build."
            echo "                  All subsequent arguments are forwarded directly to the launcher."
            echo "  -h, --help      Display this help menu."
            echo ""
            echo "Examples:"
            echo "  $0 -c"
            echo "  $0 -r +set sv_cheats 1"
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            echo "Use -h or --help for usage information."
            exit 1
            ;;
    esac
done

echo -e "${BLUE}=== RealRTCW Build Script ===${NC}"

# Verify target directory is mounted/accessible before starting
if [ ! -d "$TARGET_DIR" ]; then
    echo -e "${RED}Error: Target directory does not exist:${NC}"
    echo -e "  ${YELLOW}$TARGET_DIR${NC}"
    echo "Please make sure your NVMe drive is mounted."
    exit 1
fi

# 2. Detect container environment vs host
INSIDE_CONTAINER=false
if [ -f /run/.containerenv ] || [ -f /run/.toolboxenv ] || [ "${container}" = "podman" ] || [ "${container}" = "docker" ]; then
    INSIDE_CONTAINER=true
fi

# Determine how to run commands (directly or via distrobox)
USE_DISTROBOX=false
DISTROBOX_NAME=""
if [ "$INSIDE_CONTAINER" = false ]; then
    if command -v distrobox &> /dev/null; then
        # Check if Bazzite-dev-nvidia or Bazzite-dev-env container exists
        if distrobox list 2>/dev/null | grep -q "Bazzite-dev-nvidia"; then
            USE_DISTROBOX=true
            DISTROBOX_NAME="Bazzite-dev-nvidia"
        elif distrobox list 2>/dev/null | grep -q "Bazzite-dev-env"; then
            USE_DISTROBOX=true
            DISTROBOX_NAME="Bazzite-dev-env"
        fi
    fi
fi

# Helper function to run commands in the correct context
run_build_cmd() {
    if [ "$USE_DISTROBOX" = true ]; then
        distrobox enter "$DISTROBOX_NAME" -- "$@"
    else
        "$@"
    fi
}

if [ "$USE_DISTROBOX" = true ]; then
    echo -e "${BLUE}Running build commands inside '$DISTROBOX_NAME' distrobox container...${NC}"
else
    if [ "$INSIDE_CONTAINER" = true ]; then
        echo -e "${BLUE}Running build commands directly inside container...${NC}"
    else
        echo -e "${YELLOW}Warning: Compatible distrobox container not found. Compiling on host...${NC}"
    fi
fi

if [ "$CLEAN_BUILD" = true ]; then
    echo -e "${YELLOW}Clean build requested. Cleaning build artifacts...${NC}"
    run_build_cmd make STEAM=1 clean
fi

# Determine parallel build job factor
if [ -n "$JOBS" ]; then
    NUM_JOBS="$JOBS"
elif command -v nproc &>/dev/null; then
    NUM_JOBS=$(nproc)
elif command -v sysctl &>/dev/null; then
    NUM_JOBS=$(sysctl -n hw.ncpu)
else
    NUM_JOBS=2
fi

# Start build timer
START_TIME=$(date +%s)

# 3. Build the project
echo -e "${GREEN}Compiling RealRTCW (STEAM=1) with ${NUM_JOBS} parallel jobs...${NC}"
if ! run_build_cmd make -j"${NUM_JOBS}" STEAM=1; then
    echo -e "${RED}Build failed! Try a clean build if you encountered configuration issues:${NC}"
    echo -e "${YELLOW}  ./compile.sh --clean${NC}"
    exit 1
fi

echo -e "${GREEN}Deploying compiled binaries to Steam directory...${NC}"
if ! run_build_cmd make STEAM=1 COPYDIR="$TARGET_DIR" copyfiles; then
    echo -e "${RED}Deployment failed!${NC}"
    exit 1
fi

# Also double check and copy launcher helper script if desired
LAUNCHER_SRC="/home/sofia/Desktop/RealRTCW-native-launcher.sh"
LAUNCHER_DEST="$TARGET_DIR/RealRTCW-native-launcher.sh"
if [ -f "$LAUNCHER_SRC" ]; then
    echo -e "${GREEN}Copying launcher helper script to Steam directory...${NC}"
    cp -v "$LAUNCHER_SRC" "$LAUNCHER_DEST"
    chmod +x "$LAUNCHER_DEST"
fi

END_TIME=$(date +%s)
ELAPSED=$((END_TIME - START_TIME))

echo -e "${GREEN}Build and deployment completed in ${ELAPSED}s!${NC}"
echo -e "${BLUE}Executables deployed to:${NC}"
echo -e "  ${YELLOW}$TARGET_DIR/${NC}"

# 4. Optional Auto-Run
if [ "$RUN_AFTER_BUILD" = true ]; then
    if [ -f "$LAUNCHER_DEST" ]; then
        echo -e "${GREEN}Launching RealRTCW with arguments: ${RUN_ARGS[*]}...${NC}"
        # We run the launcher script from the Steam folder
        cd "$TARGET_DIR"
        ./RealRTCW-native-launcher.sh "${RUN_ARGS[@]}"
    else
        echo -e "${RED}Error: Launcher script not found at $LAUNCHER_DEST. Cannot launch game.${NC}"
        exit 1
    fi
fi
