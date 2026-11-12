/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2026 Ryan Challinor (contact: awwbees@gmail.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/
//
//  AndroidSupport.h
//  Bespoke
//
//  Android-specific file locations and logging.
//

#pragma once

#include "juce_core/juce_core.h"

namespace AndroidSupport
{
   //folder holding the factory files (resource/ etc). on android these are packaged into the apk as assets, which
   //aren't regular files, so the first launch after each install or update unpacks them into app storage
   juce::File GetFactoryDir();

   //default user data folder: the app's external files dir (Android/data/<package>/files), which can be reached
   //over usb and with adb, unlike internal storage. falls back to internal storage if there's no external storage
   juce::File GetDataDir();

   //writes to logcat (tag "bespoke"), since DBG is compiled out of release builds
   void Log(const char* message);

   //documents picked with the system file picker are content:// URLs rather than files. bespoke works with files,
   //so each one gets a local copy (its "mirror", named after the document) that it can be read from, and saves are
   //written to both the mirror and the document
   bool IsDocument(const juce::URL& url); //a content:// URL. (on android, juce's URL::isLocalFile() tries to look these up as files, which needs storage permissions)
   juce::String GetDocumentName(const juce::URL& document);
   juce::File GetDocumentMirror(const juce::URL& document);
   juce::Result CopyDocumentToMirror(const juce::URL& document);
   //replaces the document's contents with the source file's
   juce::Result WriteToDocument(const juce::URL& document, const juce::File& source);

   //runs a foreground service (BespokeAudioService.java) while bespoke is open. otherwise android counts bespoke as
   //a background app whenever something else is in front, such as the file picker, and the audio can't keep up.
   //it has to be started while bespoke is in front
   void StartAudioService();
   void StopAudioService();

   //tells android how long each audio callback takes compared to its deadline (a "performance hint session"), so it
   //can raise the cpu clock in time. android does this by itself for whatever is in front, but not for apps running
   //behind something else, like the file picker, where the audio otherwise drops out now and then
   class AudioThreadPerformanceHint
   {
   public:
      ~AudioThreadPerformanceHint();
      //call these at the start and end of each audio callback, on the audio thread
      void BeginCallback(int numSamples, double sampleRate);
      void EndCallback();

   private:
      void CloseSession();

      void* mSession{ nullptr }; //APerformanceHintSession
      int mThreadId{ 0 }; //that mSession was made for, or that making one failed for
      int64_t mTargetNanos{ 0 };
      int64_t mCallbackStartNanos{ 0 };
   };
}

//USB access for the libraries that drive the Ableton Move and Push 2 displays (libs/abletonmove, libs/push2), which
//can't use JUCE or JNI themselves. android apps can only open USB devices through the system, with the user's
//permission, so the device is opened by the java side (BespokeUsb.java), and its file descriptor is given to libusb
extern "C" {
//returns a file descriptor for the device, or -1 with a reason in error. the first time, this asks the user for
//permission and fails with "waiting for usb permission", so callers should retry
int BespokeAndroidUsbOpen(int vendorId, int productId, char* error, int errorSize);
}
