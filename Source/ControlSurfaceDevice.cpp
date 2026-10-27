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

    ControlSurfaceDevice.cpp
    Created: 24 Oct 2026
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "ControlSurfaceDevice.h"
#include "SynthGlobals.h"
#include "MidiDevice.h"
#include "AbletonMoveLCD.h"
#include "AbletonDeviceShared.h"
#include "abletonmove/JuceToMoveDisplayBridge.h"
#include "abletonmove/../../Move-Display.h"

#include "juce_audio_devices/juce_audio_devices.h"

using namespace AbletonDevice;

namespace
{
   //display bridges are created once and never torn down, the usb libraries don't release the device cleanly
   abletonmove::MoveDisplayBridge sMoveBridge;

   bool ContainsAll(const std::string& str, const std::vector<std::string>& tokens)
   {
      for (const auto& token : tokens)
      {
         if (str.find(token) == std::string::npos)
            return false;
      }
      return true;
   }

   std::vector<std::string> GetAvailablePorts(bool forInput)
   {
      std::vector<std::string> ports;
      if (forInput)
      {
         for (auto& device : juce::MidiInput::getAvailableDevices())
            ports.push_back(device.name.toStdString());
      }
      else
      {
         for (auto& device : juce::MidiOutput::getAvailableDevices())
            ports.push_back(device.name.toStdString());
      }
      return ports;
   }

   //standard ableton led protocol: color index as velocity/value, channel 10 to blink between the flash color and the color
   void SendAbletonLed(MidiDevice& device, int rawIndex, const ControlSurfaceLedState& state)
   {
      int channel = 1;
      if (state.flashColor != -1)
         channel = 10;

      if (rawIndex < 128)
      {
         if (state.flashColor != -1)
            device.SendNote(gTime, rawIndex, state.flashColor, false, -1);
         device.SendNote(gTime, rawIndex, state.color, false, channel);
      }
      else
      {
         if (state.flashColor != -1)
            device.SendCC(rawIndex - 128, state.flashColor);
         device.SendCC(rawIndex - 128, state.color, channel);
      }
   }
}

ControlSurfaceControlMap::ControlSurfaceControlMap()
{
   mInputToLayout.fill(-1);
   mLayoutToLed.fill(-1);
}

void ControlSurfaceControlMap::MapAllIdentity()
{
   for (int i = 0; i < kNumIndices; ++i)
   {
      mInputToLayout[i] = i;
      mLayoutToLed[i] = i;
   }
}

void ControlSurfaceControlMap::Map(int layoutIndex, int rawInputIndex, int rawLedIndex)
{
   if (rawInputIndex != -1)
      MapInput(layoutIndex, rawInputIndex);

   if (rawLedIndex != -1)
      MapLed(layoutIndex, rawLedIndex);
   else
      UnmapLed(layoutIndex);
}

void ControlSurfaceControlMap::MapInput(int layoutIndex, int rawInputIndex)
{
   assert(layoutIndex >= 0 && layoutIndex < kNumIndices);
   assert(rawInputIndex >= 0 && rawInputIndex < kNumIndices);

   for (auto& mapped : mInputToLayout)
   {
      if (mapped == layoutIndex)
         mapped = -1;
   }
   mInputToLayout[rawInputIndex] = layoutIndex;
}

void ControlSurfaceControlMap::MapLed(int layoutIndex, int rawLedIndex)
{
   assert(layoutIndex >= 0 && layoutIndex < kNumIndices);
   assert(rawLedIndex >= 0 && rawLedIndex < kNumIndices);

   for (auto& mapped : mLayoutToLed)
   {
      if (mapped == rawLedIndex)
         mapped = -1;
   }
   mLayoutToLed[layoutIndex] = rawLedIndex;
}

void ControlSurfaceControlMap::UnmapInput(int rawInputIndex)
{
   assert(rawInputIndex >= 0 && rawInputIndex < kNumIndices);
   mInputToLayout[rawInputIndex] = -1;
}

void ControlSurfaceControlMap::UnmapLed(int layoutIndex)
{
   assert(layoutIndex >= 0 && layoutIndex < kNumIndices);
   mLayoutToLed[layoutIndex] = -1;
}

int ControlSurfaceControlMap::GetLayoutIndex(int rawInputIndex) const
{
   if (rawInputIndex >= 0 && rawInputIndex < kNumIndices)
      return mInputToLayout[rawInputIndex];
   return -1;
}

int ControlSurfaceControlMap::GetRawLedIndex(int layoutIndex) const
{
   if (layoutIndex >= 0 && layoutIndex < kNumIndices)
      return mLayoutToLed[layoutIndex];
   return -1;
}

std::unique_ptr<ControlSurfaceDevice> CreateControlSurfaceDevice(ControlSurfaceDeviceType type)
{
   switch (type)
   {
      case ControlSurfaceDeviceType::AbletonMove: return std::make_unique<AbletonMoveSurface>();
      case ControlSurfaceDeviceType::AbletonPush2: return std::make_unique<AbletonPush2Surface>();
      case ControlSurfaceDeviceType::NovationLaunchControl3: return std::make_unique<NovationLaunchControl3Surface>();
      case ControlSurfaceDeviceType::Count: break;
   }
   assert(false);
   return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
// ControlSurfaceDevice

std::string ControlSurfaceDevice::FindPort(const std::vector<std::string>& availablePorts, const std::vector<std::string>& desiredNames) const
{
   for (const auto& desiredName : desiredNames)
   {
      for (const auto& port : availablePorts)
      {
         if (port == desiredName)
            return port;
      }
   }

   const auto fallbackTokens = GetPortNameFallbackTokens();
   if (!fallbackTokens.empty())
   {
      for (const auto& port : availablePorts)
      {
         if (ContainsAll(port, fallbackTokens))
            return port;
      }
   }

   return "";
}

bool ControlSurfaceDevice::IsPresent() const
{
   return !FindPort(GetAvailablePorts(K(forInput)), GetInputPortNames()).empty();
}

bool ControlSurfaceDevice::Connect(MidiDevice& device, std::string& statusMessage)
{
   statusMessage = "";

   std::string inputPort = FindPort(GetAvailablePorts(K(forInput)), GetInputPortNames());
   std::string outputPort = FindPort(GetAvailablePorts(!K(forInput)), GetOutputPortNames());
   if (inputPort.empty() || outputPort.empty())
   {
      statusMessage = GetName() + " not found";
      return false;
   }

   device.ConnectOutput(outputPort.c_str());
   device.ConnectInput(inputPort.c_str());

   if (HasDisplay())
   {
      std::string displayError;
      if (!InitDisplay(displayError))
      {
         statusMessage = displayError;
         ofLog() << GetName() << ": " << displayError;
      }
   }

   OnConnected(device);

   ofLog() << GetName() << " connected";
   return true;
}

void ControlSurfaceDevice::Disconnect(MidiDevice& device)
{
   OnDisconnecting(device);

   if (HasDisplay() && IsDisplayConnected())
      ClearDisplay(device);

   device.DisconnectInput();
   device.DisconnectOutput();
}

int ControlSurfaceDevice::GetGridControllerOption1Control() const
{
   return kRepeatButton;
}

int ControlSurfaceDevice::GetGridControllerOption2Control() const
{
   return kAccentButton;
}

void ControlSurfaceDevice::SendLed(MidiDevice& device, int layoutIndex, const ControlSurfaceLedState& state)
{
   int rawIndex = mControlMap.GetRawLedIndex(layoutIndex);
   if (rawIndex == -1 || state.color < 0)
      return;

   SendAbletonLed(device, rawIndex, state);
}

////////////////////////////////////////////////////////////////////////////////
// ableton move
// the layout is the move's own, so everything maps straight through

AbletonMoveSurface::AbletonMoveSurface()
{
   mControlMap.MapAllIdentity();
}

int AbletonMoveSurface::GetGridStartIndex() const
{
   return kMovePadsSection;
}

std::vector<std::string> AbletonMoveSurface::GetInputPortNames() const
{
#if JUCE_WINDOWS
   return { "Ableton Move MIDI" };
#else
   return { "Ableton Move Live Port" };
#endif
}

std::vector<std::string> AbletonMoveSurface::GetOutputPortNames() const
{
   return GetInputPortNames();
}

bool AbletonMoveSurface::InitDisplay(std::string& errorMessage)
{
   if (!sMoveBridge.IsInitialized())
   {
      if (auto result = sMoveBridge.Init(); result.Failed())
      {
         errorMessage = result.GetDescription();
         return false;
      }
   }
   return true;
}

bool AbletonMoveSurface::IsDisplayConnected() const
{
   return sMoveBridge.IsInitialized();
}

void AbletonMoveSurface::SendDisplay(MidiDevice& device, const AbletonMoveLCD& lcd)
{
   if (!sMoveBridge.IsInitialized())
      return;

   //the move's display takes columns of 8 vertical pixels, one byte per column
   unsigned char* pixels = sMoveBridge.GetDisplay()->GetRawBitmap();
   memset(pixels, 0, sizeof(unsigned char) * sMoveBridge.GetDisplay()->GetBitmapSize());
   constexpr int kPixelBlockRows = 8;
   constexpr int kPixelBlockColumns = 128;
   constexpr int kPixelBlockCellHeight = 8;
   for (int row = 0; row < kPixelBlockRows; ++row)
   {
      for (int col = 0; col < kPixelBlockColumns; ++col)
      {
         int cellIndex = col + row * kPixelBlockColumns;
         pixels[cellIndex] = 0;
         for (int i = 0; i < kPixelBlockCellHeight; ++i)
         {
            if (lcd.GetPixel(col, row * kPixelBlockCellHeight + i) > 100)
               pixels[cellIndex] |= 1 << i;
         }
      }
   }

   sMoveBridge.GetDisplay()->SendBitmapToDevice();
}

void AbletonMoveSurface::ClearDisplay(MidiDevice& device)
{
   if (!sMoveBridge.IsInitialized())
      return;

   memset(sMoveBridge.GetDisplay()->GetRawBitmap(), 0, sizeof(unsigned char) * sMoveBridge.GetDisplay()->GetBitmapSize());
   sMoveBridge.GetDisplay()->SendBitmapToDevice();
}

////////////////////////////////////////////////////////////////////////////////
// novation launch control 3
// only the controls below are known so far: 16 relative encoders (the last acts as the move's volume encoder), 8 track buttons,
// shift (acts as the move's snapshot button) and function (mixer). leds and the 128x64 display are driven over sysex.

namespace
{
   const int kLCNumEncoders = 16;
   const int kLCEncoderSection = 77 + 128;
   const int kLCEncoderLedSection = 13 + 128;
   const int kLCNumTrackButtons = 8;
   const int kLCTrackButtonSection = 37 + 128;
   const int kLCShiftButton = 63 + 128;
   const int kLCFunctionButton = 66 + 128;

   const unsigned char kLCSysexCommandDawMode = 0x02;
   const unsigned char kLCSysexCommandLedColor = 0x01;
   const unsigned char kLCSysexCommandDisplayConfig = 0x04;
   const unsigned char kLCSysexCommandBitmap = 0x09;

   const double kLCBitmapAckTimeoutMs = 500;
}

NovationLaunchControl3Surface::NovationLaunchControl3Surface()
{
   mSysexHeader = { 0x00, 0x20, 0x29, 0x02, 0x16 };

   for (int i = 0; i < kLCNumEncoders - 1; ++i)
      mControlMap.Map(GetMainEncoderControl(i), kLCEncoderSection + i, kLCEncoderLedSection + i);
   mControlMap.MapInput(kVolumeEncoderTurn, kLCEncoderSection + kLCNumEncoders - 1);

   for (int i = 0; i < kLCNumTrackButtons; ++i)
      mControlMap.Map(GetTrackButtonControl(i), kLCTrackButtonSection + i);

   mControlMap.Map(kDotButton, kLCShiftButton);
   mControlMap.Map(kHamburgerButton, kLCFunctionButton);
}

int NovationLaunchControl3Surface::GetGridStartIndex() const
{
   return kMovePadsSection; //no grid
}

std::vector<std::string> NovationLaunchControl3Surface::GetInputPortNames() const
{
   return { "LC3 1 DAW Out" };
}

std::vector<std::string> NovationLaunchControl3Surface::GetOutputPortNames() const
{
   return { "LC3 1 DAW In" };
}

void NovationLaunchControl3Surface::OnConnected(MidiDevice& device)
{
   std::string enterDawModeMsg = mSysexHeader;
   enterDawModeMsg.push_back(kLCSysexCommandDawMode);
   enterDawModeMsg.push_back(0x7F);
   device.SendSysEx(enterDawModeMsg);

   //don't let the device's display show its own popups when encoders move
   for (int i = 0; i < kLCNumEncoders; ++i)
   {
      std::string disableMsg = mSysexHeader;
      disableMsg.push_back(kLCSysexCommandDisplayConfig);
      disableMsg.push_back(0x0D + i);
      disableMsg.push_back(0x01);
      device.SendSysEx(disableMsg);
   }

   //relative mode for each row of encoders
   device.SendData(0xB6, 0x45, 0x7F);
   device.SendData(0xB6, 0x48, 0x7F);

   mCanSendBitmap = true;
}

void NovationLaunchControl3Surface::OnDisconnecting(MidiDevice& device)
{
   ClearDisplay(device);

   std::string exitDawModeMsg = mSysexHeader;
   exitDawModeMsg.push_back(kLCSysexCommandDawMode);
   exitDawModeMsg.push_back(0x00);
   device.SendSysEx(exitDawModeMsg);
}

int NovationLaunchControl3Surface::ConvertInputValue(int layoutIndex, int value) const
{
   //encoders are relative around 64, convert them to the move's two's complement
   if (GetMainEncoderIndex(layoutIndex) != -1 || layoutIndex == kVolumeEncoderTurn)
   {
      int delta = value - 64;
      return delta >= 0 ? delta : delta + 128;
   }
   return value;
}

void NovationLaunchControl3Surface::OnMidi(const juce::MidiMessage& message)
{
   if (message.isSysEx())
   {
      //the device acknowledges each bitmap before it can accept another
      const juce::uint8* data = message.getSysExData();
      int size = message.getSysExDataSize();
      if (size > (int)mSysexHeader.size() &&
          memcmp(data, mSysexHeader.data(), mSysexHeader.size()) == 0 &&
          data[mSysexHeader.size()] == kLCSysexCommandBitmap)
         mCanSendBitmap = true;
   }
}

void NovationLaunchControl3Surface::SendLed(MidiDevice& device, int layoutIndex, const ControlSurfaceLedState& state)
{
   int rawIndex = mControlMap.GetRawLedIndex(layoutIndex);
   if (rawIndex < 128 || state.color < 0)
      return;

   ofColor color = kColors[std::clamp(state.color, 0, (int)kColors.size() - 1)].color;
   float brightness = std::clamp(state.brightness, 0.0f, 1.0f);
   int r = int(powf(color.r / 255.0f, 2) * brightness * 127);
   int g = int(powf(color.g / 255.0f, 2) * brightness * 127);
   int b = int(powf(color.b / 255.0f, 2) * brightness * 127);

   //don't turn off completely, if we still intend to have some brightness
   //2 seems to be the dimmest displayable color (1 is still off)
   if (r < 2 && brightness > 0 && color.r > 0)
      r = 2;
   if (g < 2 && brightness > 0 && color.g > 0)
      g = 2;
   if (b < 2 && brightness > 0 && color.b > 0)
      b = 2;

   std::string colorMessage = mSysexHeader;
   colorMessage.push_back(kLCSysexCommandLedColor);
   colorMessage.push_back(0x53);
   colorMessage.push_back(rawIndex - 128);
   colorMessage.push_back(r);
   colorMessage.push_back(g);
   colorMessage.push_back(b);
   device.SendSysEx(colorMessage);
}

void NovationLaunchControl3Surface::SendDisplay(MidiDevice& device, const AbletonMoveLCD& lcd)
{
   //wait until the device says it can receive another bitmap, or give up waiting in case we missed the acknowledgement
   if (!mCanSendBitmap && gTime - mLastBitmapSendTime < kLCBitmapAckTimeoutMs)
      return;

   SendBitmap(device, &lcd);
}

void NovationLaunchControl3Surface::ClearDisplay(MidiDevice& device)
{
   SendBitmap(device, nullptr);
}

void NovationLaunchControl3Surface::SendBitmap(MidiDevice& device, const AbletonMoveLCD* lcd)
{
   //rows of 128 pixels, packed 7 pixels per byte
   std::string bitmapMsg = mSysexHeader;
   bitmapMsg.push_back(kLCSysexCommandBitmap);
   bitmapMsg.push_back(0x35);
   for (int row = 0; row < AbletonMoveLCD::kMoveDisplayHeight; ++row)
   {
      for (int byteIndex = 0; byteIndex < 19; ++byteIndex)
      {
         int byteVal = 0;
         for (int bit = 0; bit < 7; ++bit)
         {
            int col = byteIndex * 7 + bit;
            if (lcd != nullptr && col < AbletonMoveLCD::kMoveDisplayWidth && lcd->GetPixel(col, row))
               byteVal |= (1 << (6 - bit));
         }
         bitmapMsg.push_back(byteVal);
      }
   }
   device.SendSysEx(bitmapMsg);

   mCanSendBitmap = false;
   mLastBitmapSendTime = gTime;
}
