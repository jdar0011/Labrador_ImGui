#!/bin/bash
set -e

# CONFIG
APP_NAME="LabradorImgui"
BUILD_DIR="build"
APP_BUNDLE_DIR="mac_appbundle/${APP_NAME}.app"
CONTENTS_DIR="${APP_BUNDLE_DIR}/Contents"
MACOS_DIR="${CONTENTS_DIR}/MacOS"
RESOURCES_DIR="${CONTENTS_DIR}/Resources"
INFO_PLIST="${CONTENTS_DIR}/Info.plist"

# 1. Build the project
echo "Building project..."
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
cmake ..
cmake --build .
cd ..

# 2. Create .app bundle structure
echo "Creating .app bundle..."
mkdir -p "$MACOS_DIR"
mkdir -p "$RESOURCES_DIR"

# 3. Copy executable into MacOS directory
echo "Copying executable..."
cp "$BUILD_DIR/$APP_NAME" "$MACOS_DIR/"

# 4. Copy resources into Resources directory
echo "Copying resources..."
cp -R misc/media "$RESOURCES_DIR/"
cp -R misc/fonts "$RESOURCES_DIR/"
cp -R README.md "$RESOURCES_DIR/"

# 5. Copy Info.plist if it exists
if [ ! -f "$INFO_PLIST" ]; then
    echo "Creating default Info.plist..."
    cat <<EOL > "$INFO_PLIST"
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN"
 "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>CFBundleName</key>
  <string>${APP_NAME}</string>
  <key>CFBundleDisplayName</key>
  <string>${APP_NAME}</string>
  <key>CFBundleIdentifier</key>
  <string>com.example.${APP_NAME}</string>
  <key>CFBundleVersion</key>
  <string>1.0</string>
  <key>CFBundleExecutable</key>
  <string>${APP_NAME}</string>
  <key>CFBundlePackageType</key>
  <string>APPL</string>
  <key>CFBundleIconFile</key>
  <string>media/iconfile</string>
</dict>
</plist>
EOL
fi

# 6. Optionally create a .dmg
DMG_NAME="${APP_NAME}.dmg"
echo "Creating DMG..."
hdiutil create -volname "$APP_NAME" -srcfolder "$APP_BUNDLE_DIR" -ov -format UDZO "$DMG_NAME"

echo "Packaging complete!"
echo "You can distribute ${DMG_NAME} or the ${APP_BUNDLE_DIR} bundle."
