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
#include "Push2-UsbCommunicator.h"
#include <memory>
#include <atomic>

namespace ableton
{
  //=====================================================================

  class Push2Display
  {
  public:
    using pixel_t = Push2DisplayBitmap::pixel_t;

    Push2Display()
    {
      pixel_t* pData = dataSource_;
      for (uint8_t line = 0; line < kDataSourceHeight; line++)
      {
        memset(pData, 0, kDataSourceWidth*sizeof(pixel_t));
        pData += kDataSourceWidth;
      }
    }

    NBase::Result Init()
    {
      return communicator_.Init(dataSource_);
    }

    // Transfers the bitmap into the output buffer sent to
    // the push display. The push display buffer has a larger stride
    // as the given bitmap

    NBase::Result Flip(const Push2DisplayBitmap& g)
    {
      const pixel_t* src = g.PixelData();
      pixel_t* dst = dataSource_;

      const int graphicsWidth = g.GetWidth();
      assert(g.GetHeight() == kDataSourceHeight);
      for (uint8_t line = 0; line < kDataSourceHeight; line++)
      {
        memcpy(dst, src, graphicsWidth * sizeof(pixel_t));
        src += graphicsWidth;
        dst += kDataSourceWidth;
      }

      return NBase::Result::NoError;
    }

  private:
    static const int kDataSourceWidth = 1024;
    static const int kDataSourceHeight = 160;

    pixel_t dataSource_[kDataSourceWidth * kDataSourceHeight];
    UsbCommunicator communicator_;
  };
}