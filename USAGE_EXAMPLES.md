# Usage Examples

## Basic Usage

### Simple Wallpaper Display

```dart
import 'package:hkcw_engine2/hkcw_engine2.dart';

void main() async {
  // Initialize with a simple webpage
  final success = await HkcwEngine2.initializeWallpaper(
    url: 'https://www.bing.com',
    enableMouseTransparent: true,
  );
  
  if (success) {
    print('Wallpaper initialized successfully!');
  }
}
```

### Interactive Web Content

```dart
// For interactive content (games, apps), disable mouse transparency
final success = await HkcwEngine2.initializeWallpaper(
  url: 'https://example.com/game',
  enableMouseTransparent: false,  // Allow mouse interaction
);
```

### Video Wallpaper

```dart
// YouTube embed or video URL
await HkcwEngine2.initializeWallpaper(
  url: 'https://www.youtube.com/embed/VIDEO_ID?autoplay=1&loop=1&mute=1',
  enableMouseTransparent: true,
);
```

## Advanced Usage

### Dynamic URL Navigation

```dart
class WallpaperController {
  final List<String> urls = [
    'https://www.bing.com',
    'https://www.google.com',
    'https://example.com',
  ];
  
  int currentIndex = 0;
  
  Future<void> start() async {
    await HkcwEngine2.initializeWallpaper(
      url: urls[currentIndex],
      enableMouseTransparent: true,
    );
  }
  
  Future<void> next() async {
    currentIndex = (currentIndex + 1) % urls.length;
    await HkcwEngine2.navigateToUrl(urls[currentIndex]);
  }
  
  Future<void> previous() async {
    currentIndex = (currentIndex - 1 + urls.length) % urls.length;
    await HkcwEngine2.navigateToUrl(urls[currentIndex]);
  }
  
  Future<void> stop() async {
    await HkcwEngine2.stopWallpaper();
  }
}
```

### Timer-Based Rotation

```dart
import 'dart:async';

class RotatingWallpaper {
  Timer? _timer;
  final List<String> urls;
  int currentIndex = 0;
  
  RotatingWallpaper(this.urls);
  
  Future<void> start({Duration interval = const Duration(minutes: 5)}) async {
    // Initialize first wallpaper
    await HkcwEngine2.initializeWallpaper(
      url: urls[currentIndex],
      enableMouseTransparent: true,
    );
    
    // Start rotation timer
    _timer = Timer.periodic(interval, (_) async {
      currentIndex = (currentIndex + 1) % urls.length;
      await HkcwEngine2.navigateToUrl(urls[currentIndex]);
      print('Switched to: ${urls[currentIndex]}');
    });
  }
  
  void stop() {
    _timer?.cancel();
    HkcwEngine2.stopWallpaper();
  }
}

// Usage
void main() async {
  final wallpaper = RotatingWallpaper([
    'https://www.bing.com',
    'https://unsplash.com',
    'https://example.com/scene1',
  ]);
  
  await wallpaper.start(interval: Duration(minutes: 10));
}
```

### Custom HTML Content

```dart
import 'dart:io';
import 'package:path_provider/path_provider.dart';

Future<void> showCustomHTML() async {
  // Create custom HTML
  final html = '''
<!DOCTYPE html>
<html>
<head>
  <style>
    body {
      margin: 0;
      padding: 0;
      background: linear-gradient(45deg, #667eea 0%, #764ba2 100%);
      display: flex;
      justify-content: center;
      align-items: center;
      height: 100vh;
      font-family: Arial, sans-serif;
    }
    .content {
      text-align: center;
      color: white;
    }
    h1 {
      font-size: 72px;
      animation: pulse 2s infinite;
    }
    @keyframes pulse {
      0%, 100% { transform: scale(1); }
      50% { transform: scale(1.1); }
    }
  </style>
</head>
<body>
  <div class="content">
    <h1>My Custom Wallpaper</h1>
    <p>Current Time: <span id="time"></span></p>
  </div>
  <script>
    setInterval(() => {
      document.getElementById('time').textContent = new Date().toLocaleTimeString();
    }, 1000);
  </script>
</body>
</html>
''';
  
  // Save to temporary file
  final dir = await getTemporaryDirectory();
  final file = File('${dir.path}/wallpaper.html');
  await file.writeAsString(html);
  
  // Display as wallpaper
  await HkcwEngine2.initializeWallpaper(
    url: 'file:///${file.path}',
    enableMouseTransparent: true,
  );
}
```

### Error Handling

```dart
Future<void> initializeWithErrorHandling() async {
  try {
    final success = await HkcwEngine2.initializeWallpaper(
      url: 'https://example.com',
      enableMouseTransparent: true,
    );
    
    if (!success) {
      print('Failed to initialize wallpaper');
      // Show error dialog or notification
      return;
    }
    
    print('Wallpaper initialized successfully');
  } catch (e) {
    print('Error: $e');
    // Handle exception
  }
}
```

### State Management with Provider

```dart
import 'package:flutter/material.dart';
import 'package:provider/provider.dart';

class WallpaperState extends ChangeNotifier {
  bool _isRunning = false;
  String _currentUrl = '';
  bool _mouseTransparent = true;
  
  bool get isRunning => _isRunning;
  String get currentUrl => _currentUrl;
  bool get mouseTransparent => _mouseTransparent;
  
  Future<void> start(String url) async {
    final success = await HkcwEngine2.initializeWallpaper(
      url: url,
      enableMouseTransparent: _mouseTransparent,
    );
    
    if (success) {
      _isRunning = true;
      _currentUrl = url;
      notifyListeners();
    }
  }
  
  Future<void> stop() async {
    await HkcwEngine2.stopWallpaper();
    _isRunning = false;
    _currentUrl = '';
    notifyListeners();
  }
  
  Future<void> navigate(String url) async {
    if (!_isRunning) return;
    
    final success = await HkcwEngine2.navigateToUrl(url);
    if (success) {
      _currentUrl = url;
      notifyListeners();
    }
  }
  
  void toggleTransparency() {
    _mouseTransparent = !_mouseTransparent;
    notifyListeners();
    
    // Restart with new setting
    if (_isRunning) {
      stop().then((_) => start(_currentUrl));
    }
  }
}

// In your app
class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return ChangeNotifierProvider(
      create: (_) => WallpaperState(),
      child: MaterialApp(
        home: WallpaperControlPanel(),
      ),
    );
  }
}
```

## Complete Application Example

```dart
import 'package:flutter/material.dart';
import 'package:hkcw_engine2/hkcw_engine2.dart';

void main() {
  runApp(const WallpaperApp());
}

class WallpaperApp extends StatefulWidget {
  const WallpaperApp({Key? key}) : super(key: key);

  @override
  State<WallpaperApp> createState() => _WallpaperAppState();
}

class _WallpaperAppState extends State<WallpaperApp> {
  final _urlController = TextEditingController(text: 'https://www.bing.com');
  bool _isRunning = false;
  bool _mouseTransparent = true;
  
  final List<String> _presets = [
    'https://www.bing.com',
    'https://earth.google.com/web/',
    'https://www.shadertoy.com/view/Ms2SD1',
  ];

  Future<void> _start() async {
    final success = await HkcwEngine2.initializeWallpaper(
      url: _urlController.text,
      enableMouseTransparent: _mouseTransparent,
    );
    
    setState(() => _isRunning = success);
    
    if (success) {
      _showSnackBar('Wallpaper started!', Colors.green);
    } else {
      _showSnackBar('Failed to start wallpaper', Colors.red);
    }
  }

  Future<void> _stop() async {
    await HkcwEngine2.stopWallpaper();
    setState(() => _isRunning = false);
    _showSnackBar('Wallpaper stopped', Colors.orange);
  }

  Future<void> _navigate(String url) async {
    _urlController.text = url;
    if (_isRunning) {
      await HkcwEngine2.navigateToUrl(url);
      _showSnackBar('Navigated to: $url', Colors.blue);
    }
  }

  void _showSnackBar(String message, Color color) {
    ScaffoldMessenger.of(context).showSnackBar(
      SnackBar(
        content: Text(message),
        backgroundColor: color,
        duration: Duration(seconds: 2),
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'HKCW Wallpaper Engine',
      theme: ThemeData(
        primarySwatch: Colors.blue,
        useMaterial3: true,
      ),
      home: Scaffold(
        appBar: AppBar(
          title: Text('Desktop Wallpaper Engine'),
          actions: [
            IconButton(
              icon: Icon(_isRunning ? Icons.stop : Icons.play_arrow),
              onPressed: _isRunning ? _stop : _start,
            ),
          ],
        ),
        body: Padding(
          padding: EdgeInsets.all(16),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.stretch,
            children: [
              // Status
              Card(
                color: _isRunning ? Colors.green[50] : Colors.grey[200],
                child: Padding(
                  padding: EdgeInsets.all(16),
                  child: Row(
                    children: [
                      Icon(
                        _isRunning ? Icons.check_circle : Icons.stop_circle,
                        color: _isRunning ? Colors.green : Colors.grey,
                      ),
                      SizedBox(width: 8),
                      Text(
                        _isRunning ? 'Running' : 'Stopped',
                        style: TextStyle(
                          fontSize: 18,
                          fontWeight: FontWeight.bold,
                        ),
                      ),
                    ],
                  ),
                ),
              ),
              
              SizedBox(height: 16),
              
              // URL Input
              TextField(
                controller: _urlController,
                decoration: InputDecoration(
                  labelText: 'URL',
                  border: OutlineInputBorder(),
                  prefixIcon: Icon(Icons.link),
                ),
              ),
              
              SizedBox(height: 16),
              
              // Mouse Transparency Toggle
              SwitchListTile(
                title: Text('Mouse Transparency'),
                subtitle: Text('Allow clicks to pass through'),
                value: _mouseTransparent,
                onChanged: (value) {
                  setState(() => _mouseTransparent = value);
                },
              ),
              
              SizedBox(height: 16),
              
              // Preset URLs
              Text(
                'Presets:',
                style: TextStyle(
                  fontSize: 16,
                  fontWeight: FontWeight.bold,
                ),
              ),
              SizedBox(height: 8),
              Wrap(
                spacing: 8,
                children: _presets.map((url) {
                  return ActionChip(
                    label: Text(Uri.parse(url).host),
                    onPressed: () => _navigate(url),
                  );
                }).toList(),
              ),
              
              SizedBox(height: 24),
              
              // Control Buttons
              Row(
                children: [
                  Expanded(
                    child: ElevatedButton.icon(
                      onPressed: _isRunning ? null : _start,
                      icon: Icon(Icons.play_arrow),
                      label: Text('Start'),
                      style: ElevatedButton.styleFrom(
                        padding: EdgeInsets.all(16),
                      ),
                    ),
                  ),
                  SizedBox(width: 16),
                  Expanded(
                    child: ElevatedButton.icon(
                      onPressed: !_isRunning ? null : _stop,
                      icon: Icon(Icons.stop),
                      label: Text('Stop'),
                      style: ElevatedButton.styleFrom(
                        padding: EdgeInsets.all(16),
                      ),
                    ),
                  ),
                ],
              ),
            ],
          ),
        ),
      ),
    );
  }
  
  @override
  void dispose() {
    _urlController.dispose();
    super.dispose();
  }
}
```

## Testing Tips

1. **Start Simple**: Begin with static HTML pages
2. **Check Logs**: Watch console output for debugging
3. **Test URLs**: Verify URLs work in regular browser first
4. **Performance**: Monitor CPU/memory usage
5. **Desktop Icons**: Ensure icons remain clickable

## Common Patterns

### Singleton Controller

```dart
class WallpaperController {
  static final WallpaperController _instance = WallpaperController._internal();
  factory WallpaperController() => _instance;
  WallpaperController._internal();
  
  bool _initialized = false;
  
  Future<void> ensureInitialized(String url) async {
    if (_initialized) return;
    
    final success = await HkcwEngine2.initializeWallpaper(
      url: url,
      enableMouseTransparent: true,
    );
    
    _initialized = success;
  }
}
```

### Lifecycle Management

```dart
class WallpaperLifecycle extends StatefulWidget {
  @override
  State<WallpaperLifecycle> createState() => _WallpaperLifecycleState();
}

class _WallpaperLifecycleState extends State<WallpaperLifecycle> 
    with WidgetsBindingObserver {
  
  @override
  void initState() {
    super.initState();
    WidgetsBinding.instance.addObserver(this);
  }
  
  @override
  void dispose() {
    WidgetsBinding.instance.removeObserver(this);
    HkcwEngine2.stopWallpaper();
    super.dispose();
  }
  
  @override
  void didChangeAppLifecycleState(AppLifecycleState state) {
    if (state == AppLifecycleState.paused) {
      // App minimized - optionally stop wallpaper
    } else if (state == AppLifecycleState.resumed) {
      // App restored - restart if needed
    }
  }
  
  @override
  Widget build(BuildContext context) {
    return Container();
  }
}
```

