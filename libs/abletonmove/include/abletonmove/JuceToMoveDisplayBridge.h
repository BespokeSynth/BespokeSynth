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

#include <memory>

namespace abletonmove
{
  class MoveDisplay;

  /*!
   *  Implements a bridge between juce::Graphics and move OLED display format.
   */

  class MoveDisplayBridge
  {
  public:

    MoveDisplayBridge();
    ~MoveDisplayBridge();

    /*!
     * Initialises the bridge
     *
     *  \return the result of the initialisation process
     */

    NBase::Result Init();

    /*!
     *  \return true if this bridge is initialized
     */

    bool IsInitialized() const { return bool{MoveDisplay_}; }

    /*!
     * Tells the bridge the drawing is done and the bitmap can be sent to
     * the push display
     */

    void Flip(unsigned char* pixels);

    MoveDisplay* GetDisplay()
    {
       return MoveDisplay_.get();
    }

  private:
    std::unique_ptr<MoveDisplay> MoveDisplay_;    /*< The push display the bridge works on */
  };
}
