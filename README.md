# Flutter Network Service Discovery plugin

Flutter plugin that provides Network Service Discovery (NSD) API on iOS and Android
for discovering services that other devices provide on a local network.

The plugin currently only supports discovery, but not registry of services.

## Getting Started

Initialize `FlutterNsd` singleton and listen to the stream:

```dart
void init() async {
  final flutterNsd = FlutterNsd();

  flutterNsd.stream.listen((nsdServiceInfo) {
    print('Discovered service name: ${nsdServiceInfo.name}');
    print('Discovered service hostname/IP: ${nsdServiceInfo.hostname}');
    print('Discovered service port: ${nsdServiceInfo.port}');
  }, onError: (e) {
    if (e is NsdError) {
      // Check e.errorCode for the specific error
    }
  });
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

## Note about Android

Minimum Android API version supported is 21.

Android emulator doesn't support Network Service Discovery so you'll have to use a real device.

## Note about iOS

This plugin uses `NetServiceBrowser` and can therefore support iOS version 9+.

On iOS 14+, you need to modify `Info.plist` file and add two keys:
`Bonjour Services` - this is an array, the first item should be the service you're trying to
discover. For example, `_example._tcp.`.
`Privacy - Local Network Usage Description` - this key is for granting the app local network access.
 The value is the text which will be shown to the user in a permission dialog once you call
 `flutterNsd.discoverServices()`.

For more info about network discovery on iOS 14, I suggest you watch
this [video](https://developer.apple.com/videos/play/wwdc2020/10110/).

*Note that you don't need to worry about modifying `Info.plist` if you are just testing with a simulator.*

## License

Copyright 2021 Nimrod Dayan nimroddayan.com

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
