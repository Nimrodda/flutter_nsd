import UIKit
import Flutter

@UIApplicationMain
class AppDelegate: FlutterAppDelegate, NetServiceBrowserDelegate, NetServiceDelegate {
    private var netServiceBrowser: NetServiceBrowser!
    private var services = [NetService]()
    private var channel: FlutterMethodChannel!

    override func application(
            _ application: UIApplication,
            didFinishLaunchingWithOptions launchOptions: [UIApplication.LaunchOptionsKey: Any]?
    ) -> Bool {
        GeneratedPluginRegistrant.register(with: self)
        print("listening for services...")
        self.services.removeAll()
        netServiceBrowser = NetServiceBrowser()
        netServiceBrowser.delegate = self
        let controller: FlutterViewController = window?.rootViewController as! FlutterViewController
        channel = FlutterMethodChannel(name: "com.nimroddayan/flutter_nsd", binaryMessenger: controller.binaryMessenger)
        channel.setMethodCallHandler { call, result in
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
        return super.application(application, didFinishLaunchingWithOptions: launchOptions)
    }

    private func startDiscovery(_ serviceType: String) {
        print("Starting discovery for \(serviceType)")
        netServiceBrowser.searchForServices(ofType: serviceType, inDomain: "")
    }

    private func stopDiscovery() {
        print("Stopping discovery")
        netServiceBrowser.stop()
    }

    func netServiceBrowser(_ browser: NetServiceBrowser, didFind service: NetService, moreComing: Bool) {
        print("adding a service")
        self.services.append(service)
        if !moreComing {
            self.updateInterface()
        }
    }

    private func updateInterface() {
        for service in self.services {
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

    func netServiceDidResolveAddress(_ sender: NetService) {
        print("Found service: \(String(describing: sender.hostName)) \(sender.port) \(sender.name)")
        channel.invokeMethod(
                "onServiceResolved",
                arguments: ["ip": sender.hostName, "port": sender.port, "name": sender.name])
    }
}
