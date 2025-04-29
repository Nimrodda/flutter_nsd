/*
 * Copyright 2025 Nimrod Dayan nimroddayan.com
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

plugins {
  id("com.android.application")
  id("kotlin-android")
  // The Flutter Gradle Plugin must be applied after the Android and Kotlin Gradle plugins.
  id("dev.flutter.flutter-gradle-plugin")
}

android {
  namespace = "com.nimroddayan.flutternsd.example"
  compileSdk = flutter.compileSdkVersion
  ndkVersion = flutter.ndkVersion

  ndkVersion = "27.0.12077973"

  compileOptions {
    sourceCompatibility = JavaVersion.VERSION_11
    targetCompatibility = JavaVersion.VERSION_11
  }

  kotlinOptions {
    jvmTarget = JavaVersion.VERSION_11.toString()
  }

  defaultConfig {
    applicationId = "com.nimroddayan.flutternsd.example"
    minSdk = flutter.minSdkVersion
    targetSdk = flutter.targetSdkVersion
    versionCode = flutter.versionCode
    versionName = flutter.versionName
  }

  buildTypes {
    release {
      signingConfig = signingConfigs.getByName("debug")
    }
  }
}

flutter {
  source = "../.."
}

dependencies {
  implementation("com.jakewharton.timber:timber:5.0.1")
}
