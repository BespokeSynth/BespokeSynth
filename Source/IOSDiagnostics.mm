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
//  IOSDiagnostics.mm
//  Bespoke
//

#include "IOSDiagnostics.h"

#import <Foundation/Foundation.h>

namespace
{
   std::function<void(const juce::String&)> sLog;

   void HandleUncaughtException(NSException* exception)
   {
      if (!sLog)
         return;
      juce::String message = "uncaught exception: " + juce::String::fromUTF8(exception.name.UTF8String) + ": " + juce::String::fromUTF8(exception.reason.UTF8String);
      for (NSString* frame in exception.callStackSymbols)
         message << "\n   " << juce::String::fromUTF8(frame.UTF8String);
      sLog(message);
   }
}

void IOSDiagnostics::LogUncaughtExceptions(std::function<void(const juce::String& message)> log)
{
   sLog = std::move(log);
   NSSetUncaughtExceptionHandler(&HandleUncaughtException);
}
