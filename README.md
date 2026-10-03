# 🎯 mingw64-dart (Community Edition)

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![GitHub Release](https://img.shields.io/github/v/release/Bloby22/mingw64_dart?color=success)](https://github.com/Bloby22/mingw64_dart/releases)
[![Build](https://github.com/Bloby22/mingw64_dart/actions/workflows/build.yml/badge.svg)](https://github.com/Bloby22/mingw64_dart/actions/workflows/build.yml)

Dart SDK packaged for **MSYS2 MinGW64**, with a native `dart.exe` launcher.

---

## ⚡ Quick Install

In an **MSYS2 MINGW64** terminal:

```bash
pacman -U https://github.com/Bloby22/mingw64_dart/releases/latest/download/mingw-w64-x86_64-dart-latest.pkg.tar.zst
```

Verify:

```bash
dart --version
```

---

## 📦 What's Included

- Dart SDK 3.13.5
- Native `dart.exe` launcher that works inside MSYS2 bash
- MINGW64 only (UCRT64 and CLANG64 are not supported)

Versions are updated automatically: a daily GitHub Actions workflow opens a pull request with the latest stable Dart release and its official SHA256 checksum, and CI builds and tests the package before it is released.

---

## 📝 Examples

![MINGW64 Dart Setup](examples/mingw.png)

---

## 🏗️ Build From Source

In an MSYS2 MINGW64 terminal:

```bash
pacman -S --needed git base-devel mingw-w64-x86_64-gcc
git clone https://github.com/Bloby22/mingw64_dart.git
cd mingw64_dart
makepkg -si
```

The build downloads about 200 MB.

---

## 📄 License

- **This project**: MIT License
- **Dart SDK**: BSD License (Google)

---

## ⚠️ Disclaimer

This project is not affiliated with Google or MSYS2.

Community-maintained package for Windows developers.

---

**Made with ❤️ for the MSYS2 community**
