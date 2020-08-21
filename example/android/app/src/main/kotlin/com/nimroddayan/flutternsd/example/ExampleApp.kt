package com.nimroddayan.flutternsd.example

import io.flutter.app.FlutterApplication
import timber.log.Timber

class ExampleApp : FlutterApplication() {
    override fun onCreate() {
        super.onCreate()
        Timber.plant(Timber.DebugTree())
    }
}