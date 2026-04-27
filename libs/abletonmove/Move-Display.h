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
#include "Move-UsbCommunicator.h"
#include "abletonmove/JuceToMoveDisplayBridge.h"

namespace abletonmove
{
  //=====================================================================

  class MoveDisplay
  {
  public:
    static MoveDisplay *create();

    NBase::Result Init();

    // Transfers the bitmap into the output buffer sent to
    // the push display. The push display buffer has a larger stride
    // as the given bitmap

    unsigned char* GetRawBitmap()
    {
       return dataSource_;
    }

    int GetBitmapSize()
    {
       return kDataSourceWidth * kDataSourceHeight;
    }

    void SendBitmapToDevice()
    {
      communicator_.SendBitmapToDevice();
    }

  private:
    MoveDisplay() = default;

    static const int kDataSourceWidth = 128;
    static const int kDataSourceHeight = 8;

    unsigned char dataSource_[kDataSourceWidth * kDataSourceHeight]{};

    UsbCommunicator communicator_;
  };
}