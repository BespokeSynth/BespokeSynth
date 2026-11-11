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

   //logs the available midi ports now, and whenever they change
   void StartLoggingMidiPorts();

   //documents picked with the system file picker are content:// URLs rather than files. bespoke works with files,
   //so each one gets a local copy (its "mirror", named after the document) that it can be read from, and saves are
   //written to both the mirror and the document
   bool IsDocument(const juce::URL& url); //a content:// URL. (on android, juce's URL::isLocalFile() tries to look these up as files, which needs storage permissions)
   juce::String GetDocumentName(const juce::URL& document);
   juce::File GetDocumentMirror(const juce::URL& document);
   juce::Result CopyDocumentToMirror(const juce::URL& document);
   //replaces the document's contents with the source file's
   juce::Result WriteToDocument(const juce::URL& document, const juce::File& source);
}
