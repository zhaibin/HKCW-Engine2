# Troubleshooting Guide

## Build Issues

### 1. Cannot find WebView2.h

**Error:**
```
error C1083: Cannot open include file: 'WebView2.h': No such file or directory
```

**Solution:**
```bash
# Run the setup script
setup_webview2.bat

# Or manually install
cd windows
nuget.exe install Microsoft.Web.WebView2 -Version 1.0.2592.51 -OutputDirectory packages
```

**Verify Installation:**
```bash
# Check if package exists
Test-Path windows\packages\Microsoft.Web.WebView2.1.0.2592.51
# Should return: True
```

### 2. LNK1104: cannot open file 'hkcw_engine2_plugin.lib'

**Error:**
```
LINK : fatal error LNK1104: cannot open file '..\plugins\hkcw_engine2\Debug\hkcw_engine2_plugin.lib'
```

**Cause:** Plugin DLL not exporting symbols correctly

**Solution:**
```bash
# Clean and rebuild
flutter clean
flutter build windows --debug
```

**Verify:** Check that `__declspec(dllexport)` is present in CPP file:
```cpp
extern "C" {
__declspec(dllexport) void HkcwEngine2PluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  // ...
}
}
```

### 3. CMake Configuration Error

**Error:**
```
CMake Error at CMakeLists.txt:50 (add_subdirectory):
  The source directory does not contain a CMakeLists.txt file
```

**Solution:**
```bash
# Regenerate Flutter platform files
cd example
flutter create --platforms=windows .
```

### 4. Missing Visual Studio

**Error:**
```
No suitable Visual Studio installation found
```

**Solution:**
- Install [Visual Studio 2022 Build Tools](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022)
- During installation, select:
  - Desktop development with C++
  - C++ CMake tools for Windows
  - Windows 10/11 SDK

## Runtime Issues

### 1. Desktop Icons Covered by WebView

**Symptoms:**
- Icons invisible or not clickable
- WebView appears on top layer

**Diagnosis:**
Check console logs:
```
[HKCW] Finding WorkerW for Windows 11...
[HKCW] WorkerW found (Win11): 0x...
```

**Solutions:**

#### Option A: WorkerW Not Found
```
[HKCW] ERROR: WorkerW not found via Win11 method
```

**Fix:**
1. Restart Explorer:
   ```powershell
   taskkill /f /im explorer.exe
   start explorer.exe
   ```

2. Try different OS detection:
   - Modify `IsWindows11OrGreater()` logic
   - Force Win10 or Win11 mode

#### Option B: Wrong WorkerW Selected
**Fix:** Add debug logging to verify window hierarchy:

```cpp
// In FindWorkerW or FindWorkerWWindows11
wchar_t className[256];
GetClassNameW(hwnd, className, 256);
std::wcout << L"Found window: " << className << L" HWND: " << hwnd << std::endl;
```

### 2. WebView2 Not Displaying

**Symptoms:**
- Black screen
- No content visible
- No errors in log

**Diagnosis:**

Check initialization logs:
```
[HKCW] WebView2 environment created
[HKCW] WebView2 controller created
[HKCW] Navigating to: https://...
```

**Solutions:**

#### Option A: WebView2 Runtime Missing
**Check:**
```powershell
# Check if WebView2 Runtime installed
Get-ItemProperty -Path "HKLM:\SOFTWARE\WOW6432Node\Microsoft\EdgeUpdate\Clients\{F3017226-FE2A-4295-8BDF-00C3A9A7E4C5}" -Name pv
```

**Install:**
- Download from [Microsoft Edge WebView2](https://developer.microsoft.com/en-us/microsoft-edge/webview2/)
- Or: Windows Update will install automatically

#### Option B: URL Invalid or Unreachable
**Test URL:**
```dart
// Try with simple local HTML first
await HkcwEngine2.initializeWallpaper(
  url: 'about:blank',
  enableMouseTransparent: false,
);

// Then try actual URL
await HkcwEngine2.navigateToUrl('https://www.bing.com');
```

#### Option C: Window Not Visible
**Debug:**
```cpp
// In CreateWebViewHostWindow, add:
ShowWindow(hwnd, SW_SHOW);
UpdateWindow(hwnd);
SetForegroundWindow(hwnd);  // Force to front temporarily
```

### 3. Mouse Transparency Not Working

**Symptoms:**
- Can't click through to desktop
- Icons still not clickable

**Diagnosis:**
```
[HKCW] Mouse transparency enabled
```

**Solutions:**

#### Option A: Style Not Applied
**Verify:**
```cpp
LONG_PTR exStyle = GetWindowLongPtrW(webview_host_hwnd_, GWL_EXSTYLE);
std::cout << "ExStyle: " << std::hex << exStyle << std::endl;
// Should include WS_EX_TRANSPARENT (0x00000020)
```

**Fix:**
```cpp
// Apply after window creation
SetWindowLongPtrW(hwnd, GWL_EXSTYLE, 
                  WS_EX_LAYERED | WS_EX_NOACTIVATE | WS_EX_TRANSPARENT);
```

#### Option B: Z-Order Issue
**Fix:** Ensure correct parent:
```cpp
SetParent(webview_host_hwnd_, worker_w_hwnd_);
// Verify parent was set
HWND parent = GetParent(webview_host_hwnd_);
std::cout << "Parent HWND: " << parent << " Expected: " << worker_w_hwnd_ << std::endl;
```

### 4. Application Crashes on Start

**Symptoms:**
- App closes immediately
- Access violation
- No error message

**Common Causes:**

#### Option A: Null Pointer Dereference
**Check:**
```cpp
// Add null checks
if (!webview_controller_) {
  std::cout << "[HKCW] ERROR: Controller is null" << std::endl;
  return false;
}
```

#### Option B: COM Initialization Failed
**Fix:**
```cpp
// In main.cpp or plugin initialization
HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
if (FAILED(hr)) {
  std::cout << "COM initialization failed: " << std::hex << hr << std::endl;
}
```

#### Option C: WebView2 Environment Creation Failed
**Debug:**
```cpp
HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(...);
if (FAILED(hr)) {
  std::cout << "Environment creation failed: " << std::hex << hr << std::endl;
  // Common error codes:
  // 0x80070002 - File not found (Runtime missing)
  // 0x80004005 - Unspecified error
}
```

## Performance Issues

### 1. High CPU Usage

**Diagnosis:**
- Check Task Manager
- Look for webview2 or edge processes

**Solutions:**

#### Disable Hardware Acceleration
```cpp
// In WebView2 environment options
auto options = Microsoft::WRL::Make<CoreWebView2EnvironmentOptions>();
options->put_AdditionalBrowserArguments(L"--disable-gpu");
CreateCoreWebView2EnvironmentWithOptions(nullptr, user_data_folder, options.Get(), callback);
```

#### Limit Frame Rate
```html
<!-- In your HTML content -->
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<style>
  * { will-change: auto !important; }
</style>
```

### 2. High Memory Usage

**Solutions:**

#### Clear Cache Periodically
```cpp
// Add to plugin
void ClearCache() {
  if (webview_) {
    Microsoft::WRL::ComPtr<ICoreWebView2_2> webview2;
    webview_->QueryInterface(IID_PPV_ARGS(&webview2));
    if (webview2) {
      webview2->CallDevToolsProtocolMethod(
        L"Network.clearBrowserCache", L"{}", nullptr);
    }
  }
}
```

#### Use Simple Content
```dart
// Avoid heavy frameworks
// Use lightweight HTML/CSS instead of React/Vue
```

## Windows-Specific Issues

### 1. Windows 11 WorkerW Not Found

**Symptoms:**
```
[HKCW] ERROR: WorkerW not found via Win11 method
```

**Solution A:** Try Win10 method
```cpp
// Force Win10 mode
bool IsWindows11OrGreater() {
  return false;  // Force Win10 method
}
```

**Solution B:** Alternative enumeration
```cpp
// More aggressive search
HWND FindWorkerWAlternative() {
  HWND result = nullptr;
  HWND hwnd = nullptr;
  
  // Enumerate ALL windows
  while ((hwnd = FindWindowExW(nullptr, hwnd, L"WorkerW", nullptr)) != nullptr) {
    HWND child = FindWindowExW(hwnd, nullptr, L"SHELLDLL_DefView", nullptr);
    if (child != nullptr) {
      result = FindWindowExW(nullptr, hwnd, L"WorkerW", nullptr);
      if (result != nullptr) break;
    }
  }
  
  return result;
}
```

### 2. Multiple Monitor Setup

**Current Limitation:** Plugin targets primary monitor only

**Workaround:**
```cpp
// Get specific monitor bounds
RECT GetMonitorRect(int monitorIndex) {
  // Enumerate monitors
  // Return specific monitor rect
  // Adjust window position accordingly
}
```

### 3. DPI Scaling Issues

**Symptoms:**
- Window size incorrect
- Content blurry

**Solution:**
```cpp
// Set DPI awareness
SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

// Get DPI scale
UINT dpi = GetDpiForWindow(hwnd);
float scale = dpi / 96.0f;

// Adjust window size
int width = static_cast<int>(GetSystemMetrics(SM_CXSCREEN) * scale);
int height = static_cast<int>(GetSystemMetrics(SM_CYSCREEN) * scale);
```

## Debugging Tools

### Enable Verbose Logging

**In C++:**
```cpp
#define HKCW_DEBUG 1

#ifdef HKCW_DEBUG
  #define HKCW_LOG(msg) std::cout << "[HKCW] " << msg << std::endl
#else
  #define HKCW_LOG(msg)
#endif
```

**In Flutter:**
```dart
// Run with verbose
flutter run -d windows --verbose
```

### Inspect WebView2 Content

**Enable DevTools:**
```cpp
// In WebView2 setup
webview_->CallDevToolsProtocolMethod(
  L"Page.enable", L"{}", nullptr);

// Or open DevTools window
webview_controller_->OpenDevToolsWindow();
```

### Monitor Window Hierarchy

**PowerShell:**
```powershell
# List all WorkerW windows
Get-Process | Where-Object {$_.MainWindowTitle -eq ""} | 
  Select-Object ProcessName, Id, MainWindowHandle
```

**Spy++ (Visual Studio):**
- Tools > Spy++ > Find Window
- Search for "WorkerW" or "SHELLDLL_DefView"

### Check WebView2 Version

**PowerShell:**
```powershell
# Get installed version
$regPath = "HKLM:\SOFTWARE\WOW6432Node\Microsoft\EdgeUpdate\Clients\{F3017226-FE2A-4295-8BDF-00C3A9A7E4C5}"
Get-ItemProperty -Path $regPath -Name pv | Select-Object -ExpandProperty pv
```

## Getting Help

### Collect Debug Information

```bash
# 1. Build with verbose output
flutter build windows --debug --verbose > build.log 2>&1

# 2. Run and capture console
Start-Process "build\windows\x64\runner\Debug\hkcw_engine2_example.exe" -RedirectStandardOutput stdout.log -RedirectStandardError stderr.log

# 3. System information
systeminfo > sysinfo.txt

# 4. WebView2 version
# (see above)
```

### Report Issues

Include:
1. **Build log** (build.log)
2. **Runtime log** (stdout.log, stderr.log)
3. **System info** (Windows version, build number)
4. **WebView2 version**
5. **Steps to reproduce**
6. **Expected vs actual behavior**

### Community Resources

- GitHub Issues: [Create Issue](https://github.com/yourusername/hkcw-engine2/issues)
- Stack Overflow: Tag `flutter` + `windows` + `webview2`
- Discord: Flutter Community

## Known Issues & Workarounds

### Issue: Wallpaper disappears after sleep/resume

**Workaround:**
```dart
// Detect system resume and restart
WidgetsBinding.instance.addObserver(/* lifecycle observer */);
```

### Issue: Conflicting desktop customization software

**Known Conflicts:**
- Rainmeter
- Wallpaper Engine (Steam)
- DreamScene

**Workaround:** Disable other wallpaper software before using HKCW Engine2

### Issue: WebView2 black screen on first run

**Workaround:**
```dart
// Add delay before navigation
await Future.delayed(Duration(seconds: 2));
await HkcwEngine2.navigateToUrl(url);
```

## Quick Fixes Checklist

- [ ] Restart Explorer.exe
- [ ] Run `flutter clean`
- [ ] Reinstall WebView2 Runtime
- [ ] Check antivirus/firewall
- [ ] Disable other wallpaper apps
- [ ] Update Windows
- [ ] Update Flutter SDK
- [ ] Rebuild project from scratch
- [ ] Check console logs
- [ ] Try with simple URL (about:blank)

## Still Having Issues?

1. **Enable Debug Logging** (see above)
2. **Collect Information** (see above)
3. **Search Existing Issues**
4. **Create New Issue** with all details

---

**Last Updated:** 2025-10-31  
**Plugin Version:** 1.0.0

