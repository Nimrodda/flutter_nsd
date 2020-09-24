import Flutter
import UIKit

public class SwiftFlutterNsdPlugin: NSObject, FlutterPlugin, NetServiceBrowserDelegate, NetServiceDelegate {
    private var netServiceBrowser: NetServiceBrowser!
    private var services = [NetService]()
    private var channel: FlutterMethodChannel

    init(channel: FlutterMethodChannel) {
        self.channel = channel
        self.services.removeAll()
        netServiceBrowser = NetServiceBrowser()
        super.init()
    }

    public static func register(with registrar: FlutterPluginRegistrar) {
        let flutterNsdChannel = FlutterMethodChannel(name: "com.nimroddayan/flutter_nsd", binaryMessenger: registrar.messenger())
        let instance = SwiftFlutterNsdPlugin(channel: flutterNsdChannel)
        registrar.addMethodCallDelegate(instance, channel: flutterNsdChannel)
    }

    public func handle(_ call: FlutterMethodCall, result: @escaping FlutterResult) {
        switch call.method {
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
        netServiceBrowser.delegate = self
        netServiceBrowser.searchForServices(ofType: serviceType, inDomain: "")
    }

    private func stopDiscovery() {
        netServiceBrowser.delegate = nil
        netServiceBrowser.stop()
    }

    private func updateInterface() {
        for service in services {
            if service.port == -1 {
                service.delegate = self
                service.resolve(withTimeout: 10)
            }
        }
    }

    public func netServiceBrowser(_ browser: NetServiceBrowser, didFind service: NetService, moreComing: Bool) {
        services.append(service)
        if !moreComing {
            self.updateInterface()
        }
    }

    public func netServiceDidResolveAddress(_ sender: NetService) {
        channel.invokeMethod(
                "onServiceResolved",
                arguments: ["hostname": sender.hostName, "port": sender.port, "name": sender.name])
    }
}
