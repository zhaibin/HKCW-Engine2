# HKCW Engine2 - Windows 桌面壁纸引擎

## 项目简介

这是一个 Flutter Windows 插件，可以将 WebView2 嵌入到 Windows 桌面的 WorkerW 层中，作为动态壁纸显示在桌面图标下方。

## 核心功能

- ✅ 支持 Windows 10 和 Windows 11
- ✅ WebView2 显示在桌面图标下方（不遮挡图标）
- ✅ 自动检测操作系统版本并使用对应的 WorkerW 挂载方案
- ✅ 支持鼠标穿透（可选）
- ✅ 实时导航到不同的 URL

## 技术实现

### Windows 10 方案
1. 向 Progman 发送 0x052C 消息创建 WorkerW
2. 枚举窗口找到包含 SHELLDLL_DefView 的父窗口
3. 获取对应的 WorkerW 句柄
4. 将 WebView2 宿主窗口设置为 WorkerW 的子窗口

### Windows 11 方案
1. 通过 EnumWindows 枚举所有顶层窗口
2. 找到包含 SHELLDLL_DefView 的窗口
3. 获取其兄弟窗口 WorkerW（真正的壁纸层）
4. 将 WebView2 挂载为其子窗口

## 环境要求

- Windows 10/11
- Flutter 3.0+
- Visual Studio 2022 Build Tools
- WebView2 Runtime（Windows 11 自带）

## 快速开始

### 1. 安装 WebView2 SDK

首次使用需要安装 WebView2 开发包：

\`\`\`bash
setup_webview2.bat
\`\`\`

### 2. 构建并运行示例

\`\`\`bash
build_and_run.bat
\`\`\`

或者使用 Flutter 命令：

\`\`\`bash
cd example
flutter run -d windows
\`\`\`

### 3. 使用插件

启动应用后：
1. 输入要显示的 URL（例如：https://www.bing.com）
2. 勾选"启用鼠标穿透"（如果需要点击穿透到桌面）
3. 点击"Start Wallpaper"按钮
4. 检查桌面 - WebView2 应该显示在图标下方
5. 使用"Navigate to URL"可以更改显示内容
6. 点击"Stop Wallpaper"停止显示

## 项目结构

\`\`\`
HKCW-Engine2/
├── lib/                          # Dart API
│   └── hkcw_engine2.dart
├── windows/                      # Windows 平台实现
│   ├── hkcw_engine2_plugin.h     # 插件头文件
│   ├── hkcw_engine2_plugin.cpp   # 核心实现（WorkerW 逻辑）
│   ├── CMakeLists.txt            # 构建配置
│   ├── packages/                 # WebView2 SDK
│   └── include/                  # 公共头文件
├── example/                      # 示例应用
│   └── lib/main.dart
├── build_and_run.bat            # 构建并运行脚本
├── run_example.bat              # 直接运行脚本
└── setup_webview2.bat           # WebView2 安装脚本
\`\`\`

## 核心代码说明

### C++ 实现要点

\`\`\`cpp
// OS 版本检测
bool IsWindows11OrGreater() {
  // 检查 Windows 版本（Build >= 22000 为 Win11）
}

// Win10 方式查找 WorkerW
HWND FindWorkerW() {
  // 1. 向 Progman 发送 0x052C
  // 2. 枚举窗口找 SHELLDLL_DefView
  // 3. 返回对应的 WorkerW
}

// Win11 方式查找 WorkerW
HWND FindWorkerWWindows11() {
  // 1. 枚举顶层窗口
  // 2. 找到 SHELLDLL_DefView 的兄弟 WorkerW
}

// 创建 WebView2 宿主窗口
HWND CreateWebViewHostWindow() {
  // CreateWindowEx with WS_EX_LAYERED | WS_EX_NOACTIVATE
}

// 初始化 WebView2
void SetupWebView2(HWND hwnd, const string& url) {
  // CreateCoreWebView2EnvironmentWithOptions
  // 异步创建 Controller 和 WebView
}
\`\`\`

### Dart API

\`\`\`dart
// 初始化壁纸
await HkcwEngine2.initializeWallpaper(
  url: 'https://example.com',
  enableMouseTransparent: true,
);

// 导航到新 URL
await HkcwEngine2.navigateToUrl('https://newurl.com');

// 停止壁纸
await HkcwEngine2.stopWallpaper();
\`\`\`

## 调试日志

应用会在控制台输出详细日志，格式为：

\`\`\`
[HKCW] Plugin initialized
[HKCW] ========== Initializing Wallpaper ==========
[HKCW] URL: https://www.bing.com
[HKCW] Mouse Transparent: true
[HKCW] OS Detected: Windows 11
[HKCW] Finding WorkerW for Windows 11...
[HKCW] WorkerW found (Win11): 0x...
[HKCW] Creating WebView host window...
[HKCW] WebView host window created: 0x...
[HKCW] WebView host parented to WorkerW
[HKCW] Mouse transparency enabled
[HKCW] Setting up WebView2...
[HKCW] WebView2 environment created
[HKCW] WebView2 controller created
[HKCW] Navigating to: https://www.bing.com
[HKCW] ========== Initialization Complete ==========
\`\`\`

## 常见问题

### 1. 构建失败 - 找不到 WebView2.h

**解决方案**：运行 `setup_webview2.bat` 安装 WebView2 SDK

### 2. 桌面图标被遮挡

**原因**：WorkerW 查找失败或系统不支持

**解决方案**：
- 检查控制台日志，确认 WorkerW 是否找到
- 尝试重启应用
- 某些桌面定制软件可能影响功能

### 3. WebView2 不显示

**解决方案**：
- 确保已安装 WebView2 Runtime
- 检查 URL 是否正确
- 查看控制台是否有错误日志

### 4. 鼠标穿透不工作

**说明**：启用鼠标穿透后，所有鼠标事件会穿透到桌面，无法与 WebView 内容交互。这是预期行为。

## 许可证

MIT License

## 贡献

欢迎提交 Issue 和 Pull Request！

## 致谢

本项目实现了 Windows 动态壁纸的核心技术，感谢社区的支持和反馈。

