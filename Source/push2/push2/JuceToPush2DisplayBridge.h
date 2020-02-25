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

#include "Push2-Display.h"
#include "../JuceLibraryCode/JuceHeader.h"

namespace ableton
{
  /*!
   *  Implements a bridge between juce::Graphics and push2 display format.
   */

  class Push2DisplayBridge
  {
  public:

    Push2DisplayBridge();

    /*!
     * Initialises the bridge
     *
     *  \return the result of the initialisation process
     */

    NBase::Result Init(ableton::Push2Display& display);

    /*!
     * Tells the bridge the drawing is done and the bitmap can be sent to
     * the push display
     */

    void Flip(unsigned char* pixels);

  private:
    ableton::Push2Display* push2Display_;    /*< The push display the bridge works on */
  };
}
