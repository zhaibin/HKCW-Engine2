import 'dart:async';
import 'package:flutter/services.dart';

class HkcwEngine2 {
  static const MethodChannel _channel = MethodChannel('hkcw_engine2');

  /// Initialize WebView2 as desktop wallpaper
  static Future<bool> initializeWallpaper({
    required String url,
    bool enableMouseTransparent = true,
  }) async {
    try {
      final result = await _channel.invokeMethod<bool>('initializeWallpaper', {
        'url': url,
        'enableMouseTransparent': enableMouseTransparent,
      });
      return result ?? false;
    } catch (e) {
      print('Error initializing wallpaper: $e');
      return false;
    }
  }

  /// Stop and cleanup wallpaper
  static Future<bool> stopWallpaper() async {
    try {
      final result = await _channel.invokeMethod<bool>('stopWallpaper');
      return result ?? false;
    } catch (e) {
      print('Error stopping wallpaper: $e');
      return false;
    }
  }

  /// Navigate to URL
  static Future<bool> navigateToUrl(String url) async {
    try {
      final result = await _channel.invokeMethod<bool>('navigateToUrl', {
        'url': url,
      });
      return result ?? false;
    } catch (e) {
      print('Error navigating to URL: $e');
      return false;
    }
  }
}

