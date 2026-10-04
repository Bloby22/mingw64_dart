# Maintainer: BlobyCZ
_dart_version=3.13.5
_realname=dart
pkgname="${MINGW_PACKAGE_PREFIX}-${_realname}"
pkgver=${_dart_version//-/_}
pkgrel=1

pkgdesc='Dart SDK for MinGW64 (native dart.exe launcher)'
arch=('any')
mingw_arch=('mingw64')
url='https://github.com/Bloby22/mingw64_dart'
license=('spdx:MIT' 'spdx:BSD-3-Clause')
options=('!strip')

depends=("${MINGW_PACKAGE_PREFIX}-gcc-libs")
makedepends=("${MINGW_PACKAGE_PREFIX}-gcc")

_dart_archive="dartsdk-windows-x64-release.zip"

source=(
    "${_dart_archive}::https://storage.googleapis.com/dart-archive/channels/stable/release/${_dart_version}/sdk/${_dart_archive}"
    "launcher.c"
    "LICENSE"
)
sha256sums=(
    'aed8e4a8932ce8fa18ea32f990e43b4a1c9390a4c73777ab6fdf77fc9f4524d1'
    'SKIP'
    'SKIP'
)
noextract=("${_dart_archive}")

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
    gcc -O2 -s -municode -o "${srcdir}/dart.exe" "${srcdir}/launcher.c"

    [[ -f "${srcdir}/dart.exe" ]] || return 1
}

package() {
    echo "==> Installing Dart SDK..."
    install -d "${pkgdir}${MINGW_PREFIX}/lib"
    cp -a "${srcdir}/build-pkg/dart-sdk" "${pkgdir}${MINGW_PREFIX}/lib/dart-sdk"

    echo "==> Installing launcher..."
    install -Dm755 "${srcdir}/dart.exe" "${pkgdir}${MINGW_PREFIX}/bin/dart.exe"

    # Project license (MIT)
    install -Dm644 "${srcdir}/LICENSE" \
        "${pkgdir}${MINGW_PREFIX}/share/licenses/${pkgname}/LICENSE-MIT"

    # SDK license (BSD)
    if [[ -f "${srcdir}/build-pkg/dart-sdk/LICENSE" ]]; then
        install -Dm644 "${srcdir}/build-pkg/dart-sdk/LICENSE" \
            "${pkgdir}${MINGW_PREFIX}/share/licenses/${pkgname}/DART-LICENSE"
    else
        echo "WARNING: Dart SDK LICENSE not found."
    fi
}
