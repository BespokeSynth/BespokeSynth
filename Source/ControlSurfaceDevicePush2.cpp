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
   UnmapExtraLayoutButtons(mControlMap);

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
   mControlMap.Map(kMoveMuteButton, kMuteButton); //mute tracks (the move uses it for lfos, see kLFOButton)
   mControlMap.Map(kCopyButton, kDuplicateButton); //the move's copy shares the push's mute number, so it needs a new home

   //functions that are button combos on the move get their own buttons
   mControlMap.Map(kAddTrackSelectorButton, kAddTrackButton);
   mControlMap.Map(kRandomizeButton, kMasterButton);
   mControlMap.Map(kStoreSnapshotButton, kNewButton);
   mControlMap.Map(kSoundSelectorButton, kSetupButton);
   mControlMap.Map(kScalePageButton, kScaleButton);
   mControlMap.Map(kAutoZoomButton, kLayoutButton);
   mControlMap.Map(kLFOButton, kFixedLengthButton);
   mControlMap.Map(kCancelRecordingButton, kStopClipButton);
   mControlMap.Map(kBindButton, kUserButton);
   mControlMap.Map(kGridOption1Button, kRepeatButton); //the push's undo already has the move's undo number, which grid interfaces would otherwise use as option 1

   //the arrows: left and right step through pages of controls, up and down step through modules like the jog wheel
   mControlMap.Map(kPrevControlPageButton, kLeftButton);
   mControlMap.Map(kNextControlPageButton, kRightButton);
   mControlMap.Map(kPrevModuleButton, kUpButton);
   mControlMap.Map(kNextModuleButton, kDownButton);
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

//layout: a small knob screen under each of the 8 knobs along the top, and the module list, mixer and main screen along the bottom
void AbletonPush2Surface::SendDisplayPanels(MidiDevice& device, const std::vector<ControlSurfaceDisplayPanel>& panels)
{
   if (!sPush2Bridge.IsInitialized() || mPixels.empty())
      return;

   std::fill(mPixels.begin(), mPixels.end(), 0);

   const int kKnobPanelSpacing = kPush2Width / 8;
   const int kBottomPanelGap = 10;

   //panels are drawn at their lcd's own size
   int knobPanelIndex = 0;
   int otherPanelX = kBottomPanelGap;
   for (const auto& panel : panels)
   {
      if (panel.lcd == nullptr)
         continue;

      if (panel.role == ControlSurfaceDisplayPanel::Role::Knob)
      {
         DrawPanel(*panel.lcd, knobPanelIndex * kKnobPanelSpacing, 0, panel.lcd->GetWidth(), panel.lcd->GetHeight(), panel.color);
         ++knobPanelIndex;
      }
      else
      {
         DrawPanel(*panel.lcd, otherPanelX, kPush2Height - 10 - panel.lcd->GetHeight(), panel.lcd->GetWidth(), panel.lcd->GetHeight(), panel.color);
         otherPanelX += panel.lcd->GetWidth() + kBottomPanelGap;
      }
   }

   sPush2Bridge.Flip(mPixels.data());
}

//draws an lcd scaled into a rectangle of the display, averaging 3x3 samples per pixel so scaling down or up stays smooth
void AbletonPush2Surface::DrawPanel(const AbletonMoveLCD& lcd, int x, int y, int width, int height, const ofColor& color)
{
   const int kLcdWidth = lcd.GetWidth();
   const int kLcdHeight = lcd.GetHeight();
   std::vector<bool> lit(kLcdWidth * kLcdHeight);
   for (int ly = 0; ly < kLcdHeight; ++ly)
   {
      for (int lx = 0; lx < kLcdWidth; ++lx)
         lit[lx + ly * kLcdWidth] = lcd.GetPixel(lx, ly) > 100;
   }

   const int kSamples = 3;
   for (int dy = 0; dy < height; ++dy)
   {
      int screenY = y + dy;
      if (screenY < 0 || screenY >= kPush2Height)
         continue;
      unsigned char* row = &mPixels[3 * (kPush2Height - 1 - screenY) * kPush2Width]; //bridge expects rows bottom-up
      for (int dx = 0; dx < width; ++dx)
      {
         int screenX = x + dx;
         if (screenX < 0 || screenX >= kPush2Width)
            continue;
         int count = 0;
         for (int sy = 0; sy < kSamples; ++sy)
         {
            int ly = std::min(int((dy + (sy + 0.5f) / kSamples) * kLcdHeight / height), kLcdHeight - 1);
            for (int sx = 0; sx < kSamples; ++sx)
            {
               int lx = std::min(int((dx + (sx + 0.5f) / kSamples) * kLcdWidth / width), kLcdWidth - 1);
               if (lit[lx + ly * kLcdWidth])
                  ++count;
            }
         }
         if (count > 0)
         {
            unsigned char* pixel = row + 3 * screenX;
            pixel[0] = (unsigned char)(color.r * count / (kSamples * kSamples));
            pixel[1] = (unsigned char)(color.g * count / (kSamples * kSamples));
            pixel[2] = (unsigned char)(color.b * count / (kSamples * kSamples));
         }
      }
   }
}

void AbletonPush2Surface::ClearDisplay(MidiDevice& device)
{
   if (!sPush2Bridge.IsInitialized() || mPixels.empty())
      return;

   std::fill(mPixels.begin(), mPixels.end(), 0);
   sPush2Bridge.Flip(mPixels.data());
}
