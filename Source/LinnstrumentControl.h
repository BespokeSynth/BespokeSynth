/*
  ==============================================================================

    LinnstrumentControl.h
    Created: 28 Oct 2018 2:17:18pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include <iostream>
#include "MidiDevice.h"
#include "IDrawableModule.h"
#include "INoteReceiver.h"
#include "DropdownList.h"
#include "Transport.h"
#include "Scale.h"
#include "ModulationChain.h"
#include "Slider.h"

class IAudioSource;

class LinnstrumentControl : public IDrawableModule, public INoteReceiver, public IDropdownListener, public IScaleListener, public IFloatSliderListener, public MidiDeviceListener
{
public:
   LinnstrumentControl();
   virtual ~LinnstrumentControl();
   static IDrawableModule* Create() { return new LinnstrumentControl(); }
   
   string GetTitleLabel() override { return "linnstrument control"; }
   void CreateUIControls() override;
   
   void Init() override;
   void Poll() override;
   
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}
   
   void OnMidiNote(MidiNote& note) override;
   void OnMidiControl(MidiControl& control) override;
   
   void OnScaleChanged() override;
   
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void DropdownClicked(DropdownList* list) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override {}
   void CheckboxUpdated(Checkbox* checkbox) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   
private:
   enum LinnstrumentColor
   {
      kLinnColor_Off,
      kLinnColor_Red,
      kLinnColor_Yellow,
      kLinnColor_Green,
      kLinnColor_Cyan,
      kLinnColor_Blue,
      kLinnColor_Magenta,
      kLinnColor_Black,
      kLinnColor_White,
      kLinnColor_Orange,
      kLinnColor_Lime,
      kLinnColor_Pink
   };
   
   void InitController();
   void BuildControllerList();
   
   void UpdateScaleDisplay();
   void SendScaleInfo();
   void SetGridColor(int x, int y, LinnstrumentColor color);
   LinnstrumentColor GetGridColor(int x, int y);
   int GridToPitch(int x, int y);
   void SetPitchColor(int pitch, LinnstrumentColor color);
   void SendNRPN(int param, int value);
   
   //IDrawableModule
   void DrawModule() override;
   bool Enabled() const override { return true; }
   void GetModuleDimensions(float& w, float& h) override { w=190; h=7+17*4; }
   
   int mControllerIndex;
   DropdownList* mControllerList;
   
   struct NoteAge
   {
      NoteAge() { mTime = 0; mColor = 0; mVoiceIndex = -1; }
      double mTime;
      int mColor;
      int mVoiceIndex;
      int mOutputPitch;
      void Update(int pitch, LinnstrumentControl* linnstrument);
   };
   
   NoteAge mNoteAge[128];
   float mDecayMs;
   FloatSlider* mDecaySlider;
   bool mBlackout;
   Checkbox* mBlackoutCheckbox;
   bool mLightOctaves;
   Checkbox* mLightOctavesCheckbox;
   int mLinnstrumentOctave;
   bool mGuitarLines;
   Checkbox* mGuitarLinesCheckbox;
   bool mControlPlayedLights;
   
   int mLastReceivedNRPNParamMSB;
   int mLastReceivedNRPNParamLSB;
   int mLastReceivedNRPNValueMSB;
   int mLastReceivedNRPNValueLSB;
   
   ModulationParameters mModulators[kNumVoices];
   
   double mRequestedOctaveTime;
   
   MidiDevice mDevice;
};
