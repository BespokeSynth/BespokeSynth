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

#include "abletonmove/Result.h"
#include "abletonmove/JuceToMoveDisplayBridge.h"

#include <thread>
#include <assert.h>

#include <atomic>

// Forward declarations. This avoid having to include libusb.h from here
// which leads to declaration conflicts with juce

class libusb_transfer;
class libusb_device_handle;

namespace abletonmove
{
  /*!
   *  This class manages the communication with the Push 2 display over usb.
   *
   */

  class UsbCommunicator
  {
  public:
    static const int kSendBufferSize = 128 * 8; // buffer length in bytes

    UsbCommunicator();
    ~UsbCommunicator();

    /*!
     *  Inialises the communicator. This will look for the usb descriptor matching
     *  the display, allocate transfer buffers and start sending data.
     *
     *  \param dataSource: The buffer holding the data to be sent to the display.
     *  \return the result of the initialisation
     */

    NBase::Result Init(unsigned char* dataSource);

    /*!
     *  Callback for when a transfer is finished and the next one needs to be
     *  initiated
     */

    void OnTransferFinished(libusb_transfer* transfer);

    /*!
     *  Continuously poll events from libusb, possibly treating any error reported
     */

    void PollUsbForEvents();

    void SendBitmapToDevice();

  private:

    /*!
     *  Initiate the send process
     */

    NBase::Result startSending();

    /*!
     *  Send the next slice of data using the provided transfer struct
     */

    NBase::Result sendData();

    unsigned char* dataSource_;
    libusb_device_handle* handle_;
    libusb_transfer* frameHeaderTransfer_;
    libusb_transfer* frameDataTransfer_{ nullptr };
    std::thread pollThread_;
    std::atomic<bool> terminate_;
    bool isWaitingForFrameToFinish_{ false };

  };
}
