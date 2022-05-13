/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2021 Ryan Challinor (contact: awwbees@gmail.com)

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
//  Logging.h
//
//  Created by Noxy Nixie on 8th of May 2022.
//

#pragma once

#ifndef __Bespoke__Logger__
#define __Bespoke__Logger__

#include <string>
#include <memory>
#include <fstream>
#include "juce_core/juce_core.h"

namespace Bespoke
{
   enum class LogLevel
   {
      Debug,
      Info,
      Warning,
      Error,
      Fatal
   };

   class Logger
   {
   public:
      Logger(std::string logFile = "BespokeSynth.log", LogLevel maxLevel = LogLevel::Info);
      ~Logger();

      void Log(std::string message, LogLevel level = LogLevel::Info, bool toCout = true);
      static std::string LogLevelToString(LogLevel level)
      {
         switch (level)
         {
            case LogLevel::Debug:
               return "DEBUG:  ";
            case LogLevel::Info:
               return "INFO:   ";
            case LogLevel::Warning:
               return "WARNING:";
            case LogLevel::Error:
               return "ERROR:  ";
            case LogLevel::Fatal:
               return "FATAL:  ";
         }
         return "UNKNOWN:";
      }
      void setMaxLogLevel(LogLevel level) { maxLogLevel = level; }

   private:
      LogLevel maxLogLevel;
      std::ofstream file;
      std::mutex logMutex;
   };

}

#endif /* defined(__Bespoke__Logger__) */
