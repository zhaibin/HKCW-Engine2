# HKCW Engine2 - Windows Desktop Wallpaper Engine

A Flutter Windows plugin that embeds WebView2 as an interactive desktop wallpaper, displaying web content behind desktop icons.

## ✨ Features

- 🖼️ **WebView2 Integration** - Display any web content as desktop wallpaper
- 🎯 **Proper Z-Order** - WebView renders behind desktop icons (not covering them)
- 🖱️ **Mouse Transparency** - Optional click-through to desktop
- 📺 **Multi-Resolution** - Tested on 5120x2784 displays
- 🪟 **Windows 11 Support** - Optimized for Windows 11 desktop architecture
- ⚡ **Performance** - Hardware-accelerated web rendering

## 🚀 Quick Start

### Installation

Add to your `pubspec.yaml`:

```yaml
dependencies:
  hkcw_engine2:
    path: ../
```

### Usage

```dart
import 'package:hkcw_engine2/hkcw_engine2.dart';

// Start wallpaper (with mouse transparency)
await HkcwEngine2.initializeWallpaper(
  url: 'https://www.bing.com',
  enableMouseTransparent: true,
);

// Interactive wallpaper (clickable)
await HkcwEngine2.initializeWallpaper(
  url: 'https://game.example.com',
  enableMouseTransparent: false,
);

// Stop wallpaper
await HkcwEngine2.stopWallpaper();

// Navigate to different URL
await HkcwEngine2.navigateToUrl('https://new-url.com');
```

## 🛠️ Setup

### Prerequisites

- Windows 10/11
- Flutter 3.0+
- Visual Studio 2022 Build Tools
- WebView2 Runtime (included in Windows 11)

### Build

```bash
# Install WebView2 SDK
.\scripts\setup_webview2.bat

# Build and run example
cd example
flutter run -d windows

# Or use quick build script
.\scripts\build_and_run.bat
```

## 🏗️ Technical Architecture

### Core Implementation

The plugin uses a sophisticated approach to place WebView2 in the correct layer:

1. **Find Progman window** - The desktop's root window
2. **Create WS_CHILD window** - As a child of Progman
3. **Set Z-Order** - Position behind SHELLDLL_DefView (icon layer)
4. **Initialize WebView2** - Embed browser engine

```cpp
// Simplified core logic
HWND progman = FindWindowW(L"Progman", nullptr);
HWND host = CreateWindowExW(
    WS_EX_NOACTIVATE,
    L"STATIC", L"WebView2Host",
    WS_CHILD | WS_VISIBLE,
    0, 0, width, height,
    progman, nullptr, nullptr, nullptr
);

// Position behind desktop icons
HWND shelldll = FindWindowExW(progman, nullptr, L"SHELLDLL_DefView", nullptr);
SetWindowPos(host, shelldll, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
```

### Window Hierarchy

```
Progman (Desktop Window)
 ├─ SHELLDLL_DefView (Desktop Icons) - Z-Order: Front
 └─ WebView2 Host (Our Window)       - Z-Order: Back
     └─ WebView2 Controller
         └─ Browser Content
```

## 📖 Documentation

### User Guides
- [中文文档](docs/README_CN.md) - Chinese documentation
- [Usage Examples](docs/USAGE_EXAMPLES.md) - Code examples
- [Troubleshooting](docs/TROUBLESHOOTING.md) - Common issues

### Technical Guides
- [Solution Summary](docs/SOLUTION_SUMMARY.md) - Technical deep dive
- [Technical Notes](docs/TECHNICAL_NOTES.md) - Implementation details
- [Build Info](docs/BUILD_INFO.md) - Compilation details

### Optimization & Performance
- [Optimization Guide](docs/OPTIMIZATION_GUIDE.md) - Performance tuning
- [Optimization Completed](docs/OPTIMIZATION_COMPLETED.md) - P0/P1 optimizations
- [Test Results](docs/OPTIMIZATION_TEST_RESULTS.md) - Verification results

### Development
- [GitHub Setup](docs/GITHUB_SETUP.md) - Push to GitHub guide
- [Final Summary](docs/FINAL_SUMMARY.md) - Project overview

## 🎮 Example App

The example app provides a full-featured control panel:

- URL input with presets
- Mouse transparency toggle
- Start/Stop controls
- Status indicators

![Example App Screenshot]

## 🔧 Configuration

### Mouse Transparency

**Enabled** (default):
- Clicks pass through to desktop
- Desktop icons remain clickable
- WebView content not interactive

**Disabled**:
- Can interact with web content
- Useful for games, interactive dashboards
- Desktop icons may be obscured

### Window Size

Automatically uses work area (excludes taskbar):
```cpp
SystemParametersInfoW(SPI_GETWORKAREA, 0, &workArea, 0);
```

## 🧪 Testing

Tested on:
- ✅ Windows 11 (Build 22000+)
- ✅ 5120x2784 resolution
- ✅ Multiple WorkerW configurations
- ✅ Various web content types

## 🤝 Contributing

Contributions welcome! Please read our contributing guidelines.

## 📝 License

MIT License - see [LICENSE](LICENSE) file

## 🙏 Acknowledgments

- Inspired by Wallpaper Engine and Lively Wallpaper
- Uses Microsoft Edge WebView2
- Built with Flutter

## 📞 Support

- GitHub Issues: [Report a bug](https://github.com/yourusername/hkcw-engine2/issues)
- Discussions: [Ask questions](https://github.com/yourusername/hkcw-engine2/discussions)

## 🗺️ Roadmap

- [ ] Multi-monitor support
- [ ] Performance profiling tools
- [ ] Custom transparency levels
- [ ] Video wallpaper presets
- [ ] Configuration file support
- [ ] System tray integration

---

**Made with ❤️ using Flutter and WebView2**

