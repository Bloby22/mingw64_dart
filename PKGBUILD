# Maintainer: BlobyCZ
pkgname=mingw-w64-x86_64-mingw64-dart
pkgver=0.1.0
pkgrel=1
pkgdesc='CLI tool for downloading Dart SDK and Flutter release info from Google Cloud Storage'
arch=('x86_64')
url='https://github.com/BlobyCZ/mingw64_dart'
license=('MIT')
depends=('mingw-w64-x86_64-curl' 'mingw-w64-x86_64-gcc-libs')

build() {
    cd "$startdir"
    mkdir -p build-pkg
    g++ -std=c++20 -O2 -DNDEBUG \
        -I"$startdir/include" \
        "$startdir/src/API/google_storage.cpp" \
        "$startdir/src/API/release.cpp" \
        "$startdir/src/Utils/files.cpp" \
        "$startdir/src/Utils/json.cpp" \
        "$startdir/src/Utils/strings.cpp" \
        "$startdir/src/release_client.cpp" \
        -o "build-pkg/release_client.exe" \
        -lcurl
}

package() {
    install -Dm755 "$startdir/build-pkg/release_client.exe" "$pkgdir/mingw64/bin/release_client.exe"
    install -Dm644 "$startdir/LICENSE" "$pkgdir/mingw64/share/licenses/$pkgname/LICENSE"
}
