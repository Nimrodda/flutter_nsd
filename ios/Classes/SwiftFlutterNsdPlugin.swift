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
            let serviceType = args?["serviceType"] as? String
            if serviceType == nil {
                result(FlutterError(code: "1001", message: "Service type cannot be null", details: nil))
                return
            }

            self.startDiscovery(serviceType!)
            result(nil)
        case "stopDiscovery":
            self.stopDiscovery()
            result(nil)
        default:
            result(FlutterMethodNotImplemented)
        }
    }

    private func startDiscovery(_ serviceType: String) {
        print("Starting discovery for \(serviceType)")
        netServiceBrowser.delegate = self
        netServiceBrowser.searchForServices(ofType: serviceType, inDomain: "")
    }

    private func stopDiscovery() {
        print("Stopping discovery")
        netServiceBrowser.delegate = nil
        netServiceBrowser.stop()
    }

    private func updateInterface() {
        for service in services {
            if service.port == -1 {
                print("service \(service.name) of type \(service.type)" +
                        " not yet resolved")
                service.delegate = self
                service.resolve(withTimeout: 10)
            } else {
                print("service \(service.name) of type \(service.type)," +
                        "port \(service.port), addresses \(service.addresses)")
            }
        }
    }

    public func netServiceBrowser(_ browser: NetServiceBrowser, didFind service: NetService, moreComing: Bool) {
        print("adding a service")
        services.append(service)
        if !moreComing {
            self.updateInterface()
        }
    }

    public func netServiceDidResolveAddress(_ sender: NetService) {
        print("Found service: \(String(describing: sender.hostName)) \(sender.port) \(sender.name)")
        channel.invokeMethod(
                "onServiceResolved",
                arguments: ["hostname": sender.hostName, "port": sender.port, "name": sender.name])
    }
}
