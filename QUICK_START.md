# 🚀 快速开始指南

## 1️⃣ 安装 WebView2 SDK

```bash
cd scripts
setup_webview2.bat
```

## 2️⃣ 构建并运行

```bash
cd scripts
build_and_run.bat
```

## 3️⃣ 使用应用

1. **输入 URL** - 例如：`https://www.bing.com`
2. **选择模式**:
   - ✅ 勾选"鼠标透明" = 点击穿透（纯壁纸）
   - ❌ 取消勾选 = 可交互（游戏/应用）
3. **点击 "Start Wallpaper"** - 启动壁纸
4. **查看桌面** - WebView 应在图标下方显示

## 🎯 特性

- ✅ WebView2 显示在桌面图标下方
- ✅ 任务栏不被遮挡
- ✅ 图标可正常点击
- ✅ 支持鼠标穿透/交互模式切换

## 📖 详细文档

- [完整中文文档](docs/README_CN.md)
- [使用示例](docs/USAGE_EXAMPLES.md)
- [故障排除](docs/TROUBLESHOOTING.md)

## 🔧 命令参考

### 开发模式
```bash
cd example
flutter run -d windows
```

### 构建 Release
```bash
cd example
flutter build windows --release
```

### 直接运行已编译版本
```bash
# Debug
example\build\windows\x64\runner\Debug\hkcw_engine2_example.exe

# Release
example\build\windows\x64\runner\Release\hkcw_engine2_example.exe
```

---

**更多信息**: [README.md](README.md) | [项目结构](PROJECT_STRUCTURE.md)

