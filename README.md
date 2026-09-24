# mingw64-dart

> A native Flutter and Dart release CLI for MSYS2 MinGW64

`flutter` downloads Flutter release metadata and Dart SDK archives directly
from Google Cloud Storage. It is written in C++20, built with MinGW-w64, and
distributed as an MSYS2 package.

## Highlights

- Query Flutter releases by platform, channel, and result limit
- Read the latest Dart SDK version for stable, beta, or dev
- Download and verify Dart SDK archives with SHA-256
- Extract the SDK and print the correct MSYS2 PATH command
- Calculate SHA-256 checksums for local files
- Install as `flutter`, with `flutter-cli` and `release_client` compatibility names

## Requirements

- MSYS2 MinGW64 environment
- GCC with C++20 support
- CMake 3.20 or newer
- `mingw-w64-x86_64-curl`
- Perl for the maintenance scripts

Install the native dependencies from a MinGW64 terminal:

```sh
pacman -S --needed mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake \
  mingw-w64-x86_64-curl make perl
```

## Install From Package

Build and install the local package from an MSYS2 MinGW64 terminal:

```sh
cd /d/Dev/mingw64_dart
export PATH=/mingw64/bin:/usr/bin:$PATH
makepkg -f
pacman -U mingw64-dart-0.1.3-3-x86_64.pkg.tar.zst
```

The package installs these commands into `/mingw64/bin`:

```text
flutter.exe
flutter-cli.exe
release_client.exe
```

Verify the installation:

```sh
flutter --version
flutter --help
```

## Build From Source

Use the Makefile wrapper:

```sh
make build
```

Or configure CMake directly:

```sh
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build --config Release
```

Available Make targets:

| Target | Description |
| --- | --- |
| `make build` | Configure and build with CMake |
| `make test` | Run CTest |
| `make rebuild` | Clean and build again |
| `make clean` | Remove the CMake build directory |
| `make help` | Show available targets |

## CLI Reference

```text
flutter releases [--platform <platform>] [--channel <channel>] [--limit <n>]
    List Flutter releases from releases_<platform>.json.

flutter dart-latest [--channel <channel>]
    Print the newest Dart SDK version for a channel.

flutter dartsdk [--dest <dir>] [--channel <channel>]
    Download, verify, and extract the latest Windows x64 Dart SDK.

flutter sdk <version> [--dest <dir>] [--channel <channel>]
             [--os <os>] [--arch <arch>] [--extract|--no-extract]
    Download a specific SDK archive. The version may be "latest".

flutter sha256 <file>
    Calculate the SHA-256 checksum of a local file.
```

## Examples

List the latest stable Flutter releases:

```sh
flutter releases
```

List five beta releases for Linux:

```sh
flutter releases --platform linux --channel beta --limit 5
```

Download and extract the latest Dart SDK into a directory with spaces:

```sh
flutter dartsdk --dest "/d/Program Lang/dart"
```

Run the PATH command printed by the tool, for example:

```sh
export PATH="/d/Program Lang/dart/dart-sdk/bin:$PATH"
dart --version
```

Persist the PATH for future MSYS2 sessions:

```sh
echo 'export PATH="/d/Program Lang/dart/dart-sdk/bin:$PATH"' >> ~/.bashrc
source ~/.bashrc
```

Download a specific SDK archive without extraction:

```sh
flutter sdk 3.9.4 --dest ./sdk --no-extract
```

Download and extract a specific SDK archive:

```sh
flutter sdk latest --dest ./sdk --extract
```

## Packaging and Maintenance

Build the package with `makepkg`:

```sh
makepkg -f
```

Use `makepkg -f`, not `makepkg -C`, in this repository. The project source
directory is named `src/`, and aggressive clean mode can treat it as a package
staging directory.

Validate the PKGBUILD build inputs and package layout:

```sh
perl scripts/makepkg.pl
perl scripts/makepkg.pl --no-build
```

Bump the package version:

```sh
perl scripts/bump.pl --patch
perl scripts/bump.pl --minor
perl scripts/bump.pl --major
perl scripts/bump.pl --bump-pkgrel
```

The version script updates `pkgver` and `pkgrel` in `PKGBUILD` and synchronizes
the CMake project version when one is present.

## Exit Codes

| Code | Meaning |
| --- | --- |
| `0` | Command completed successfully |
| `1` | Runtime failure, network error, or checksum mismatch |
| `2` | Invalid command or missing argument |

## Project Layout

```text
include/       Public C++ headers
src/           C++ implementation
scripts/       Perl packaging and version tools
PKGBUILD       MSYS2 package definition
CMakeLists.txt CMake build definition
Makefile       Build convenience targets
```

## License

MIT. See [LICENSE](LICENSE). Third-party components may be covered by
[LICENSE-BSD](LICENSE-BSD).
