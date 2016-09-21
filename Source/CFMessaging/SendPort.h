//
//  SendPort.h
//  NI_SoftwareSide
//
//  Created by Ryan Challinor on 11/6/14.
//
//

#ifndef __NI_SoftwareSide__SendPort__
#define __NI_SoftwareSide__SendPort__

#include <CoreFoundation/CoreFoundation.h>
#include <string>

using namespace std;

class SendPort
{
public:
   SendPort() : mSendPort(NULL) {}
   ~SendPort();
   void CreateSender(const char* portName);
   void SendData(uint32_t messageId, CFDataRef data, CFDataRef& replyData);
   void Close();
private:
   string mPortName;
   CFMessagePortRef mSendPort;
};

#endif /* defined(__NI_SoftwareSide__SendPort__) */
