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
//  FileStream.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 4/26/15.
//
//

#include "FileStream.h"
#include "ModularSynth.h"

#include "juce_core/juce_core.h"

//static
bool FileStreamIn::s32BitMode = false;

static std::unique_ptr<juce::FileOutputStream> MakeFileOutStream(const std::string& path)
{
   auto stream = std::make_unique<juce::FileOutputStream>(juce::File{ path });
   stream->setPosition(0);
   stream->truncate();
   return stream;
}

FileStreamOut::FileStreamOut(const std::string& file)
: mStream(MakeFileOutStream(file))
{
}

FileStreamOut::FileStreamOut(juce::MemoryBlock& block, bool appendToExistingBlockContent)
: mStream(std::make_unique<juce::MemoryOutputStream>(block, appendToExistingBlockContent))
{
}

FileStreamOut::FileStreamOut(std::unique_ptr<juce::OutputStream>&& stream)
: mStream(std::move(stream))
{
}

FileStreamOut::~FileStreamOut()
{
   mStream->flush();
}

FileStreamIn::FileStreamIn(const std::string& file)
: mStream(std::make_unique<juce::FileInputStream>(juce::File{ file }))
{
}

FileStreamIn::FileStreamIn(const juce::MemoryBlock& block)
: mStream(std::make_unique<juce::MemoryInputStream>(block, false))
{
}

FileStreamIn::FileStreamIn(std::unique_ptr<juce::InputStream>&& stream)
: mStream(std::move(stream))
{
}

FileStreamIn::~FileStreamIn() = default;

FileStreamOut& FileStreamOut::operator<<(const int& var)
{
   mStream->write(&var, sizeof(int));
   return *this;
}

FileStreamOut& FileStreamOut::operator<<(const uint32_t& var)
{
   mStream->write(&var, sizeof(uint32_t));
   return *this;
}

FileStreamOut& FileStreamOut::operator<<(const bool& var)
{
   mStream->write(&var, sizeof(bool));
   return *this;
}

FileStreamOut& FileStreamOut::operator<<(const float& var)
{
   mStream->write(&var, sizeof(float));
   return *this;
}

FileStreamOut& FileStreamOut::operator<<(const double& var)
{
   mStream->write(&var, sizeof(double));
   return *this;
}

FileStreamOut& FileStreamOut::operator<<(const std::string& var)
{
   const uint64_t len = var.length();
   mStream->write(&len, sizeof(len));
   mStream->write(var.data(), len);
   return *this;
}

FileStreamOut& FileStreamOut::operator<<(const char& var)
{
   mStream->write(&var, sizeof(char));
   return *this;
}

void FileStreamOut::Write(const float* buffer, int size)
{
   mStream->write(buffer, sizeof(float) * size);
}

void FileStreamOut::WriteGeneric(const void* buffer, int size)
{
   mStream->write(buffer, size);
}

std::int64_t FileStreamOut::GetSize() const
{
   return mStream->getPosition();
}

FileStreamIn& FileStreamIn::operator>>(int& var)
{
   mStream->read(&var, sizeof(int));
   return *this;
}

FileStreamIn& FileStreamIn::operator>>(uint32_t& var)
{
   mStream->read(&var, sizeof(uint32_t));
   return *this;
}

FileStreamIn& FileStreamIn::operator>>(bool& var)
{
   mStream->read(&var, sizeof(bool));
   return *this;
}

FileStreamIn& FileStreamIn::operator>>(float& var)
{
   mStream->read(&var, sizeof(float));
   return *this;
}

FileStreamIn& FileStreamIn::operator>>(double& var)
{
   mStream->read(&var, sizeof(double));
   return *this;
}

FileStreamIn& FileStreamIn::operator>>(std::string& var)
{
   uint64_t len64;
   if (s32BitMode)
   {
      uint32_t len32;
      mStream->read(&len32, sizeof(len32));
      len64 = len32;
   }
   else
   {
      mStream->read(&len64, sizeof(len64));
   }

   if (TheSynth->IsLoadingModule())
      LoadStateValidate(len64 < sMaxStringLength); //probably garbage beyond this point
   else
      assert(len64 < sMaxStringLength); //probably garbage beyond this point

   size_t len = len64;
   var.resize(len);
   mStream->read(var.data(), len);
   return *this;
}

FileStreamIn& FileStreamIn::operator>>(char& var)
{
   mStream->read(&var, sizeof(char));
   return *this;
}

void FileStreamIn::Read(float* buffer, int size)
{
   mStream->read(buffer, sizeof(float) * size);
}

void FileStreamIn::ReadGeneric(void* buffer, int size)
{
   mStream->read(buffer, size);
}

void FileStreamIn::Peek(void* buffer, int size)
{
   auto pos = mStream->getPosition();
   mStream->read(buffer, size);
   mStream->setPosition(pos);
}

bool FileStreamIn::Eof() const
{
   return mStream->isExhausted();
}

int FileStreamIn::GetFilePosition() const
{
   return int(mStream->getPosition());
}

bool FileStreamIn::OpenedOk() const
{
   if (auto* file = dynamic_cast<juce::FileInputStream*>(mStream.get()))
      return file->openedOk();

   return true;
}
