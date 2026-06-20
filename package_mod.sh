#!/usr/bin/env bash

# Exit immediately if a command exits with a non-zero status
set -e

# Get the directory of this script
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

# Color codes
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[0;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

MOD_NAME="flashlight_mod"
OUTPUT_DIR="build/$MOD_NAME"
ZIP_NAME="z_flashlight.pk3"

echo -e "${BLUE}=== RealRTCW Mod Packaging Script ===${NC}"

# 1. Clean and create output mod directory structure
echo -e "Creating mod directory structure..."
rm -rf "$OUTPUT_DIR"
mkdir -p "$OUTPUT_DIR"

# 2. Copy compiled binary shared libraries (Linux)
echo -e "Copying compiled shared libraries (Linux)..."
SO_SRC_DIR="build/release-linux-x86_64-steam/main"

if [ -f "$SO_SRC_DIR/cgame.sp.x86_64.so" ] && \
   [ -f "$SO_SRC_DIR/qagame.sp.x86_64.so" ] && \
   [ -f "$SO_SRC_DIR/ui.sp.x86_64.so" ]; then
    cp -v "$SO_SRC_DIR"/*.so "$OUTPUT_DIR/"
else
    echo -e "${RED}Error: Compiled shared libraries (.so) not found in $SO_SRC_DIR!${NC}"
    echo "Please compile the project first using ./compile.sh"
    exit 1
fi

# 2b. Copy compiled Windows DLLs if they exist
DLL_SRC_DIR="build/release-mingw32-x86_64-steam/main"
echo -e "Checking for compiled Windows DLLs..."
if [ -f "$DLL_SRC_DIR/cgame_sp_x64.dll" ] && \
   [ -f "$DLL_SRC_DIR/qagame_sp_x64.dll" ] && \
   [ -f "$DLL_SRC_DIR/ui_sp_x64.dll" ]; then
    echo -e "Copying compiled Windows DLLs..."
    cp -v "$DLL_SRC_DIR"/*.dll "$OUTPUT_DIR/"
else
    echo -e "${YELLOW}Warning: Windows DLLs not found in $DLL_SRC_DIR. Mod will only run on Linux.${NC}"
fi

# 3. Create description file for Addons / Mods menu
echo -e "Generating description.txt..."
cat << 'EOF' > "$OUTPUT_DIR/description.txt"
WWII Realistic Flashlight Mod
-----------------------------
Adds a WWII-style warm incandescent flashlight (toggle with /flashlight command).
Includes realistic stealth-breaking AI detection rules:
- AI sees the light beam from up to 800 units if looking towards you.
- Shining directly on an AI's back alerts them from up to 600 units.
- Ambient room glow alerts nearby AI within 250 units.
EOF

# 4. Zip the description file using Store method (no compression) into pk3
echo -e "Packaging $ZIP_NAME..."
cd "$OUTPUT_DIR"
zip -0 "$ZIP_NAME" description.txt
rm description.txt
cd "$SCRIPT_DIR"

echo -e "${GREEN}Mod successfully packaged!${NC}"
echo -e "Packaged folder location: ${YELLOW}$SCRIPT_DIR/$OUTPUT_DIR/${NC}"
echo ""
echo -e "To test locally, copy the entire ${YELLOW}$MOD_NAME/${NC} folder to your RealRTCW directory:"
echo -e "  ${BLUE}cp -r $SCRIPT_DIR/$OUTPUT_DIR /run/media/system/NVME_GAME_1/SteamLibrary/steamapps/common/RealRTCW/${NC}"
echo ""
echo -e "Then run the game with the addon enabled:"
echo -e "  ${BLUE}./RealRTCW-native-launcher.sh +set fs_game $MOD_NAME${NC}"
echo ""
echo -e "For Steam Workshop submission, use the Steam Workshop Uploader and point it to the packaged folder."
