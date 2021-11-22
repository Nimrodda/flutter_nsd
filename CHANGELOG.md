## 1.4.0-alpha01

Add support for Windows using the https://github.com/mjansson/mdns library


## 1.3.0

* Added MacOS desktop support
* Fixed #10: Make pubspec description valid

## 1.2.0

* #6 Android: Fixed failure to resolve service sequentially (Thanks @julianscheel).

## 1.1.0

* Fixed #3: discovery can't start after hot-restart (thanks @julianscheel). `NsdError` now has
a property that provides more info about the error via the new enum `NsdErrorCode`. Check the
example for more info how to use it.

## 1.0.0

* Changed version scheme to adhere to pub version guidelines (dropping the 'alpha')
* Updated README.md with detailed instructions on support for iOS 14
* Removed Flutter version bounds so that the plugin can be used with Flutter 2+

## 1.0.0-alpha03

* Increased Dart SDK to 2.12.0-0
* Added Null Safety support
* Added TXT record to NsdServiceInfo (thanks @pheki)

## 1.0.0-alpha02

* Fixed 'stream has already been listened to'

## 1.0.0-alpha01

* Support service discovery on iOS and Android
