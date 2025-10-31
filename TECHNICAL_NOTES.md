# Technical Implementation Notes

## Core Architecture

### Plugin Structure

```
HkcwEngine2Plugin (C++)
  ├─ RegisterWithRegistrar()      // Plugin registration
  ├─ HandleMethodCall()            // Dart <-> C++ bridge
  ├─ InitializeWallpaper()         // Main initialization
  ├─ FindWorkerW()                 // Win10 WorkerW finder
  ├─ FindWorkerWWindows11()        // Win11 WorkerW finder
  ├─ CreateWebViewHostWindow()    // Window creation
  ├─ SetupWebView2()               // WebView2 initialization
  ├─ StopWallpaper()               // Cleanup
  └─ NavigateToUrl()               // Navigation
```

## WorkerW Layer Injection

### Windows 10 Mechanism

```cpp
HWND FindWorkerW() {
  // 1. Find Progman window
  HWND progman = FindWindowW(L"Progman", nullptr);
  
  // 2. Send magic message to create WorkerW
  SendMessageTimeoutW(progman, 0x052C, 0, 0, SMTO_NORMAL, 1000, nullptr);
  
  // 3. Enumerate windows to find SHELLDLL_DefView parent
  EnumWindows([](HWND hwnd, LPARAM lParam) {
    HWND child = FindWindowExW(hwnd, nullptr, L"SHELLDLL_DefView", nullptr);
    if (child != nullptr) {
      // Found! Get the next WorkerW sibling
      HWND workerw = FindWindowExW(nullptr, hwnd, L"WorkerW", nullptr);
      // This is the wallpaper layer
      return workerw;
    }
    return TRUE;
  });
}
```

**Why this works:**
- Windows desktop uses multiple WorkerW windows
- After 0x052C message, system creates a new WorkerW
- SHELLDLL_DefView (desktop icons) is child of specific WorkerW
- The WorkerW *after* the one containing DefView is the wallpaper layer

### Windows 11 Mechanism

```cpp
HWND FindWorkerWWindows11() {
  // 1. Directly enumerate top-level windows
  EnumWindows([](HWND hwnd, LPARAM lParam) {
    HWND child = FindWindowExW(hwnd, nullptr, L"SHELLDLL_DefView", nullptr);
    if (child != nullptr) {
      // Check if parent is WorkerW
      wchar_t className[256];
      GetClassNameW(hwnd, className, 256);
      if (wcscmp(className, L"WorkerW") == 0) {
        return hwnd;  // This WorkerW is the wallpaper layer
      }
      // Or find sibling WorkerW
      HWND workerw = FindWindowExW(nullptr, hwnd, L"WorkerW", nullptr);
      return workerw;
    }
    return TRUE;
  });
}
```

**Win11 Differences:**
- Progman messaging sometimes unreliable
- Window hierarchy slightly different
- Direct enumeration more stable

## WebView2 Integration

### Window Creation

```cpp
HWND CreateWebViewHostWindow() {
  // Register window class
  WNDCLASSEXW wc = {sizeof(wc)};
  wc.lpfnWndProc = DefWindowProcW;
  wc.lpszClassName = L"HKCWWebViewHost";
  RegisterClassExW(&wc);
  
  // Create full-screen layered window
  HWND hwnd = CreateWindowExW(
    WS_EX_LAYERED | WS_EX_NOACTIVATE,  // Layered + no focus
    L"HKCWWebViewHost",
    L"HKCW WebView Host",
    WS_POPUP | WS_VISIBLE,             // Popup (no border)
    0, 0,                               // Full screen position
    GetSystemMetrics(SM_CXSCREEN),
    GetSystemMetrics(SM_CYSCREEN),
    nullptr,                            // No parent yet
    nullptr,
    GetModuleHandle(nullptr),
    nullptr
  );
  
  return hwnd;
}
```

**Key Flags:**
- `WS_EX_LAYERED`: Enables transparency and compositing
- `WS_EX_NOACTIVATE`: Prevents window from stealing focus
- `WS_POPUP`: No title bar or borders
- `WS_VISIBLE`: Show immediately

### Parenting to WorkerW

```cpp
bool InitializeWallpaper(const string& url, bool enable_transparent) {
  // 1. Find WorkerW based on OS version
  HWND worker_w = IsWindows11OrGreater() ? 
                  FindWorkerWWindows11() : 
                  FindWorkerW();
  
  // 2. Create WebView host
  HWND webview_host = CreateWebViewHostWindow();
  
  // 3. Set parent relationship
  SetParent(webview_host, worker_w);
  
  // 4. Optional: Enable mouse transparency
  if (enable_transparent) {
    LONG_PTR exStyle = GetWindowLongPtrW(webview_host, GWL_EXSTYLE);
    SetWindowLongPtrW(webview_host, GWL_EXSTYLE, exStyle | WS_EX_TRANSPARENT);
  }
  
  // 5. Set alpha
  SetLayeredWindowAttributes(webview_host, 0, 255, LWA_ALPHA);
  
  // 6. Show and initialize WebView2
  ShowWindow(webview_host, SW_SHOW);
  SetupWebView2(webview_host, url);
}
```

**Parent-Child Relationship:**
```
Progman (Desktop)
 └─ WorkerW (Background layer)
     └─ WebView Host Window ← Our window here
         └─ WebView2 Control ← Browser content
 └─ WorkerW (Icon layer)
     └─ SHELLDLL_DefView (Desktop icons)
```

### WebView2 Async Initialization

```cpp
void SetupWebView2(HWND hwnd, const string& url) {
  wstring wurl(url.begin(), url.end());
  
  // Create environment asynchronously
  auto env_callback = Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
    [this, hwnd, wurl](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
      // Environment created
      
      // Create controller asynchronously
      auto controller_callback = Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
        [this, hwnd, wurl](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
          // Controller created
          webview_controller_ = controller;
          webview_controller_->get_CoreWebView2(&webview_);
          
          // Set bounds to match window
          RECT bounds;
          GetClientRect(hwnd, &bounds);
          webview_controller_->put_Bounds(bounds);
          
          // Navigate to URL
          webview_->Navigate(wurl.c_str());
          
          return S_OK;
        });
      
      env->CreateCoreWebView2Controller(hwnd, controller_callback.Get());
      return S_OK;
    });
  
  CreateCoreWebView2EnvironmentWithOptions(nullptr, user_data_folder, nullptr, env_callback.Get());
}
```

**Async Chain:**
1. Create Environment (runtime initialization)
2. Create Controller (browser instance)
3. Get WebView interface
4. Set bounds
5. Navigate to URL

## Mouse Transparency

### How It Works

```cpp
// Enable transparency
SetWindowLongPtrW(hwnd, GWL_EXSTYLE, exStyle | WS_EX_TRANSPARENT);
```

**Effect:**
- All mouse events pass through window
- Window becomes "invisible" to mouse
- Desktop icons remain clickable
- WebView content not interactive when enabled

**Use Cases:**
- Pure wallpaper display (no interaction)
- Animated backgrounds
- Video wallpapers

**Disable for:**
- Interactive web content
- HTML5 games
- Clickable links

## OS Version Detection

```cpp
bool IsWindows11OrGreater() {
  OSVERSIONINFOEXW osvi = { sizeof(osvi) };
  DWORDLONG mask = VerSetConditionMask(
    VerSetConditionMask(0, VER_MAJORVERSION, VER_GREATER_EQUAL),
    VER_BUILDNUMBER, VER_GREATER_EQUAL
  );
  
  osvi.dwMajorVersion = 10;
  osvi.dwBuildNumber = 22000;  // Win11 starts at build 22000
  
  return VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_BUILDNUMBER, mask) != FALSE;
}
```

**Build Numbers:**
- Windows 10: 10240 - 19045
- Windows 11: 22000+

## Memory Management

### Smart Pointers

```cpp
// WebView2 COM interfaces use ComPtr
Microsoft::WRL::ComPtr<ICoreWebView2Controller> webview_controller_;
Microsoft::WRL::ComPtr<ICoreWebView2> webview_;
```

**Benefits:**
- Automatic reference counting
- Exception-safe
- No manual Release() calls

### Cleanup

```cpp
bool StopWallpaper() {
  // 1. Close WebView2 controller
  if (webview_controller_) {
    webview_controller_->Close();
    webview_controller_ = nullptr;
  }
  
  // 2. Release WebView interface
  webview_ = nullptr;
  
  // 3. Destroy window
  if (webview_host_hwnd_) {
    DestroyWindow(webview_host_hwnd_);
    webview_host_hwnd_ = nullptr;
  }
  
  // 4. Clear WorkerW reference
  worker_w_hwnd_ = nullptr;
  
  return true;
}
```

## Flutter Integration

### Method Channel

```cpp
void HandleMethodCall(const MethodCall<EncodableValue>& call,
                     unique_ptr<MethodResult<EncodableValue>> result) {
  if (call.method_name() == "initializeWallpaper") {
    const auto* args = get_if<EncodableMap>(call.arguments());
    string url = get<string>(args->find(EncodableValue("url"))->second);
    bool transparent = get<bool>(args->find(EncodableValue("enableMouseTransparent"))->second);
    
    bool success = InitializeWallpaper(url, transparent);
    result->Success(EncodableValue(success));
  }
  // ... other methods
}
```

### Dart API

```dart
static Future<bool> initializeWallpaper({
  required String url,
  bool enableMouseTransparent = true,
}) async {
  final result = await _channel.invokeMethod<bool>('initializeWallpaper', {
    'url': url,
    'enableMouseTransparent': enableMouseTransparent,
  });
  return result ?? false;
}
```

## Performance Considerations

1. **WebView2 Initialization**: Async, ~1-2 seconds
2. **WorkerW Search**: Fast, <100ms
3. **Window Creation**: Instant
4. **Memory Usage**: ~50-100MB base + webpage content
5. **CPU**: Depends on web content (video/animation)

## Known Limitations

1. **Desktop Customization Software**: May conflict with WorkerW manipulation
2. **Multiple Monitors**: Currently single monitor only
3. **DPI Scaling**: Works but not explicitly handled
4. **Windows Updates**: WorkerW behavior may change

## Future Enhancements

- [ ] Multi-monitor support
- [ ] Window position/size control
- [ ] Custom transparency levels
- [ ] Hardware acceleration options
- [ ] Performance profiling
- [ ] Error recovery mechanisms

## References

- [WebView2 Documentation](https://docs.microsoft.com/en-us/microsoft-edge/webview2/)
- [Windows Desktop Window Manager](https://docs.microsoft.com/en-us/windows/win32/dwm/dwm-overview)
- [Win32 Window Classes](https://docs.microsoft.com/en-us/windows/win32/winmsg/window-classes)

