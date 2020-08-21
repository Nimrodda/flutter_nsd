import 'dart:async';

import 'package:flutter/services.dart';

class FlutterNsd {
  static const MethodChannel _channel = const MethodChannel('com.nimroddayan/flutter_nsd');

  StreamController<NsdServiceInfo> _streamController;

  Future<Stream<NsdServiceInfo>> discoverServices(String serviceType) async {
    if (_streamController != null) return _streamController.stream;

    _streamController = StreamController<NsdServiceInfo>();

    await _channel.invokeMethod('startDiscovery', {'serviceType': '$serviceType'});

    _channel.setMethodCallHandler((call) {
      switch (call.method) {
        case 'onStartDiscoveryFailed':
          _streamController.addError(NsdException());
          break;
        case 'onStopDiscoveryFailed':
          _streamController.addError(NsdException());
          break;
        case 'onResolveFailed':
          _streamController.addError(NsdException());
          break;
        case 'onServiceResolved':
          final String ip = call.arguments['ip'];
          final int port = call.arguments['port'];
          final String name = call.arguments['name'];
          _streamController.add(NsdServiceInfo(ip, port, name));
          break;
        default:
          _streamController.addError(UnsupportedError('Method ${call.method} is unsupported'));
      }
      return null;
    });

    return _streamController.stream;
  }

  void stopDiscovery() async {
    if (_streamController != null) {
      await _channel.invokeMethod('stopDiscovery');
      _channel.setMethodCallHandler(null);
      await _streamController.close();
      _streamController = null;
    }
  }
}

class NsdServiceInfo {
  final String ip;
  final int port;
  final String name;

  NsdServiceInfo(this.ip, this.port, this.name);
}

class NsdException extends Error {
}