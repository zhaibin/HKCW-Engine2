#include "hkcw_engine2_plugin.h"
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>
#include <windows.h>
#include <shellapi.h>
#include <string>
#include <memory>
#include <iostream>
#include <sstream>

namespace hkcw_engine2 {

namespace {

// Window class name for WebView2 host
const wchar_t kWebViewHostClassName[] = L"HKCWWebViewHost";

// Global instance for callbacks
HkcwEngine2Plugin* g_plugin_instance = nullptr;

// Enum callback for finding WorkerW
struct EnumWindowsContext {
  HWND shelldll_parent = nullptr;
  HWND worker_w = nullptr;
  bool is_win11_mode = false;
};

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
  auto* context = reinterpret_cast<EnumWindowsContext*>(lParam);
  
  // Get window class name for debugging
  wchar_t className[256];
  GetClassNameW(hwnd, className, 256);
  
  HWND child = FindWindowExW(hwnd, nullptr, L"SHELLDLL_DefView", nullptr);
  if (child != nullptr) {
    std::wcout << L"[HKCW] Found SHELLDLL_DefView in window class: " << className << L" HWND: " << hwnd << std::endl;
    
    // Found SHELLDLL_DefView, store its parent
    context->shelldll_parent = hwnd;
    
    // For Win10: Get the next WorkerW sibling
    if (!context->is_win11_mode) {
      context->worker_w = FindWindowExW(nullptr, hwnd, L"WorkerW", nullptr);
      if (context->worker_w) {
        std::cout << "[HKCW] Found next WorkerW sibling: " << context->worker_w << std::endl;
      }
    } else {
      // For Win11: The parent itself might be WorkerW or we need to find sibling
      if (wcscmp(className, L"WorkerW") == 0) {
        std::cout << "[HKCW] Parent is WorkerW, using it directly" << std::endl;
        context->worker_w = hwnd;
      } else {
        std::wcout << L"[HKCW] Parent is " << className << L", looking for WorkerW sibling" << std::endl;
        // Try to find WorkerW sibling
        context->worker_w = FindWindowExW(nullptr, hwnd, L"WorkerW", nullptr);
        if (context->worker_w) {
          std::cout << "[HKCW] Found WorkerW sibling: " << context->worker_w << std::endl;
        }
      }
    }
    return FALSE; // Stop enumeration
  }
  return TRUE; // Continue enumeration
}

bool IsWindows11OrGreater() {
  OSVERSIONINFOEXW osvi = { sizeof(osvi), 0, 0, 0, 0, {0}, 0, 0 };
  DWORDLONG const dwlConditionMask = VerSetConditionMask(
      VerSetConditionMask(0, VER_MAJORVERSION, VER_GREATER_EQUAL),
      VER_BUILDNUMBER, VER_GREATER_EQUAL);
  
  osvi.dwMajorVersion = 10;
  osvi.dwBuildNumber = 22000; // Windows 11 starts at build 22000
  
  return VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_BUILDNUMBER, dwlConditionMask) != FALSE;
}

}  // namespace

void HkcwEngine2Plugin::RegisterWithRegistrar(
    flutter::PluginRegistrarWindows *registrar) {
  auto channel =
      std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
          registrar->messenger(), "hkcw_engine2",
          &flutter::StandardMethodCodec::GetInstance());

  auto plugin = std::make_unique<HkcwEngine2Plugin>();
  g_plugin_instance = plugin.get();

  channel->SetMethodCallHandler(
      [plugin_pointer = plugin.get()](const auto &call, auto result) {
        plugin_pointer->HandleMethodCall(call, std::move(result));
      });

  registrar->AddPlugin(std::move(plugin));
}

HkcwEngine2Plugin::HkcwEngine2Plugin() {
  std::cout << "[HKCW] Plugin initialized" << std::endl;
}

HkcwEngine2Plugin::~HkcwEngine2Plugin() {
  StopWallpaper();
}

void HkcwEngine2Plugin::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue> &method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  
  std::cout << "[HKCW] Method called: " << method_call.method_name() << std::endl;

  if (method_call.method_name() == "initializeWallpaper") {
    const auto* arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (!arguments) {
      result->Error("INVALID_ARGS", "Arguments must be a map");
      return;
    }

    auto url_it = arguments->find(flutter::EncodableValue("url"));
    auto transparent_it = arguments->find(flutter::EncodableValue("enableMouseTransparent"));
    
    if (url_it == arguments->end()) {
      result->Error("INVALID_ARGS", "Missing 'url' argument");
      return;
    }

    std::string url = std::get<std::string>(url_it->second);
    bool enable_transparent = transparent_it != arguments->end() 
        ? std::get<bool>(transparent_it->second) : true;

    bool success = InitializeWallpaper(url, enable_transparent);
    result->Success(flutter::EncodableValue(success));
  }
  else if (method_call.method_name() == "stopWallpaper") {
    bool success = StopWallpaper();
    result->Success(flutter::EncodableValue(success));
  }
  else if (method_call.method_name() == "navigateToUrl") {
    const auto* arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (!arguments) {
      result->Error("INVALID_ARGS", "Arguments must be a map");
      return;
    }

    auto url_it = arguments->find(flutter::EncodableValue("url"));
    if (url_it == arguments->end()) {
      result->Error("INVALID_ARGS", "Missing 'url' argument");
      return;
    }

    std::string url = std::get<std::string>(url_it->second);
    bool success = NavigateToUrl(url);
    result->Success(flutter::EncodableValue(success));
  }
  else {
    result->NotImplemented();
  }
}

HWND HkcwEngine2Plugin::FindWorkerW() {
  std::cout << "[HKCW] Finding WorkerW for Windows 10..." << std::endl;
  
  // Send message to Progman to create WorkerW
  HWND progman = FindWindowW(L"Progman", nullptr);
  if (!progman) {
    std::cout << "[HKCW] ERROR: Progman not found" << std::endl;
    return nullptr;
  }

  std::cout << "[HKCW] Progman found: " << progman << std::endl;

  // List all WorkerW windows BEFORE message
  std::cout << "[HKCW] WorkerW windows BEFORE 0x052C message:" << std::endl;
  HWND hwnd = nullptr;
  int count = 0;
  while ((hwnd = FindWindowExW(nullptr, hwnd, L"WorkerW", nullptr)) != nullptr) {
    std::cout << "[HKCW]   WorkerW #" << ++count << ": " << hwnd << std::endl;
  }

  // Trigger WorkerW creation
  LRESULT result = SendMessageTimeoutW(progman, 0x052C, 0, 0, SMTO_NORMAL, 1000, nullptr);
  std::cout << "[HKCW] SendMessage result: " << result << std::endl;
  Sleep(300); // Wait longer for WorkerW creation

  // List all WorkerW windows AFTER message
  std::cout << "[HKCW] WorkerW windows AFTER 0x052C message:" << std::endl;
  hwnd = nullptr;
  count = 0;
  while ((hwnd = FindWindowExW(nullptr, hwnd, L"WorkerW", nullptr)) != nullptr) {
    std::cout << "[HKCW]   WorkerW #" << ++count << ": " << hwnd << std::endl;
    // Check if this WorkerW contains SHELLDLL_DefView
    HWND defView = FindWindowExW(hwnd, nullptr, L"SHELLDLL_DefView", nullptr);
    if (defView) {
      std::cout << "[HKCW]     -> Contains SHELLDLL_DefView!" << std::endl;
    }
  }

  // Enumerate windows to find SHELLDLL_DefView and its WorkerW
  EnumWindowsContext context;
  context.is_win11_mode = false;
  EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&context));

  if (context.worker_w) {
    std::cout << "[HKCW] WorkerW found (Win10): " << context.worker_w << std::endl;
    return context.worker_w;
  }
  
  // Alternative method: Find WorkerW that comes right after Progman in Z-order
  std::cout << "[HKCW] Trying alternative method: Find WorkerW after Progman in Z-order..." << std::endl;
  HWND workerw = FindWindowExW(nullptr, progman, L"WorkerW", nullptr);
  if (workerw) {
    std::cout << "[HKCW] Found WorkerW after Progman: " << workerw << std::endl;
    return workerw;
  }

  // Last resort: Just use the first WorkerW (often the wallpaper layer)
  std::cout << "[HKCW] Last resort: Using first WorkerW..." << std::endl;
  workerw = FindWindowW(L"WorkerW", nullptr);
  if (workerw) {
    std::cout << "[HKCW] Using first WorkerW: " << workerw << std::endl;
    return workerw;
  }

  std::cout << "[HKCW] ERROR: WorkerW not found via Win10 method" << std::endl;
  return nullptr;
}

HWND HkcwEngine2Plugin::FindWorkerWWindows11() {
  std::cout << "[HKCW] Finding WorkerW for Windows 11..." << std::endl;
  
  // Find Progman first
  HWND progman = FindWindowW(L"Progman", nullptr);
  if (!progman) {
    std::cout << "[HKCW] ERROR: Progman not found" << std::endl;
    return nullptr;
  }

  std::cout << "[HKCW] Progman found: " << progman << std::endl;

  // Send message to create WorkerW (same as Win10)
  SendMessageTimeoutW(progman, 0x052C, 0, 0, SMTO_NORMAL, 1000, nullptr);
  Sleep(200); // Wait a bit longer for WorkerW creation
  
  // For Win11, enumerate to find SHELLDLL_DefView and its WorkerW
  EnumWindowsContext context;
  context.is_win11_mode = true;
  EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&context));

  if (context.worker_w) {
    std::cout << "[HKCW] WorkerW found (Win11): " << context.worker_w << std::endl;
  } else {
    std::cout << "[HKCW] ERROR: WorkerW not found via Win11 method" << std::endl;
  }

  return context.worker_w;
}

HWND HkcwEngine2Plugin::CreateWebViewHostWindow() {
  std::cout << "[HKCW] Creating WebView host window..." << std::endl;

  if (!worker_w_hwnd_) {
    std::cout << "[HKCW] ERROR: No parent window (WorkerW) available" << std::endl;
    return nullptr;
  }

  // Get work area (desktop minus taskbar)
  RECT workArea;
  SystemParametersInfoW(SPI_GETWORKAREA, 0, &workArea, 0);
  
  int x = 0;  // Relative to parent
  int y = 0;
  int width = workArea.right - workArea.left;
  int height = workArea.bottom - workArea.top;
  
  std::cout << "[HKCW] Creating child window: " << width << "x" << height << std::endl;

  // Create as CHILD window of WorkerW (this is the key!)
  // Temporarily remove transparency to test if WebView shows
  HWND hwnd = CreateWindowExW(
      WS_EX_NOACTIVATE,  // Remove TRANSPARENT and LAYERED for testing
      L"STATIC",  // Use built-in STATIC class
      L"WebView2Host",
      WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,  // CHILD window
      x, y, width, height,
      worker_w_hwnd_,  // Parent window (WorkerW)
      nullptr,
      GetModuleHandle(nullptr),
      nullptr);

  if (!hwnd) {
    DWORD error = GetLastError();
    std::cout << "[HKCW] ERROR: Failed to create window, error: " << error << std::endl;
    return nullptr;
  }

  std::cout << "[HKCW] WebView host window created: " << hwnd << std::endl;
  return hwnd;
}

void HkcwEngine2Plugin::SetupWebView2(HWND hwnd, const std::string& url) {
  std::cout << "[HKCW] Setting up WebView2..." << std::endl;

  // Convert URL to wstring
  std::wstring wurl(url.begin(), url.end());

  // Get user data folder
  wchar_t user_data_folder[MAX_PATH];
  GetEnvironmentVariableW(L"APPDATA", user_data_folder, MAX_PATH);
  wcscat_s(user_data_folder, L"\\HKCWEngine2");

  auto callback = Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
      [this, hwnd, wurl](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
        if (FAILED(result)) {
          std::cout << "[HKCW] ERROR: Failed to create WebView2 environment: " << std::hex << result << std::endl;
          return result;
        }

        std::cout << "[HKCW] WebView2 environment created" << std::endl;

        auto controller_callback = Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
            [this, hwnd, wurl](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
              if (FAILED(result)) {
                std::cout << "[HKCW] ERROR: Failed to create WebView2 controller: " << std::hex << result << std::endl;
                return result;
              }

              std::cout << "[HKCW] WebView2 controller created" << std::endl;

              webview_controller_ = controller;
              webview_controller_->get_CoreWebView2(&webview_);

              // Set bounds to match window
              RECT bounds;
              GetClientRect(hwnd, &bounds);
              std::cout << "[HKCW] Setting WebView bounds: " << bounds.left << "," << bounds.top 
                        << " " << (bounds.right - bounds.left) << "x" << (bounds.bottom - bounds.top) << std::endl;
              
              HRESULT hr = webview_controller_->put_Bounds(bounds);
              if (FAILED(hr)) {
                std::cout << "[HKCW] ERROR: Failed to set bounds: " << std::hex << hr << std::endl;
              }
              
              // Make sure WebView is visible
              webview_controller_->put_IsVisible(TRUE);
              std::cout << "[HKCW] WebView2 visibility set to TRUE" << std::endl;

              // Navigate to URL
              webview_->Navigate(wurl.c_str());
              
              // Convert wstring to string for logging
              std::string url_str;
              for (wchar_t c : wurl) {
                if (c < 128) url_str.push_back(static_cast<char>(c));
              }
              std::cout << "[HKCW] Navigating to: " << url_str << std::endl;

              is_initialized_ = true;
              return S_OK;
            });

        env->CreateCoreWebView2Controller(hwnd, controller_callback.Get());
        return S_OK;
      });

  HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(
      nullptr, user_data_folder, nullptr, callback.Get());

  if (FAILED(hr)) {
    std::cout << "[HKCW] ERROR: CreateCoreWebView2EnvironmentWithOptions failed: " << std::hex << hr << std::endl;
  }
}

bool HkcwEngine2Plugin::InitializeWallpaper(const std::string& url, bool enable_mouse_transparent) {
  std::cout << "[HKCW] ========== Initializing Wallpaper ==========" << std::endl;
  std::cout << "[HKCW] URL: " << url << std::endl;
  std::cout << "[HKCW] Mouse Transparent: " << (enable_mouse_transparent ? "true" : "false") << std::endl;

  if (is_initialized_) {
    std::cout << "[HKCW] Already initialized, stopping first..." << std::endl;
    StopWallpaper();
  }

  // Try to find Progman (desktop window)
  HWND progman = FindWindowW(L"Progman", nullptr);
  if (!progman) {
    std::cout << "[HKCW] ERROR: Progman not found" << std::endl;
    return false;
  }
  
  std::cout << "[HKCW] Found Progman: " << progman << std::endl;
  
  // Win11 correct strategy:
  // 1. Send 0x052C multiple times to ensure WorkerW creation
  // 2. SHELLDLL_DefView will be in the FIRST WorkerW (icon layer)
  // 3. The SECOND WorkerW (next sibling) is the wallpaper layer
  
  std::cout << "[HKCW] Sending 0x052C messages to trigger WorkerW split..." << std::endl;
  for (int i = 0; i < 3; i++) {
    SendMessageW(progman, 0x052C, 0, 0);
    Sleep(100);
  }
  
  HWND wallpaper_workerw = nullptr;
  HWND icon_workerw = nullptr;
  
  // Find the WorkerW that contains SHELLDLL_DefView (this is the icon layer)
  std::cout << "[HKCW] Searching for SHELLDLL_DefView location..." << std::endl;
  HWND hwnd = nullptr;
  int workerw_count = 0;
  
  while ((hwnd = FindWindowExW(nullptr, hwnd, L"WorkerW", nullptr)) != nullptr) {
    workerw_count++;
    HWND shelldll = FindWindowExW(hwnd, nullptr, L"SHELLDLL_DefView", nullptr);
    if (shelldll) {
      icon_workerw = hwnd;
      std::cout << "[HKCW] Found SHELLDLL_DefView in WorkerW #" << workerw_count 
                << " (icon layer): " << icon_workerw << std::endl;
      
      // Find the NEXT WorkerW sibling - this is the wallpaper layer!
      wallpaper_workerw = FindWindowExW(nullptr, icon_workerw, L"WorkerW", nullptr);
      if (wallpaper_workerw) {
        std::cout << "[HKCW] Found NEXT WorkerW (wallpaper layer): " << wallpaper_workerw << std::endl;
      } else {
        std::cout << "[HKCW] WARNING: No WorkerW found after icon layer, will use icon WorkerW" << std::endl;
        wallpaper_workerw = icon_workerw;
      }
      break;
    }
  }
  
  // Check Progman as fallback
  if (!icon_workerw) {
    HWND shelldll_in_progman = FindWindowExW(progman, nullptr, L"SHELLDLL_DefView", nullptr);
    if (shelldll_in_progman) {
      std::cout << "[HKCW] SHELLDLL_DefView still in Progman, 0x052C did not work" << std::endl;
      std::cout << "[HKCW] Using Progman as parent (this may not work correctly)" << std::endl;
      wallpaper_workerw = progman;
    }
  }
  
  // Last resort
  if (!wallpaper_workerw) {
    std::cout << "[HKCW] ERROR: Could not find suitable parent window" << std::endl;
    wallpaper_workerw = progman;
  }
  
  worker_w_hwnd_ = wallpaper_workerw;
  std::cout << "[HKCW] Final parent window: " << worker_w_hwnd_ << std::endl;

  // Create WebView host window (already parented to WorkerW inside)
  webview_host_hwnd_ = CreateWebViewHostWindow();
  if (!webview_host_hwnd_) {
    std::cout << "[HKCW] ERROR: Failed to create WebView host window" << std::endl;
    return false;
  }

  std::cout << "[HKCW] WebView host created as child of WorkerW" << std::endl;
  
  // If parent is Progman, we need to set Z-order behind SHELLDLL_DefView
  HWND shelldll = FindWindowExW(worker_w_hwnd_, nullptr, L"SHELLDLL_DefView", nullptr);
  if (shelldll) {
    std::cout << "[HKCW] Found SHELLDLL_DefView in parent, setting Z-order behind it..." << std::endl;
    // Place our window behind SHELLDLL_DefView
    SetWindowPos(webview_host_hwnd_, shelldll, 0, 0, 0, 0, 
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    std::cout << "[HKCW] Z-order set: WebView -> behind -> SHELLDLL_DefView" << std::endl;
  }
  
  // Verify window is actually visible
  BOOL isVisible = IsWindowVisible(webview_host_hwnd_);
  RECT rect;
  GetWindowRect(webview_host_hwnd_, &rect);
  std::cout << "[HKCW] Window visible: " << isVisible 
            << ", Rect: " << rect.left << "," << rect.top 
            << " " << (rect.right - rect.left) << "x" << (rect.bottom - rect.top) << std::endl;
  
  // Enable mouse transparency if requested
  if (enable_mouse_transparent) {
    // Set window as layered and transparent for mouse events
    LONG_PTR exStyle = GetWindowLongPtrW(webview_host_hwnd_, GWL_EXSTYLE);
    SetWindowLongPtrW(webview_host_hwnd_, GWL_EXSTYLE, exStyle | WS_EX_LAYERED | WS_EX_TRANSPARENT);
    SetLayeredWindowAttributes(webview_host_hwnd_, 0, 255, LWA_ALPHA);
    std::cout << "[HKCW] Mouse transparency ENABLED" << std::endl;
  } else {
    std::cout << "[HKCW] Mouse transparency DISABLED (interactive mode)" << std::endl;
  }

  // Show window
  ShowWindow(webview_host_hwnd_, SW_SHOW);
  UpdateWindow(webview_host_hwnd_);

  // Initialize WebView2
  SetupWebView2(webview_host_hwnd_, url);

  std::cout << "[HKCW] ========== Initialization Complete ==========" << std::endl;
  return true;
}

bool HkcwEngine2Plugin::StopWallpaper() {
  std::cout << "[HKCW] Stopping wallpaper..." << std::endl;

  if (webview_controller_) {
    webview_controller_->Close();
    webview_controller_ = nullptr;
  }

  webview_ = nullptr;

  if (webview_host_hwnd_) {
    DestroyWindow(webview_host_hwnd_);
    webview_host_hwnd_ = nullptr;
  }

  worker_w_hwnd_ = nullptr;
  is_initialized_ = false;

  std::cout << "[HKCW] Wallpaper stopped" << std::endl;
  return true;
}

bool HkcwEngine2Plugin::NavigateToUrl(const std::string& url) {
  if (!webview_) {
    std::cout << "[HKCW] ERROR: WebView not initialized" << std::endl;
    return false;
  }

  std::wstring wurl(url.begin(), url.end());
  HRESULT hr = webview_->Navigate(wurl.c_str());
  
  if (SUCCEEDED(hr)) {
    std::cout << "[HKCW] Navigated to: " << url << std::endl;
    return true;
  } else {
    std::cout << "[HKCW] ERROR: Navigation failed: " << std::hex << hr << std::endl;
    return false;
  }
}

}  // namespace hkcw_engine2

// Export C API for plugin registration
extern "C" {

__declspec(dllexport) void HkcwEngine2PluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  hkcw_engine2::HkcwEngine2Plugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}

}  // extern "C"

