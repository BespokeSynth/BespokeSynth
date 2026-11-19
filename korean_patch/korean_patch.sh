#!/bin/sh
# BespokeSynth 한국어 패치 설치/제거 스크립트 (macOS / Linux)
#
#   사용법: sh korean_patch.sh install   [BespokeSynth 리소스 폴더]
#           sh korean_patch.sh uninstall [BespokeSynth 리소스 폴더]
#
# 리소스 폴더(frabk.ttf가 들어 있는 폴더)를 지정하지 않으면 일반적인 설치 위치에서 찾습니다.

set -e

ACTION="${1:-install}"
RESOURCE_DIR="$2"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

say() { printf '%s\n' "$*"; }
fail() { say "오류: $*" >&2; exit 1; }

# 패치 파일 위치: 배포용 zip이면 files/, 저장소에서 바로 실행하면 ../resource
if [ -f "$SCRIPT_DIR/files/tooltips_kor.txt" ]; then
   PAYLOAD_DIR="$SCRIPT_DIR/files"
elif [ -f "$SCRIPT_DIR/../resource/tooltips_kor.txt" ]; then
   PAYLOAD_DIR="$SCRIPT_DIR/../resource"
else
   fail "패치 파일(tooltips_kor.txt, NanumGothic-*.ttf)을 찾을 수 없습니다."
fi

find_resource_dir() {
   for d in \
      "/Applications/BespokeSynth.app/Contents/Resources" \
      "$HOME/Applications/BespokeSynth.app/Contents/Resources" \
      "/usr/share/BespokeSynth/resource" \
      "/usr/local/share/BespokeSynth/resource" \
      "/opt/BespokeSynth/resource" \
      "/opt/bespokesynth/resource"; do
      if [ -f "$d/frabk.ttf" ]; then
         printf '%s' "$d"
         return 0
      fi
   done
   return 1
}

if [ -z "$RESOURCE_DIR" ]; then
   RESOURCE_DIR="$(find_resource_dir)" || fail "BespokeSynth 설치 위치를 찾지 못했습니다. frabk.ttf가 들어 있는 리소스 폴더를 직접 지정하세요.
   예) sh korean_patch.sh $ACTION /path/to/BespokeSynth/resource"
fi
# .app 경로를 넘겨도 되도록
if [ -d "$RESOURCE_DIR/Contents/Resources" ]; then
   RESOURCE_DIR="$RESOURCE_DIR/Contents/Resources"
fi
[ -f "$RESOURCE_DIR/frabk.ttf" ] || fail "'$RESOURCE_DIR'에 frabk.ttf가 없습니다. BespokeSynth 리소스 폴더가 맞는지 확인하세요."

# 쓰기 권한이 없으면 sudo로 파일을 복사
SUDO=""
if [ ! -w "$RESOURCE_DIR" ]; then
   command -v sudo >/dev/null 2>&1 || fail "'$RESOURCE_DIR'에 쓸 권한이 없습니다."
   say "리소스 폴더에 쓰려면 관리자 권한이 필요합니다 (sudo 비밀번호를 물을 수 있습니다)."
   SUDO="sudo"
fi

# 사용자 데이터 폴더 (userprefs.json 위치). bespoke와 같은 규칙을 따릅니다.
find_data_dir() {
   if [ -n "$BESPOKE_DATA_DIR" ]; then
      printf '%s' "$BESPOKE_DATA_DIR"
      return
   fi
   docs="$HOME/Documents"
   if [ "$(uname)" != "Darwin" ] && command -v xdg-user-dir >/dev/null 2>&1; then
      xdg_docs="$(xdg-user-dir DOCUMENTS 2>/dev/null || true)"
      [ -n "$xdg_docs" ] && [ -d "$xdg_docs" ] && docs="$xdg_docs"
   fi
   printf '%s' "$docs/BespokeSynth"
}
DATA_DIR="$(find_data_dir)"
PREFS="$DATA_DIR/userprefs.json"

# userprefs.json의 "tooltips" 값을 설정 (다른 설정은 그대로 둠)
set_tooltips_pref() {
   value="$1"
   if [ ! -f "$PREFS" ]; then
      if [ "$value" = "tooltips_eng.txt" ]; then
         return
      fi
      mkdir -p "$DATA_DIR"
      printf '{\n   "tooltips" : "%s"\n}\n' "$value" > "$PREFS"
      return
   fi
   cp "$PREFS" "$PREFS.bak"
   if grep -q '"tooltips"' "$PREFS"; then
      perl -0pi -e 's/("tooltips"\s*:\s*)"[^"]*"/$1"'"$value"'"/' "$PREFS"
   else
      perl -0pi -e 's/\{/{\n   "tooltips" : "'"$value"'",/' "$PREFS"
   fi
}

install_patch() {
   say "BespokeSynth 리소스 폴더: $RESOURCE_DIR"

   # 원본 폰트 백업 (이미 백업이 있으면 덮어쓰지 않음)
   for f in frabk.ttf frabk_m.ttf; do
      if [ ! -f "$RESOURCE_DIR/$f.orig" ]; then
         $SUDO cp "$RESOURCE_DIR/$f" "$RESOURCE_DIR/$f.orig"
      fi
   done

   # 기존 bespoke는 frabk.ttf만 읽으므로, 한글이 들어 있는 나눔고딕을 같은 이름으로 넣습니다.
   $SUDO cp "$PAYLOAD_DIR/NanumGothic-Regular.ttf" "$RESOURCE_DIR/frabk.ttf"
   $SUDO cp "$PAYLOAD_DIR/NanumGothic-Bold.ttf" "$RESOURCE_DIR/frabk_m.ttf"
   for f in NanumGothic-Regular.ttf NanumGothic-Bold.ttf NanumGothic_OFL.txt tooltips_kor.txt; do
      $SUDO cp "$PAYLOAD_DIR/$f" "$RESOURCE_DIR/$f"
   done

   set_tooltips_pref "tooltips_kor.txt"
   say "설정 파일: $PREFS"

   if [ "$(uname)" = "Darwin" ]; then
      say ""
      say "macOS 참고: 앱 내부 파일을 바꿨기 때문에 실행이 막히면 아래 명령으로 다시 서명하세요."
      say "   codesign --force --deep --sign - \"${RESOURCE_DIR%/Contents/Resources}\""
   fi
   say ""
   say "한국어 패치를 설치했습니다. BespokeSynth를 다시 시작하세요. (툴팁 켜기/끄기: F1)"
}

uninstall_patch() {
   say "BespokeSynth 리소스 폴더: $RESOURCE_DIR"
   for f in frabk.ttf frabk_m.ttf; do
      if [ -f "$RESOURCE_DIR/$f.orig" ]; then
         $SUDO mv "$RESOURCE_DIR/$f.orig" "$RESOURCE_DIR/$f"
      else
         say "경고: $f 원본 백업($f.orig)이 없어 폰트를 복원하지 못했습니다."
      fi
   done
   $SUDO rm -f "$RESOURCE_DIR/tooltips_kor.txt"

   set_tooltips_pref "tooltips_eng.txt"
   say ""
   say "한국어 패치를 제거했습니다. BespokeSynth를 다시 시작하세요."
}

case "$ACTION" in
   install) install_patch ;;
   uninstall) uninstall_patch ;;
   *) fail "알 수 없는 명령 '$ACTION' (install 또는 uninstall)" ;;
esac
