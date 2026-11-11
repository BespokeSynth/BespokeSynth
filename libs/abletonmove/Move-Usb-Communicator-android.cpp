// Android implementation of UsbCommunicator (see Move-Usb-Communicator.cpp for the libusb one used elsewhere).
//
// Android apps can't enumerate or open USB devices directly, so the device is opened by bespoke's java side, which
// asks the user for permission, and this writes frames through the file descriptor it returns
// (see BespokeAndroidUsb* in Source/AndroidSupport.cpp). Like the libusb version, a frame is sent when
// SendBitmapToDevice() asks for one, from a thread of its own so the caller never waits on usb.

#include "Move-UsbCommunicator.h"

#include <chrono>
#include <string>

extern "C"
{
  int BespokeAndroidUsbOpen(int vendorId, int productId, int interfaceNumber, char* error, int errorSize);
  void BespokeAndroidUsbClose(int fd);
  bool BespokeAndroidUsbBulkWrite(int fd, unsigned char endpoint, const unsigned char* data, int length, int timeoutMs);
}

using namespace abletonmove;

namespace
{
  const int kAbletonVendorID = 0x2982;
  const int kMoveProductID = 0x1958;
  const int kDisplayInterface = 0;
  const unsigned char kMoveBulkEPOut = 0x01;
  const int kTimeoutMs = 1000;
  const unsigned char kFrameHeader[16] = { 0xFF, 0xCC, 0xAA, 0x88, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  const auto kReconnectInterval = std::chrono::seconds(1);
}

//------------------------------------------------------------------------------

UsbCommunicator::UsbCommunicator()
  : handle_(NULL)
{
}

//------------------------------------------------------------------------------

NBase::Result UsbCommunicator::Init(unsigned char* dataSource)
{
  dataSource_ = dataSource;

  char error[256] = "";
  fd_ = BespokeAndroidUsbOpen(kAbletonVendorID, kMoveProductID, kDisplayInterface, error, sizeof(error));
  if (fd_ < 0)
    return NBase::Result(std::string(error));

  terminate_ = false;
  pollThread_ = std::thread(&UsbCommunicator::PollUsbForEvents, this);
  SendBitmapToDevice(); // show whatever's there now
  return NBase::Result::NoError;
}

//------------------------------------------------------------------------------

UsbCommunicator::~UsbCommunicator()
{
  {
    std::lock_guard<std::mutex> lock(sendMutex_);
    terminate_ = true;
  }
  sendCondition_.notify_one();
  if (pollThread_.joinable())
    pollThread_.join();
  if (fd_ >= 0)
    BespokeAndroidUsbClose(fd_);
}

//------------------------------------------------------------------------------

void UsbCommunicator::OnTransferFinished(libusb_transfer*)
{
  // transfers are synchronous on android
}

//------------------------------------------------------------------------------

void UsbCommunicator::SendBitmapToDevice()
{
  {
    std::lock_guard<std::mutex> lock(sendMutex_);
    frameRequested_ = true;
  }
  sendCondition_.notify_one();
}

//------------------------------------------------------------------------------

// on android, this is the thread that sends frames to the display
void UsbCommunicator::PollUsbForEvents()
{
  while (true)
  {
    {
      std::unique_lock<std::mutex> lock(sendMutex_);
      // wake up periodically while disconnected, to try to reopen the display
      sendCondition_.wait_for(lock, kReconnectInterval, [this] { return frameRequested_ || terminate_.load(); });
      if (terminate_.load())
        return;
    }

    if (fd_ < 0)
    {
      fd_ = BespokeAndroidUsbOpen(kAbletonVendorID, kMoveProductID, kDisplayInterface, nullptr, 0);
      if (fd_ < 0)
        continue;
    }

    {
      std::lock_guard<std::mutex> lock(sendMutex_);
      if (!frameRequested_)
        continue;
      frameRequested_ = false;
    }

    if (!BespokeAndroidUsbBulkWrite(fd_, kMoveBulkEPOut, kFrameHeader, sizeof(kFrameHeader), kTimeoutMs) ||
        !BespokeAndroidUsbBulkWrite(fd_, kMoveBulkEPOut, dataSource_, kSendBufferSize, kTimeoutMs))
    {
      BespokeAndroidUsbClose(fd_);
      fd_ = -1;
      std::lock_guard<std::mutex> lock(sendMutex_);
      frameRequested_ = true; // resend once it's back
    }
  }
}
