// Android implementation of UsbCommunicator (see Push2-Usb-Communicator.cpp for the libusb one used elsewhere).
//
// Android apps can't enumerate or open USB devices directly, so the device is opened by bespoke's java side, which
// asks the user for permission, and this writes frames through the file descriptor it returns
// (see BespokeAndroidUsb* in Source/AndroidSupport.cpp). Frames are streamed continuously, like the libusb version,
// since the display blanks itself if it stops receiving them.

#include "Push2-UsbCommunicator.h"

#include <chrono>
#include <string>

extern "C"
{
  int BespokeAndroidUsbOpen(int vendorId, int productId, int interfaceNumber, char* error, int errorSize);
  void BespokeAndroidUsbClose(int fd);
  bool BespokeAndroidUsbBulkWrite(int fd, unsigned char endpoint, const unsigned char* data, int length, int timeoutMs);
}

using namespace ableton;

namespace
{
  const int kAbletonVendorID = 0x2982;
  const int kPush2ProductID = 0x1967;
  const int kDisplayInterface = 0;
  const unsigned char kPush2BulkEPOut = 0x01;
  const int kTimeoutMs = 1000;
  const unsigned char kFrameHeader[16] = { 0xFF, 0xCC, 0xAA, 0x88, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  const int kFrameSize = UsbCommunicator::kLineSize * 160;
  const auto kFrameInterval = std::chrono::milliseconds(16); // ~60fps
  const auto kReconnectInterval = std::chrono::seconds(1);
}

//------------------------------------------------------------------------------

UsbCommunicator::UsbCommunicator()
  : handle_(NULL)
{
}

//------------------------------------------------------------------------------

NBase::Result UsbCommunicator::Init(const pixel_t* dataSource)
{
  dataSource_ = dataSource;

  char error[256] = "";
  fd_ = BespokeAndroidUsbOpen(kAbletonVendorID, kPush2ProductID, kDisplayInterface, error, sizeof(error));
  if (fd_ < 0)
    return NBase::Result(std::string(error));

  terminate_ = false;
  pollThread_ = std::thread(&UsbCommunicator::PollUsbForEvents, this);
  return NBase::Result::NoError;
}

//------------------------------------------------------------------------------

UsbCommunicator::~UsbCommunicator()
{
  terminate_ = true;
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

// on android, this is the thread that sends frames to the display
void UsbCommunicator::PollUsbForEvents()
{
  while (!terminate_.load())
  {
    auto frameStart = std::chrono::steady_clock::now();

    if (fd_ < 0)
    {
      // the display was unplugged, so keep trying to reopen it
      std::this_thread::sleep_for(kReconnectInterval);
      fd_ = BespokeAndroidUsbOpen(kAbletonVendorID, kPush2ProductID, kDisplayInterface, nullptr, 0);
      continue;
    }

    if (!BespokeAndroidUsbBulkWrite(fd_, kPush2BulkEPOut, kFrameHeader, sizeof(kFrameHeader), kTimeoutMs) ||
        !BespokeAndroidUsbBulkWrite(fd_, kPush2BulkEPOut, (const unsigned char*)dataSource_, kFrameSize, kTimeoutMs))
    {
      BespokeAndroidUsbClose(fd_);
      fd_ = -1;
      continue;
    }

    std::this_thread::sleep_until(frameStart + kFrameInterval);
  }
}
