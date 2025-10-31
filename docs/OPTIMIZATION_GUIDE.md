# 优化指南 - HKCW Engine2

## 🎯 优化领域分析

### 1. 📊 内存控制优化

#### 当前问题
- ❌ WebView2 环境持续占用内存（~100-200MB）
- ❌ 网页内容无限制加载
- ❌ 没有内存监控机制
- ❌ 缓存无清理策略

#### 优化方案

##### 1.1 内存限制配置
```cpp
// 在 SetupWebView2 中添加内存限制
void HkcwEngine2Plugin::SetupWebView2(HWND hwnd, const std::string& url) {
  auto options = Microsoft::WRL::Make<CoreWebView2EnvironmentOptions>();
  
  // 设置内存限制
  wchar_t args[512];
  swprintf_s(args, L"--max-old-space-size=256 --max-semi-space-size=32");
  options->put_AdditionalBrowserArguments(args);
  
  // 禁用不必要的功能
  options->put_AdditionalBrowserArguments(
    L"--disable-background-networking "
    L"--disable-backgrounding-occluded-windows "
    L"--disable-extensions");
  
  CreateCoreWebView2EnvironmentWithOptions(
    nullptr, user_data_folder, options.Get(), callback.Get());
}
```

##### 1.2 定期清理缓存
```cpp
class HkcwEngine2Plugin {
private:
  std::chrono::steady_clock::time_point last_cleanup_;
  const std::chrono::minutes cleanup_interval_{30};

  void PeriodicCleanup() {
    auto now = std::chrono::steady_clock::now();
    if (now - last_cleanup_ > cleanup_interval_) {
      ClearWebViewCache();
      last_cleanup_ = now;
    }
  }
  
  void ClearWebViewCache() {
    if (!webview_) return;
    
    Microsoft::WRL::ComPtr<ICoreWebView2_2> webview2;
    webview_->QueryInterface(IID_PPV_ARGS(&webview2));
    if (webview2) {
      // 清除缓存
      Microsoft::WRL::ComPtr<ICoreWebView2Profile> profile;
      webview2->get_Profile(&profile);
      if (profile) {
        Microsoft::WRL::ComPtr<ICoreWebView2Profile2> profile2;
        profile->QueryInterface(IID_PPV_ARGS(&profile2));
        if (profile2) {
          profile2->ClearBrowsingData(
            COREWEBVIEW2_BROWSING_DATA_KINDS_DISK_CACHE,
            nullptr);
        }
      }
    }
  }
};
```

##### 1.3 内存监控
```cpp
struct MemoryStats {
  size_t working_set_size;
  size_t private_bytes;
  size_t virtual_size;
};

MemoryStats GetMemoryUsage() {
  PROCESS_MEMORY_COUNTERS_EX pmc;
  GetProcessMemoryInfo(GetCurrentProcess(), 
                       (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
  
  return {
    pmc.WorkingSetSize,
    pmc.PrivateUsage,
    pmc.PagefileUsage
  };
}

// Dart API
Future<Map<String, int>> getMemoryStats() async {
  final stats = await _channel.invokeMethod<Map>('getMemoryStats');
  return {
    'workingSet': stats['workingSet'],
    'privateBytes': stats['privateBytes'],
    'virtualSize': stats['virtualSize'],
  };
}
```

---

### 2. ⚡ 性能优化

#### 当前问题
- ❌ 初始化耗时 2+ 秒
- ❌ WebView2 环境每次重建
- ❌ 窗口重绘无优化
- ❌ 没有硬件加速控制

#### 优化方案

##### 2.1 环境复用
```cpp
// 静态变量保存 WebView2 环境
static Microsoft::WRL::ComPtr<ICoreWebView2Environment> g_webview_env;

void HkcwEngine2Plugin::SetupWebView2Fast(HWND hwnd, const std::string& url) {
  if (g_webview_env) {
    // 复用已有环境，快速创建
    g_webview_env->CreateCoreWebView2Controller(hwnd, controller_callback.Get());
  } else {
    // 首次创建，保存环境
    CreateCoreWebView2EnvironmentWithOptions(
      nullptr, user_data_folder, nullptr,
      Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
        [this, hwnd](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
          g_webview_env = env;  // 保存复用
          env->CreateCoreWebView2Controller(hwnd, controller_callback.Get());
          return S_OK;
        }).Get());
  }
}
```

##### 2.2 硬件加速优化
```cpp
// 控制硬件加速
void SetHardwareAcceleration(bool enable) {
  auto options = Microsoft::WRL::Make<CoreWebView2EnvironmentOptions>();
  
  if (!enable) {
    options->put_AdditionalBrowserArguments(
      L"--disable-gpu "
      L"--disable-gpu-compositing "
      L"--disable-software-rasterizer");
  } else {
    options->put_AdditionalBrowserArguments(
      L"--enable-gpu-rasterization "
      L"--enable-zero-copy");
  }
}
```

##### 2.3 渲染优化
```cpp
// 设置帧率限制
void SetFrameRateLimit(int fps) {
  if (!webview_) return;
  
  wchar_t script[128];
  swprintf_s(script, 
    L"window.requestAnimationFrame = (function() {"
    L"  var interval = 1000/%d;"
    L"  return function(cb) {"
    L"    setTimeout(cb, interval);"
    L"  };"
    L"})();", fps);
  
  webview_->ExecuteScript(script, nullptr);
}

// 暂停/恢复渲染
void PauseRendering(bool pause) {
  if (webview_controller_) {
    webview_controller_->put_IsVisible(!pause);
  }
}
```

##### 2.4 延迟加载
```dart
class LazyWallpaperLoader {
  Timer? _debounceTimer;
  
  void loadWallpaper(String url, {Duration delay = const Duration(seconds: 2)}) {
    _debounceTimer?.cancel();
    _debounceTimer = Timer(delay, () async {
      await HkcwEngine2.initializeWallpaper(url: url);
    });
  }
}
```

---

### 3. 🔒 安全性优化

#### 当前问题
- ❌ 无 URL 白名单机制
- ❌ JavaScript 执行无限制
- ❌ 网络请求未过滤
- ❌ 用户数据文件夹权限过高

#### 优化方案

##### 3.1 URL 白名单
```cpp
class URLValidator {
public:
  bool IsAllowed(const std::string& url) {
    // 白名单检查
    if (whitelist_.empty()) return true;
    
    for (const auto& pattern : whitelist_) {
      if (MatchesPattern(url, pattern)) {
        return true;
      }
    }
    
    // 黑名单检查
    for (const auto& pattern : blacklist_) {
      if (MatchesPattern(url, pattern)) {
        return false;
      }
    }
    
    return whitelist_.empty();
  }
  
  void AddWhitelist(const std::string& pattern) {
    whitelist_.push_back(pattern);
  }
  
  void AddBlacklist(const std::string& pattern) {
    blacklist_.push_back(pattern);
  }

private:
  std::vector<std::string> whitelist_;
  std::vector<std::string> blacklist_;
  
  bool MatchesPattern(const std::string& url, const std::string& pattern) {
    // 简单通配符匹配
    return url.find(pattern) != std::string::npos;
  }
};
```

##### 3.2 内容安全策略
```cpp
void SetContentSecurityPolicy() {
  if (!webview_) return;
  
  // 添加导航过滤
  webview_->add_NavigationStarting(
    Callback<ICoreWebView2NavigationStartingEventHandler>(
      [this](ICoreWebView2* sender, 
             ICoreWebView2NavigationStartingEventArgs* args) -> HRESULT {
        LPWSTR uri;
        args->get_Uri(&uri);
        
        std::wstring wuri(uri);
        std::string url(wuri.begin(), wuri.end());
        
        // 验证 URL
        if (!url_validator_.IsAllowed(url)) {
          args->put_Cancel(TRUE);
          std::cout << "[HKCW] Blocked navigation to: " << url << std::endl;
        }
        
        CoTaskMemFree(uri);
        return S_OK;
      }).Get(), nullptr);
}
```

##### 3.3 权限控制
```cpp
void ConfigurePermissions() {
  if (!webview_) return;
  
  // 禁用危险权限
  webview_->add_PermissionRequested(
    Callback<ICoreWebView2PermissionRequestedEventHandler>(
      [](ICoreWebView2* sender,
         ICoreWebView2PermissionRequestedEventArgs* args) -> HRESULT {
        COREWEBVIEW2_PERMISSION_KIND kind;
        args->get_PermissionKind(&kind);
        
        // 默认拒绝所有权限
        switch (kind) {
          case COREWEBVIEW2_PERMISSION_KIND_CLIPBOARD_READ:
          case COREWEBVIEW2_PERMISSION_KIND_MICROPHONE:
          case COREWEBVIEW2_PERMISSION_KIND_CAMERA:
          case COREWEBVIEW2_PERMISSION_KIND_GEOLOCATION:
            args->put_State(COREWEBVIEW2_PERMISSION_STATE_DENY);
            break;
          default:
            args->put_State(COREWEBVIEW2_PERMISSION_STATE_DENY);
        }
        
        return S_OK;
      }).Get(), nullptr);
}
```

##### 3.4 用户数据隔离
```cpp
void CreateIsolatedUserDataFolder() {
  wchar_t temp_path[MAX_PATH];
  GetTempPathW(MAX_PATH, temp_path);
  
  // 创建临时隔离目录
  wchar_t user_data_folder[MAX_PATH];
  swprintf_s(user_data_folder, L"%s\\HKCWEngine2_%d", 
             temp_path, GetCurrentProcessId());
  
  CreateDirectoryW(user_data_folder, nullptr);
  
  // 设置权限（仅当前用户）
  // TODO: 使用 SetSecurityInfo 限制访问
}
```

---

### 4. 🛡️ 稳定性优化

#### 当前问题
- ❌ 无异常恢复机制
- ❌ 内存泄漏风险
- ❌ 窗口生命周期管理不完善
- ❌ 无心跳检测

#### 优化方案

##### 4.1 异常捕获与恢复
```cpp
class SafeWebViewManager {
public:
  bool Initialize(const std::string& url) {
    try {
      return InitializeImpl(url);
    } catch (const std::exception& e) {
      LogError("Initialization failed: " + std::string(e.what()));
      Cleanup();
      
      // 重试机制
      if (retry_count_ < max_retries_) {
        retry_count_++;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        return Initialize(url);
      }
      
      return false;
    }
  }
  
private:
  int retry_count_ = 0;
  const int max_retries_ = 3;
  
  void LogError(const std::string& error) {
    // 写入日志文件
    std::ofstream log("hkcw_errors.log", std::ios::app);
    auto now = std::chrono::system_clock::now();
    log << "[" << now.time_since_epoch().count() << "] " 
        << error << std::endl;
  }
};
```

##### 4.2 内存泄漏检测
```cpp
class ResourceTracker {
public:
  static ResourceTracker& Instance() {
    static ResourceTracker instance;
    return instance;
  }
  
  void TrackWindow(HWND hwnd) {
    tracked_windows_.insert(hwnd);
  }
  
  void UntrackWindow(HWND hwnd) {
    tracked_windows_.erase(hwnd);
  }
  
  void CleanupAll() {
    for (HWND hwnd : tracked_windows_) {
      if (IsWindow(hwnd)) {
        DestroyWindow(hwnd);
      }
    }
    tracked_windows_.clear();
  }
  
  size_t GetTrackedCount() const {
    return tracked_windows_.size();
  }

private:
  std::set<HWND> tracked_windows_;
};
```

##### 4.3 心跳检测
```cpp
class HealthMonitor {
public:
  void Start() {
    monitor_thread_ = std::thread([this]() {
      while (running_) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        CheckHealth();
      }
    });
  }
  
  void Stop() {
    running_ = false;
    if (monitor_thread_.joinable()) {
      monitor_thread_.join();
    }
  }

private:
  void CheckHealth() {
    // 检查窗口是否有效
    if (webview_host_hwnd_ && !IsWindow(webview_host_hwnd_)) {
      std::cout << "[HKCW] WARNING: Host window destroyed unexpectedly!" << std::endl;
      OnWindowLost();
    }
    
    // 检查内存使用
    auto memory = GetMemoryUsage();
    if (memory.working_set_size > 500 * 1024 * 1024) {  // 500MB
      std::cout << "[HKCW] WARNING: High memory usage: " 
                << (memory.working_set_size / 1024 / 1024) << " MB" << std::endl;
      RequestCleanup();
    }
    
    // 检查 WebView 响应
    CheckWebViewResponsive();
  }
  
  void CheckWebViewResponsive() {
    if (!webview_) return;
    
    auto start = std::chrono::steady_clock::now();
    webview_->ExecuteScript(L"1+1", 
      Callback<ICoreWebView2ExecuteScriptCompletedHandler>(
        [start](HRESULT result, LPCWSTR resultObjectAsJson) -> HRESULT {
          auto elapsed = std::chrono::steady_clock::now() - start;
          if (elapsed > std::chrono::seconds(3)) {
            std::cout << "[HKCW] WARNING: WebView responding slowly!" << std::endl;
          }
          return S_OK;
        }).Get());
  }
  
  std::thread monitor_thread_;
  std::atomic<bool> running_{true};
};
```

##### 4.4 优雅退出
```cpp
HkcwEngine2Plugin::~HkcwEngine2Plugin() {
  std::cout << "[HKCW] Plugin destructor called" << std::endl;
  
  // 停止监控
  if (health_monitor_) {
    health_monitor_->Stop();
  }
  
  // 保存状态
  SaveState();
  
  // 清理 WebView
  if (webview_controller_) {
    webview_controller_->Close();
    webview_controller_ = nullptr;
  }
  
  webview_ = nullptr;
  
  // 清理窗口
  if (webview_host_hwnd_ && IsWindow(webview_host_hwnd_)) {
    DestroyWindow(webview_host_hwnd_);
    webview_host_hwnd_ = nullptr;
  }
  
  // 清理资源跟踪
  ResourceTracker::Instance().CleanupAll();
  
  std::cout << "[HKCW] Plugin cleanup complete" << std::endl;
}

void SaveState() {
  // 保存当前 URL、设置等
  std::ofstream state("hkcw_state.json");
  state << "{ \"last_url\": \"" << current_url_ << "\" }" << std::endl;
}
```

---

### 5. 🔄 其他优化

#### 5.1 多显示器支持
```cpp
struct MonitorInfo {
  HMONITOR handle;
  RECT bounds;
  bool is_primary;
};

std::vector<MonitorInfo> EnumerateMonitors() {
  std::vector<MonitorInfo> monitors;
  
  EnumDisplayMonitors(nullptr, nullptr,
    [](HMONITOR hMonitor, HDC, LPRECT, LPARAM lParam) -> BOOL {
      auto* monitors = reinterpret_cast<std::vector<MonitorInfo>*>(lParam);
      
      MONITORINFO mi = { sizeof(mi) };
      GetMonitorInfo(hMonitor, &mi);
      
      monitors->push_back({
        hMonitor,
        mi.rcMonitor,
        (mi.dwFlags & MONITORINFOF_PRIMARY) != 0
      });
      
      return TRUE;
    }, reinterpret_cast<LPARAM>(&monitors));
  
  return monitors;
}

// 为每个显示器创建独立 WebView
void InitializeMultiMonitor() {
  auto monitors = EnumerateMonitors();
  
  for (const auto& monitor : monitors) {
    auto* instance = new WebViewInstance();
    instance->Initialize(monitor.bounds);
    instances_.push_back(instance);
  }
}
```

#### 5.2 配置文件支持
```cpp
#include <nlohmann/json.hpp>

struct Config {
  std::string default_url;
  bool mouse_transparent;
  int cache_size_mb;
  bool hardware_acceleration;
  std::vector<std::string> whitelist;
  
  static Config Load(const std::string& path) {
    std::ifstream f(path);
    nlohmann::json j = nlohmann::json::parse(f);
    
    Config config;
    config.default_url = j.value("default_url", "about:blank");
    config.mouse_transparent = j.value("mouse_transparent", true);
    config.cache_size_mb = j.value("cache_size_mb", 100);
    config.hardware_acceleration = j.value("hardware_acceleration", true);
    config.whitelist = j.value("whitelist", std::vector<std::string>{});
    
    return config;
  }
};
```

#### 5.3 性能统计
```cpp
class PerformanceMonitor {
public:
  void RecordEvent(const std::string& name) {
    events_[name] = std::chrono::steady_clock::now();
  }
  
  int64_t GetElapsed(const std::string& name) {
    auto it = events_.find(name);
    if (it == events_.end()) return -1;
    
    auto elapsed = std::chrono::steady_clock::now() - it->second;
    return std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
  }
  
  void PrintStats() {
    std::cout << "=== Performance Stats ===" << std::endl;
    std::cout << "Initialization: " << GetElapsed("init") << "ms" << std::endl;
    std::cout << "WebView Creation: " << GetElapsed("webview_create") << "ms" << std::endl;
    std::cout << "Navigation: " << GetElapsed("navigate") << "ms" << std::endl;
  }

private:
  std::map<std::string, std::chrono::steady_clock::time_point> events_;
};
```

---

## 📋 优化优先级

### 高优先级 (P0)
1. ✅ **内存泄漏检测与修复** - 防止长期运行崩溃
2. ✅ **异常恢复机制** - 提高稳定性
3. ✅ **URL 白名单** - 基础安全保障

### 中优先级 (P1)
4. ⚠️ **环境复用** - 显著提升启动速度
5. ⚠️ **缓存清理策略** - 控制内存增长
6. ⚠️ **权限控制** - 增强安全性

### 低优先级 (P2)
7. 📌 **多显示器支持** - 功能增强
8. 📌 **性能监控** - 开发辅助
9. 📌 **配置文件** - 用户友好

---

## 🎯 实施建议

### 第一阶段 (1-2 天)
- 添加基础异常处理
- 实现内存监控
- 添加 URL 验证

### 第二阶段 (3-5 天)
- 环境复用优化
- 缓存清理机制
- 权限控制系统

### 第三阶段 (1 周)
- 多显示器支持
- 配置文件系统
- 完整的性能监控

---

## 📊 预期效果

| 指标 | 当前 | 优化后 | 提升 |
|------|------|--------|------|
| 启动时间 | 2.0s | 0.5s | **75%** ↓ |
| 内存占用 | 150MB | 80MB | **47%** ↓ |
| 稳定运行时间 | 数小时 | 数天 | **10x** ↑ |
| 崩溃率 | 偶尔 | 极少 | **90%** ↓ |
| 安全漏洞 | 多个 | 极少 | **95%** ↓ |

---

**优化是持续的过程，建议分阶段实施，逐步验证效果！** 🚀

