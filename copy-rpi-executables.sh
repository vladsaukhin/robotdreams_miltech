#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="${1:-build/aarch64-debug}"
REMOTE_HOST="${REMOTE_HOST:-rpi4vs}"
REMOTE_DIR="${REMOTE_DIR:-/home/vsaukhin54/rpi-run/}"

if [[ ! -d "$BUILD_DIR" ]]; then
  echo "Build directory does not exist: $BUILD_DIR"
  exit 1
fi

if ! command -v file >/dev/null 2>&1; then
  echo "'file' command is required. Install it with: sudo apt install file"
  exit 1
fi

TMP_FILE_LIST="$(mktemp)"
trap 'rm -f "$TMP_FILE_LIST"' EXIT

echo "Searching for ELF executables in: $BUILD_DIR"

while IFS= read -r -d '' file_path; do
  if file "$file_path" | grep -qE 'ELF .*executable'; then
    relative_path="${file_path#"$BUILD_DIR"/}"
    printf '%s\0' "$relative_path" >> "$TMP_FILE_LIST"
    echo "  found: $relative_path"
  fi
done < <(find "$BUILD_DIR" -type f -print0)

if [[ ! -s "$TMP_FILE_LIST" ]]; then
  echo "No executable ELF files found in $BUILD_DIR"
  exit 0
fi

echo "Creating remote directory: $REMOTE_HOST:$REMOTE_DIR"
ssh "$REMOTE_HOST" "mkdir -p '$REMOTE_DIR'"

echo "Copying executables to Raspberry Pi..."
rsync -av --from0 --files-from="$TMP_FILE_LIST" "$BUILD_DIR"/ "$REMOTE_HOST:$REMOTE_DIR"/

echo "Making copied files executable on Raspberry Pi..."
ssh "$REMOTE_HOST" "cd '$REMOTE_DIR' && xargs -0 chmod +x" < "$TMP_FILE_LIST"

echo "Done."
echo "Copied to: $REMOTE_HOST:$REMOTE_DIR"