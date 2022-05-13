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
//  Logging.cpp
//
//  Created by Noxy Nixie on 8th of May 2022.
//

#include "Logger.h"

#include <iomanip>
#include <ctime>
#include <string>
#include <sstream>
#include <fstream>
#include <memory>
#include "juce_core/juce_core.h"
#include "SynthGlobals.h"

namespace Bespoke
{
   Logger::Logger(std::string logFile, LogLevel maxLevel)
   : maxLogLevel(maxLevel)
   {
      juce::File(ofToDataPath("logs")).createDirectory();
      file.open(ofToDataPath("logs/" + logFile), std::ofstream::app);
   }

   Logger::~Logger()
   {
      file.flush();
      file.close();
   }

   void Logger::Log(std::string message, LogLevel level /* = LogLevel::Info */, bool toCout /* = true */)
   {
      if (level < maxLogLevel)
         return;
      std::time_t t = std::time(nullptr);
      std::tm tm = *std::localtime(&t);
      std::stringstream output;
      std::lock_guard<std::mutex> lock(logMutex);
      output << "[" << std::put_time(&tm, "%FT%T%z") << "] "
             << ofToString(gTime / 1000, 8) << " "
             << LogLevelToString(level) << " "
             << message << std::endl;

      file << output.str() << std::flush;
      if (!toCout)
         return;
      if (level > LogLevel::Warning)
         std::cerr << output.str() << std::flush;
      else
         std::cout << output.str() << std::flush;
   }
}
