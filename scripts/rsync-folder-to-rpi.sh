#!/usr/bin/env bash
set -euo pipefail

REMOTE_HOST="${REMOTE_HOST:-rpi4vs}"
REMOTE_BASE_DIR="${REMOTE_BASE_DIR:-/home/vsaukhin54/remote-sync}"

if [[ $# -lt 1 ]]; then
  echo "Usage:"
  echo "  $0 <local-folder> [remote-folder-name]"
  echo
  echo "Examples:"
  echo "  $0 src"
  echo "  $0 ./src my-project-src"
  echo "  REMOTE_BASE_DIR=/home/vlad/rpi-run/my-project $0 ./config config"
  exit 1
fi

LOCAL_FOLDER="$1"

if [[ ! -d "$LOCAL_FOLDER" ]]; then
  echo "Error: local folder does not exist: $LOCAL_FOLDER"
  exit 1
fi

LOCAL_FOLDER="$(realpath "$LOCAL_FOLDER")"
LOCAL_FOLDER_NAME="$(basename "$LOCAL_FOLDER")"

REMOTE_FOLDER_NAME="${2:-$LOCAL_FOLDER_NAME}"
REMOTE_DIR="${REMOTE_BASE_DIR}/${REMOTE_FOLDER_NAME}"

echo "[rsync] Local folder:  $LOCAL_FOLDER"
echo "[rsync] Remote host:   $REMOTE_HOST"
echo "[rsync] Remote folder: $REMOTE_DIR"

ssh "$REMOTE_HOST" "mkdir -p '$REMOTE_DIR'"

rsync -az --delete \
  --info=progress2 \
  --exclude='.git' \
  --exclude='.vscode' \
  --exclude='.devcontainer' \
  --exclude='build' \
  --exclude='build-*' \
  --exclude='cmake-build-*' \
  --exclude='*.o' \
  --exclude='*.a' \
  --exclude='*.so' \
  --exclude='*.d' \
  "$LOCAL_FOLDER"/ "$REMOTE_HOST:$REMOTE_DIR"/

echo "[rsync] Done."