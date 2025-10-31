# 解决方案总结

## 🎯 最终工作方案

### 问题背景
在 Windows 11 系统上实现 WebView2 桌面壁纸引擎，要求：
1. WebView2 显示在桌面图标**下方**（不遮挡图标）
2. 桌面图标可正常点击
3. 任务栏不被覆盖

### 关键发现

#### 用户系统特点
- **Windows 11** 系统
- **0x052C 消息不起作用** - SHELLDLL_DefView 保持在 Progman 中
- **没有 WorkerW 层分离** - 标准的 WorkerW 查找方法失效

#### 最终解决方案

```cpp
// 1. 使用 Progman 作为父窗口
HWND progman = FindWindowW(L"Progman", nullptr);

// 2. 创建 WS_CHILD 窗口（关键！）
HWND hwnd = CreateWindowExW(
    WS_EX_NOACTIVATE,
    L"STATIC",
    L"WebView2Host",
    WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
    0, 0, width, height,
    progman,  // 父窗口是 Progman
    nullptr,
    GetModuleHandle(nullptr),
    nullptr
);

// 3. 设置 Z-order 在 SHELLDLL_DefView 后面（关键！）
HWND shelldll = FindWindowExW(progman, nullptr, L"SHELLDLL_DefView", nullptr);
SetWindowPos(hwnd, shelldll, 0, 0, 0, 0, 
             SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

// 4. 可选：启用鼠标透明
if (enable_mouse_transparent) {
    LONG_PTR exStyle = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
    SetWindowLongPtrW(hwnd, GWL_EXSTYLE, 
                      exStyle | WS_EX_LAYERED | WS_EX_TRANSPARENT);
    SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);
}

// 5. 初始化 WebView2
CreateCoreWebView2EnvironmentWithOptions(...);
controller->put_Bounds(bounds);
controller->put_IsVisible(TRUE);
webview->Navigate(url);
```

### 核心要点

#### ✅ 成功关键

1. **WS_CHILD 窗口样式**
   - 必须使用 `WS_CHILD`，不能用 `WS_POPUP`
   - 子窗口会正确参与父窗口的 Z-order 管理

2. **Z-order 设置**
   ```cpp
   SetWindowPos(webview_host, shelldll, ...);
   ```
   - 将 WebView 窗口放在 SHELLDLL_DefView **后面**
   - 这样图标层在上，WebView 在下

3. **Progman 作为父窗口**
   - 在 0x052C 不工作的系统上，直接用 Progman
   - SHELLDLL_DefView 是 Progman 的子窗口

4. **窗口裁剪**
   - `WS_CLIPSIBLINGS | WS_CLIPCHILDREN`
   - 确保窗口边界正确处理

#### ❌ 失败尝试

1. **WorkerW 枚举方法**
   - 在此系统上，WorkerW 没有包含 SHELLDLL_DefView
   - 0x052C 消息不触发 WorkerW 分离

2. **WS_POPUP 窗口**
   - 创建为顶层窗口，无法正确处理 Z-order

3. **SetParent 后设置父窗口**
   - 不如直接在 CreateWindowEx 时指定父窗口

### 窗口层次结构

```
Progman (桌面窗口)
 ├─ SHELLDLL_DefView (桌面图标层) - Z-order: 前
 └─ WebView Host (我们的窗口)     - Z-order: 后
     └─ WebView2 Controller
         └─ Browser Content
```

### 功能特性

#### 鼠标透明控制
```dart
// 启用透明 - 点击穿透到桌面
await HkcwEngine2.initializeWallpaper(
  url: 'https://www.bing.com',
  enableMouseTransparent: true,  // 默认
);

// 禁用透明 - 可与 WebView 交互
await HkcwEngine2.initializeWallpaper(
  url: 'https://game.com',
  enableMouseTransparent: false,  // 交互模式
);
```

#### 窗口大小
- 使用 `SystemParametersInfoW(SPI_GETWORKAREA, ...)` 获取工作区
- 自动排除任务栏区域
- 5120x2784 分辨率正常工作

### 兼容性说明

#### 适用场景
✅ Windows 11（0x052C 不工作的系统）
✅ SHELLDLL_DefView 在 Progman 中的配置
✅ 高分辨率显示器 (5120x2784)

#### 可能需要调整的场景
⚠️ Windows 10 标准配置
⚠️ SHELLDLL_DefView 在 WorkerW 中的系统
⚠️ 多显示器配置

### 调试日志示例

成功运行时的日志：
```
[HKCW] Found Progman: 00000000000203B2
[HKCW] SHELLDLL_DefView still in Progman, 0x052C did not work
[HKCW] Using Progman as parent
[HKCW] Creating child window: 5120x2784
[HKCW] WebView host window created: 0000000000A407C4
[HKCW] Found SHELLDLL_DefView in parent, setting Z-order behind it...
[HKCW] Z-order set: WebView -> behind -> SHELLDLL_DefView
[HKCW] Window visible: 1, Rect: 0,0 5120x2784
[HKCW] Mouse transparency ENABLED
[HKCW] WebView2 controller created
[HKCW] Setting WebView bounds: 0,0 5120x2784
[HKCW] WebView2 visibility set to TRUE
[HKCW] Navigating to: https://www.bing.com
```

### 性能指标

- **初始化时间**: ~2 秒（包含 WebView2 环境创建）
- **内存占用**: ~100MB（基础 + 网页内容）
- **CPU 使用**: 取决于网页内容（静态页面 <5%）

### 后续优化建议

1. **多显示器支持**
   - 枚举显示器
   - 为每个显示器创建独立的 WebView

2. **性能优化**
   - 禁用 GPU 加速选项（如需要）
   - 控制帧率

3. **通用兼容**
   - 同时支持 Progman 和 WorkerW 方案
   - 自动检测并选择最佳策略

4. **用户配置**
   - 窗口位置和大小自定义
   - 透明度级别控制
   - 性能选项

---

**测试环境**:
- OS: Windows 11
- 分辨率: 5120x2784
- Flutter: 3.x
- WebView2 Runtime: 1.0.2592.51

**最后更新**: 2025-10-31

