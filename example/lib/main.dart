import 'package:flutter/material.dart';
import 'dart:async';

import 'package:flutter_nsd/flutter_nsd.dart';

void main() {
  runApp(MyApp());
}

class MyApp extends StatelessWidget {
  final _flutterNsd = FlutterNsd();
  final services = <NsdServiceInfo>[];

  Future<void> startDiscovery() async {
    await _flutterNsd.discoverServices('_example._tcp.');
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
            Row(
              mainAxisAlignment: MainAxisAlignment.spaceEvenly,
              children: <Widget>[
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
            Expanded(
              child: StreamBuilder(
                stream: _flutterNsd.stream,
                builder: (context, snapshot) {
                  if (snapshot.hasData) {
                    services.add(snapshot.data as NsdServiceInfo);
                    return ListView.builder(
                      itemBuilder: (context, index) => ListTile(
                        title: Text(services[index].name),
                      ),
                      itemCount: services.length,
                    );
                  } else {
                    return Center(child: CircularProgressIndicator());
                  }
                },
              ),
            ),
          ],
        ),
      ),
    );
  }
}
