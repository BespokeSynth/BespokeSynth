#!/bin/sh
# 배포용 한국어 패치 zip을 만듭니다 (이미 BespokeSynth를 설치한 사용자용).
#
#   사용법: sh korean_patch/make_package.sh [출력 폴더]
#   결과물: BespokeSynth-Korean-Patch.zip

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
OUT_DIR="$(cd "${1:-$REPO_DIR}" && pwd)"
NAME="BespokeSynth-Korean-Patch"

STAGE="$(mktemp -d)"
trap 'rm -rf "$STAGE"' EXIT

mkdir -p "$STAGE/$NAME/files"
cp "$SCRIPT_DIR/README.md" "$STAGE/$NAME/README.md"
cp "$SCRIPT_DIR/install_windows.bat" "$SCRIPT_DIR/uninstall_windows.bat" "$SCRIPT_DIR/korean_patch_windows.ps1" "$SCRIPT_DIR/korean_patch.sh" "$STAGE/$NAME/"
for f in tooltips_kor.txt NanumGothic-Regular.ttf NanumGothic-Bold.ttf NanumGothic_OFL.txt; do
   cp "$REPO_DIR/resource/$f" "$STAGE/$NAME/files/$f"
done

rm -f "$OUT_DIR/$NAME.zip"
(cd "$STAGE" && zip -qr "$OUT_DIR/$NAME.zip" "$NAME")
echo "만들었습니다: $OUT_DIR/$NAME.zip"
