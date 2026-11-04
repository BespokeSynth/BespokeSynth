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

namespace IOSDocuments
{
   //handler is called on the main thread, while the file is readable (some opened files are only accessible
   //temporarily, so the handler should copy anything it needs to keep)
   void SetOpenFileHandler(std::function<void(const juce::File& file)> handler);
}
