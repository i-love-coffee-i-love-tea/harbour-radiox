#!/bin/bash
# Build harbour-radiox for Sailfish OS via sfdk shadow build.
#
# Prerequisites:
#   - Sailfish SDK installed with Docker engine
#   - sfdk in PATH (e.g. ~/SailfishOS/bin/sfdk)
#   - Build target set: e.g. SailfishOS-5.1.0.11-aarch64

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
TARGET="${SAILFISH_TARGET:-SailfishOS-5.1.0.11-aarch64}"
BUILD_DIR="$SCRIPT_DIR/build"

mkdir -p "$BUILD_DIR"

# Symlink source directories into build dir so the RPM spec's relative paths work
for d in src qml rpm; do
    [ ! -e "$BUILD_DIR/$d" ] && ln -s "$SCRIPT_DIR/$d" "$BUILD_DIR/$d"
done

echo "=== Building harbour-radiox via sfdk (target: $TARGET) ==="
cd "$BUILD_DIR"
sfdk -c target="$TARGET" build "$SCRIPT_DIR"

echo "=== Copying RPM to rpms/ ==="
mkdir -p "$SCRIPT_DIR/rpms"
if ls "$BUILD_DIR"/RPMS/*.rpm >/dev/null 2>&1; then
    mv "$BUILD_DIR"/RPMS/*.rpm "$SCRIPT_DIR/rpms/"
fi

echo "=== Done ==="
ls -lh "$SCRIPT_DIR/rpms/"*.rpm 2>/dev/null || true