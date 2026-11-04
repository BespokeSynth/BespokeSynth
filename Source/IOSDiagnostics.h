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
//  IOSDiagnostics.h
//  Bespoke
//

#pragma once

#include "juce_core/juce_core.h"

#include <functional>

namespace IOSDiagnostics
{
   //logs the name, reason and call stack of uncaught Objective-C exceptions (e.g. UIKit assertions), which bespoke's
   //crash handler otherwise only sees as an abort with no explanation
   void LogUncaughtExceptions(std::function<void(const juce::String& message)> log);

   //logs audio session interruptions, route changes and media server resets
   void LogAudioSessionEvents(std::function<void(const juce::String& message)> log);

   juce::String GetAudioSessionState();
}
