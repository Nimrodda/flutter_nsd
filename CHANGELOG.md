
## 1.5.0
Fixed #22: NsdServiceInfo now includes a list of addresses for the service (ipv4 or ipv6)
*addresses are currently not supported on iOS


## 1.4.0
Added support for Kotlin 1.5.20

## 1.3.3

Fixed #33: Windows: TXT records are returned as string instead of UInt8List, causing failure (Thanks @jnstahl)
Fixed #36: Windows: txt records are sent as [key: key] instead of [key: value] (Thanks @jnstahl)

## 1.3.2

This release is identical in functionality to the previous one. It only fixes pub.dev analysis
errors.

* Fixed: pub.dev analsis error

## 1.3.1

This release is identical in functionality to the previous one. It only fixes pub.dev analysis
errors.

* Fixed #27: Address pub.dev static analysis
* Fixed various lint errors

## 1.3.0

* Fixed #10: Make pubspec description valid
* Added #11 MacOS desktop support
* Added #16 Support for Windows (Thanks @jnstahl)

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
