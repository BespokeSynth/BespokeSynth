//
//  FileStream.h
//  Bespoke
//
//  Created by Ryan Challinor on 4/26/15.
//
//

#ifndef __Bespoke__FileStream__
#define __Bespoke__FileStream__

#include "../JuceLibraryCode/JuceHeader.h"
#include "OpenFrameworksPort.h"

class FileStreamOut
{
public:
   FileStreamOut(const char* file);
   ~FileStreamOut();
   FileStreamOut& operator<<(const int& var);
   FileStreamOut& operator<<(const uint32_t &var);
   FileStreamOut& operator<<(const bool& var);
   FileStreamOut& operator<<(const float& var);
   FileStreamOut& operator<<(const double& var);
   FileStreamOut& operator<<(const string& var);
   FileStreamOut& operator<<(const char& var);
   void Write(const float* buffer, int size);
   void WriteGeneric(const void* buffer, int size);
private:
   FileOutputStream mStream;
};

class FileStreamIn
{
public:
   FileStreamIn(const char* file);
   FileStreamIn& operator>>(int& var);
   FileStreamIn& operator>>(uint32_t &var);
   FileStreamIn& operator>>(bool& var);
   FileStreamIn& operator>>(float& var);
   FileStreamIn& operator>>(double& var);
   FileStreamIn& operator>>(string& var);
   FileStreamIn& operator>>(char& var);
   void Read(float* buffer, int size);
   void ReadGeneric(void* buffer, int size);
   void Peek(void* buffer, int size);
   int GetFilePosition();
   bool OpenedOk() { return mStream.openedOk(); }
   
   bool Eof();
private:
   FileInputStream mStream;
};

#endif /* defined(__Bespoke__FileStream__) */
