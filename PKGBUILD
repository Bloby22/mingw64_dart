# Maintainer: BlobyCZ
pkgname=mingw-w64-x86_64-dart
pkgver=0.1.7
pkgrel=1

pkgdesc='Dart SDK for MinGW64 (native dart.exe launcher)'
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

_dart_version=3.13.5

_dart_archive="dartsdk-windows-x64-release.zip"

source=(
    "${_dart_archive}::https://storage.googleapis.com/dart-archive/channels/stable/release/${_dart_version}/sdk/${_dart_archive}"
)

sha256sums=(
    'aed8e4a8932ce8fa18ea32f990e43b4a1c9390a4c73777ab6fdf77fc9f4524d1'
)

noextract=(
    "${_dart_archive}"
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

    chmod -R u+w "${srcdir}/build-pkg/dart-sdk"

    echo "==> Building native launcher..."

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

    wchar_t dart[BUF];
    _snwprintf(dart, BUF, L"%ls\\lib\\dart-sdk\\bin\\dart.exe", prefix);
    if (!file_exists(dart)) {
        fwprintf(stderr, L"dart: not found: %ls\n", dart);
        return 1;
    }
    _snwprintf(cmd, len, L"\"%ls\" %ls", dart, rest);
    return run(cmd);
}
EOF

    gcc -O2 -s -municode -o "${srcdir}/dart.exe" "${srcdir}/launcher.c"

    [[ -f "${srcdir}/dart.exe" ]] || return 1
}

package() {
    echo "==> Installing Dart SDK..."
    install -d "${pkgdir}${MINGW_PREFIX}/lib"
    cp -a "${srcdir}/build-pkg/dart-sdk" "${pkgdir}${MINGW_PREFIX}/lib/dart-sdk"

    echo "==> Installing launcher..."
    install -Dm755 "${srcdir}/dart.exe" "${pkgdir}${MINGW_PREFIX}/bin/dart.exe"

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

    # SDK license (BSD)
    if [[ -f "${srcdir}/build-pkg/dart-sdk/LICENSE" ]]; then
        install -Dm644 "${srcdir}/build-pkg/dart-sdk/LICENSE" \
            "${pkgdir}${MINGW_PREFIX}/share/licenses/${pkgname}/DART-LICENSE"
    fi
}
