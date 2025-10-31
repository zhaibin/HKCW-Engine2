#ifndef FLUTTER_PLUGIN_HKCW_ENGINE2_PLUGIN_H_
#define FLUTTER_PLUGIN_HKCW_ENGINE2_PLUGIN_H_

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <windows.h>
#include <wrl.h>
#include <WebView2.h>
#include <memory>
#include <set>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <atomic>
#include <fstream>
#include <psapi.h>

namespace hkcw_engine2 {

// P0-1: Resource Tracker for memory leak detection
class ResourceTracker {
public:
  static ResourceTracker& Instance() {
    static ResourceTracker instance;
    return instance;
  }
  
  void TrackWindow(HWND hwnd);
  void UntrackWindow(HWND hwnd);
  void CleanupAll();
  size_t GetTrackedCount() const;

private:
  std::set<HWND> tracked_windows_;
};

// P0-3: URL Validator for security
class URLValidator {
public:
  bool IsAllowed(const std::string& url);
  void AddWhitelist(const std::string& pattern);
  void AddBlacklist(const std::string& pattern);
  void ClearWhitelist();
  void ClearBlacklist();

private:
  std::vector<std::string> whitelist_;
  std::vector<std::string> blacklist_;
  bool MatchesPattern(const std::string& url, const std::string& pattern);
};

class HkcwEngine2Plugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);

  HkcwEngine2Plugin();
  virtual ~HkcwEngine2Plugin();

  HkcwEngine2Plugin(const HkcwEngine2Plugin&) = delete;
  HkcwEngine2Plugin& operator=(const HkcwEngine2Plugin&) = delete;

 private:
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);

  bool InitializeWallpaper(const std::string& url, bool enable_mouse_transparent);
  bool StopWallpaper();
  bool NavigateToUrl(const std::string& url);

  HWND FindWorkerW();
  HWND FindWorkerWWindows11();
  HWND CreateWebViewHostWindow();
  void SetupWebView2(HWND hwnd, const std::string& url);
  
  // P0-2: Exception recovery
  bool InitializeWithRetry(const std::string& url, bool enable_mouse_transparent, int max_retries = 3);
  void LogError(const std::string& error);
  
  // P1-2: Cache management
  void ClearWebViewCache();
  void PeriodicCleanup();
  
  // P1-3: Permission control
  void ConfigurePermissions();
  void SetupSecurityHandlers();

  HWND webview_host_hwnd_ = nullptr;
  HWND worker_w_hwnd_ = nullptr;
  Microsoft::WRL::ComPtr<ICoreWebView2Controller> webview_controller_;
  Microsoft::WRL::ComPtr<ICoreWebView2> webview_;
  bool is_initialized_ = false;
  
  // P0: Retry tracking
  int init_retry_count_ = 0;
  
  // P0-3: URL validation
  URLValidator url_validator_;
  
  // P1-2: Cache cleanup timing
  std::chrono::steady_clock::time_point last_cleanup_;
  
  // P1-1: Shared WebView2 environment
  static Microsoft::WRL::ComPtr<ICoreWebView2Environment> shared_environment_;
};

}  // namespace hkcw_engine2

#endif  // FLUTTER_PLUGIN_HKCW_ENGINE2_PLUGIN_H_

