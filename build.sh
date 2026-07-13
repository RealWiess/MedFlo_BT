#!/bin/bash
# MedFlo BT Build Script
# 編譯 MedFlo BT 韌體並產出 FW 檔
# 使用 Keil MDK (ARM Compiler 6) + fromelf + build_fw.py
#
# 環境需求:
#   - Keil MDK 安裝於 C:/Keil_v5
#   - Python 3
#
# 輸出:
#   - tai_evb_YYYYMMDD_HHMMSS_atf.fw (專案根目錄)

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
KEIL_UV4="/c/Keil_v5/UV4/UV4.exe"
FROMELF="/c/Keil_v5/ARM/ARMCLANG/bin/fromelf"
PROJECT_DIR="$SCRIPT_DIR/samples/template_drivers_test"
KEIL_PROJ="$PROJECT_DIR/keil/mdk_clang.uvprojx"
BUILD_DIR="$PROJECT_DIR/keil/build"
FW_OUT_DIR="$PROJECT_DIR/outdir/tai_evb/_firmware"
ZEPHYR_DIR="$PROJECT_DIR/outdir/tai_evb/zephyr"
TMP_DIR="/tmp/medflo_build_$$"

echo "=== MedFlo BT Build ==="
echo "Cleaning previous build..."
rm -rf "$BUILD_DIR"

echo "Compiling with Keil MDK..."
"$KEIL_UV4" -r "$KEIL_PROJ" -o /tmp/medflo_build.log 2>&1
grep -E "Program Size|Error" /tmp/medflo_build.log

if ! grep -q '0 Error(s)' /tmp/medflo_build.log; then
    echo "BUILD FAILED!"
    exit 1
fi

echo "Extracting binary from AXF..."
rm -rf "$TMP_DIR"
mkdir -p "$TMP_DIR"
"$FROMELF" --bin --output="$TMP_DIR" "$BUILD_DIR/zephyr.axf"
cp "$TMP_DIR/ER_VECTOR" "$BUILD_DIR/zephyr.bin"

mkdir -p "$ZEPHYR_DIR"
cp "$TMP_DIR/ER_VECTOR" "$ZEPHYR_DIR/zephyr.bin"
rm -rf "$TMP_DIR"

echo "Packaging firmware..."
cd "$SCRIPT_DIR"
python tools/build_fw.py -t -a template_drivers_test -b tai_evb -s tai -p

# Copy to project root with timestamp
FW_SRC=$(ls -t "$FW_OUT_DIR"/*.fw | head -1)
TIMESTAMP=$(date +%Y%m%d_%H%M%S)
FW_DST="$SCRIPT_DIR/tai_evb_${TIMESTAMP}_atf.fw"
cp "$FW_SRC" "$FW_DST"

echo "=== Build Complete ==="
echo "FW: $FW_DST"
echo "Code: $(grep 'Program Size' /tmp/medflo_build.log | sed 's/Program Size: //')"
