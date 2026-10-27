/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2026 Ryan Challinor (contact: awwbees@gmail.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/
/*
  ==============================================================================

    ControlSurfaceDevicePush2.cpp
    Created: 24 Oct 2026
    Author:  Ryan Challinor

    The push 2 profile lives in its own file because the push 2 and move
    display libraries can't be included in the same translation unit.

  ==============================================================================
*/

#include "ControlSurfaceDevice.h"
#include "SynthGlobals.h"
#include "MidiDevice.h"
#include "AbletonMoveLCD.h"
#include "AbletonDeviceShared.h"
#include "push2/JuceToPush2DisplayBridge.h"
#include "push2/Push2-Bitmap.h"

using namespace AbletonDevice;

namespace
{
   //created once and never torn down, the usb library doesn't release the device cleanly
   ableton::Push2DisplayBridge sPush2Bridge;
}

////////////////////////////////////////////////////////////////////////////////
// ableton push 2
// the move's controls share most of the push 2's midi numbers, so start from identity and remap the ones that differ
// https://raw.githubusercontent.com/Ableton/push-interface/master/doc/MidiMapping.png

namespace
{
   const int kPush2Width = ableton::Push2DisplayBitmap::kWidth;
   const int kPush2Height = ableton::Push2DisplayBitmap::kHeight;
}

AbletonPush2Surface::AbletonPush2Surface()
{
   mControlMap.MapAllIdentity();

   //the move's main encoders, volume encoder (push's master encoder), shift, play, record, page left/right, octave up/down, and
   //track buttons (push's scene launch buttons) all share the push's numbers. pads are notes 36-99 instead of the move's 68-99.

   //move's jog wheel -> push's tempo encoder and tap tempo button
   mControlMap.MapInput(kClickyEncoderTouch, 10); //tempo encoder touch
   mControlMap.UnmapLed(kClickyEncoderTouch);

   //move's 16 step buttons -> the rows of buttons above and below the screen
   for (int i = 0; i < 8; ++i)
   {
      mControlMap.Map(kStepButtonSection + i, kAboveScreenButtonRow + i);
      mControlMap.Map(kStepButtonSection + 8 + i, kBelowScreenButtonRow + i);
   }

   //the move's small leds above the step buttons, and its encoder leds, don't exist on the push
   for (int i = 0; i < kNumStepButtons; ++i)
   {
      mControlMap.UnmapLed(kStepButtonLedSection + i);
      if (mControlMap.GetLayoutIndex(kStepButtonLedSection + i) == kStepButtonLedSection + i)
         mControlMap.UnmapInput(kStepButtonLedSection + i);
   }
   for (int i = 0; i < kNumMainEncoders; ++i)
      mControlMap.UnmapLed(kMainEncoderSection + i);

   //buttons whose numbers mean something different on the move
   mControlMap.Map(kMoveDeleteButton, kDeleteButton);
   mControlMap.Map(kMoveUndoButton, kUndoButton);
   mControlMap.Map(kDotButton, kSessionButton); //snapshot view
   mControlMap.Map(kBackButton, kDeviceButton); //global modules
   mControlMap.Map(kHamburgerButton, kMixButton); //mixer
   mControlMap.Map(kCaptureButton, kBrowseButton); //navigate the canvas
   mControlMap.Map(kLoopButton, kAutomateButton); //record automation
   mControlMap.Map(kMoveMuteButton, kMuteButton); //lfo
}

int AbletonPush2Surface::GetGridStartIndex() const
{
   return kPadsSection;
}

std::vector<std::string> AbletonPush2Surface::GetInputPortNames() const
{
#if JUCE_WINDOWS
   return { "Ableton Push 2" };
#else
   return { "Ableton Push 2 Live Port" };
#endif
}

std::vector<std::string> AbletonPush2Surface::GetOutputPortNames() const
{
   return GetInputPortNames();
}

bool AbletonPush2Surface::InitDisplay(std::string& errorMessage)
{
   if (!sPush2Bridge.IsInitialized())
   {
      if (auto result = sPush2Bridge.Init(ableton::DeviceType::Push2); result.Failed())
      {
         errorMessage = result.GetDescription();
         return false;
      }
   }
   mPixels.assign(3 * kPush2Width * kPush2Height, 0);
   return true;
}

void AbletonPush2Surface::OnConnected(MidiDevice& device)
{
   //touch strip: host sends leds, strip sends pitch bend
   std::string touchStripConfig = { 0x00, 0x21, 0x1D, 0x01, 0x01, 0x17, 0x03 };
   device.SendSysEx(touchStripConfig);
}

bool AbletonPush2Surface::IsDisplayConnected() const
{
   return sPush2Bridge.IsInitialized();
}

void AbletonPush2Surface::SendDisplay(MidiDevice& device, const AbletonMoveLCD& lcd)
{
   if (!sPush2Bridge.IsInitialized() || mPixels.empty())
      return;

   //draw the move-sized display scaled up to the push's full height, centered horizontally
   const float kScale = float(kPush2Height) / AbletonMoveLCD::kMoveDisplayHeight;
   const int kDrawWidth = int(AbletonMoveLCD::kMoveDisplayWidth * kScale);
   const int kOffsetX = (kPush2Width - kDrawWidth) / 2;

   std::fill(mPixels.begin(), mPixels.end(), 0);
   for (int y = 0; y < kPush2Height; ++y)
   {
      int lcdY = int(y / kScale);
      unsigned char* row = &mPixels[3 * (kPush2Height - 1 - y) * kPush2Width]; //bridge expects rows bottom-up
      for (int x = 0; x < kDrawWidth; ++x)
      {
         if (lcd.GetPixel(int(x / kScale), lcdY) > 100)
         {
            unsigned char* pixel = row + 3 * (x + kOffsetX);
            pixel[0] = 255;
            pixel[1] = 255;
            pixel[2] = 255;
         }
      }
   }

   sPush2Bridge.Flip(mPixels.data());
}

void AbletonPush2Surface::ClearDisplay(MidiDevice& device)
{
   if (!sPush2Bridge.IsInitialized() || mPixels.empty())
      return;

   std::fill(mPixels.begin(), mPixels.end(), 0);
   sPush2Bridge.Flip(mPixels.data());
}
