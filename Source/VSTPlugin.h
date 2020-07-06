//
//  VSTPlugin.h
//  Bespoke
//
//  Created by Ryan Challinor on 1/18/16.
//
//

#ifndef __Bespoke__VSTPlugin__
#define __Bespoke__VSTPlugin__

#include "IAudioProcessor.h"
#include "PolyphonyMgr.h"
#include "INoteReceiver.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "DropdownList.h"
#include "Checkbox.h"
#include "EnvOscillator.h"
#include "Ramp.h"
#include "ClickButton.h"
#include "../JuceLibraryCode/JuceHeader.h"
#include "VSTPlayhead.h"
#include "VSTWindow.h"

class ofxJSONElement;
//class NSWindowOverlay;

namespace VSTLookup
{
   void GetAvailableVSTs(vector<string>& vsts, bool rescan);
   void FillVSTList(DropdownList* list);
   string GetVSTPath(string vstName);
}

class VSTPlugin : public IAudioProcessor, public INoteReceiver, public IDrawableModule, public IDropdownListener, public IFloatSliderListener, public IIntSliderListener, public IButtonListener
{
public:
   VSTPlugin();
   virtual ~VSTPlugin() override;
   static IDrawableModule* Create() { return new VSTPlugin(); }
   
   string GetTitleLabel() override;
   void CreateUIControls() override;
   
   void SetVol(float vol) { mVol = vol; }
   
   void Poll() override;
   void Exit() override;
   
   juce::AudioProcessor* GetAudioProcessor() { return mPlugin.get(); }
   
   void SetVST(string vstName);
   void OnVSTWindowClosed();
   
   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override;
   
   //INoteReceiver
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   void SendCC(int control, int value, int voiceIdx = -1) override;
   
   void DropdownUpdated(DropdownList* list, int oldVal) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal) override;
   void CheckboxUpdated(Checkbox* checkbox) override;
   void ButtonClicked(ClickButton* button) override;
   
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;
   
private:
   //IDrawableModule
   void PreDrawModule() override;
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override;
   bool Enabled() const override { return mEnabled; }
   
   string GetPluginName();
   void CreateParameterSliders();
   
   float mVol;
   FloatSlider* mVolSlider;
   int mProgramChange;
   DropdownList* mProgramChangeSelector;
   ClickButton* mOpenEditorButton;
   int mOverlayWidth;
   int mOverlayHeight;
   
   std::unique_ptr<AudioProcessor> mPlugin;
   juce::ScopedPointer<VSTWindow> mWindow;
   juce::MidiBuffer mMidiBuffer;
   juce::MidiBuffer mFutureMidiBuffer;
   juce::CriticalSection mMidiInputLock;
   int mNumInputs;
   int mNumOutputs;
   
   struct ParameterSlider
   {
      float mValue;
      FloatSlider* mSlider;
      AudioProcessorParameter* mParameter;
   };
   
   vector<ParameterSlider> mParameterSliders;
   
   int mChannel;
   bool mUseVoiceAsChannel;
   float mPitchBendRange;
   int mModwheelCC;
   
   struct ChannelModulations
   {
      ModulationParameters mModulation;
      float mLastPitchBend;
      float mLastModWheel;
      float mLastPressure;
   };
   
   vector<ChannelModulations> mChannelModulations;
   
   ofMutex mVSTMutex;
   VSTPlayhead mPlayhead;
   
   //NSWindowOverlay* mWindowOverlay;
   
   enum DisplayMode
   {
      kDisplayMode_Sliders,
      kDisplayMode_PluginOverlay
   };
   
   DisplayMode mDisplayMode;
};

#endif /* defined(__Bespoke__VSTPlugin__) */
