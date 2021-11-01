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
//  FileStream.h
//  Bespoke
//
//  Created by Ryan Challinor on 4/26/15.
//
//

#ifndef __Bespoke__FileStream__
#define __Bespoke__FileStream__

#include <cstdint>
#include <memory>
#include <string>

namespace juce {
   class FileInputStream;
   class FileOutputStream;
}

class FileStreamOut
{
public:
   explicit FileStreamOut(const std::string& file);
   FileStreamOut(const char*) = delete; // Hint: UTF-8 encoded std::string required
   ~FileStreamOut();
   FileStreamOut& operator<<(const int& var);
   FileStreamOut& operator<<(const std::uint32_t &var);
   FileStreamOut& operator<<(const bool& var);
   FileStreamOut& operator<<(const float& var);
   FileStreamOut& operator<<(const double& var);
   FileStreamOut& operator<<(const std::string& var);
   FileStreamOut& operator<<(const char& var);
   void Write(const float* buffer, int size);
   void WriteGeneric(const void* buffer, int size);
private:
   std::unique_ptr<juce::FileOutputStream> mStream;
};

class FileStreamIn
{
public:
   explicit FileStreamIn(const std::string& file);
   FileStreamIn(const char*) = delete; // Hint: UTF-8 encoded std::string required
   ~FileStreamIn();
   FileStreamIn& operator>>(int& var);
   FileStreamIn& operator>>(std::uint32_t &var);
   FileStreamIn& operator>>(bool& var);
   FileStreamIn& operator>>(float& var);
   FileStreamIn& operator>>(double& var);
   FileStreamIn& operator>>(std::string& var);
   FileStreamIn& operator>>(char& var);
   void Read(float* buffer, int size);
   void ReadGeneric(void* buffer, int size);
   void Peek(void* buffer, int size);
   int GetFilePosition() const;
   bool OpenedOk() const;
   bool Eof() const;
   static bool s32BitMode;
private:
    std::unique_ptr<juce::FileInputStream> mStream;
};

#endif /* defined(__Bespoke__FileStream__) */
