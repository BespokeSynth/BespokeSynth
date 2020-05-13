//
//  FileStream.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 4/26/15.
//
//

#include "FileStream.h"
#include "ModularSynth.h"
#include "SynthGlobals.h"

FileStreamOut::FileStreamOut(const char* file)
: mStream(File(file))
{
   mStream.setPosition(0);
   mStream.truncate();
}

FileStreamOut::~FileStreamOut()
{
   mStream.flush();
}

FileStreamIn::FileStreamIn(const char* file)
: mStream(File(file))
{
}

FileStreamOut& FileStreamOut::operator<<(const int &var)
{
   mStream.write((const void*)&var, sizeof(int));
   return *this;
}

FileStreamOut& FileStreamOut::operator<<(const uint32_t &var)
{
   mStream.write((const void*)&var, sizeof(uint32_t));
   return *this;
}

FileStreamOut& FileStreamOut::operator<<(const bool &var)
{
   mStream.write((const void*)&var, sizeof(bool));
   return *this;
}

FileStreamOut& FileStreamOut::operator<<(const float &var)
{
   mStream.write((const void*)&var, sizeof(float));
   return *this;
}

FileStreamOut& FileStreamOut::operator<<(const double &var)
{
   mStream.write((const void*)&var, sizeof(double));
   return *this;
}

FileStreamOut& FileStreamOut::operator<<(const string &var)
{
   size_t len = var.length();
   mStream.write((const void*)&len, sizeof(size_t));
   for (int i=0; i<len; ++i)
      mStream.write((const void*)&var[i], sizeof(char));
   return *this;
}

FileStreamOut& FileStreamOut::operator<<(const char &var)
{
   mStream.write(&var, sizeof(char));
   return *this;
}

void FileStreamOut::Write(const float* buffer, int size)
{
   mStream.write((const void*)buffer, sizeof(float)*size);
}

void FileStreamOut::WriteGeneric(const void* buffer, int size)
{
   mStream.write((const void*)buffer, size);
}

FileStreamIn& FileStreamIn::operator>>(int &var)
{
   mStream.read((void*)&var, sizeof(int));
   return *this;
}

FileStreamIn& FileStreamIn::operator>>(uint32_t &var)
{
   mStream.read((void*)&var, sizeof(uint32_t));
   return *this;
}

FileStreamIn& FileStreamIn::operator>>(bool &var)
{
   mStream.read((void*)&var, sizeof(bool));
   return *this;
}

FileStreamIn& FileStreamIn::operator>>(float &var)
{
   mStream.read((void*)&var, sizeof(float));
   return *this;
}

FileStreamIn& FileStreamIn::operator>>(double &var)
{
   mStream.read((void*)&var, sizeof(double));
   return *this;
}

FileStreamIn& FileStreamIn::operator>>(string &var)
{
   size_t len;
   mStream.read((void*)&len, sizeof(size_t));
   
   if (TheSynth->IsLoadingModule())
      LoadStateValidate(len < 99999);   //probably garbage beyond this point
   else
      assert(len < 99999);   //probably garbage beyond this point
   
   var.resize(len);
   for (int i=0; i<len; ++i)
      mStream.read((void*)&var[i], sizeof(char));
   return *this;
}

FileStreamIn& FileStreamIn::operator>>(char &var)
{
   mStream.read(&var, sizeof(char));
   return *this;
}

void FileStreamIn::Read(float* buffer, int size)
{
   mStream.read((void*)buffer, sizeof(float)*size);
}

void FileStreamIn::ReadGeneric(void* buffer, int size)
{
   mStream.read((void*)buffer, size);
}
                        
void FileStreamIn::Peek(void* buffer, int size)
{
   auto pos = mStream.getPosition();
   mStream.read((void*)buffer, size);
   mStream.setPosition(pos);
}

bool FileStreamIn::Eof()
{
   return mStream.isExhausted();
}

int FileStreamIn::GetFilePosition()
{
   return (int)mStream.getPosition();
}
