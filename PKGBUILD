# Maintainer: BlobyCZ
pkgname=mingw-w64-x86_64-dart
pkgver=0.1.5
pkgrel=1

pkgdesc='Dart and Flutter SDKs for MinGW64 (native flutter.exe and dart.exe launchers)'
arch=('any')
mingw_arch=('x86_64')

url='https://github.com/Bloby22/mingw64_dart'
license=('MIT' 'BSD')

options=('!strip')

depends=(
    'mingw-w64-x86_64-gcc-libs'
)

makedepends=(
    'mingw-w64-x86_64-gcc'
)

_dart_version=3.13.4
_flutter_version=3.47.5

_dart_archive="dartsdk-windows-x64-release.zip"
_flutter_archive="flutter_windows_${_flutter_version}-stable.zip"

source=(
    "${_dart_archive}::https://storage.googleapis.com/dart-archive/channels/stable/release/${_dart_version}/sdk/${_dart_archive}"
    "${_flutter_archive}::https://storage.googleapis.com/flutter_infra_release/releases/stable/windows/${_flutter_archive}"
)

sha256sums=(
    'c38bcecee16b348694d4acc72b3781e5fa0e8766a4d0d1576182c7204ab3d763'
    '0ccd71931f49c2fbe394b1eeb6d79af3d624058a043ea0d03d34160581624fb8'
)

noextract=(
    "${_dart_archive}"
    "${_flutter_archive}"
)

build() {
    mkdir -p "${srcdir}/build-pkg"

    echo "==> Extracting Dart SDK..."
    rm -rf "${srcdir}/build-pkg/dart-sdk"
    bsdtar -xf "${srcdir}/${_dart_archive}" -C "${srcdir}/build-pkg"

    if [[ ! -d "${srcdir}/build-pkg/dart-sdk" ]]; then
        echo "ERROR: Dart SDK extraction failed."
        return 1
    fi

    echo "==> Extracting Flutter SDK..."
    rm -rf "${srcdir}/build-pkg/flutter"
    bsdtar -xf "${srcdir}/${_flutter_archive}" -C "${srcdir}/build-pkg"

    if [[ ! -d "${srcdir}/build-pkg/flutter" ]]; then
        echo "ERROR: Flutter SDK extraction failed."
        return 1
    fi

    chmod -R u+w "${srcdir}/build-pkg/dart-sdk"
    chmod -R u+w "${srcdir}/build-pkg/flutter"

    echo "==> Building native launchers..."

    cat > "${srcdir}/launcher.c" <<'EOF'
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#define BUF (MAX_PATH * 4)

static int file_exists(const wchar_t *p) {
    DWORD a = GetFileAttributesW(p);
    return a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY);
}

/* Command line after argv[0], original quoting preserved. */
static const wchar_t *skip_argv0(const wchar_t *c) {
    if (*c == L'"') {
        c++;
        while (*c && *c != L'"') c++;
        if (*c) c++;
    } else {
        while (*c && *c != L' ' && *c != L'\t') c++;
    }
    while (*c == L' ' || *c == L'\t') c++;
    return c;
}

static void strip_last(wchar_t *p) {
    wchar_t *s = wcsrchr(p, L'\\');
    if (s) *s = 0;
}

static int run(wchar_t *cmd) {
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    DWORD code = 1;

    /* Ctrl+C is handled by the child process */
    SetConsoleCtrlHandler(NULL, TRUE);

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcessW(NULL, cmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        fwprintf(stderr, L"launcher: failed to start process (error %lu)\n",
                 GetLastError());
        return 1;
    }
    WaitForSingleObject(pi.hProcess, INFINITE);
    GetExitCodeProcess(pi.hProcess, &code);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return (int)code;
}

int wmain(void) {
    wchar_t prefix[BUF];
    DWORD n = GetModuleFileNameW(NULL, prefix, BUF);
    if (n == 0 || n >= BUF) {
        fwprintf(stderr, L"launcher: cannot determine executable path\n");
        return 1;
    }
    strip_last(prefix);   /* <prefix>\bin */
    strip_last(prefix);   /* <prefix> */

    const wchar_t *rest = skip_argv0(GetCommandLineW());
    size_t len = wcslen(rest) + 6 * BUF;
    wchar_t *cmd = (wchar_t *)malloc(len * sizeof(wchar_t));
    if (!cmd) return 1;

#ifdef LAUNCH_DART
    wchar_t dart[BUF];
    _snwprintf(dart, BUF, L"%ls\\lib\\dart-sdk\\bin\\dart.exe", prefix);
    if (!file_exists(dart)) {
        fwprintf(stderr, L"dart: not found: %ls\n", dart);
        return 1;
    }
    _snwprintf(cmd, len, L"\"%ls\" %ls", dart, rest);
    return run(cmd);
#else
    wchar_t root[BUF], bat[BUF], dart[BUF], snap[BUF], pkgcfg[BUF];
    _snwprintf(root, BUF, L"%ls\\lib\\flutter", prefix);
    _snwprintf(bat, BUF, L"%ls\\bin\\flutter.bat", root);
    _snwprintf(dart, BUF, L"%ls\\bin\\cache\\dart-sdk\\bin\\dart.exe", root);
    _snwprintf(snap, BUF, L"%ls\\bin\\cache\\flutter_tools.snapshot", root);
    _snwprintf(pkgcfg, BUF,
               L"%ls\\packages\\flutter_tools\\.dart_tool\\package_config.json", root);

    if (file_exists(dart) && file_exists(snap) && file_exists(pkgcfg)) {
        /* Normal run: bundled Dart + Flutter tool snapshot, no cmd.exe */
        SetEnvironmentVariableW(L"FLUTTER_ROOT", root);
        _snwprintf(cmd, len,
                   L"\"%ls\" --disable-dart-dev --packages=\"%ls\" \"%ls\" %ls",
                   dart, pkgcfg, snap, rest);
    } else if (file_exists(bat)) {
        /* First run only: flutter.bat bootstraps the cache */
        _snwprintf(cmd, len, L"cmd.exe /d /s /c \"\"%ls\" %ls\"", bat, rest);
    } else {
        fwprintf(stderr, L"flutter: SDK not found: %ls\n", bat);
        return 1;
    }
    return run(cmd);
#endif
}
EOF

    gcc -O2 -s -municode -DLAUNCH_DART \
        -o "${srcdir}/dart.exe" "${srcdir}/launcher.c"
    gcc -O2 -s -municode \
        -o "${srcdir}/flutter.exe" "${srcdir}/launcher.c"

    [[ -f "${srcdir}/dart.exe" && -f "${srcdir}/flutter.exe" ]] || return 1
}

package() {
    echo "==> Installing Dart SDK..."
    install -d "${pkgdir}${MINGW_PREFIX}/lib"
    cp -a "${srcdir}/build-pkg/dart-sdk" "${pkgdir}${MINGW_PREFIX}/lib/dart-sdk"

    echo "==> Installing Flutter SDK..."
    cp -a "${srcdir}/build-pkg/flutter" "${pkgdir}${MINGW_PREFIX}/lib/flutter"

    echo "==> Installing launchers..."
    install -Dm755 "${srcdir}/dart.exe" "${pkgdir}${MINGW_PREFIX}/bin/dart.exe"
    install -Dm755 "${srcdir}/flutter.exe" "${pkgdir}${MINGW_PREFIX}/bin/flutter.exe"

    # Licenses
    install -d "${pkgdir}${MINGW_PREFIX}/share/licenses/${pkgname}"

    # Project license (MIT)
    cat > "${pkgdir}${MINGW_PREFIX}/share/licenses/${pkgname}/LICENSE-MIT" <<'EOF'
Copyright © 2026 BlobyCZ

Permission is hereby granted, free of charge, to any person obtaining a copy of this software
and associated documentation files (the “Software”), to deal in the Software without
restriction, including without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
EOF

    # SDK licenses (BSD)
    if [[ -f "${srcdir}/build-pkg/dart-sdk/LICENSE" ]]; then
        install -Dm644 "${srcdir}/build-pkg/dart-sdk/LICENSE" \
            "${pkgdir}${MINGW_PREFIX}/share/licenses/${pkgname}/DART-LICENSE"
    fi

    if [[ -f "${srcdir}/build-pkg/flutter/LICENSE" ]]; then
        install -Dm644 "${srcdir}/build-pkg/flutter/LICENSE" \
            "${pkgdir}${MINGW_PREFIX}/share/licenses/${pkgname}/FLUTTER-LICENSE"
    fi
}
