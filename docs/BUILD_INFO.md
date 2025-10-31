# 构建信息

## 编译完成

### ✅ Debug 版本
- 路径: `example/build/windows/x64/runner/Debug/hkcw_engine2_example.exe`
- 大小: ~较大（包含调试符号）
- 用途: 开发调试，控制台输出日志

### ✅ Release 版本
- 路径: `example/build/windows/x64/runner/Release/hkcw_engine2_example.exe`
- 大小: ~较小（优化编译）
- 用途: 生产环境，性能优化

## 运行说明

### 手动控制
现已取消自动启动壁纸，需手动操作：

1. 启动程序
2. 输入 URL（如：https://www.bing.com）
3. 选择是否启用鼠标透明
4. 点击 "Start Wallpaper" 按钮
5. 点击 "Stop Wallpaper" 停止

### 快速启动脚本

#### Debug 版本
```batch
@echo off
cd example
build\windows\x64\runner\Debug\hkcw_engine2_example.exe
```

#### Release 版本
```batch
@echo off
cd example
build\windows\x64\runner\Release\hkcw_engine2_example.exe
```

## 功能特性

### 鼠标透明模式
- **启用**: 点击穿透到桌面，适合纯壁纸显示
- **禁用**: 可与 WebView 内容交互，适合互动应用

### 支持的 URL 类型
- 网页: `https://www.example.com`
- 本地文件: `file:///C:/path/to/page.html`
- 视频: YouTube embed 等

### 系统要求
- Windows 11 (测试通过)
- Windows 10 (理论兼容)
- WebView2 Runtime (Windows 11 自带)

## 文件结构

```
example/
└── build/
    └── windows/
        └── x64/
            └── runner/
                ├── Debug/
                │   ├── hkcw_engine2_example.exe  ← Debug 可执行文件
                │   ├── flutter_windows.dll
                │   ├── hkcw_engine2_plugin.dll
                │   └── data/
                └── Release/
                    ├── hkcw_engine2_example.exe  ← Release 可执行文件
                    ├── flutter_windows.dll
                    ├── hkcw_engine2_plugin.dll
                    └── data/
```

## 性能对比

| 版本 | 启动速度 | 内存占用 | 文件大小 | 日志输出 |
|------|---------|---------|---------|---------|
| Debug | 慢 | 高 | 大 | ✅ 详细 |
| Release | 快 | 低 | 小 | ❌ 无 |

## 分发说明

如需分发给其他用户，打包 Release 文件夹下的所有文件：
- hkcw_engine2_example.exe
- flutter_windows.dll
- hkcw_engine2_plugin.dll
- data/ 文件夹（包含资源）

---

**构建时间**: 2025-10-31
**Flutter 版本**: 3.x
**编译器**: MSVC 2022

