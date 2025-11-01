# 动态内容点击解决方案

## 🎯 问题描述

**场景**: 网页中的广告或动态内容由 JavaScript 延迟生成
- ❌ **问题**: `HKCW.onClick()` 在元素生成前注册，找不到元素
- ❌ **问题**: 广告加载后位置变化，点击区域失效
- ❌ **问题**: 复杂 JS 生成的 DOM 结构难以预测

---

## 💡 解决方案

### 方案 1: 延迟注册（简单）

#### 增加延迟时间
```javascript
// 原来：2秒延迟
HKCW.onClick('#ad-area', callback);  // 2秒后注册

// 改进：等待广告加载
setTimeout(() => {
  HKCW.onClick('#ad-area', callback);
}, 5000);  // 5秒后注册，确保广告已加载
```

**优点**: 简单直接  
**缺点**: 固定延迟不灵活

---

### 方案 2: MutationObserver 监听（推荐）

#### 自动检测 DOM 变化并注册

**在 SDK 中添加**:
```javascript
// 扩展 HKCW SDK
HKCW.onClickDynamic = function(selector, callback, options = {}) {
  const self = this;
  
  // 立即尝试注册
  this.onClick(selector, callback, options);
  
  // 监听 DOM 变化
  const observer = new MutationObserver((mutations) => {
    // 检查是否有新元素匹配选择器
    const element = document.querySelector(selector);
    if (element) {
      // 检查是否已注册
      const already_registered = self._clickHandlers.some(h => h.element === element);
      if (!already_registered) {
        console.log('[HKCW] Dynamic element detected, re-registering:', selector);
        self.onClick(selector, callback, options);
      }
    }
  });
  
  // 开始监听
  observer.observe(document.body, {
    childList: true,
    subtree: true
  });
  
  return observer;  // 返回 observer 以便后续停止
};
```

**使用**:
```javascript
// 自动处理动态内容
HKCW.onClickDynamic('#ad-banner', () => {
  HKCW.openURL('https://ad-link.com');
});
```

**优点**: 自动检测，无需手动控制  
**缺点**: 持续监听有轻微性能开销

---

### 方案 3: 手动刷新点击区域

#### 添加刷新 API

**在 SDK 中添加**:
```javascript
HKCW.refreshClickAreas = function() {
  console.log('[HKCW] Refreshing click areas...');
  
  // 重新计算所有已注册元素的边界
  this._clickHandlers.forEach(handler => {
    const newBounds = this._calculateElementBounds(handler.element);
    if (newBounds) {
      handler.bounds = newBounds;
      console.log('[HKCW] Updated bounds for:', handler.element.id || handler.element);
    }
  });
  
  console.log('[HKCW] Refreshed', this._clickHandlers.length, 'click areas');
};
```

**使用**:
```javascript
// 页面加载完成
window.addEventListener('load', () => {
  HKCW.onClick('#my-button', callback);
});

// 广告加载完成后手动刷新
window.addEventListener('adLoaded', () => {
  HKCW.refreshClickAreas();
});

// 或定期刷新
setInterval(() => {
  HKCW.refreshClickAreas();
}, 10000);  // 每10秒刷新
```

**优点**: 完全控制，灵活  
**缺点**: 需要知道广告何时加载

---

### 方案 4: 事件委托（最佳）

#### 使用父容器 + 条件判断

**在 SDK 中添加**:
```javascript
HKCW.onClickArea = function(parentSelector, childMatcher, callback, options = {}) {
  // 在父容器上注册
  this.onClick(parentSelector, (x, y) => {
    // 检查点击的具体子元素
    const cssX = x / this.dpiScale;
    const cssY = y / this.dpiScale;
    
    const element = document.elementFromPoint(cssX, cssY);
    
    // 判断是否匹配子元素条件
    if (typeof childMatcher === 'function') {
      if (childMatcher(element)) {
        callback(x, y, element);
      }
    } else if (typeof childMatcher === 'string') {
      if (element && element.matches(childMatcher)) {
        callback(x, y, element);
      }
    }
  }, options);
};
```

**使用**:
```javascript
// 方法 A: 使用选择器
HKCW.onClickArea('#ad-container', '.ad-item', (x, y, element) => {
  const adId = element.dataset.adId;
  HKCW.openURL('https://ad.com?id=' + adId);
});

// 方法 B: 使用判断函数
HKCW.onClickArea('#content', (el) => {
  return el.classList.contains('clickable') || el.dataset.link;
}, (x, y, element) => {
  HKCW.openURL(element.dataset.link);
});
```

**优点**:
- ✅ 支持动态生成的子元素
- ✅ 无需知道具体元素 ID
- ✅ 自动处理 DOM 变化

**缺点**: 需要父容器固定

---

### 方案 5: 全局点击 + 条件过滤

#### 捕获所有桌面点击，在回调中判断

**在 SDK 中添加**:
```javascript
HKCW.onAnyClick = function(callback) {
  // 监听所有桌面点击
  window.addEventListener('hkcw:mouse', (event) => {
    if (event.detail.type === 'mouseup' && event.detail.button === 0) {
      const x = event.detail.x;
      const y = event.detail.y;
      const cssX = x / this.dpiScale;
      const cssY = y / this.dpiScale;
      
      // 获取点击位置的元素
      const element = document.elementFromPoint(cssX, cssY);
      
      // 回调中判断
      callback(x, y, element);
    }
  });
};
```

**使用**:
```javascript
HKCW.onAnyClick((x, y, element) => {
  // 检查元素类型
  if (element.classList.contains('ad-banner')) {
    HKCW.openURL(element.dataset.url);
  } else if (element.id === 'weather-card') {
    showWeatherDetails();
  } else if (element.matches('.clickable')) {
    handleClick(element);
  }
});
```

**优点**:
- ✅ 最灵活
- ✅ 支持任意动态内容
- ✅ 无需预注册

**缺点**: 每次点击都执行判断逻辑

---

## 🎯 推荐方案对比

| 方案 | 复杂度 | 性能 | 灵活性 | 推荐场景 |
|------|--------|------|--------|----------|
| 延迟注册 | ⭐ | ⭐⭐⭐⭐⭐ | ⭐⭐ | 固定延迟可预测的广告 |
| MutationObserver | ⭐⭐⭐ | ⭐⭐⭐⭐ | ⭐⭐⭐⭐ | 动态生成的已知元素 |
| 手动刷新 | ⭐⭐ | ⭐⭐⭐⭐⭐ | ⭐⭐⭐ | 可控的广告加载事件 |
| 事件委托 | ⭐⭐ | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ | **推荐** - 动态子元素 |
| 全局点击 | ⭐⭐ | ⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ | 复杂交互逻辑 |

---

## 🚀 实施建议

### 立即可用方案（事件委托）

**修改测试页面**:
```javascript
// 不使用具体元素 ID
HKCW.onClick('#ad-container', callback);

// 在回调中判断具体点击的子元素
function handleAdClick(x, y) {
  const cssX = x / HKCW.dpiScale;
  const cssY = y / HKCW.dpiScale;
  
  const element = document.elementFromPoint(cssX, cssY);
  
  if (element.classList.contains('ad-item')) {
    HKCW.openURL(element.dataset.url);
  }
}
```

---

### 增强 SDK（添加到 HKCW-WEB）

**建议在 https://theme-web.haokan.mobi/sdk/hkcw-engine.js 添加**:

```javascript
// 1. 动态元素支持
HKCW.onClickDynamic = function(selector, callback, options) {
  // MutationObserver 实现
};

// 2. 刷新 API
HKCW.refreshClickAreas = function() {
  // 重新计算边界
};

// 3. 事件委托
HKCW.onClickArea = function(parent, child, callback) {
  // 委托实现
};

// 4. 全局点击
HKCW.onAnyClick = function(callback) {
  // 全局监听
};
```

---

## 📝 完整示例

### 示例 1: 广告横幅（延迟加载）

```html
<div id="ad-container"></div>

<script>
  // 广告 SDK 异步加载
  loadAdSDK().then(() => {
    // 广告插入到 #ad-container
  });
  
  // 方法 A: 等待广告加载
  window.addEventListener('adLoaded', () => {
    HKCW.onClick('.ad-banner', () => {
      HKCW.openURL('https://ad-link.com');
    });
  });
  
  // 方法 B: 使用父容器
  HKCW.onClick('#ad-container', (x, y) => {
    const cssX = x / HKCW.dpiScale;
    const cssY = y / HKCW.dpiScale;
    const el = document.elementFromPoint(cssX, cssY);
    
    if (el && el.classList.contains('ad-banner')) {
      HKCW.openURL(el.dataset.url || 'https://default-ad.com');
    }
  });
</script>
```

---

### 示例 2: 复杂布局（多个广告位）

```html
<div id="ad-sidebar">
  <!-- JS 动态插入广告 -->
</div>

<script>
  // 使用事件委托处理所有广告
  HKCW.onClick('#ad-sidebar', (x, y) => {
    const cssX = x / HKCW.dpiScale;
    const cssY = y / HKCW.dpiScale;
    const element = document.elementFromPoint(cssX, cssY);
    
    // 向上查找广告元素
    let adElement = element;
    while (adElement && adElement !== document.body) {
      if (adElement.dataset.adUrl) {
        HKCW.openURL(adElement.dataset.adUrl);
        return;
      }
      if (adElement.classList.contains('ad-item')) {
        const url = adElement.querySelector('a')?.href;
        if (url) HKCW.openURL(url);
        return;
      }
      adElement = adElement.parentElement;
    }
  });
</script>
```

---

### 示例 3: 定期刷新（保守方案）

```javascript
// 注册所有已知区域
function registerClickAreas() {
  const buttons = document.querySelectorAll('.clickable-ad');
  buttons.forEach((btn, index) => {
    HKCW.onClick(btn, () => {
      HKCW.openURL(btn.dataset.url);
    }, {debug: true});
  });
}

// 初始注册
setTimeout(registerClickAreas, 3000);

// 定期重新注册（处理动态内容）
setInterval(registerClickAreas, 15000);  // 每15秒
```

---

### 示例 4: 监听广告SDK事件

```javascript
// 监听第三方广告SDK的加载事件
window.addEventListener('message', (event) => {
  if (event.data.type === 'adLoaded') {
    console.log('[HKCW] Ad loaded, registering click areas...');
    
    // 重新扫描并注册
    document.querySelectorAll('[data-ad-clickable]').forEach(ad => {
      HKCW.onClick(ad, () => {
        HKCW.openURL(ad.dataset.adUrl);
      });
    });
  }
});
```

---

## 🔧 Native 端增强

### 添加刷新 API（C++ 端）

**在 Dart API 中添加**:
```dart
// lib/hkcw_engine2.dart
class HkcwEngine2 {
  // 刷新网页（重新计算点击区域）
  static Future<bool> refreshClickAreas() async {
    final result = await _channel.invokeMethod<bool>('refreshClickAreas');
    return result ?? false;
  }
}
```

**在 C++ 中实现**:
```cpp
// windows/hkcw_engine2_plugin.cpp
else if (method_call.method_name() == "refreshClickAreas") {
  bool success = RefreshClickAreas();
  result->Success(flutter::EncodableValue(success));
}

bool HkcwEngine2Plugin::RefreshClickAreas() {
  if (!webview_) return false;
  
  // 调用 JavaScript 刷新函数
  std::wstring script = L"if (window.HKCW && window.HKCW.refreshClickAreas) { HKCW.refreshClickAreas(); }";
  webview_->ExecuteScript(script.c_str(), nullptr);
  
  return true;
}
```

**使用**:
```dart
// 广告加载后从 Flutter 端触发刷新
Timer.periodic(Duration(seconds: 15), (_) {
  HkcwEngine2.refreshClickAreas();
});
```

---

## 🎨 实用模式

### 模式 1: 固定容器 + 动态内容

```html
<div id="fixed-container" style="position: fixed; width: 300px; height: 200px;">
  <!-- 广告 JS 会在这里插入内容 -->
</div>

<script>
  // 只注册固定容器
  HKCW.onClick('#fixed-container', (x, y) => {
    // 实时查询点击位置的元素
    const cssX = x / HKCW.dpiScale;
    const cssY = y / HKCW.dpiScale;
    const target = document.elementFromPoint(cssX, cssY);
    
    // 根据实际元素决定行为
    if (target.tagName === 'A') {
      HKCW.openURL(target.href);
    } else if (target.dataset.action) {
      executeAction(target.dataset.action);
    }
  });
</script>
```

**适用**: 广告在固定容器内变化

---

### 模式 2: CSS 类选择器

```javascript
// 使用类而不是 ID
HKCW.onClick('.ad-banner', callback);  // 匹配第一个
HKCW.onClick('.clickable-item', callback);  // 匹配第一个

// 或者遍历所有
document.querySelectorAll('.ad-item').forEach((ad, index) => {
  HKCW.onClick(ad, () => {
    HKCW.openURL(ad.dataset.url);
  });
});
```

---

### 模式 3: 延迟 + 重试

```javascript
function tryRegisterAd(selector, callback, maxRetries = 10) {
  let retries = 0;
  
  const attemptRegister = () => {
    const element = document.querySelector(selector);
    if (element) {
      HKCW.onClick(element, callback, {debug: true});
      console.log('[HKCW] Registered:', selector);
    } else {
      retries++;
      if (retries < maxRetries) {
        setTimeout(attemptRegister, 1000);  // 1秒后重试
      } else {
        console.warn('[HKCW] Failed to register:', selector);
      }
    }
  };
  
  attemptRegister();
}

// 使用
tryRegisterAd('#late-loading-ad', () => {
  HKCW.openURL('https://ad.com');
});
```

---

## 🎯 最佳实践

### 推荐方案：**事件委托 + elementFromPoint**

```javascript
// 1. 在已知的容器上注册
HKCW.onClick('#main-content', handleContentClick);

// 2. 回调中实时检测
function handleContentClick(x, y) {
  const cssX = x / HKCW.dpiScale;
  const cssY = y / HKCW.dpiScale;
  
  // 实时获取点击的元素
  const element = document.elementFromPoint(cssX, cssY);
  
  // 根据元素属性判断
  if (element.dataset.adUrl) {
    HKCW.openURL(element.dataset.adUrl);
  } else if (element.classList.contains('action-button')) {
    executeAction(element.dataset.action);
  } else if (element.closest('.ad-container')) {
    const adContainer = element.closest('.ad-container');
    HKCW.openURL(adContainer.dataset.url);
  }
}
```

**优势**:
- ✅ 处理任意动态内容
- ✅ 无需知道元素何时生成
- ✅ 性能开销极小
- ✅ 代码简洁

---

## 💻 完整解决方案（可直接使用）

```javascript
// ====================================
// HKCW 动态内容处理工具
// ====================================

const HKCWDynamic = {
  // 方法1: 监听容器，实时检测子元素
  onClickContainer: function(containerSelector, handler) {
    HKCW.onClick(containerSelector, (x, y) => {
      const cssX = x / HKCW.dpiScale;
      const cssY = y / HKCW.dpiScale;
      const element = document.elementFromPoint(cssX, cssY);
      handler(element, x, y);
    });
  },
  
  // 方法2: 自动重试注册
  onClickRetry: function(selector, callback, options = {}) {
    const maxRetries = options.maxRetries || 10;
    const interval = options.interval || 1000;
    let retries = 0;
    
    const attempt = () => {
      const el = document.querySelector(selector);
      if (el) {
        HKCW.onClick(el, callback, options);
        console.log('[HKCW] Registered:', selector);
      } else if (retries++ < maxRetries) {
        setTimeout(attempt, interval);
      }
    };
    
    attempt();
  },
  
  // 方法3: 智能检测（推荐）
  smartClick: function(containerSelector, rules) {
    HKCW.onClick(containerSelector, (x, y) => {
      const cssX = x / HKCW.dpiScale;
      const cssY = y / HKCW.dpiScale;
      const element = document.elementFromPoint(cssX, cssY);
      
      // 应用规则
      for (const rule of rules) {
        if (rule.selector && element.matches(rule.selector)) {
          rule.callback(element, x, y);
          return;
        }
        if (rule.class && element.classList.contains(rule.class)) {
          rule.callback(element, x, y);
          return;
        }
        if (rule.data && element.dataset[rule.data]) {
          rule.callback(element, x, y);
          return;
        }
      }
    });
  }
};

// ====================================
// 使用示例
// ====================================

// 示例1: 容器监听
HKCWDynamic.onClickContainer('#ad-sidebar', (element) => {
  if (element.dataset.adUrl) {
    HKCW.openURL(element.dataset.adUrl);
  }
});

// 示例2: 自动重试
HKCWDynamic.onClickRetry('#lazy-loaded-ad', () => {
  HKCW.openURL('https://ad.com');
}, {maxRetries: 20, interval: 500});

// 示例3: 智能检测（推荐）
HKCWDynamic.smartClick('#content', [
  {
    selector: '.ad-banner',
    callback: (el) => HKCW.openURL(el.dataset.url)
  },
  {
    class: 'weather-card',
    callback: (el) => showWeather()
  },
  {
    data: 'clickAction',
    callback: (el) => executeAction(el.dataset.clickAction)
  }
]);
```

---

## 🎯 总结

### ✅ 推荐方案

**最佳**: **事件委托 + elementFromPoint**

**原因**:
1. ✅ 支持所有动态内容
2. ✅ 无需知道加载时机
3. ✅ 性能开销极小
4. ✅ 代码简洁易维护

**实现**:
```javascript
// 在容器上注册，回调中实时检测
HKCW.onClick('#container', (x, y) => {
  const el = document.elementFromPoint(x/DPI, y/DPI);
  if (el.matches('.ad')) HKCW.openURL(el.dataset.url);
});
```

**性能**: 每次点击额外 ~0.01ms（可忽略）

---

**建议**: 当前 Native 端无需修改，在网页 JavaScript 中使用事件委托模式即可完美解决动态广告点击问题！ ✅

