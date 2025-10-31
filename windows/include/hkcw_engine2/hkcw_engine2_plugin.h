#ifndef FLUTTER_PLUGIN_HKCW_ENGINE2_PLUGIN_H_
#define FLUTTER_PLUGIN_HKCW_ENGINE2_PLUGIN_H_

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <windows.h>
#include <wrl.h>
#include <WebView2.h>
#include <memory>

// C API for plugin registration
#ifdef __cplusplus
extern "C" {
#endif

#ifdef FLUTTER_PLUGIN_IMPL
#define FLUTTER_PLUGIN_EXPORT __declspec(dllexport)
#else
#define FLUTTER_PLUGIN_EXPORT __declspec(dllimport)
#endif

FLUTTER_PLUGIN_EXPORT void HkcwEngine2PluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar);

#ifdef __cplusplus
}  // extern "C"
#endif

namespace hkcw_engine2 {

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

  HWND webview_host_hwnd_ = nullptr;
  HWND worker_w_hwnd_ = nullptr;
  Microsoft::WRL::ComPtr<ICoreWebView2Controller> webview_controller_;
  Microsoft::WRL::ComPtr<ICoreWebView2> webview_;
  bool is_initialized_ = false;
};

}  // namespace hkcw_engine2

#endif  // FLUTTER_PLUGIN_HKCW_ENGINE2_PLUGIN_H_

