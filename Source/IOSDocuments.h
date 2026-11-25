/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2022 Ryan Challinor (contact: awwbees@gmail.com)

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
//  IOSDocuments.h
//  Bespoke
//
//  Handles documents opened in bespoke from other apps (Files, AirDrop, Messages, etc).
//  JUCE's iOS app delegate doesn't handle opened URLs, so this adds that to it (at load time, see IOSDocuments.mm).
//

#pragma once

#include "juce_core/juce_core.h"

#include <functional>
#include <string>

namespace IOSDocuments
{
   //handler is called on the main thread, while the file is readable (some opened files are only accessible
   //temporarily, so the handler should copy anything it needs to keep)
   void SetOpenFileHandler(std::function<void(const juce::File& file)> handler);

   //files outside of our sandbox (e.g. in iCloud Drive) can only be accessed through the URL from the file chooser. its
   //bookmark can be stored, to get a URL with the same access after a restart. returns an empty string if there isn't one
   std::string GetBookmark(const juce::URL& url);

   struct ResolvedBookmark
   {
      juce::URL mURL; //empty if the file is gone, or we don't have access to it anymore
      bool mIsDownloaded{ false };
   };
   ResolvedBookmark ResolveBookmark(const std::string& bookmark);

   //waits for a file's contents to be downloaded and readable, without blocking. onDone is called on the main thread, with
   //an empty string if it succeeded
   void CoordinateRead(const juce::URL& url, std::function<void(const juce::String& error)> onDone);
}
