#include "include/flutter_nsd/flutter_nsd_plugin.h"

// This must be included before many other Windows headers.
#include <windows.h>



// For getPlatformVersion; remove unless needed for your plugin implementation.
#include <VersionHelpers.h>

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <map>
#include <memory>
#include <sstream>
#include <thread>


#include "mdns_impl.h"



namespace {



  class MdnsResult {
  public:
    std::string name;
    std::string hostname;
    std::string dnsname;
    std::string servicename;
    std::string ipv4address;
    std::string ipv6address;
    int port;
    std::map<flutter::EncodableValue, flutter::EncodableValue> txt;
  };

  class MdnsRequest {

  public:
    MdnsRequest(std::shared_ptr<flutter::MethodChannel<flutter::EncodableValue>> _channel);
    void start(std::string serviceName);
    void stop();
    void callbackPTR(const void* base, boolean last, STRING_ARG_DECL(service), STRING_ARG_DECL(name));
    void callbackSRV(const void* base, boolean last, STRING_ARG_DECL(name), STRING_ARG_DECL(hostname), int port);
    void callbackA(const void* base, boolean last, STRING_ARG_DECL(service), STRING_ARG_DECL(address));
    void callbackAAAA(const void* base, boolean last, STRING_ARG_DECL(service), STRING_ARG_DECL(address));
    void callbackTXT(const void* base, boolean last, STRING_ARG_DECL(key), STRING_ARG_DECL(value));
    void callbackU(const void* base, boolean last);

    void process(const void* base, boolean last);
    void send(MdnsResult&);


  private:
    void runner(std::string serviceName);
    static void ThreadFunc(MdnsRequest* p, std::string serviceName);
    volatile boolean keepRunning;
    std::shared_ptr<flutter::MethodChannel<flutter::EncodableValue>> channel;
    std::map<const void*, MdnsResult> packets;

  };


  MdnsRequest::MdnsRequest(std::shared_ptr<flutter::MethodChannel<flutter::EncodableValue>> _channel)

  {
    channel = _channel;
  }


  void MdnsRequest::send(MdnsResult& packet) {
    channel->InvokeMethod("onServiceResolved",
      std::make_unique<flutter::EncodableValue>(flutter::EncodableValue(flutter::EncodableMap{
               {flutter::EncodableValue("hostname"), flutter::EncodableValue(packet.hostname)},
               {flutter::EncodableValue("ipv4address"), flutter::EncodableValue(packet.ipv4address)},
               {flutter::EncodableValue("ipv6address"), flutter::EncodableValue(packet.ipv6address)},
               {flutter::EncodableValue("port"), flutter::EncodableValue(packet.port)},
               {flutter::EncodableValue("name"), flutter::EncodableValue(packet.name)},
               {flutter::EncodableValue("txt"), flutter::EncodableValue(packet.txt)}
        }
      ))
    );

  }

  void MdnsRequest::process(const void* base, boolean last) {
    if (last) {
      auto packet = packets[base];
      send(packet);
      packets.erase(base);
    }
  }



  void MdnsRequest::callbackPTR(const void* base, boolean last, STRING_ARG_DECL(service), STRING_ARG_DECL(name)) {
    MAKE_STRING(service);
    MAKE_STRING(name);
    MdnsResult& packet = packets[base];
    packet.name = _name;
    packet.servicename = _service;
    process(base, last);
  }

  void MdnsRequest::callbackSRV(const void* base, boolean last, STRING_ARG_DECL(name), STRING_ARG_DECL(hostname), int port) {
    MAKE_STRING(name);
    MAKE_STRING(hostname);
    printf("resolved: %s, %s\n\r", _name.c_str(), _hostname.c_str());

    MdnsResult& packet = packets[base];
    packet.name = _name;
    packet.hostname = _hostname;
    packet.port = port;
    process(base, last);

  }



  void MdnsRequest::callbackA(const void* base, boolean last, STRING_ARG_DECL(hostname), STRING_ARG_DECL(address)) {
    MAKE_STRING(hostname);
    MAKE_STRING(address);
    MdnsResult& packet = packets[base];
    packet.dnsname = _hostname;
    packet.ipv4address = _address;
    process(base, last);

  }
  void MdnsRequest::callbackAAAA(const void* base, boolean last, STRING_ARG_DECL(hostname), STRING_ARG_DECL(address)) {
    MAKE_STRING(hostname);
    MAKE_STRING(address);
    MdnsResult& packet = packets[base];
    packet.dnsname = _hostname;
    packet.ipv6address = _address;
    process(base, last);
  }
  void MdnsRequest::callbackTXT(const void* base, boolean last, STRING_ARG_DECL(key), STRING_ARG_DECL(value)) {
    MAKE_STRING(key);
    MAKE_STRING(value);
    MdnsResult& packet = packets[base];
    packet.txt[flutter::EncodableValue(_key)] = flutter::EncodableValue(_value);
    process(base, last);
  }

  void MdnsRequest::callbackU(const void* base, boolean last) {

  }

  void MdnsRequest::runner(std::string serviceName) {
    while (keepRunning) {
      DWORD result = mdns_query(this, (serviceName + "local.").c_str());
      for (auto it = packets.begin(); it != packets.end(); it++) {
        send(it->second);
      }
      if (result == -1) return;
    }
    delete this;
  }


  void MdnsRequest::ThreadFunc(MdnsRequest *p, std::string serviceName)
  {
    p->runner(serviceName);
  }


  void MdnsRequest::start(std::string serviceName)
  {
    keepRunning = TRUE;
    std::thread myThread(ThreadFunc, this, serviceName);
    myThread.detach();
  }

  void MdnsRequest::stop()
  {
    keepRunning = false;
  }


class FlutterNsdPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);

  FlutterNsdPlugin(std::shared_ptr<flutter::MethodChannel<flutter::EncodableValue>>);

  virtual ~FlutterNsdPlugin();

 private:
  // Called when a method is called on this plugin's channel from Dart.
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
  std::shared_ptr<flutter::MethodChannel<flutter::EncodableValue>> channel;

  MdnsRequest* currentRequest;

};

// static
void FlutterNsdPlugin::RegisterWithRegistrar(
    flutter::PluginRegistrarWindows *registrar) {
  auto channel =
      std::make_shared<flutter::MethodChannel<flutter::EncodableValue>>(
          registrar->messenger(), "com.nimroddayan/flutter_nsd",
          &flutter::StandardMethodCodec::GetInstance());

  auto plugin = std::make_unique<FlutterNsdPlugin>(channel);

  channel->SetMethodCallHandler(
      [plugin_pointer = plugin.get()](const auto &call, auto result) {
        plugin_pointer->HandleMethodCall(call, std::move(result));
      });

  registrar->AddPlugin(std::move(plugin));
}



FlutterNsdPlugin::FlutterNsdPlugin(std::shared_ptr<flutter::MethodChannel<flutter::EncodableValue>> _channel)

{
  channel = _channel;
  currentRequest = NULL;

  WORD versionWanted = MAKEWORD(1, 1);
  WSADATA wsaData;
  if (WSAStartup(versionWanted, &wsaData)) {

    printf("Failed to initialize WinSock\n");
  }
}

FlutterNsdPlugin::~FlutterNsdPlugin() {
  WSACleanup();
}



void FlutterNsdPlugin::HandleMethodCall(
  const flutter::MethodCall<flutter::EncodableValue>& method_call,
  std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  if (method_call.method_name().compare("getPlatformVersion") == 0) {
    std::ostringstream version_stream;
    version_stream << "Windows ";
    if (IsWindows10OrGreater()) {
      version_stream << "10+";
    }
    else if (IsWindows8OrGreater()) {
      version_stream << "8";
    }
    else if (IsWindows7OrGreater()) {
      version_stream << "7";
    }
    result->Success(flutter::EncodableValue(version_stream.str()));
  }
  else if (method_call.method_name().compare("startDiscovery") == 0) {
    const auto* arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());
    auto serviceType = arguments->find(flutter::EncodableValue("serviceType"));
    std::string type = "";
    if (serviceType != arguments->end()) {
      type = std::get<std::string>(serviceType->second);
    }

    if (currentRequest != NULL) {
      auto req = currentRequest;
      currentRequest = NULL; // request will delete itself after the thread exits, we can detach the pointer here
      req->stop(); // will stop the background thread after the next timeout; returns immediately
    }
    currentRequest = new MdnsRequest(channel);
    currentRequest->start(type);
    result->Success();
  }
  else if (method_call.method_name().compare("stopDiscovery") == 0) {
    if (currentRequest != NULL) {
      auto req = currentRequest;
      currentRequest = NULL;
      req->stop();

    }
    result->Success();
  }
  else {
    result->NotImplemented();
  }
}






}  // namespace



void call_HandlePTRRecord(const void* p, const void* base, boolean last, STRING_ARG_DECL(service), STRING_ARG_DECL(name)) {
  ((MdnsRequest*)p)->callbackPTR(base, last, STRING_ARG_CALL(service), STRING_ARG_CALL(name));
}


  void call_HandleSRVRecord(const void* p, const void* base, boolean last, STRING_ARG_DECL(name), STRING_ARG_DECL(hostname), int port) {
    ((MdnsRequest*)p)->callbackSRV(base, last, STRING_ARG_CALL(name), STRING_ARG_CALL(hostname), port);
  }


  void call_HandleARecord(const void* p, const void* base, boolean last, STRING_ARG_DECL(hostname), STRING_ARG_DECL(address)) {
    ((MdnsRequest*)p)->callbackA(base, last, STRING_ARG_CALL(hostname), STRING_ARG_CALL(address));
  }

  void call_HandleAAAARecord(const void* p, const void* base, boolean last, STRING_ARG_DECL(hostname), STRING_ARG_DECL(address)) {
    ((MdnsRequest*)p)->callbackAAAA(base, last, STRING_ARG_CALL(hostname), STRING_ARG_CALL(address));
  }

  void call_HandleTXTRecord(const void* p, const void* base, boolean last, STRING_ARG_DECL(key), STRING_ARG_DECL(value)) {
    ((MdnsRequest*)p)->callbackTXT(base, last, STRING_ARG_CALL(key), STRING_ARG_CALL(key));
  }


  void call_HandleURecord(const void* p, const void* base, boolean last) {
    ((MdnsRequest*)p)->callbackU(base, last);
  }


void FlutterNsdPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  FlutterNsdPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}


