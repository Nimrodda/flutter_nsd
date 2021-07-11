/*
 * Copyright 2021 Nimrod Dayan nimroddayan.com
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 *
 */

package com.nimroddayan.flutternsd

import android.net.nsd.NsdManager
import android.net.nsd.NsdServiceInfo
import android.os.Handler
import android.os.Looper
import androidx.annotation.NonNull
import androidx.core.content.ContextCompat.getSystemService
import io.flutter.embedding.engine.plugins.FlutterPlugin
import io.flutter.plugin.common.MethodCall
import io.flutter.plugin.common.MethodChannel
import io.flutter.plugin.common.MethodChannel.MethodCallHandler
import io.flutter.plugin.common.MethodChannel.Result
import io.flutter.plugin.common.PluginRegistry.Registrar
import timber.log.Timber
import java.util.LinkedList
import java.util.Queue

/** FlutterNsdPlugin */
class FlutterNsdPlugin : FlutterPlugin, MethodCallHandler {
  /// The MethodChannel that will the communication between Flutter and native Android
  ///
  /// This local reference serves to register the plugin with the Flutter Engine and unregister it
  /// when the Flutter Engine is detached from the Activity
  private lateinit var channel: MethodChannel

  private var nsdManager: NsdManager? = null

  private lateinit var serviceType: String
  private lateinit var mainHandler: Handler

  /// The serviceResolveQueue is used to sequence the service resolve calls
  ///
  /// Android's NsdManager does not allow multiple resolve procedures in
  /// parallel, thus they must be sequenced to avoid running into errors if
  /// devices are discovered quickly after another.
  private var serviceResolveQueue: Queue<NsdServiceInfo> = LinkedList()

  override fun onAttachedToEngine(@NonNull flutterPluginBinding: FlutterPlugin.FlutterPluginBinding) {
    nsdManager = getSystemService(flutterPluginBinding.applicationContext, NsdManager::class.java)
    channel = MethodChannel(flutterPluginBinding.binaryMessenger, "com.nimroddayan/flutter_nsd")
    mainHandler = Handler(Looper.getMainLooper())
    channel.setMethodCallHandler(this)
    Timber.d("Plugin initialized successfully")
  }

  // This static function is optional and equivalent to onAttachedToEngine. It supports the old
  // pre-Flutter-1.12 Android projects. You are encouraged to continue supporting
  // plugin registration via this function while apps migrate to use the new Android APIs
  // post-flutter-1.12 via https://flutter.dev/go/android-project-migration.
  //
  // It is encouraged to share logic between onAttachedToEngine and registerWith to keep
  // them functionally equivalent. Only one of onAttachedToEngine or registerWith will be called
  // depending on the user's project. onAttachedToEngine or registerWith must both be defined
  // in the same class.
  companion object {
    @Suppress("unused")
    @JvmStatic
    fun registerWith(registrar: Registrar) {
      val channel = MethodChannel(registrar.messenger(), "com.nimroddayan/flutter_nsd")
      channel.setMethodCallHandler(FlutterNsdPlugin())
    }
  }

  override fun onMethodCall(@NonNull call: MethodCall, @NonNull result: Result) {
    if (nsdManager == null) {
      result.error("1000", "NsdManager not initialized", null)
    }

    Timber.d("Method called: ${call.method}")
    when (call.method) {
      "startDiscovery" -> {
        val serviceType = try {
          call.argument<String>("serviceType")
        } catch (ex: Exception) {
          result.error("1002", "service type must be a string", null)
          return
        }

        if (serviceType == null) {
          result.error("1001", "service type cannot be null", null)
          return
        }

        startDiscovery(serviceType)
        result.success(null)
      }
      "stopDiscovery" -> {
        stopDiscovery()
        result.success(null)
      }
      else -> {
        result.notImplemented()
      }
    }
  }

  override fun onDetachedFromEngine(@NonNull binding: FlutterPlugin.FlutterPluginBinding) {
    channel.setMethodCallHandler(null)
  }

  private fun startDiscovery(serviceType: String) {
    Timber.d("Staring NSD for service type: $serviceType")
    this.serviceType = serviceType
    try {
      nsdManager?.discoverServices(serviceType, NsdManager.PROTOCOL_DNS_SD, discoveryListener)
    } catch (ex: IllegalArgumentException) {
      // Happens when the listener is already registered - indicating that there's already
      // an ongoing discovery
      Timber.w(ex, "Cannot start, NSD is already running")
      mainHandler.post {
        channel.invokeMethod("onStartDiscoveryFailed", null)
      }
    }
  }

  private fun stopDiscovery() {
    Timber.d("Stopping NSD")
    try {
      nsdManager?.stopServiceDiscovery(discoveryListener)
    } catch (ex: IllegalArgumentException) {
      // Happens when the listener is not registered - indicating that there was no ongoing
      // discovery.
      Timber.w(ex, "Cannot stop NSD when it's not started")
      mainHandler.post {
        channel.invokeMethod("onStopDiscoveryFailed", null)
      }
    }
  }

  private fun resolveService(service: NsdServiceInfo?) {
    try {
      nsdManager?.resolveService(service, resolveListener)
    } catch (e: Exception) {
      Timber.w(e, "Cannot resolve service, service resolve in progress")
    }
  }

  private val discoveryListener = object : NsdManager.DiscoveryListener {

    // Called as soon as service discovery begins.
    override fun onDiscoveryStarted(regType: String) {
      Timber.d("NSD started")
    }

    override fun onServiceFound(service: NsdServiceInfo) {
      Timber.d("Service found serviceName: ${service.serviceName} serviceType: ${service.serviceType}")
      if (serviceType == service.serviceType) {
        Timber.d("Resolving service $service")
        serviceResolveQueue.add(service)

        // Resolve service if no other service is currently being
        // resolved. Otherwise do nothing here, the service will be
        // resolved once other services from the queue were resolved.
        if (serviceResolveQueue.count() == 1) {
          resolveService(serviceResolveQueue.peek())
        }
      }
    }

    override fun onServiceLost(service: NsdServiceInfo) {
      Timber.v("Service lost $service")
    }

    override fun onDiscoveryStopped(serviceType: String) {
      Timber.d("NSD stopped")
      mainHandler.post {
        channel.invokeMethod("onDiscoveryStopped", null)
      }
    }

    override fun onStartDiscoveryFailed(serviceType: String, errorCode: Int) {
      Timber.w("Failed to start NSD. Error code $errorCode")
      nsdManager?.stopServiceDiscovery(this)
      mainHandler.post {
        channel.invokeMethod("onStartDiscoveryFailed", errorCode)
      }
    }

    override fun onStopDiscoveryFailed(serviceType: String, errorCode: Int) {
      Timber.w("Failed to stop NSD. Error code $errorCode")
      nsdManager?.stopServiceDiscovery(this)
      mainHandler.post {
        channel.invokeMethod("onStopDiscoveryFailed", errorCode)
      }
    }
  }

  private val resolveListener = object : NsdManager.ResolveListener {
    override fun onResolveFailed(serviceInfo: NsdServiceInfo?, errorCode: Int) {
      Timber.w("Failed to resolve service $serviceInfo error code: $errorCode")
      mainHandler.post {
        channel.invokeMethod("onResolveFailed", errorCode)
      }

      processQueue()
    }

    override fun onServiceResolved(serviceInfo: NsdServiceInfo?) {
      val hostname = serviceInfo?.host?.canonicalHostName
      val port = serviceInfo?.port
      val name = serviceInfo?.serviceName
      val txt = serviceInfo?.attributes

      Timber.v("Resolved service: $name-$hostname:$port $txt")
      val result = mapOf(
        "hostname" to hostname,
        "port" to port,
        "name" to name,
        "txt" to txt
      )
      mainHandler.post {
        channel.invokeMethod("onServiceResolved", result)
      }

      processQueue()
    }

    private fun processQueue() {
      serviceResolveQueue.remove()
      // Resolve next service from queue
      if (!serviceResolveQueue.isEmpty()) {
        resolveService(serviceResolveQueue.peek())
      }
    }
  }
}
