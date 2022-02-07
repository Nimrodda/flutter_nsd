[![flutter_nsd](https://github.com/Nimrodda/flutter_nsd/actions/workflows/build.yaml/badge.svg)](https://github.com/Nimrodda/flutter_nsd/actions/workflows/build.yaml)

# Flutter Network Service Discovery plugin

A Flutter plugin for Network Service Discovery (mDNS) on Android, iOS, MacOS and Windows. The plugin uses the platform's own API when possible.

The plugin currently only supports discovery, but not registry of services.

## Install

Add the dependency to pubspec.yaml:

```
dependencies:
  flutter_nsd: ^1.3.1
```

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
  await flutterNsd.discoverServices('_http._tcp.');
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

* `Bonjour Services` - this is an array, the first item should be the service you're trying to
discover. For example, `_http._tcp.`.
* `Privacy - Local Network Usage Description` - this key is for granting the app local network access.
 The value is the text which will be shown to the user in a permission dialog once you call
 `flutterNsd.discoverServices()`.

 Example:

 ```xml
<key>NSLocalNetworkUsageDescription</key>
<string>Reasoning for the user why you need this permission goes here</string>
<key>NSBonjourServices</key>
<array>
    <string>_http._tcp.</string>
</array>
```

For more info about network discovery on iOS 14, I suggest you watch
this [video](https://developer.apple.com/videos/play/wwdc2020/10110/).

*Note that you don't need to worry about modifying `Info.plist` if you are just testing with a simulator.*

## Note about Windows

Windows does not have native support for MDNS, therefore it is implemented using the https://github.com/mjansson/mdns library
which implements MDNS using sockets. The library will return separate results for ipv4 and ipv6.

The current implementation will send MDNS multicast every 10 seconds until stopped.

Due to the native socket calls, any app using this plugin on windows will trigger a dialog from Windows to allow network access on the first launch.

For Windows development you will need Visual Studio 2019 or higher with the C++ workload installed, see https://docs.flutter.dev/desktop#additional-windows-requirements

## Testing

On MacOS it's easy to test network service discovery via the following command which will create a mock service:

```
dns-sd -R TestService _http._tcp . 3000
```

Then scan for this service using the example app on any of the supported platforms.

## License

Copyright 2022 Nimrod Dayan nimroddayan.com

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
