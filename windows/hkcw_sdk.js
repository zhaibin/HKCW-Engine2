// HKCW Engine SDK v3.1.0 - JavaScript Bridge
// Auto-injected into WebView2

(function() {
  'use strict';

  // HKCW Global Object
  window.HKCW = {
    version: '3.1.0',
    dpiScale: window.devicePixelRatio || 1,
    screenWidth: screen.width * (window.devicePixelRatio || 1),
    screenHeight: screen.height * (window.devicePixelRatio || 1),
    interactionEnabled: false,
    
    // Debug mode
    _debugMode: false,
    _clickHandlers: [],
    _mouseCallbacks: [],
    _keyboardCallbacks: [],
    
    // Initialize
    _init: function() {
      console.log('========================================');
      console.log('HKCW Engine v' + this.version);
      console.log('========================================');
      console.log('Screen: ' + this.screenWidth + 'x' + this.screenHeight);
      console.log('DPI Scale: ' + this.dpiScale + 'x');
      console.log('User Agent: ' + navigator.userAgent);
      console.log('========================================');
      
      // Detect debug mode from URL
      this._detectDebugMode();
      
      // Setup event listeners
      this._setupEventListeners();
    },
    
    // Detect debug mode from URL parameter
    _detectDebugMode: function() {
      const urlParams = new URLSearchParams(window.location.search);
      if (urlParams.has('debug')) {
        this._debugMode = true;
        console.log('[HKCW] Debug mode ENABLED via URL parameter');
      }
    },
    
    // Enable debug mode manually
    enableDebug: function() {
      this._debugMode = true;
      console.log('[HKCW] Debug mode ENABLED manually');
    },
    
    // Log with debug control
    _log: function(message, forceLog) {
      if (this._debugMode || forceLog) {
        console.log('[HKCW] ' + message);
      }
    },
    
    // Calculate element bounds in physical pixels
    _calculateElementBounds: function(element) {
      const rect = element.getBoundingClientRect();
      const dpi = this.dpiScale;
      
      return {
        left: Math.round(rect.left * dpi),
        top: Math.round(rect.top * dpi),
        right: Math.round(rect.right * dpi),
        bottom: Math.round(rect.bottom * dpi),
        width: Math.round(rect.width * dpi),
        height: Math.round(rect.height * dpi)
      };
    },
    
    // Show debug border
    _showDebugBorder: function(bounds, element) {
      const border = document.createElement('div');
      border.className = 'hkcw-debug-border';
      border.style.cssText = 
        'position: fixed;' +
        'left: ' + (bounds.left / this.dpiScale) + 'px;' +
        'top: ' + (bounds.top / this.dpiScale) + 'px;' +
        'width: ' + (bounds.width / this.dpiScale) + 'px;' +
        'height: ' + (bounds.height / this.dpiScale) + 'px;' +
        'border: 2px solid red;' +
        'box-shadow: 0 0 10px red;' +
        'pointer-events: none;' +
        'z-index: 999999;';
      document.body.appendChild(border);
    },
    
    // Check if point is in bounds
    _isInBounds: function(x, y, bounds) {
      return x >= bounds.left && x <= bounds.right &&
             y >= bounds.top && y <= bounds.bottom;
    },
    
    // Handle click event from native
    _handleClick: function(x, y) {
      this._log('Click at physical: (' + x + ',' + y + ') CSS: (' + 
                (x / this.dpiScale) + ',' + (y / this.dpiScale) + ')');
      
      for (let i = 0; i < this._clickHandlers.length; i++) {
        const handler = this._clickHandlers[i];
        if (this._isInBounds(x, y, handler.bounds)) {
          this._log('  -> HIT: ' + (handler.element.id || handler.element.className));
          handler.callback(x, y);
          break;
        }
      }
    },
    
    // Register click handler
    onClick: function(element, callback, options) {
      const self = this;
      options = options || {};
      
      // Delay registration to ensure DOM is ready
      setTimeout(function() {
        // Get element
        let el = element;
        if (typeof element === 'string') {
          el = document.querySelector(element);
        }
        
        if (!el) {
          console.error('[HKCW] Element not found:', element);
          return;
        }
        
        // Calculate bounds
        const bounds = self._calculateElementBounds(el);
        
        // Register handler
        self._clickHandlers.push({
          element: el,
          callback: callback,
          bounds: bounds
        });
        
        // Debug output
        const showDebug = (options.debug !== undefined) ? options.debug : self._debugMode;
        if (showDebug) {
          console.log('----------------------------------------');
          console.log('Click Handler Registered:');
          console.log('  Element:', el.id || el.className || 'unknown');
          console.log('  Physical: [' + bounds.left + ',' + bounds.top + '] ~ [' + 
                      bounds.right + ',' + bounds.bottom + ']');
          console.log('  Size: ' + bounds.width + 'x' + bounds.height + ' px');
          console.log('  CSS: [' + (bounds.left / self.dpiScale) + ',' + 
                      (bounds.top / self.dpiScale) + '] ' + 
                      Math.round(bounds.width / self.dpiScale) + 'x' + 
                      Math.round(bounds.height / self.dpiScale));
          console.log('----------------------------------------');
          
          self._showDebugBorder(bounds, el);
        }
      }, 2000);
    },
    
    // Open URL in default browser
    openURL: function(url) {
      this._log('Opening URL: ' + url);
      
      // Call native method via postMessage
      if (window.chrome && window.chrome.webview) {
        window.chrome.webview.postMessage({
          type: 'openURL',
          url: url
        });
      } else {
        console.warn('[HKCW] Native bridge not available, opening in current window');
        window.open(url, '_blank');
      }
    },
    
    // Notify wallpaper is ready
    ready: function(name) {
      this._log('Wallpaper ready: ' + name, true);
      
      if (window.chrome && window.chrome.webview) {
        window.chrome.webview.postMessage({
          type: 'ready',
          name: name
        });
      }
    },
    
    // Register mouse event callback
    onMouse: function(callback) {
      this._mouseCallbacks.push(callback);
      this._log('Mouse callback registered (total: ' + this._mouseCallbacks.length + ')');
    },
    
    // Register keyboard event callback
    onKeyboard: function(callback) {
      this._keyboardCallbacks.push(callback);
      this._log('Keyboard callback registered (total: ' + this._keyboardCallbacks.length + ')');
    },
    
    // Setup event listeners
    _setupEventListeners: function() {
      const self = this;
      
      // Listen for custom events from native
      window.addEventListener('hkcw:mouse', function(event) {
        const detail = event.detail;
        self._mouseCallbacks.forEach(function(cb) {
          cb(detail);
        });
      });
      
      window.addEventListener('hkcw:keyboard', function(event) {
        const detail = event.detail;
        self._keyboardCallbacks.forEach(function(cb) {
          cb(detail);
        });
      });
      
      window.addEventListener('hkcw:click', function(event) {
        const detail = event.detail;
        self._handleClick(detail.x, detail.y);
      });
      
      window.addEventListener('hkcw:interactionMode', function(event) {
        self.interactionEnabled = event.detail.enabled;
        self._log('Interaction mode: ' + (self.interactionEnabled ? 'ON' : 'OFF'), true);
      });
    }
  };
  
  // Auto initialize
  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', function() {
      HKCW._init();
    });
  } else {
    HKCW._init();
  }
  
  console.log('[HKCW] SDK loaded successfully');
})();

