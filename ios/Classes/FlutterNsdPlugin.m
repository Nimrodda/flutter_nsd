#import "FlutterNsdPlugin.h"
#if __has_include(<flutter_nsd/flutter_nsd-Swift.h>)
#import <flutter_nsd/flutter_nsd-Swift.h>
#else
// Support project import fallback if the generated compatibility header
// is not copied when this plugin is created as a library.
// https://forums.swift.org/t/swift-static-libraries-dont-copy-generated-objective-c-header/19816
#import "flutter_nsd-Swift.h"
#endif

@implementation FlutterNsdPlugin
+ (void)registerWithRegistrar:(NSObject<FlutterPluginRegistrar>*)registrar {
  [SwiftFlutterNsdPlugin registerWithRegistrar:registrar];
}
@end
