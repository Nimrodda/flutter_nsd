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
  runApp(const MyApp());
}

class MyApp extends StatefulWidget {
  const MyApp({Key? key}) : super(key: key);

  @override
  State createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  final flutterNsd = FlutterNsd();
  final services = <NsdServiceInfo>{};
  bool initialStart = true;
  bool _scanning = false;

  _MyAppState();

  @override
  void initState() {
    super.initState();

    // Try one restart if initial start fails, which happens on hot-restart of
    // the flutter app.
    flutterNsd.discoveredServicesStream.listen(
      (NsdServiceInfo service) {
        setState(() {
          services.add(service);
        });
      },
      onError: (e) async {
        if (e is NsdError) {
          if (e.errorCode == NsdErrorCode.startDiscoveryFailed &&
              initialStart) {
            await stopDiscovery();
          } else if (e.errorCode == NsdErrorCode.discoveryStopped &&
              initialStart) {
            initialStart = false;
            await startDiscovery();
          }
        }
      },
    );
    flutterNsd.lostServicesStream.listen((NsdServiceInfo service) {
      setState(() {
        services.remove(service);
      });
    });
  }

  Future<void> startDiscovery() async {
    if (_scanning) return;

    setState(() {
      services.clear();
      _scanning = true;
    });
    await flutterNsd.discoverServices('_http._tcp.');
  }

  Future<void> stopDiscovery() async {
    if (!_scanning) return;

    setState(() {
      services.clear();
      _scanning = false;
    });
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
                ElevatedButton(
                  child: const Text('Start'),
                  onPressed: () async => startDiscovery(),
                ),
                ElevatedButton(
                  child: const Text('Stop'),
                  onPressed: () async => stopDiscovery(),
                ),
              ],
            ),
            Expanded(
              child: _buildMainWidget(context),
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildMainWidget(BuildContext context) {
    if (services.isEmpty && _scanning) {
      return const Center(
        child: CircularProgressIndicator(),
      );
    } else if (services.isEmpty && !_scanning) {
      return const SizedBox.shrink();
    } else {
      return ListView.builder(
        itemBuilder: (context, index) => ListTile(
          title: Text(services.elementAt(index).name ?? 'Invalid service name'),
        ),
        itemCount: services.length,
      );
    }
  }
}
