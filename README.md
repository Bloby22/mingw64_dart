# mingw64-dart

> A native Dart SDK and Flutter release CLI for MSYS2 MinGW64.

`flutter` downloads release metadata and Dart SDK archives from Google Cloud
Storage. Downloads are verified with SHA-256 before optional extraction.

## Requirements

- [MSYS2](https://www.msys2.org/) with a MinGW64 terminal
- GCC with C++20 support
- `mingw-w64-x86_64-curl`
- CMake 3.20+ and `make`

Install the dependencies:

```sh
pacman -S --needed mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake \
  mingw-w64-x86_64-curl make
```

## Build

```sh
git clone https://github.com/BlobyCZ/mingw64_dart.git
cd mingw64_dart
make build
```

The executable is created in `build/release_client.exe`.

To build and install an MSYS2 package instead:

```sh
makepkg -f
pacman -U mingw-w64-x86_64-mingw64-dart-*.pkg.tar.zst
```

The package provides `flutter`, `flutter-cli`, `release_client`, and the real
`dart.exe` from the official Dart SDK. The complete SDK is installed under
`/mingw64/lib/dart-sdk`.

## Usage

Download and extract the latest stable Dart SDK for Windows x64:

```sh
flutter dartsdk --dest ./sdk
```

Add the extracted SDK to the current MSYS2 session:

```sh
export PATH="$PWD/sdk/dart-sdk/bin:$PATH"
dart --version
```

Get the newest SDK version for a channel:

```sh
flutter dart-latest --channel stable
```

Download a specific SDK without extracting it:

```sh
flutter sdk 3.9.4 --dest ./sdk --no-extract
```

List Flutter releases:

```sh
flutter releases --platform windows --channel stable --limit 10
```

Calculate a local file's SHA-256 checksum:

```sh
flutter sha256 path/to/file.zip
```

Run `flutter --help` for the complete command reference. Supported channels are
`stable`, `beta`, and `dev`.

## Development

```sh
make test       # Run CTest
make rebuild    # Clean and build again
make clean      # Remove the build directory
```

## License

MIT. See [LICENSE](LICENSE). Third-party components may be covered by
[LICENSE-BSD](LICENSE-BSD).
