/*
  ==============================================================================

    EnvelopeModulator.h
    Created: 16 Nov 2017 10:28:34pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "IDrawableModule.h"
#include "RadioButton.h"
#include "Slider.h"
#include "ClickButton.h"
#include "DropdownList.h"
#include "ADSR.h"
#include "EnvelopeEditor.h"
#include "NoteEffectBase.h"
#include "IModulator.h"

class EnvelopeModulator : public IDrawableModule, public IRadioButtonListener, public IFloatSliderListener, public IButtonListener, public IDropdownListener, public IIntSliderListener, public NoteEffectBase, public IModulator
{
public:
   EnvelopeModulator();
   static IDrawableModule* Create() { return new EnvelopeModulator(); }
   void Delete() { delete this; }
   void DrawModule() override;
   
   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   bool Enabled() const override { return mEnabled; }
   string GetTitleLabel() override { return "envelope"; }
   void CreateUIControls() override;
   void MouseReleased() override;
   bool MouseMoved(float x, float y) override;
   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override;
   
   void PlayNote(double time, int pitch, int velocity, int voiceIdx = -1, ModulationParameters modulation = ModulationParameters()) override;
   
   //IModulator
   float Value(int samplesIn = 0) override;
   bool Active() const override { return mEnabled; }
   
   //IPatchable
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;
   
   void CheckboxUpdated(Checkbox* checkbox) override;
   void RadioButtonUpdated(RadioButton* radio, int oldVal) override {}
   void IntSliderUpdated(IntSlider* slider, int oldVal) override {}
   void FloatSliderUpdated(FloatSlider* slider, float oldVal) override;
   void ButtonClicked(ClickButton* button) override;
   void DropdownUpdated(DropdownList* list, int oldVal) override {}
   
   void GetModuleDimensions(float& width, float& height) override { width = mWidth; height = mHeight; }
   
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in) override;
   
protected:
   ~EnvelopeModulator();
   
private:
   void OnClicked(int x, int y, bool right) override;
   
   float mWidth;
   float mHeight;
   
   EnvelopeControl mEnvelopeControl;
   ::ADSR mAdsr;
   
   float mADSRViewLength;
   FloatSlider* mADSRViewLengthSlider;
   Checkbox* mHasSustainStageCheckbox;
   IntSlider* mSustainStageSlider;
   FloatSlider* mMaxSustainSlider;
};
