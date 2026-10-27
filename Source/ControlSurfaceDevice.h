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

    ControlSurfaceDevice.h
    Created: 24 Oct 2026
    Author:  Ryan Challinor

    Hardware profiles for the controlsurface module.

    controlsurface (and the grid controllers it hosts) work in terms of the
    ableton move's control layout: the indices in AbletonDevice:: (0-127 for
    notes, 128-255 for CCs), extended by the "control surface layout" helpers
    in AbletonDeviceShared.h. each ControlSurfaceDevice translates between its
    hardware's raw midi and that layout, and says which parts of it exist.

    to support a new device: subclass ControlSurfaceDevice, fill in mControlMap
    in the constructor, override what differs, add a ControlSurfaceDeviceType,
    and add it to CreateControlSurfaceDevice().

  ==============================================================================
*/

#pragma once

#include <array>
#include <atomic>
#include <memory>
#include <string>
#include <vector>

class MidiDevice;
class AbletonMoveLCD;
namespace juce
{
   class MidiMessage;
}

enum class ControlSurfaceDeviceType
{
   AbletonMove,
   AbletonPush2,
   NovationLaunchControl3,
   Count
};

struct ControlSurfaceLedState
{
   ControlSurfaceLedState() {}
   ControlSurfaceLedState(int _color, int _flashColor, float _brightness)
   : color(_color)
   , flashColor(_flashColor)
   , brightness(_brightness)
   {}
   bool operator==(const ControlSurfaceLedState& other) const
   {
      return color == other.color && flashColor == other.flashColor && brightness == other.brightness;
   }
   bool operator!=(const ControlSurfaceLedState& other) const { return !(*this == other); }

   int color{ -1 }; //index into AbletonDevice::kColors, -1 for unset
   int flashColor{ -1 };
   float brightness{ 1.0f };
};

//maps a device's raw midi controls to and from the move layout
//all indices are 0-127 for notes, 128-255 for CCs
class ControlSurfaceControlMap
{
public:
   ControlSurfaceControlMap();

   void MapAllIdentity();
   //maps a raw control to a layout index. any other raw control already mapped to that layout index (or led) is unmapped, so overrides can be applied on top of MapAllIdentity().
   void Map(int layoutIndex, int rawInputIndex, int rawLedIndex);
   void Map(int layoutIndex, int rawIndex) { Map(layoutIndex, rawIndex, rawIndex); }
   void MapInput(int layoutIndex, int rawInputIndex);
   void MapLed(int layoutIndex, int rawLedIndex);
   void UnmapInput(int rawInputIndex);
   void UnmapLed(int layoutIndex);

   int GetLayoutIndex(int rawInputIndex) const;
   int GetRawLedIndex(int layoutIndex) const;

   static constexpr int kNumIndices = 256;

private:
   std::array<int, kNumIndices> mInputToLayout{};
   std::array<int, kNumIndices> mLayoutToLed{};
};

class ControlSurfaceDevice
{
public:
   virtual ~ControlSurfaceDevice() {}

   virtual ControlSurfaceDeviceType GetType() const = 0;
   virtual std::string GetName() const = 0;

   //connection
   bool IsPresent() const; //are this device's midi ports available?
   bool Connect(MidiDevice& device, std::string& statusMessage);
   void Disconnect(MidiDevice& device);

   //layout
   virtual int GetNumMainEncoders() const { return 8; }
   virtual int GetEncoderPageSize() const { return 8; } //how many controls one press of "next page" moves by
   virtual int GetNumTrackButtons() const = 0;
   virtual int GetNumStepButtons() const { return 0; }
   virtual int GetGridStartIndex() const = 0;
   virtual int GetGridNumCols() const { return 8; }
   virtual int GetGridNumRows() const { return 0; }
   int GetGridNumPads() const { return GetGridNumCols() * GetGridNumRows(); }
   bool HasGrid() const { return GetGridNumPads() > 0; }
   virtual bool HasEncoderTouch() const { return true; }
   virtual float GetEncoderSensitivity() const { return 1.0f; }
   virtual int GetGridControllerOption1Control() const;
   virtual int GetGridControllerOption2Control() const;

   //input
   int GetLayoutIndex(int rawIndex) const { return mControlMap.GetLayoutIndex(rawIndex); }
   virtual int ConvertInputValue(int layoutIndex, int value) const { return value; } //convert to the move's conventions, e.g. two's complement relative encoders
   virtual void OnMidi(const juce::MidiMessage& message) {} //called on the midi input thread

   //leds
   bool HasLed(int layoutIndex) const { return mControlMap.GetRawLedIndex(layoutIndex) != -1; }
   virtual bool SupportsLedFlashing() const { return true; } //if false, controlsurface will blink leds itself
   virtual bool SupportsLedBrightness() const { return false; }
   virtual void SendLed(MidiDevice& device, int layoutIndex, const ControlSurfaceLedState& state);

   //display
   virtual bool HasDisplay() const { return true; }
   virtual bool IsDisplayConnected() const { return false; }
   virtual void SendDisplay(MidiDevice& device, const AbletonMoveLCD& lcd) {} //called on the render thread
   virtual void ClearDisplay(MidiDevice& device) {}

protected:
   virtual std::vector<std::string> GetInputPortNames() const = 0;
   virtual std::vector<std::string> GetOutputPortNames() const = 0;
   virtual std::vector<std::string> GetPortNameFallbackTokens() const { return {}; } //if no exact port name is found, use a port whose name contains all of these
   virtual bool InitDisplay(std::string& errorMessage) { return true; }
   virtual void OnConnected(MidiDevice& device) {}
   virtual void OnDisconnecting(MidiDevice& device) {}

   ControlSurfaceControlMap mControlMap;

private:
   std::string FindPort(const std::vector<std::string>& availablePorts, const std::vector<std::string>& desiredNames) const;
};

std::unique_ptr<ControlSurfaceDevice> CreateControlSurfaceDevice(ControlSurfaceDeviceType type);

class AbletonMoveSurface : public ControlSurfaceDevice
{
public:
   AbletonMoveSurface();
   ControlSurfaceDeviceType GetType() const override { return ControlSurfaceDeviceType::AbletonMove; }
   std::string GetName() const override { return "ableton move"; }

   int GetNumTrackButtons() const override { return 4; }
   int GetNumStepButtons() const override { return 16; }
   int GetGridStartIndex() const override;
   int GetGridNumRows() const override { return 4; }

   bool IsDisplayConnected() const override;
   void SendDisplay(MidiDevice& device, const AbletonMoveLCD& lcd) override;
   void ClearDisplay(MidiDevice& device) override;

protected:
   std::vector<std::string> GetInputPortNames() const override;
   std::vector<std::string> GetOutputPortNames() const override;
   bool InitDisplay(std::string& errorMessage) override;
};

class AbletonPush2Surface : public ControlSurfaceDevice
{
public:
   AbletonPush2Surface();
   ControlSurfaceDeviceType GetType() const override { return ControlSurfaceDeviceType::AbletonPush2; }
   std::string GetName() const override { return "ableton push 2"; }

   int GetNumTrackButtons() const override { return 8; }
   int GetNumStepButtons() const override { return 16; }
   int GetGridStartIndex() const override;
   int GetGridNumRows() const override { return 8; }

   bool IsDisplayConnected() const override;
   void SendDisplay(MidiDevice& device, const AbletonMoveLCD& lcd) override;
   void ClearDisplay(MidiDevice& device) override;

protected:
   std::vector<std::string> GetInputPortNames() const override;
   std::vector<std::string> GetOutputPortNames() const override;
   bool InitDisplay(std::string& errorMessage) override;
   void OnConnected(MidiDevice& device) override;

private:
   std::vector<unsigned char> mPixels;
};

class NovationLaunchControl3Surface : public ControlSurfaceDevice
{
public:
   NovationLaunchControl3Surface();
   ControlSurfaceDeviceType GetType() const override { return ControlSurfaceDeviceType::NovationLaunchControl3; }
   std::string GetName() const override { return "novation launch control 3"; }

   int GetNumMainEncoders() const override { return 15; } //the 16th encoder acts as the move's volume encoder
   int GetNumTrackButtons() const override { return 8; }
   int GetGridStartIndex() const override;
   bool HasEncoderTouch() const override { return false; }
   float GetEncoderSensitivity() const override { return 1.6f; }

   int ConvertInputValue(int layoutIndex, int value) const override;
   void OnMidi(const juce::MidiMessage& message) override;

   bool SupportsLedFlashing() const override { return false; }
   bool SupportsLedBrightness() const override { return true; }
   void SendLed(MidiDevice& device, int layoutIndex, const ControlSurfaceLedState& state) override;

   bool IsDisplayConnected() const override { return true; } //the display is driven over midi
   void SendDisplay(MidiDevice& device, const AbletonMoveLCD& lcd) override;
   void ClearDisplay(MidiDevice& device) override;

protected:
   std::vector<std::string> GetInputPortNames() const override;
   std::vector<std::string> GetOutputPortNames() const override;
   std::vector<std::string> GetPortNameFallbackTokens() const override { return { "LC3", "DAW" }; }
   void OnConnected(MidiDevice& device) override;
   void OnDisconnecting(MidiDevice& device) override;

private:
   void SendBitmap(MidiDevice& device, const AbletonMoveLCD* lcd);

   std::string mSysexHeader;
   std::atomic<bool> mCanSendBitmap{ true };
   std::atomic<double> mLastBitmapSendTime{ 0 };
};
