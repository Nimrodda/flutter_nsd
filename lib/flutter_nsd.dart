/*
 * Copyright 2021 Nimrod Dayan nimroddayan.com
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 *
 */

import 'dart:async';

import 'package:flutter/services.dart';

/// Singleton API for managing Network Service Discovery
///
/// Get the singleton by calling FlutterNsd() and listen to its [stream] for [NsdServiceInfo] emissions.
/// Then invoke [discoverServices] to start service discovery.
/// Stop discovery by calling [stopDiscovery] when you're done.
///
class FlutterNsd {
  static const MethodChannel _channel =
      MethodChannel('com.nimroddayan/flutter_nsd');
  static final FlutterNsd _instance = FlutterNsd._internal();

  final _streamController = StreamController<NsdServiceInfo>();
  late Stream<NsdServiceInfo> _stream;

  /// Factory for getting [FlutterNsd] singleton object
  factory FlutterNsd() {
    return _instance;
  }

  FlutterNsd._internal() {
    _stream = _streamController.stream.asBroadcastStream();
  }

  /// Stream that emits a [NsdServiceInfo] for each service discovered or a [NsdError] in case of an error.
  Stream<NsdServiceInfo> get stream => _stream;

  /// Start network service discovery for [serviceType] for an infinite amount
  /// of time (or until the app process is killed). Make sure to call [stopDiscovery] when you're done.
  Future<void> discoverServices(String serviceType) async {
    await _channel.invokeMethod('startDiscovery', {'serviceType': serviceType});

    _channel.setMethodCallHandler((MethodCall call) async {
      switch (call.method) {
        case 'onStartDiscoveryFailed':
          _streamController.addError(NsdError(
            errorCode: NsdErrorCode.startDiscoveryFailed,
          ));
          break;
        case 'onStopDiscoveryFailed':
          _streamController.addError(NsdError(
            errorCode: NsdErrorCode.stopDiscoveryFailed,
          ));
          break;
        case 'onResolveFailed':
          _streamController.addError(NsdError(
            errorCode: NsdErrorCode.onResolveFailed,
          ));
          break;
        case 'onDiscoveryStopped':
          _streamController.addError(NsdError(
            errorCode: NsdErrorCode.discoveryStopped,
          ));
          _channel.setMethodCallHandler(null);
          break;
        case 'onServiceResolved':
          final nsdServiceInfo = _parseArgs(call);
          _streamController.add(nsdServiceInfo);
          break;
        case 'onServiceLost':
        // TODO issue #28
          break;
        default:
          throw MissingPluginException();
      }
    });
  }

  NsdServiceInfo _parseArgs(MethodCall call) {
    final String hostname = call.arguments['hostname'];
    final int port = call.arguments['port'];
    final String name = call.arguments['name'];
    final Map<String, Uint8List> txt = Map.from(call.arguments['txt']);
    List<String>? hostAddresses;
    List<Object?>? rawAddresses = call.arguments['hostAddresses'];
    if (rawAddresses != null) {
      hostAddresses = rawAddresses.where((e) => e != null).map((e) => e.toString()).toList();
    }
    var nsdServiceInfo = NsdServiceInfo(hostname, port, name, txt, hostAddresses: hostAddresses);
    return nsdServiceInfo;
  }

  /// Stop network service discovery
  Future<void> stopDiscovery() async {
    await _channel.invokeMethod('stopDiscovery');
  }
}

/// Info class for holding discovered service
class NsdServiceInfo {
  final String? hostname;
  final List<String>? hostAddresses;
  final int? port;
  final String? name;
  final Map<String, Uint8List>? txt;

  NsdServiceInfo(this.hostname, this.port, this.name, this.txt, {this.hostAddresses});
}

/// List of possible error codes of NsdError
enum NsdErrorCode {
  startDiscoveryFailed,
  stopDiscoveryFailed,
  onResolveFailed,
  discoveryStopped,
}

// Generic error thrown when an error has occurred during discovery
class NsdError extends Error {
  /// The cause of this [NsdError].
  final NsdErrorCode errorCode;

  NsdError({required this.errorCode});
}
