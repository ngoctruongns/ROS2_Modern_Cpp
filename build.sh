#!/usr/bin/env bash
# Helper build script.
#   ./build.sh 01        -> build topic bắt đầu bằng "01"
#   ./build.sh all       -> build tất cả topic từ root CMake
#   ./build.sh capstone  -> build capstone
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

target="${1:-all}"

if [[ "$target" == "all" ]]; then
  cmake -S "$ROOT" -B "$ROOT/build"
  cmake --build "$ROOT/build" -j"$(nproc)"
  echo "==> Built ALL. Binaries in each topic's build tree under $ROOT/build"
  exit 0
fi

# Tìm thư mục topic khớp tiền tố (vd "01" khớp "01_smart_pointers")
dir=$(find "$ROOT" -maxdepth 1 -type d -name "${target}*" | head -1 || true)
[[ -z "$dir" ]] && dir="$ROOT/$target"
if [[ ! -d "$dir" ]]; then
  echo "Không tìm thấy topic: $target" >&2
  exit 1
fi

cmake -S "$dir" -B "$dir/build"
cmake --build "$dir/build" -j"$(nproc)"
echo "==> Built $dir. Chạy: ls $dir/build"
