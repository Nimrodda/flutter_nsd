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

import 'package:flutter/material.dart';
import 'dart:async';

import 'package:flutter_nsd/flutter_nsd.dart';

void main() {
  runApp(MyApp());
}

class MyApp extends StatefulWidget {
  @override
  State createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  final flutterNsd = FlutterNsd();
  final services = <NsdServiceInfo>[];
  bool initialStart = true;

  _MyAppState() {
    // Try one restart if initial start fails, which happens on hot-restart of
    // the flutter app.
    flutterNsd.stream.listen((_) {}, onError: (e) async {
      if (e is NsdError) {
        if (e.errorCode == NsdErrorCode.startDiscoveryFailed && initialStart) {
          await stopDiscovery();
        } else if (e.errorCode == NsdErrorCode.discoveryStopped && initialStart) {
          initialStart = false;
          await startDiscovery();
        }
      }
    });
  }

  Future<void> startDiscovery() async {
    await flutterNsd.discoverServices('_example._tcp.');
  }

  Future<void> stopDiscovery() async {
    flutterNsd.stopDiscovery();
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
                stream: flutterNsd.stream,
                builder: (context, snapshot) {
                  if (snapshot.hasData) {
                    services.add(snapshot.data as NsdServiceInfo);
                    return ListView.builder(
                      itemBuilder: (context, index) => ListTile(
                        title: Text(services[index].name ?? 'Invalid service name'),
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
