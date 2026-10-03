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
    gcc -O2 -s -municode -o "${srcdir}/dart.exe" "${startdir}/src/launcher.c"

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
    install -Dm644 "${startdir}/LICENSE" \
        "${pkgdir}${MINGW_PREFIX}/share/licenses/${pkgname}/LICENSE-MIT"

    # SDK license (BSD)
    if [[ -f "${srcdir}/build-pkg/dart-sdk/LICENSE" ]]; then
        install -Dm644 "${srcdir}/build-pkg/dart-sdk/LICENSE" \
            "${pkgdir}${MINGW_PREFIX}/share/licenses/${pkgname}/DART-LICENSE"
    fi
}
