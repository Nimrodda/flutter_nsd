import 'package:flutter/material.dart';
import 'dart:async';

import 'package:flutter_nsd/flutter_nsd.dart';

void main() {
  runApp(MyApp());
}

class MyApp extends StatefulWidget {
  @override
  _MyAppState createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  String _discoveredService = 'None';

  FlutterNsd _flutterNsd;

  @override
  void initState() {
    super.initState();
    _flutterNsd = FlutterNsd();
  }

  Future<void> startDiscovery() async {
    final stream = await _flutterNsd.discoverServices('_questshare._tcp.');
    await for (final nsdServiceInfo in stream) {
      setState(() {
        _discoveredService = '${nsdServiceInfo.ip}:${nsdServiceInfo.port}';
      });
    }
  }

  Future<void> stopDiscovery() async {
    _flutterNsd.stopDiscovery();
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(
          title: const Text('NSD Example'),
        ),
        body: Column(
          children: <Widget>[
            Text('Discovered service: $_discoveredService\n'),
            RaisedButton(
              child: Text('Start'),
              onPressed: () async => startDiscovery(),
            ),
            RaisedButton(
              child: Text('Stop'),
              onPressed: () async => stopDiscovery(),
            ),
          ],
        ),
      ),
    );
  }
}
