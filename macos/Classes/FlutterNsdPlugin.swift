import Cocoa
import FlutterMacOS

public class FlutterNsdPlugin: NSObject, FlutterPlugin {
    public static func register(with registrar: FlutterPluginRegistrar) {
        let channel = FlutterMethodChannel(name: "com.nimroddayan/flutter_nsd", binaryMessenger: registrar.messenger)
        let instance = FlutterNsdPlugin()
        registrar.addMethodCallDelegate(instance, channel: channel)
    }
    
    public func handle(_ call: FlutterMethodCall, result: @escaping FlutterResult) {
        switch call.method {
        case "getPlatformVersion":
            result("macOS " + ProcessInfo.processInfo.operatingSystemVersionString)
        case "startDiscovery":
            let args = call.arguments as? [String: Any]
            guard let serviceType = args?["serviceType"] as? String else {
                result(FlutterError(code: "1001", message: "Service type cannot be null", details: nil))
                return
            }
            
            self.startDiscovery(serviceType)
            result(nil)
        case "stopDiscovery":
            self.stopDiscovery()
            result(nil)
        default:
            result(FlutterMethodNotImplemented)
        }
    }
    
    private func startDiscovery(_ serviceType: String) {
        NSLog("startDiscovery");
    }
    
    private func stopDiscovery() {
        NSLog("stopDiscovery");
    }
}
