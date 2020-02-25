// Copyright (c) 2017 Ableton AG, Berlin
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#include "Push2-UsbCommunicator.h"

#include <cstdint>

#ifdef _WIN32
// see following link for a discussion of the
// warning suppression:
// http://sourceforge.net/mailarchive/forum.php?
// thread_name=50F6011C.2020000%40akeo.ie&forum_name=libusbx-devel

// Disable: warning C4200: nonstandard extension used:
// zero-sized array in struct/union
#pragma warning(disable:4200)
#endif

#include "../libusb/libusb.h"

//------------------------------------------------------------------------------

namespace
{
  using namespace ableton;

  //----------------------------------------------------------------------------

  // Uses libusb to create a device handle for the push display

  NBase::Result SFindPushDisplayDeviceHandle(libusb_device_handle** pHandle)
  {
    using namespace NBase;

    int errorCode;

    // Initialises the library
    if ((errorCode = libusb_init(NULL)) < 0)
    {
      return NBase::Result("Failed to initialize usblib");
    }

    libusb_set_debug(NULL, LIBUSB_LOG_LEVEL_ERROR);

    // Get a list of connected devices
    libusb_device** devices;
    ssize_t count;
    count = libusb_get_device_list(NULL, &devices);
    if (count < 0)
    {
      return Result("could not get usb device list");
    }

    // Look for the one matching push2's decriptors
    libusb_device* device;
    libusb_device_handle* device_handle = NULL;

    char ErrorMsg[256];

    // set message in case we get to the end of the list w/o finding a device
    sprintf(ErrorMsg, "display device not found\n");

    for (int i = 0; (device = devices[i]) != NULL; i++)
    {
      struct libusb_device_descriptor descriptor;
      if ((errorCode = libusb_get_device_descriptor(device, &descriptor)) < 0)
      {
        sprintf(ErrorMsg, "could not get usb device descriptor, error: %d", errorCode);
        continue;
      }

      const uint16_t kAbletonVendorID = 0x2982;
      const uint16_t kPush2ProductID = 0x1967;

      if (descriptor.bDeviceClass == LIBUSB_CLASS_PER_INTERFACE
          && descriptor.idVendor == kAbletonVendorID
          && descriptor.idProduct == kPush2ProductID)
      {
        if ((errorCode = libusb_open(device, &device_handle)) < 0)
        {
          sprintf(ErrorMsg, "could not open device, error: %d", errorCode);
        }
        else if ((errorCode = libusb_claim_interface(device_handle, 0)) < 0)
        {
          sprintf(ErrorMsg, "could not claim device with interface 0, error: %d", errorCode);
          libusb_close(device_handle);
          device_handle = NULL;
        }
        else
        {
          break; // successfully opened
        }
      }
    }

    *pHandle = device_handle;
    libusb_free_device_list(devices, 1);

    return device_handle ? Result::NoError : Result(ErrorMsg);
  }

  //----------------------------------------------------------------------------

  // Callback received whenever a transfer has been completed.
  // We defer the processing to the communicator class

  void LIBUSB_CALL SOnTransferFinished(libusb_transfer* transfer)
  {
    static_cast<ableton::UsbCommunicator*>(transfer->user_data)->OnTransferFinished(transfer);
  }

  //----------------------------------------------------------------------------

  // Allocate a libusb_transfer mapped to a transfer buffer. It also sets
  // up the callback needed to communicate the transfer is done

  libusb_transfer* SAllocateAndPrepareTransferChunk(
                      libusb_device_handle* handle,
                      UsbCommunicator* instance,
                      unsigned char* buffer,
                      int bufferSize)
  {
    // Allocate a transfer structure
    libusb_transfer* transfer = libusb_alloc_transfer(0);
    if (!transfer)
    {
      return nullptr;
    }

    // Sets the transfer characteristic
    const unsigned char kPush2BulkEPOut = 0x01;

    libusb_fill_bulk_transfer(transfer, handle, kPush2BulkEPOut,
                              buffer, bufferSize,
                              SOnTransferFinished, instance, 1000);
    return transfer;
  }
}


//------------------------------------------------------------------------------

UsbCommunicator::UsbCommunicator()
  :handle_(NULL)
{
}


//------------------------------------------------------------------------------

NBase::Result UsbCommunicator::Init(const pixel_t* dataSource)
{
  using namespace NBase;

  // Capture the data source
  dataSource_ = dataSource;

  // Initialise the handle
  NBase::Result result = SFindPushDisplayDeviceHandle(&handle_);
  RETURN_IF_FAILED_MESSAGE(result, "Failed to initialize handle");
  assert(handle_ != NULL);

  // Initialise the transfer
  result = startSending();
  RETURN_IF_FAILED_MESSAGE(result, "Failed to initiate send");

  // We initiate a thread so we can recieve events from libusb
  terminate_ = false;
  pollThread_ = std::thread(&UsbCommunicator::PollUsbForEvents, this);

  return NBase::Result::NoError;
}


//------------------------------------------------------------------------------

UsbCommunicator::~UsbCommunicator()
{
  // shutdown the polling thread
  terminate_ = true;
  if (pollThread_.joinable())
  {
    pollThread_.join();
  }
}


//------------------------------------------------------------------------------

NBase::Result UsbCommunicator::startSending()
{
  using namespace NBase;

  currentLine_ = 0;

  // Allocates a transfer struct for the frame header

  static unsigned char frameHeader[16] =
  {
    0xFF, 0xCC, 0xAA, 0x88,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
  };

  frameHeaderTransfer_ =
  SAllocateAndPrepareTransferChunk(handle_, this, frameHeader, sizeof(frameHeader));

  for (int i = 0; i < kSendBufferCount; i++)
  {
    unsigned char* buffer = (sendBuffers_ + i * kSendBufferSize);

    // Allocates a transfer struct for the send buffers

    libusb_transfer* transfer =
    SAllocateAndPrepareTransferChunk(handle_, this, buffer, kSendBufferSize);

    // Start a request for this buffer
    Result result = sendNextSlice(transfer);
    RETURN_IF_FAILED(result);
  }

  return Result::NoError;
}


//------------------------------------------------------------------------------

NBase::Result UsbCommunicator::sendNextSlice(libusb_transfer* transfer)
{
  using namespace NBase;

  // Start of a new frame, so send header first
  if (currentLine_ == 0)
  {
    if (libusb_submit_transfer(frameHeaderTransfer_) < 0)
    {
      return Result("could not submit frame header transfer");
    }
  }

  // Copy the next slice of the source data (represented by currentLine_)
  // to the transfer buffer

  unsigned char *dst = transfer->buffer;

  const char* src = (const char*)dataSource_ + kLineSize * currentLine_;
  unsigned char* end = dst + kSendBufferSize;

  while (dst < end)
  {
    *dst++ = *src++;
  }

  // Send it
  if (libusb_submit_transfer(transfer) < 0)
  {
    return Result("could not submit display data transfer,");
  }

  // Update slice position
  currentLine_ += kLineCountPerSendBuffer;

  if (currentLine_ >= 160)
  {
    currentLine_ = 0;
  }

  return Result::NoError;
}


//------------------------------------------------------------------------------

void UsbCommunicator::OnTransferFinished(libusb_transfer* transfer)
{
  if (transfer->status != LIBUSB_TRANSFER_COMPLETED)
  {
    assert(0);
    switch (transfer->status)
    {
      case LIBUSB_TRANSFER_ERROR:     printf("transfer failed\n"); break;
      case LIBUSB_TRANSFER_TIMED_OUT: printf("transfer timed out\n"); break;
      case LIBUSB_TRANSFER_CANCELLED: printf("transfer was cancelled\n"); break;
      case LIBUSB_TRANSFER_STALL:     printf("endpoint stalled/control request not supported\n"); break;
      case LIBUSB_TRANSFER_NO_DEVICE: printf("device was disconnected\n"); break;
      case LIBUSB_TRANSFER_OVERFLOW:  printf("device sent more data than requested\n"); break;
      default:
        printf("snd transfer failed with status: %d\n", transfer->status);
        break;
    }
  }
  else if (transfer->length != transfer->actual_length)
  {
    assert(0);
    printf("only transferred %d of %d bytes\n", transfer->actual_length, transfer->length);
  }
  else if (transfer == frameHeaderTransfer_)
  {
    onFrameCompleted();
  }
  else
  {
    NBase::Result result = sendNextSlice(transfer);
    assert(result.Succeeded());
  }
}


//------------------------------------------------------------------------------

void UsbCommunicator::onFrameCompleted()
{
  // Insert code here if you want anything to happen after each frame
}


//------------------------------------------------------------------------------

void UsbCommunicator::PollUsbForEvents()
{
  static struct timeval timeout_500ms = {0 , 500000};
  int terminate_main_loop = 0;

  while (!terminate_main_loop && !terminate_.load())
  {
    if (libusb_handle_events_timeout_completed(NULL, &timeout_500ms, &terminate_main_loop) < 0)
    {
      assert(0);
    }
  }
}
