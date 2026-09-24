# mingw64_dart

CLI tool for downloading Dart SDK archives and Flutter release info from Google
Cloud Storage, built for MSYS2 / MinGW-w64 environments. Packaged as
`mingw-w64-x86_64-mingw64-dart`.

## Features

- List Flutter SDK releases (`releases_<platform>.json`) by platform, channel
  and limit
- Show the latest Dart SDK version for a channel (stable / beta / dev)
- Download `dartsdk-<os>-<arch>-release.zip` with a progress bar and automatic
  SHA-256 verification when a checksum file exists
- Compute SHA-256 of any local file

## Requirements

- C++20 compiler (GCC / MinGW-w64)
- CMake >= 3.20
- libcurl (development headers), e.g. `mingw-w64-x86_64-curl` on MSYS2
- Perl (only for the helper scripts in `scripts/`)

## Build

```sh
make build          # configure + build via CMake (build/)
# or manually:
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build --config Release
```

Other targets: `make test`, `make rebuild`, `make clean`, `make help`.

On Windows the CMake setup auto-detects curl under `$MINGW_PREFIX` and common
MSYS2 locations (`C:/msys64/mingw64`, `D:/msys64/mingw64`, ...).

## Packaging (MSYS2 / PKGBUILD)

```sh
makepkg -f          # builds build-pkg/release_client.exe and installs into pkg/
```

`PKGBUILD` compiles the sources directly with `g++ -std=c++20 -O2` and packages
the binary into `mingw64/bin/release_client.exe` plus the license file.

### Helper scripts

Two Perl scripts in `scripts/` help maintain the package:

```sh
perl scripts/bump-version.pl --patch      # 0.1.0 -> 0.1.1, pkgrel reset to 1
perl scripts/bump-version.pl --minor      # 0.1.0 -> 0.2.0
perl scripts/bump-version.pl --major      # 0.1.0 -> 1.0.0
perl scripts/bump-version.pl 0.2.0        # set an explicit version
perl scripts/bump-version.pl --bump-pkgrel  # same version, pkgrel + 1
```

`bump-version.pl` updates `pkgver`/`pkgrel` in `PKGBUILD` and syncs
`project(... VERSION ...)` in `CMakeLists.txt` when present.

```sh
perl scripts/makepkg-check.pl             # rebuild like PKGBUILD does + verify
perl scripts/makepkg-check.pl --no-build  # verify existing build-pkg/ only
```

`makepkg-check.pl` mirrors the PKGBUILD `g++` invocation, checks that all
sources exist, verifies the artifacts `package()` would install, and runs a
`--help` smoke test on the binary.

## Usage

```
release_client flutter [--platform <platform>] [--channel <channel>] [--limit <n>]
    List releases from releases_<platform>.json (default: windows, stable, 10).
release_client dart-latest [--channel <channel>]
    Print the newest Dart SDK version from dart-archive (default: stable).
release_client sdk <version> [--dest <dir>] [--channel <channel>]
                [--os <os>] [--arch <arch>]
    Download dartsdk-<os>-<arch>-release.zip and verify SHA-256.
    Version may be "latest" (default) or explicit, e.g. 3.9.4.
release_client sha256 <file>
    Compute SHA-256 of a file.
```

## License

MIT — see [LICENSE](LICENSE). Third-party code may be covered by
[LICENSE-BSD](LICENSE-BSD).
