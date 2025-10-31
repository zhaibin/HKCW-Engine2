# 项目结构说明

## 📁 目录结构

```
HKCW-Engine2/
│
├── 📚 docs/                          # 文档目录
│   ├── README.md                     # 文档索引
│   ├── README_CN.md                  # 中文完整文档
│   ├── USAGE_EXAMPLES.md             # 使用示例
│   ├── TROUBLESHOOTING.md            # 故障排除
│   ├── SOLUTION_SUMMARY.md           # 解决方案总结
│   ├── TECHNICAL_NOTES.md            # 技术实现细节
│   ├── BUILD_INFO.md                 # 构建信息
│   ├── OPTIMIZATION_GUIDE.md         # 优化指南
│   ├── OPTIMIZATION_COMPLETED.md     # 优化完成报告
│   ├── OPTIMIZATION_TEST_RESULTS.md  # 测试结果
│   ├── GITHUB_SETUP.md               # GitHub 推送指南
│   └── FINAL_SUMMARY.md              # 项目总结
│
├── 🔧 scripts/                       # 脚本目录
│   ├── build_and_run.bat             # 构建并运行
│   ├── run_example.bat               # 直接运行示例
│   ├── setup_webview2.bat            # WebView2 SDK 安装
│   └── PUSH_TO_GITHUB.bat            # GitHub 推送脚本
│
├── 📦 lib/                           # Dart 库
│   └── hkcw_engine2.dart             # Flutter API
│
├── 🪟 windows/                       # Windows 平台实现
│   ├── hkcw_engine2_plugin.h         # 插件头文件
│   ├── hkcw_engine2_plugin.cpp       # 核心 C++ 实现
│   ├── CMakeLists.txt                # CMake 配置
│   ├── packages.config               # NuGet 包配置
│   ├── packages/                     # WebView2 SDK
│   │   └── Microsoft.Web.WebView2.../
│   └── include/                      # 公共头文件
│       └── hkcw_engine2/
│           ├── hkcw_engine2_plugin.h
│           └── hkcw_engine2_plugin_c_api.h
│
├── 🎮 example/                       # 示例应用
│   ├── lib/
│   │   └── main.dart                 # 示例应用主文件
│   ├── windows/                      # Windows 运行器
│   ├── pubspec.yaml                  # 依赖配置
│   └── build/                        # 编译产物
│       └── windows/x64/runner/
│           ├── Debug/                # Debug 版本
│           └── Release/              # Release 版本
│
├── 📄 根目录文件
│   ├── README.md                     # 项目主文档（英文）
│   ├── pubspec.yaml                  # 插件配置
│   ├── .gitignore                    # Git 忽略配置
│   └── PROJECT_STRUCTURE.md          # 本文件
│
└── 🔒 .git/                          # Git 版本控制

```

## 📚 文档说明

### 用户文档（面向使用者）
- `README.md` - 项目主页，快速开始
- `docs/README_CN.md` - 完整中文指南
- `docs/USAGE_EXAMPLES.md` - 代码示例
- `docs/TROUBLESHOOTING.md` - 问题解决

### 技术文档（面向开发者）
- `docs/SOLUTION_SUMMARY.md` - 技术方案
- `docs/TECHNICAL_NOTES.md` - 实现细节
- `docs/BUILD_INFO.md` - 编译说明

### 优化文档（性能与安全）
- `docs/OPTIMIZATION_GUIDE.md` - 优化策略
- `docs/OPTIMIZATION_COMPLETED.md` - P0/P1 优化报告
- `docs/OPTIMIZATION_TEST_RESULTS.md` - 测试验证

### 开发文档
- `docs/GITHUB_SETUP.md` - GitHub 操作指南
- `docs/FINAL_SUMMARY.md` - 项目总览

## 🔧 脚本说明

### 开发脚本
- `scripts/build_and_run.bat` - 一键构建并运行 Debug 版本
- `scripts/run_example.bat` - 直接运行示例（使用 flutter run）

### 环境配置
- `scripts/setup_webview2.bat` - 下载并安装 WebView2 SDK

### 部署脚本
- `scripts/PUSH_TO_GITHUB.bat` - 推送代码到 GitHub

## 🎯 核心文件

### Dart API
- `lib/hkcw_engine2.dart` - Flutter 插件 API
  - `initializeWallpaper()` - 初始化壁纸
  - `stopWallpaper()` - 停止壁纸
  - `navigateToUrl()` - 导航到 URL

### C++ 核心
- `windows/hkcw_engine2_plugin.cpp` - 主要实现
  - WorkerW 查找逻辑
  - WebView2 初始化
  - 资源管理
  - 优化功能

### 配置文件
- `pubspec.yaml` - Flutter 插件配置
- `windows/CMakeLists.txt` - CMake 构建配置
- `windows/packages.config` - NuGet 包配置
- `.gitignore` - Git 忽略规则

## 🚀 快速导航

### 我想...

#### 使用这个插件
→ 阅读 `README.md` 和 `docs/README_CN.md`

#### 构建项目
→ 运行 `scripts/setup_webview2.bat`  
→ 运行 `scripts/build_and_run.bat`

#### 了解实现原理
→ 阅读 `docs/SOLUTION_SUMMARY.md`  
→ 阅读 `docs/TECHNICAL_NOTES.md`

#### 优化性能
→ 阅读 `docs/OPTIMIZATION_GUIDE.md`

#### 解决问题
→ 阅读 `docs/TROUBLESHOOTING.md`

#### 贡献代码
→ 阅读 `docs/GITHUB_SETUP.md`

## 📊 统计信息

- **总文档数**: 12 个
- **总脚本数**: 4 个
- **代码文件**: 10+ 个
- **示例文件**: 5+ 个

## 🔄 更新日志

### v1.1.0 (2025-10-31) - 优化版
- ✅ 添加 P0 优化（内存/异常/安全）
- ✅ 添加 P1 优化（性能/缓存/权限）
- ✅ 重组项目结构（docs + scripts）
- ✅ 完善文档体系

### v1.0.0 (2025-10-31) - 初始版本
- ✅ 核心功能实现
- ✅ Windows 10/11 支持
- ✅ 基础文档

---

**项目主页**: [README.md](../README.md)  
**中文文档**: [README_CN.md](README_CN.md)

