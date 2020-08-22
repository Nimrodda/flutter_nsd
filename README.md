# Flutter Network Service Discovery plugin

This Flutter plugin provides Network Service Discovery (NSD) API on iOS and Android
for discovering services that other devices provide on a local network.

The plugin is currently under development and right now only supports discovery,
but not registry of services.

## Note about iOS

To support legacy iOS devices, `NetServiceBrowser` is used, which means that you will need
to apply for entitlement from Apple for Local Network Access. Also, you'll have to
make some modifications to your Info.plist file. For more info visit:
https://developer.apple.com/videos/play/wwdc2020/10110/

*You don't need to worry about this if you are just testing with a simulator.*

## iOS Help needed!

I'm an Android developer, so it might be that the iOS implementation isn't great.
I'd really appreciate help from an iOS developer, who's willing to review the code and improve it.

## Getting Started

Initialize `FlutterNsd` singleton and listen to the stream:

```dart
void init() async {
  final flutterNsd = FlutterNsd();
  final stream = flutterNsd.stream;

  await for (final nsdServiceInfo in stream) {
    print('Discovered service name: ${nsdServiceInfo.name}');
    print('Discovered service hostname/IP: ${nsdServiceInfo.hostname}');
    print('Discovered service port: ${nsdServiceInfo.port}');
  }
}
```

Start discovery when needed. At this point, if any services are discovered, your stream listener
will get notified. In case of an error, `NsdError` is emitted to the stream (Currently it's
just a generic error, but in the future, it will be more specific).


```dart
void startDiscoveryButton() async {
  await flutterNsd.discoverServices('_example._tcp.');
}
```

Stop discovery when done. If you don't call this method, discovery will continue until the app
process is killed.

```dart
void stopDiscoveryButton() async {
  await flutterNsd.stopDiscovery();
}

```

See the example project for a more detailed Flutter app example.

## License

Copyright 2020 Nimrod Dayan nimroddayan.com

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
