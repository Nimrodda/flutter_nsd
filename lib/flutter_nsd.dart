import 'dart:async';

import 'package:flutter/services.dart';

class FlutterNsd {
  static const MethodChannel _channel = const MethodChannel('com.nimroddayan/flutter_nsd');
  static final FlutterNsd _instance = FlutterNsd._internal();

  final _streamController = StreamController<NsdServiceInfo>();

  factory FlutterNsd() {
    return _instance;
  }

  FlutterNsd._internal();

  Stream<NsdServiceInfo> get stream => _streamController.stream;

  Future<void> discoverServices(String serviceType) async {
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
        case 'onDiscoveryStopped':
          _channel.setMethodCallHandler(null);
          break;
        case 'onServiceResolved':
          final String ip = call.arguments['hostname'];
          final int port = call.arguments['port'];
          final String name = call.arguments['name'];
          _streamController.add(NsdServiceInfo(ip, port, name));
          break;
        default:
          _streamController.addError(UnsupportedError('Method ${call.method} is unsupported'));
      }
      return null;
    });
  }

  Future<void> stopDiscovery() async {
    await _channel.invokeMethod('stopDiscovery');
  }
}

class NsdServiceInfo {
  final String hostname;
  final int port;
  final String name;

  NsdServiceInfo(this.hostname, this.port, this.name);
}

class NsdException extends Error {
}
