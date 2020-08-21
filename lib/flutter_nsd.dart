import 'dart:async';

import 'package:flutter/services.dart';

class FlutterNsd {
  static const MethodChannel _channel =
      const MethodChannel('flutter_nsd');

  static Future<String> get platformVersion async {
    final String version = await _channel.invokeMethod('getPlatformVersion');
    return version;
  }
}
