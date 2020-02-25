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

#pragma once

#include "Result.h"
#include "Push2-Bitmap.h"

#include <thread>
#include <assert.h>

#include <atomic>

// Forward declarations. This avoid having to include libusb.h from here
// which leads to declaration conflicts with juce

class libusb_transfer;
class libusb_device_handle;

namespace ableton
{
  /*!
   *  This class manages the communication with the Push 2 display over usb.
   *
   */

  class UsbCommunicator
  {
  public:
    using pixel_t = Push2DisplayBitmap::pixel_t;

    // The display frame size is 960*160*2=300k, but we use 64 extra filler
    // pixels per line so that we get exactly 2048 bytes per line. The purpose
    // is that the device receives exactly 4 buffers of 512 bytes each per line,
    // so that the line boundary (which is where we save to SDRAM) does not fall
    // into the middle of a received buffer. Therefore we actually send
    // 1024*160*2=320k bytes per frame.

    static const int kLineSize        = 2048; // total line size
    static const int kLineCountPerSendBuffer   = 8;

    // The data sent to the display is sliced into chunks of kLineCountPerSendBuffer
    // and we use kSendBufferCount buffers to communicate so we can prepare the next
    // request without having to wait for the current one to be finished
    // The sent buffer size (kSendBufferSize) must a multiple of these 2k per line,
    // and is selected for optimal performance.

    static const int kSendBufferCount = 3;
    static const int kSendBufferSize  = kLineCountPerSendBuffer * kLineSize; // buffer length in bytes

    UsbCommunicator();
    ~UsbCommunicator();

    /*!
     *  Inialises the communicator. This will look for the usb descriptor matching
     *  the display, allocate transfer buffers and start sending data.
     *
     *  \param dataSource: The buffer holding the data to be sent to the display.
     *  \return the result of the initialisation
     */

    NBase::Result Init(const pixel_t* dataSource);

    /*!
     *  Callback for when a transfer is finished and the next one needs to be
     *  initiated
     */

    void OnTransferFinished(libusb_transfer* transfer);

    /*!
     *  Continuously poll events from libusb, possibly treating any error reported
     */

    void PollUsbForEvents();

  private:

    /*!
     *  Initiate the send process
     */

    NBase::Result startSending();

    /*!
     *  Send the next slice of data using the provided transfer struct
     */

    NBase::Result sendNextSlice(libusb_transfer* transfer);

    /*!
     *  Callback for when a full frame has been sent
     *  Note that there's no real need of doing double buffering since the
     *  display deals nicely with it already
     */

    void onFrameCompleted();

    const pixel_t* dataSource_;
    libusb_device_handle* handle_;
    libusb_transfer* frameHeaderTransfer_;
    std::thread pollThread_;
    uint8_t currentLine_;
    std::atomic<bool> terminate_;
    unsigned char sendBuffers_[kSendBufferCount * kSendBufferSize];

  };
}
