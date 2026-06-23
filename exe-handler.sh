#!/bin/bash
# FiveMLinuxSDK - .exe File Handler
# Called when user double-clicks a .exe file
# Registers Wine as handler for .exe files

FML_DIR="$HOME/.fivem-linux"
PREFIX_DIR="$FML_DIR/prefix"
LOG_FILE="$FML_DIR/exe-handler.log"

log() { echo "[$(date '+%H:%M:%S')] $1" >> "$LOG_FILE"; }

setup_exe_handler() {
    log "Setting up .exe file handler..."

    mkdir -p "$HOME/.local/share/mime/packages"
    mkdir -p "$HOME/.local/share/applications"
    mkdir -p "$HOME/.config/mimeapps.list"

    # MIME type for .exe files
    cat > "$HOME/.local/share/mime/packages/fivem-exe.xml" << 'MIMEEOF'
<?xml version="1.0" encoding="UTF-8"?>
<mime-info xmlns="http://www.freedesktop.org/standards/shared-mime-info">
  <mime-type type="application/x-ms-dos-executable">
    <comment>Windows Executable</comment>
    <glob pattern="*.exe"/>
  </mime-type>
</mime-info>
MIMEEOF

    # Desktop entry for Wine
    cat > "$HOME/.local/share/applications/wine-exe.desktop" << 'DESKTOPEOF'
[Desktop Entry]
Name=Run with Wine
Comment=Run Windows executable with Wine
Exec=wine %f
Icon=wine
Terminal=false
Type=Application
MimeType=application/x-ms-dos-executable;
Categories=Utility;
DESKTOPEOF

    # Desktop entry for FiveM specifically
    cat > "$HOME/.local/share/applications/fivem-launcher.desktop" << 'FIVEMEOF'
[Desktop Entry]
Name=FiveM (Linux)
Comment=Launch FiveM via Wine
Exec=bash -c 'export WINEPREFIX=$HOME/.fivem-linux/prefix; export WINEDEBUG=-all; wine %f'
Icon=application-x-executable
Terminal=false
Type=Application
MimeType=application/x-ms-dos-executable;
Categories=Game;
FIVEMEOF

    # Update MIME database
    update-mime-database "$HOME/.local/share/mime" 2>/dev/null

    # Set default handler
    if ! grep -q "application/x-ms-dos-executable" "$HOME/.config/mimeapps.list" 2>/dev/null; then
        echo "[Default Applications]" >> "$HOME/.config/mimeapps.list"
        echo "application/x-ms-dos-executable=wine-exe.desktop" >> "$HOME/.config/mimeapps.list"
    fi

    log "Handler setup complete"
}

show_help() {
    echo "FiveMLinuxSDK - .exe Handler Setup"
    echo ""
    echo "Usage: $0 [command]"
    echo ""
    echo "Commands:"
    echo "  setup      Register .exe file handler"
    echo "  status     Show handler status"
    echo ""
}

case "${1:-setup}" in
    setup)
        setup_exe_handler
        echo "Done! .exe files will now open with Wine."
        echo "Right-click any .exe > 'Open with Other Application' > 'Run with Wine'"
        ;;
    status)
        echo "MIME type registered: $(grep -c 'x-ms-dos-executable' "$HOME/.local/share/mime/packages/"*.xml 2>/dev/null || echo 0)"
        echo "Default handler: $(grep 'x-ms-dos-executable' "$HOME/.config/mimeapps.list" 2>/dev/null | head -1 || echo 'Not set')"
        ;;
    *)
        show_help
        ;;
esac
