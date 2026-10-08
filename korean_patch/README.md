# BespokeSynth 한국어 패치 (이미 설치한 사용자용)

이미 설치되어 있는 BespokeSynth에 **한국어 툴팁**과 **한글 폰트**를 추가하는 패치입니다.
BespokeSynth를 다시 설치하거나 빌드할 필요가 없습니다.

- 모듈과 컨트롤에 마우스를 올리면 나오는 설명(툴팁)이 한국어로 표시됩니다.
- 메뉴, 버튼, 모듈 이름 등은 프로그램에 영어로 들어 있어 영어로 남습니다.
- 한글을 표시하기 위해 BespokeSynth의 기본 글꼴을 **나눔고딕**으로 바꿉니다. 영문 글자 모양도 나눔고딕으로 바뀝니다.

## 패치 내용

| 파일 | 설명 |
|---|---|
| `files/tooltips_kor.txt` | 한국어 툴팁 (영어 툴팁 전체 번역) |
| `files/NanumGothic-Regular.ttf`, `files/NanumGothic-Bold.ttf` | 나눔고딕 글꼴 (SIL Open Font License 1.1) |
| `files/NanumGothic_OFL.txt` | 나눔고딕 라이선스 |

설치 스크립트가 하는 일:

1. BespokeSynth의 `resource` 폴더를 찾습니다.
2. 원래 글꼴 `frabk.ttf`, `frabk_m.ttf`를 `frabk.ttf.orig`, `frabk_m.ttf.orig`로 백업합니다.
3. 나눔고딕을 `frabk.ttf`, `frabk_m.ttf`라는 이름으로 넣고, `tooltips_kor.txt`를 복사합니다.
   (기존 BespokeSynth는 `frabk.ttf`라는 이름의 글꼴만 읽기 때문에 같은 이름으로 넣어야 한글이 보입니다.)
4. 설정 파일 `userprefs.json`의 `tooltips` 항목을 `tooltips_kor.txt`로 바꿉니다. 다른 설정은 건드리지 않습니다.

## 설치하기

먼저 **BespokeSynth를 종료**하세요.

### Windows

1. `BespokeSynth-Korean-Patch.zip`의 압축을 풉니다.
2. `install_windows.bat`을 더블클릭합니다.
3. "이 앱이 디바이스를 변경하도록 허용하시겠어요?" 창이 뜨면 **예**를 누릅니다.
   (`C:\Program Files\BespokeSynth` 안의 파일을 바꾸려면 관리자 권한이 필요합니다.)
4. "한국어 패치를 설치했습니다"가 나오면 BespokeSynth를 실행합니다.

BespokeSynth를 기본 위치가 아닌 곳에 설치했다면 설치 위치를 물어봅니다.
`resource` 폴더 경로(예: `D:\BespokeSynth\resource`)를 입력하세요.

> "Windows의 PC 보호" 창이 뜨면 **추가 정보 → 실행**을 누르세요.

### macOS

1. `BespokeSynth-Korean-Patch.zip`의 압축을 풉니다.
2. **터미널**을 열고 `sh `(뒤에 공백 포함)를 입력한 뒤, 압축을 푼 폴더의 `korean_patch.sh`를 터미널 창으로 끌어다 놓고, 이어서 ` install`을 입력해 Enter를 누릅니다.
   ```sh
   sh ~/Downloads/BespokeSynth-Korean-Patch/korean_patch.sh install
   ```
3. `/Applications`가 아닌 곳에 BespokeSynth.app이 있다면 앱 경로를 함께 적습니다.
   ```sh
   sh korean_patch.sh install /경로/BespokeSynth.app
   ```

> **macOS에서 앱이 열리지 않을 때**: 앱 안의 파일을 바꿨기 때문에 macOS가 실행을 막을 수 있습니다. 터미널에서 아래 명령으로 다시 서명하면 됩니다.
> ```sh
> codesign --force --deep --sign - /Applications/BespokeSynth.app
> ```
> "터미널이 다른 앱을 수정하는 것을 막았습니다" 같은 알림이 뜨면 **시스템 설정 → 개인정보 보호 및 보안 → 앱 관리**에서 터미널을 허용하세요.

### Linux

```sh
sh korean_patch.sh install
```

`/usr/share/BespokeSynth/resource` 등 일반적인 설치 위치를 자동으로 찾습니다. 관리자 권한이 필요하면 `sudo` 비밀번호를 묻습니다.
압축판(포터블) 등 다른 곳에 설치했다면 `resource` 폴더를 직접 지정하세요.

```sh
sh korean_patch.sh install ~/apps/BespokeSynth/resource
```

> Flatpak으로 설치한 경우 설치 폴더가 읽기 전용이라 이 패치를 쓸 수 없습니다.

## 확인하기

BespokeSynth를 실행하고 아무 모듈이나 컨트롤 위에 마우스를 올려 보세요. 한국어 설명이 나오면 성공입니다.
툴팁이 안 보이면 **F1** 키를 눌러 툴팁을 켜세요.

## 되돌리기 (제거)

- **Windows**: `uninstall_windows.bat`을 더블클릭합니다.
- **macOS / Linux**: `sh korean_patch.sh uninstall` (설치할 때 경로를 지정했다면 같은 경로를 뒤에 붙입니다)

원래 글꼴을 백업에서 복원하고, 툴팁 설정을 영어(`tooltips_eng.txt`)로 되돌립니다.

## 수동 설치

스크립트를 쓰지 않고 직접 설치할 수도 있습니다.

1. BespokeSynth의 `resource` 폴더를 엽니다.
   - Windows: `C:\Program Files\BespokeSynth\resource`
   - macOS: `BespokeSynth.app` 우클릭 → **패키지 내용 보기** → `Contents/Resources`
   - Linux: `/usr/share/BespokeSynth/resource`
2. `frabk.ttf`, `frabk_m.ttf`를 다른 곳에 백업합니다.
3. `files/NanumGothic-Regular.ttf`를 `frabk.ttf`로, `files/NanumGothic-Bold.ttf`를 `frabk_m.ttf`로 이름을 바꿔 덮어씁니다.
4. `files/tooltips_kor.txt`를 `resource` 폴더에 복사합니다.
5. BespokeSynth를 실행하고 **settings → paths** 탭의 `tooltips` 칸을 `tooltips_kor.txt`로 바꾼 뒤 저장하고 다시 시작합니다.
   (또는 `문서/BespokeSynth/userprefs.json`에서 `"tooltips"` 값을 `"tooltips_kor.txt"`로 바꿉니다.)

## 자주 묻는 질문

**한글이 네모(□)로 보여요.**
글꼴 교체가 안 된 상태입니다. 설치 스크립트를 다시 실행하거나, 위의 수동 설치 3번을 확인하세요.

**툴팁이 아예 안 나와요.**
F1 키로 툴팁을 켜세요. 그래도 안 나오면 `resource` 폴더에 `tooltips_kor.txt`가 있는지 확인하세요. 설정은 한국어인데 파일이 없으면 툴팁이 표시되지 않습니다.

**BespokeSynth를 업데이트했더니 영어로 돌아갔어요.**
업데이트하면 `resource` 폴더가 새 파일로 바뀝니다. 패치를 다시 설치하세요.

**설정 파일은 어디에 있나요?**
`문서(Documents)/BespokeSynth/userprefs.json`입니다. `BESPOKE_DATA_DIR` 환경 변수를 설정했다면 그 폴더에 있습니다. 스크립트는 바꾸기 전에 `userprefs.json.bak`으로 백업합니다.

## 라이선스

나눔고딕은 NHN Corporation의 글꼴로, SIL Open Font License 1.1에 따라 배포됩니다 (`files/NanumGothic_OFL.txt`).
글꼴 파일은 수정하지 않았고, 기존 BespokeSynth가 읽을 수 있도록 파일 이름만 바꿔 설치합니다.
