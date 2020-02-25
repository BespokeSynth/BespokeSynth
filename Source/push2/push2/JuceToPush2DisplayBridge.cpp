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

#include "JuceToPush2DisplayBridge.h"
#include "Push2-Bitmap.h"
#include "Macros.h"
#include <assert.h>

using namespace ableton;

//------------------------------------------------------------------------------

Push2DisplayBridge::Push2DisplayBridge()
   : push2Display_(nullptr)
   {};


//------------------------------------------------------------------------------

NBase::Result Push2DisplayBridge::Init(ableton::Push2Display& display)
{
   push2Display_ = &display;
   return NBase::Result::NoError;
}


//------------------------------------------------------------------------------

void Push2DisplayBridge::Flip(unsigned char* pixels)
{
   // Make sure the class was properly initialised
   assert(push2Display_);

   // Create a push display bitmap and get access to the pixel data
   Push2DisplayBitmap bitmap;
   Push2DisplayBitmap::pixel_t* data = bitmap.PixelData();

   const int sizex = bitmap.GetWidth();
   const int sizey = bitmap.GetHeight();

   // Convert the pixels, applying the xor masking needed for the display to work
   static const uint16_t xOrMasks[2] = { 0xf3e7, 0xffe7 };

   for (int y = 0; y < sizey; y++)
   {
      for (int x = 0; x < sizex ; x++)
      {
         int index = 3 * ((sizey - 1 - y) * sizex + x);
         uint16_t pixel;
         pixel = Push2DisplayBitmap::SPixelFromRGB(pixels[index], pixels[index+1], pixels[index+2]);
         *data++ = pixel ^ xOrMasks[x % 2];
      }
      data += (bitmap.GetWidth() - sizex);
   }

   // Send the constructed push2 bitmap to the display
   NBase::Result result = push2Display_->Flip(bitmap);
   assert(result.Succeeded());
}


