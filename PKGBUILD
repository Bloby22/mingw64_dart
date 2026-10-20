# Maintainer: BlobyCZ
_dart_version=3.13.5
_realname=dart
pkgname="${MINGW_PACKAGE_PREFIX}-${_realname}"
pkgver=3.13.5
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
    local sdk_dir="${srcdir}/build-pkg/dart-sdk"

    echo "==> Preparing build directory..."
    mkdir -p "${srcdir}/build-pkg"

    echo "==> Extracting Dart SDK..."
    rm -rf "${sdk_dir}"
    bsdtar -xf "${srcdir}/${_dart_archive}" \
        -C "${srcdir}/build-pkg"

    if [[ ! -d "${sdk_dir}" ]]; then
        echo "ERROR: Dart SDK extraction failed."
        return 1
    fi

    chmod -R u+w "${sdk_dir}"

    echo "==> Building native launcher..."
    gcc -O2 -s -municode \
        -o "${srcdir}/dart.exe" \
        "${srcdir}/launcher.c"

    if [[ ! -f "${srcdir}/dart.exe" ]]; then
        echo "ERROR: Launcher compilation failed."
        return 1
    fi
}

package() {
    local sdk_dir="${srcdir}/build-pkg/dart-sdk"
    local install_dir="${pkgdir}${MINGW_PREFIX}"
    local license_dir="${install_dir}/share/licenses/${pkgname}"

    echo "==> Checking build outputs..."
    [[ -d "${sdk_dir}" ]] || {
        echo "ERROR: Dart SDK directory not found."
        return 1
    }

    [[ -f "${srcdir}/dart.exe" ]] || {
        echo "ERROR: dart.exe launcher not found."
        return 1
    }

    echo "==> Installing Dart SDK..."
    install -d "${install_dir}/lib"
    cp -a "${sdk_dir}" "${install_dir}/lib/dart-sdk"

    echo "==> Installing launcher..."
    install -Dm755 "${srcdir}/dart.exe" \
        "${install_dir}/bin/dart.exe"

    echo "==> Installing project license..."
    install -Dm644 "${srcdir}/LICENSE" \
        "${license_dir}/LICENSE-MIT"

    echo "==> Installing Dart SDK license..."
    if [[ -f "${sdk_dir}/LICENSE" ]]; then
        install -Dm644 "${sdk_dir}/LICENSE" \
            "${license_dir}/DART-LICENSE"
    else
        echo "WARNING: Dart SDK LICENSE not found."
    fi
}
