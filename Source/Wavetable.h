/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2021 Ryan Challinor (contact: awwbees@gmail.com)

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
//
//  Wavetable.h
//  Bespoke
//
//  A two-oscillator wavetable instrument modeled loosely on how Ableton's
//  Wavetable device and Serum/Vital work. Oscillator A and Oscillator B are
//  now full, symmetric voices - each has its own table, position (scan)
//  knob, volume, detune, unison count, and unison width, laid out in its own
//  column so both are clearly visible at once (mirroring the existing plain
//  Oscillator module's layout). B can hard-sync to A and cross-modulate it
//  (FM/PD/AM/RM), and is also heard directly via its own volume.
//
//  Both oscillators read from either a built-in table (basic/harmonics/pwm/
//  formant) or a table built from a sample you drag onto the module (sliced
//  into scan frames, Ableton-style - no manual drawing). A shared Warp stage
//  (Bend/Asym/Flip/Mirror/Quantize/Odd/Even) matches Serum's wavetable warp
//  modes. The filter and amp envelope are shared between A and B rather than
//  duplicated per-oscillator, to keep this reasonably simple.
//

#pragma once

#include "IAudioSource.h"
#include "PolyphonyMgr.h"
#include "WavetableVoice.h"
#include "WavetableTables.h"
#include "INoteReceiver.h"
#include "IDrawableModule.h"
#include "Slider.h"
#include "DropdownList.h"
#include "ADSRDisplay.h"
#include "Checkbox.h"
#include "RadioButton.h"
#include "ClickButton.h"
#include "IControlVisualizer.h"
#include <memory>

class ofxJSONElement;
class Sample;

class Wavetable : public IAudioSource, public INoteReceiver, public IDrawableModule, public IDropdownListener, public IFloatSliderListener, public IIntSliderListener, public IRadioButtonListener, public IControlVisualizer, public IButtonListener
{
public:
   Wavetable();
   ~Wavetable();
   static IDrawableModule* Create() { return new Wavetable(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return true; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   void SetDetune(float detune) { mVoiceParams.mDetune = detune; }

   //IAudioSource
   void Process(double time) override;
   void SetEnabled(bool enabled) override;

   //INoteReceiver
   void PlayNote(NoteMessage note) override;
   void SendCC(int control, int value, int voiceIdx = -1) override {}

   //IControlVisualizer
   void DrawVisualizationToScreen(AbletonMoveLCD* screen, IUIControl* control) override;

   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;
   void FloatSliderUpdated(FloatSlider* slider, float oldVal, double time) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override;
   void CheckboxUpdated(Checkbox* checkbox, double time) override;
   void RadioButtonUpdated(RadioButton* list, int oldVal, double time) override;
   void ButtonClicked(ClickButton* button, double time) override;

   //drag a sample onto oscillator A's column to load it there, onto oscillator
   //B's column to load it into B
   void SampleDropped(int x, int y, Sample* sample) override;

   bool HasDebugDraw() const override { return true; }

   virtual void LoadLayout(const ofxJSONElement& moduleInfo) override;
   virtual void SetUpFromSaveData() override;

   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   int GetModuleSaveStateRev() const override { return 1; }

   bool IsEnabled() const override { return mEnabled; }

private:
   //IDrawableModule
   void DrawModule() override;
   void DrawModuleUnclipped() override;
   float GetDrawValue(float phase);
   //draws a single-cycle preview of an oscillator's current wavetable frame (at its scan
   //position, with the shared warp applied) inside the given rect - the Serum/Vital/Ableton
   //style "you can actually see the oscillator" display requested in the flow
   void DrawOscWaveform(ofRectangle rect, const std::shared_ptr<WavetableFrameSet>& table, float position, ofColor color, bool active);
   void SetTableChoice(bool isB, int choice);
   void SaveImportedTable(FileStreamOut& out, const std::shared_ptr<WavetableFrameSet>& table);
   std::shared_ptr<WavetableFrameSet> LoadImportedTable(FileStreamIn& in);

   PolyphonyMgr mPolyMgr;
   NoteInputBuffer mNoteInputBuffer;
   WavetableVoiceParams mVoiceParams;

   //column 1: oscillator A
   static const int kSampleTableChoice = -1;
   int mTableChoiceA{ 0 };
   DropdownList* mTableADropdown{ nullptr };
   FloatSlider* mPositionASlider{ nullptr };
   FloatSlider* mVolSlider{ nullptr };
   FloatSlider* mDetuneSlider{ nullptr };
   IntSlider* mUnisonSlider{ nullptr };
   FloatSlider* mUnisonWidthSlider{ nullptr };
   Checkbox* mUseOscACheckbox{ nullptr }; //osc A on/off (mirrors osc B)
   std::shared_ptr<WavetableFrameSet> mImportedTableA;

   //column 2: oscillator B - mirrors column 1's controls one for one
   Checkbox* mUseOscBCheckbox{ nullptr };
   int mTableChoiceB{ 0 };
   DropdownList* mTableBDropdown{ nullptr };
   FloatSlider* mPositionBSlider{ nullptr };
   FloatSlider* mVolBSlider{ nullptr };
   FloatSlider* mDetuneBSlider{ nullptr };
   IntSlider* mUnisonBSlider{ nullptr };
   FloatSlider* mUnisonWidthBSlider{ nullptr };
   Checkbox* mSyncBCheckbox{ nullptr };
   std::shared_ptr<WavetableFrameSet> mImportedTableB;

   //column 3: shape (cross-modulation + warp, shared)
   int mModType{ 0 };
   DropdownList* mModTypeDropdown{ nullptr };
   FloatSlider* mModAmountSlider{ nullptr };
   int mWarpType{ 0 };
   DropdownList* mWarpTypeDropdown{ nullptr };
   FloatSlider* mWarpAmountSlider{ nullptr };

   //osc A's own filter + filter-envelope + amp-envelope (independent from B)
   FloatSlider* mFilterCutoffMaxSlider{ nullptr };
   FloatSlider* mFilterCutoffMinSlider{ nullptr };
   FloatSlider* mFilterQSlider{ nullptr };
   ADSRDisplay* mFilterADSRDisplay{ nullptr };
   ADSRDisplay* mADSRDisplay{ nullptr };

   //osc B's own filter + filter-envelope + amp-envelope (independent from A)
   FloatSlider* mFilterCutoffMaxBSlider{ nullptr };
   FloatSlider* mFilterCutoffMinBSlider{ nullptr };
   FloatSlider* mFilterQBSlider{ nullptr };
   ADSRDisplay* mFilterADSRDisplayB{ nullptr };
   ADSRDisplay* mADSRDisplayB{ nullptr };


   ChannelBuffer mWriteBuffer;

   struct DebugLine
   {
      std::string text;
      ofColor color;
   };

   std::array<DebugLine, 20> mDebugLines;
   int mDebugLinesPos{ 0 };
};
