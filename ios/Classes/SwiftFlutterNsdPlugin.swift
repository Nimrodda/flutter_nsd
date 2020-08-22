import Flutter
import UIKit

public class SwiftFlutterNsdPlugin: NSObject, FlutterPlugin {
  var channel: FlutterMethodChannel!
  var netServiceBrowser: NetServiceBrowser!
  var services = [NetService]()

  public static func register(with registrar: FlutterPluginRegistrar) {
    channel = FlutterMethodChannel(name: "com.nimroddayan/flutter_nsd", binaryMessenger: registrar.messenger())
    let instance = SwiftFlutterNsdPlugin()
    self.services.removeAll()
    netServiceBrowser = NetServiceBrowser()
    netServiceBrowser.delegate = self
    registrar.addMethodCallDelegate(instance, channel: channel)
  }

  public func handle(_ call: FlutterMethodCall, result: @escaping FlutterResult) {
    switch call.method {
    case "startDiscovery":
      let serviceType = call.arguments["serviceType"] as? String?
      if serviceType == nil {
        result.error("1001", "service type cannot be null")
        return
      }

      startDiscovery(serviceType)
      result.success(nil)
    }
  }

  private func startDiscovery(_ serviceType: String) {
    netServiceBrowser.searchForServices(ofType: serviceType, inDomain: "")
  }
}
