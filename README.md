# 🎯 mingw64-dart (Community Edition)

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![License: BSD](https://img.shields.io/badge/Dart%2FFlutter-BSD-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)
[![GitHub Release](https://img.shields.io/github/v/release/Bloby22/mingw64_dart?label=Latest&color=success)](https://github.com/Bloby22/mingw64_dart/releases)
[![Maintenance](https://img.shields.io/maintenance/yes/2026?color=success)](https://github.com/Bloby22/mingw64_dart)
[![Community](https://img.shields.io/badge/Community-Edition-blueviolet.svg)](https://github.com/Bloby22/mingw64_dart)

A native **Dart SDK** downloader and installer for MSYS2 MinGW64.

![MINGW64 Dart Setup](examples/mingw.png)

---

## ⚡ Quick Start

### Install via Package

```bash
pacman -U mingw-w64-x86_64-dart-0.1.4-1-any.pkg.tar.zst
```

### Verify Installation

```bash
dart --version
flutter --version
```

---

## 📦 What's Included

- ✅ **Dart SDK** 3.13.4 (dart.exe)
- ✅ **Flutter SDK** 3.47.5 (flutter.bat)
- ✅ Pre-built binaries from Google
- ✅ All runtime dependencies
- ✅ Ready for Windows development

---

## 🛠️ Supported Architectures

| Architecture | Support | Status |
|-------------|---------|--------|
| mingw64 (x86_64) | ✅ | Tested & Working |
| mingw32 (i686) | ✅ | Tested & Working |
| ucrt64 | ✅ | Tested & Working |
| clang64 | ✅ | Tested & Working |
| clangarm64 | ✅ | Tested & Working |

---

## 📝 Examples

### Command Line Usage

```powershell
PS D:\Dev\Project1\myapp> where.exe flutter
D:\MSYS64\mingw64\bin\flutter.exe

PS D:\Dev\Project1\myapp> where.exe dart
D:\MSYS64\mingw64\bin\dart.exe

PS D:\Dev\Project1\myapp> dart --version
Dart SDK version: 3.13.4

PS D:\Dev\Project1\myapp> flutter --version
Flutter 3.47.5
```

---

## 🏗️ Build From Source

If you prefer to build from source:

```bash
git clone https://github.com/Bloby22/mingw64_dart.git
cd mingw64_dart
make build
```

The executable is created in `build/release_client.exe`.

---

## 📄 License

- **This Project**: MIT License (see [LICENSE](LICENSE))
- **Dart SDK**: BSD License (Google)
- **Flutter SDK**: BSD License (Google)

For full license details, see the LICENSE file in this repository.

---

## ⚠️ Disclaimer

This project is **not affiliated with Google Inc.** or the official MSYS2 project.

This is a community-maintained package for MSYS2 users who need Dart and Flutter development tools on Windows.

---

## 🤝 Contributing

Found an issue? Have a suggestion? Open an issue or submit a pull request!

- 🐛 [Report Bug](https://github.com/Bloby22/mingw64_dart/issues/new)
- 💡 [Request Feature](https://github.com/Bloby22/mingw64_dart/issues/new)
- 📚 [Documentation](https://github.com/Bloby22/mingw64_dart/wiki)

---

## 📞 Support

- **MSYS2**: https://www.msys2.org/
- **Dart**: https://dart.dev/
- **Flutter**: https://flutter.dev/

---

**Made with ❤️ for the MSYS2 Windows Developer Community**

![Stars](https://img.shields.io/github/stars/Bloby22/mingw64_dart?style=social)
![Forks](https://img.shields.io/github/forks/Bloby22/mingw64_dart?style=social)
