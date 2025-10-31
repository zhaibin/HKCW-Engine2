import 'package:flutter/material.dart';
import 'package:hkcw_engine2/hkcw_engine2.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatefulWidget {
  const MyApp({Key? key}) : super(key: key);

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  final TextEditingController _urlController = TextEditingController(
    text: 'file:///E:/Projects/HKCW-Engine2/test_api.html',
  );
  bool _isRunning = false;
  bool _mouseTransparent = false;  // Disable for API testing

  @override
  void initState() {
    super.initState();
    // Auto-start for API testing
    Future.delayed(Duration(seconds: 1), () {
      _startWallpaper();
    });
  }

  Future<void> _startWallpaper() async {
    final url = _urlController.text.trim();
    if (url.isEmpty) {
      _showMessage('Please enter a URL');
      return;
    }

    print('[APP] Starting wallpaper with URL: $url');
    final success = await HkcwEngine2.initializeWallpaper(
      url: url,
      enableMouseTransparent: _mouseTransparent,
    );

    setState(() {
      _isRunning = success;
    });

    if (success) {
      _showMessage('Wallpaper started successfully!');
    } else {
      _showMessage('Failed to start wallpaper');
    }
  }

  Future<void> _stopWallpaper() async {
    print('[APP] Stopping wallpaper');
    final success = await HkcwEngine2.stopWallpaper();

    setState(() {
      _isRunning = false;
    });

    if (success) {
      _showMessage('Wallpaper stopped');
    } else {
      _showMessage('Failed to stop wallpaper');
    }
  }

  Future<void> _navigateToUrl() async {
    final url = _urlController.text.trim();
    if (url.isEmpty) {
      _showMessage('Please enter a URL');
      return;
    }

    print('[APP] Navigating to URL: $url');
    final success = await HkcwEngine2.navigateToUrl(url);

    if (success) {
      _showMessage('Navigation successful');
    } else {
      _showMessage('Navigation failed');
    }
  }

  void _showMessage(String message) {
    print('[APP] $message');
    if (mounted) {
      final messenger = ScaffoldMessenger.maybeOf(context);
      if (messenger != null) {
        messenger.showSnackBar(
          SnackBar(content: Text(message)),
        );
      }
    }
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'HKCW Engine2 Demo',
      theme: ThemeData(
        primarySwatch: Colors.blue,
        useMaterial3: true,
      ),
      home: Scaffold(
        appBar: AppBar(
          title: const Text('HKCW Engine2 - Desktop Wallpaper'),
          backgroundColor: Colors.blue,
        ),
        body: Padding(
          padding: const EdgeInsets.all(24.0),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.stretch,
            children: [
              Card(
                elevation: 4,
                child: Padding(
                  padding: const EdgeInsets.all(16.0),
                  child: Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      Text(
                        'Status: ${_isRunning ? "Running" : "Stopped"}',
                        style: TextStyle(
                          fontSize: 18,
                          fontWeight: FontWeight.bold,
                          color: _isRunning ? Colors.green : Colors.red,
                        ),
                      ),
                      const SizedBox(height: 8),
                      Text(
                        'WebView2 will be displayed as desktop wallpaper behind icons',
                        style: TextStyle(
                          fontSize: 14,
                          color: Colors.grey[600],
                        ),
                      ),
                    ],
                  ),
                ),
              ),
              const SizedBox(height: 24),
              TextField(
                controller: _urlController,
                decoration: const InputDecoration(
                  labelText: 'URL',
                  hintText: 'Enter URL to display',
                  border: OutlineInputBorder(),
                  prefixIcon: Icon(Icons.link),
                ),
              ),
              const SizedBox(height: 16),
              CheckboxListTile(
                title: const Text('Enable Mouse Transparency'),
                subtitle: const Text('Allow clicks to pass through to desktop'),
                value: _mouseTransparent,
                onChanged: (value) {
                  setState(() {
                    _mouseTransparent = value ?? true;
                  });
                },
              ),
              const SizedBox(height: 24),
              Row(
                children: [
                  Expanded(
                    child: ElevatedButton.icon(
                      onPressed: _isRunning ? null : _startWallpaper,
                      icon: const Icon(Icons.play_arrow),
                      label: const Text('Start Wallpaper'),
                      style: ElevatedButton.styleFrom(
                        padding: const EdgeInsets.all(16),
                        backgroundColor: Colors.green,
                        foregroundColor: Colors.white,
                      ),
                    ),
                  ),
                  const SizedBox(width: 16),
                  Expanded(
                    child: ElevatedButton.icon(
                      onPressed: !_isRunning ? null : _stopWallpaper,
                      icon: const Icon(Icons.stop),
                      label: const Text('Stop Wallpaper'),
                      style: ElevatedButton.styleFrom(
                        padding: const EdgeInsets.all(16),
                        backgroundColor: Colors.red,
                        foregroundColor: Colors.white,
                      ),
                    ),
                  ),
                ],
              ),
              const SizedBox(height: 16),
              ElevatedButton.icon(
                onPressed: !_isRunning ? null : _navigateToUrl,
                icon: const Icon(Icons.navigation),
                label: const Text('Navigate to URL'),
                style: ElevatedButton.styleFrom(
                  padding: const EdgeInsets.all(16),
                ),
              ),
              const SizedBox(height: 24),
              Card(
                color: Colors.blue[50],
                child: Padding(
                  padding: const EdgeInsets.all(16.0),
                  child: Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      const Row(
                        children: [
                          Icon(Icons.info_outline, color: Colors.blue),
                          SizedBox(width: 8),
                          Text(
                            'Instructions:',
                            style: TextStyle(
                              fontWeight: FontWeight.bold,
                              fontSize: 16,
                            ),
                          ),
                        ],
                      ),
                      const SizedBox(height: 8),
                      Text(
                        '1. Enter a URL (e.g., https://www.bing.com)\n'
                        '2. Click "Start Wallpaper"\n'
                        '3. Check your desktop - WebView2 should appear behind icons\n'
                        '4. Use "Navigate to URL" to change content\n'
                        '5. Click "Stop Wallpaper" to remove',
                        style: TextStyle(color: Colors.grey[800]),
                      ),
                    ],
                  ),
                ),
              ),
            ],
          ),
        ),
      ),
    );
  }
}

