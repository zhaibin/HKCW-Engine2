cmake_minimum_required(VERSION 3.14)
set(PROJECT_NAME "hkcw_engine2")
project(${PROJECT_NAME} LANGUAGES CXX)

# This value is used when generating builds using this plugin, so it must
# not be changed
set(PLUGIN_NAME "hkcw_engine2_plugin")

add_library(${PLUGIN_NAME} SHARED
  "hkcw_engine2_plugin.cpp"
)

apply_standard_settings(${PLUGIN_NAME})
set_target_properties(${PLUGIN_NAME} PROPERTIES
  CXX_VISIBILITY_PRESET hidden)
target_compile_definitions(${PLUGIN_NAME} PRIVATE FLUTTER_PLUGIN_IMPL)
target_include_directories(${PLUGIN_NAME} INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(${PLUGIN_NAME} PRIVATE flutter flutter_wrapper_plugin)

# WebView2 - use NuGet package
set(WEBVIEW2_PACKAGE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/packages/Microsoft.Web.WebView2.1.0.2592.51")
if(EXISTS "${WEBVIEW2_PACKAGE_DIR}")
  # Add WebView2 include directory to both private and interface (public)
  target_include_directories(${PLUGIN_NAME} PRIVATE "${WEBVIEW2_PACKAGE_DIR}/build/native/include")
  target_include_directories(${PLUGIN_NAME} INTERFACE "${WEBVIEW2_PACKAGE_DIR}/build/native/include")
  
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    target_link_libraries(${PLUGIN_NAME} PRIVATE "${WEBVIEW2_PACKAGE_DIR}/build/native/x64/WebView2LoaderStatic.lib")
  else()
    target_link_libraries(${PLUGIN_NAME} PRIVATE "${WEBVIEW2_PACKAGE_DIR}/build/native/x86/WebView2LoaderStatic.lib")
  endif()
else()
  message(FATAL_ERROR "WebView2 package not found at ${WEBVIEW2_PACKAGE_DIR}. Please run nuget install.")
endif()

# Windows SDK libraries
target_link_libraries(${PLUGIN_NAME} PRIVATE
  shlwapi
  version
)

# List of absolute paths to libraries that should be bundled with the plugin
set(hkcw_engine2_bundled_libraries
  ""
  PARENT_SCOPE
)

