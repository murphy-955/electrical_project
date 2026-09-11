#!/bin/bash
# 用法: .tools/compile.sh <xxx.c> [fqbn]
# 将 Arduino 草图(.c)复制为临时 .ino 并编译检查，默认目标板为 arduino:avr:uno
set -e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
CLI_DIR="$ROOT/.tools/arduino-cli"
CLI="$CLI_DIR/arduino-cli"
CONF="$CLI_DIR/arduino-cli.yaml"

UPLOAD=0
if [ "$1" = "--upload" ] || [ "$1" = "-u" ]; then
  UPLOAD=1
  shift
fi

SRC="$1"
FQBN="${2:-arduino:avr:uno}"
PORT="${ARDUINO_PORT:-/dev/ttyUSB0}"
if [ -z "$SRC" ]; then
  echo "用法: $0 [--upload] <xxx.c> [fqbn]  (上传端口用环境变量 ARDUINO_PORT 指定, 默认 /dev/ttyUSB0)" >&2
  exit 1
fi

SKETCH_DIR="$(mktemp -d)"
trap 'rm -rf "$SKETCH_DIR"' EXIT
NAME="$(basename "$SRC" .c)"
mkdir -p "$SKETCH_DIR/$NAME"
cp "$SRC" "$SKETCH_DIR/$NAME/$NAME.ino"

if [ "$UPLOAD" = "1" ]; then
  "$CLI" --config-file "$CONF" compile --fqbn "$FQBN" --upload -p "$PORT" "$SKETCH_DIR/$NAME"
else
  "$CLI" --config-file "$CONF" compile --fqbn "$FQBN" "$SKETCH_DIR/$NAME"
fi
