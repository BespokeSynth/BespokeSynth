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
//  Wavetable.cpp
//  Bespoke
//

#include "Wavetable.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "IAudioReceiver.h"
#include "ofxJSONElement.h"
#include "ModularSynth.h"
#include "Profiler.h"
#include "UIControlMacros.h"
#include "Sample.h"
#include "ChannelBuffer.h"

Wavetable::Wavetable()
: mPolyMgr(this)
, mNoteInputBuffer(this)
, mWriteBuffer(gBufferSize)
{
   mVoiceParams.mAdsr.Set(10, 0, 1, 10);
   mVoiceParams.mVol = .25f;
   mVoiceParams.mSyncMode = Oscillator::SyncMode::None;
   mVoiceParams.mSyncFreq = 200;
   mVoiceParams.mSyncRatio = 1;
   mVoiceParams.mMult = 1;
   mVoiceParams.mDetune = 0;
   mVoiceParams.mFilterAdsr.Set(1, 0, 1, 1000);
   mVoiceParams.mFilterCutoffMax = WAVETABLE_NO_CUTOFF;
   mVoiceParams.mFilterCutoffMin = 10;
   mVoiceParams.mFilterQ = sqrt(2) / 2;
   mVoiceParams.mPhaseOffset = 0;
   mVoiceParams.mUnison = 1;
   mVoiceParams.mUnisonWidth = 0;
   mVoiceParams.mVelToVolume = 1.0f;
   mVoiceParams.mVelToEnvelope = 0;
   mVoiceParams.mLiteCPUMode = false;

   //default to the "basic" built-in table for both oscillators, so a
   //freshly-spawned module makes sound immediately
   const auto& builtIns = WavetableTables::GetBuiltInTables();
   mVoiceParams.mTableA = builtIns[0];
   mVoiceParams.mTableB = builtIns[0];
   mVoiceParams.mPositionA = 0;
   mVoiceParams.mPositionB = 0;
   mVoiceParams.mUseOscB = false;
   mVoiceParams.mVolB = .25f;
   mVoiceParams.mDetuneB = 0;
   mVoiceParams.mUnisonB = 1;
   mVoiceParams.mUnisonWidthB = 0;
   mVoiceParams.mSyncB = false;
   mVoiceParams.mModType = WavetableModType::None;
   mVoiceParams.mModAmount = 0;
   mVoiceParams.mWarpType = WavetableWarpType::None;
   mVoiceParams.mWarpAmount = .5f;

   //osc B's own amp env + filter (independent from A's)
   mVoiceParams.mAdsrB.Set(10, 0, 1, 10);
   mVoiceParams.mFilterAdsrB.Set(1, 0, 1, 1000);
   mVoiceParams.mFilterCutoffMaxB = WAVETABLE_NO_CUTOFF;
   mVoiceParams.mFilterCutoffMinB = 10;
   mVoiceParams.mFilterQB = sqrt(2) / 2;

   mPolyMgr.Init([](IDrawableModule* owner)
                 {
                    return std::unique_ptr<IMidiVoice>(new WavetableVoice(owner));
                 },
                 &mVoiceParams);
}

namespace
{
   //Serum/Vital/Ableton-style layout where each oscillator is a self-contained strip: its own
   //waveform preview + controls, immediately followed by its OWN filter + amp envelope. Shared
   //Shape (mod/warp) and pitch/velocity utilities sit to the right. Column widths are trimmed
   //to their contents so there's no wasted space.
   //
   //   [ osc A ] [ A filt/env ] [ osc B ] [ B filt/env ] [ shape ] [ shared ]
   const float kColWidthOsc = 128;
   const float kColWidthFiltEnv = 90; //holds a filter env display, fmax/fmin/q, and an amp env display
   const float kColWidthShape = 128; //widened so 'mod amount'/'warp amount' values don't overflow
   const float kColWidthVoice = 120; //synth-wide voice controls (voices / spread / randomize)
   const float kGap = 6;

   const float kOscAX = 3;
   const float kFiltAX = kOscAX + kColWidthOsc + kGap;
   const float kOscBX = kFiltAX + kColWidthFiltEnv + kGap;
   const float kFiltBX = kOscBX + kColWidthOsc + kGap;
   const float kShapeX = kFiltBX + kColWidthFiltEnv + kGap;
   const float kVoiceX = kShapeX + kColWidthShape + kGap;
   const float kContentTop = 15; //leaves room for the column header label drawn above each column

   //waveform preview geometry (shared by both oscillator columns)
   const float kWaveformHeight = 46;
   const float kWaveformGap = 4;
   const float kCheckRowHeight = 15; //both oscillators now have an on/off checkbox row above the waveform
   const float kOscAWaveTop = kContentTop + kCheckRowHeight;
   const float kOscBWaveTop = kContentTop + kCheckRowHeight;
   //controls (dropdown + sliders) begin just under the waveform preview
   const float kOscAControlsTop = kOscAWaveTop + kWaveformHeight + kWaveformGap;
   const float kOscBControlsTop = kOscBWaveTop + kWaveformHeight + kWaveformGap;

   //the amp-env display in each filt/env column sits below the filter section
   const float kAmpEnvGap = 15; //room for the "env" sub-label above the amp env display
}

void Wavetable::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   float width, height;

   //column 1: oscillator A - now has its own on/off checkbox row above the waveform (mirrors osc B),
   //then the controls start beneath the waveform preview
   UIBLOCK(kOscAX, kContentTop, kColWidthOsc);
   //unique save-name ("aon") so it doesn't collide with osc B's "on" checkbox on save/load (a name
   //collision was loading osc A as OFF -> silent). The visible "on" text is drawn manually below.
   CHECKBOX(mUseOscACheckbox, "aon", &mVoiceParams.mUseOscA);
   ENDUIBLOCK(width, height);
   mUseOscACheckbox->SetDisplayText(false);

   UIBLOCK(kOscAX, kOscAControlsTop, kColWidthOsc);
   DROPDOWN(mTableADropdown, "table a", &mTableChoiceA, kColWidthOsc - 3);
   UIBLOCK_NEWLINE();
   FLOATSLIDER(mPositionASlider, "position", &mVoiceParams.mPositionA, 0, 1);
   FLOATSLIDER(mVolSlider, "vol", &mVoiceParams.mVol, 0, 1);
   FLOATSLIDER_DIGITS(mDetuneSlider, "detune", &mVoiceParams.mDetune, -.05f, .05f, 3);
   INTSLIDER(mUnisonSlider, "unison", &mVoiceParams.mUnison, 1, WavetableVoice::kMaxUnison);
   FLOATSLIDER(mUnisonWidthSlider, "width", &mVoiceParams.mUnisonWidth, 0, 1);
   ENDUIBLOCK(width, height);
   mWidth = MAX(width, mWidth);
   mHeight = MAX(height, mHeight);

   //osc A's own filter (with its own filter envelope) + its own amp envelope, stacked in one
   //compact column right beside osc A
   UIBLOCK(kFiltAX, kContentTop, kColWidthFiltEnv);
   UICONTROL_CUSTOM(mFilterADSRDisplay, new ADSRDisplay(UICONTROL_BASICS("envfilter"), kColWidthFiltEnv, 30, &mVoiceParams.mFilterAdsr));
   FLOATSLIDER(mFilterCutoffMaxSlider, "fmax", &mVoiceParams.mFilterCutoffMax, 10, WAVETABLE_NO_CUTOFF);
   FLOATSLIDER(mFilterCutoffMinSlider, "fmin", &mVoiceParams.mFilterCutoffMin, 10, WAVETABLE_NO_CUTOFF);
   FLOATSLIDER_DIGITS(mFilterQSlider, "q", &mVoiceParams.mFilterQ, .1, 20, 3);
   UIBLOCK_SHIFTY(kAmpEnvGap);
   UICONTROL_CUSTOM(mADSRDisplay, new ADSRDisplay(UICONTROL_BASICS("env"), kColWidthFiltEnv, 30, &mVoiceParams.mAdsr));
   ENDUIBLOCK(width, height);
   mWidth = MAX(width, mWidth);
   mHeight = MAX(height, mHeight);

   //oscillator B - mirrors osc A one control for one. Its on/off and hard-sync-to-A checkboxes
   //sit in a compact row above the waveform preview. B is always fully visible (never hidden
   //when off) so the module's footprint never jumps around.
   UIBLOCK(kOscBX, kContentTop, kColWidthOsc);
   CHECKBOX(mUseOscBCheckbox, "on", &mVoiceParams.mUseOscB);
   UIBLOCK_SHIFTRIGHT();
   CHECKBOX(mSyncBCheckbox, "sync to a", &mVoiceParams.mSyncB);
   ENDUIBLOCK(width, height);
   mWidth = MAX(width, mWidth);

   UIBLOCK(kOscBX, kOscBControlsTop, kColWidthOsc);
   DROPDOWN(mTableBDropdown, "table b", &mTableChoiceB, kColWidthOsc - 3);
   UIBLOCK_NEWLINE();
   FLOATSLIDER(mPositionBSlider, "position b", &mVoiceParams.mPositionB, 0, 1);
   FLOATSLIDER(mVolBSlider, "vol b", &mVoiceParams.mVolB, 0, 1);
   FLOATSLIDER_DIGITS(mDetuneBSlider, "detune b", &mVoiceParams.mDetuneB, -.05f, .05f, 3);
   INTSLIDER(mUnisonBSlider, "unison b", &mVoiceParams.mUnisonB, 1, WavetableVoice::kMaxUnison);
   FLOATSLIDER(mUnisonWidthBSlider, "width b", &mVoiceParams.mUnisonWidthB, 0, 1);
   ENDUIBLOCK(width, height);
   mWidth = MAX(width, mWidth);
   mHeight = MAX(height, mHeight);

   //osc B's own filter + filter envelope + amp envelope, right beside osc B
   UIBLOCK(kFiltBX, kContentTop, kColWidthFiltEnv);
   UICONTROL_CUSTOM(mFilterADSRDisplayB, new ADSRDisplay(UICONTROL_BASICS("envfilterb"), kColWidthFiltEnv, 30, &mVoiceParams.mFilterAdsrB));
   FLOATSLIDER(mFilterCutoffMaxBSlider, "fmaxb", &mVoiceParams.mFilterCutoffMaxB, 10, WAVETABLE_NO_CUTOFF);
   FLOATSLIDER(mFilterCutoffMinBSlider, "fminb", &mVoiceParams.mFilterCutoffMinB, 10, WAVETABLE_NO_CUTOFF);
   FLOATSLIDER_DIGITS(mFilterQBSlider, "qb", &mVoiceParams.mFilterQB, .1, 20, 3);
   UIBLOCK_SHIFTY(kAmpEnvGap);
   UICONTROL_CUSTOM(mADSRDisplayB, new ADSRDisplay(UICONTROL_BASICS("envb"), kColWidthFiltEnv, 30, &mVoiceParams.mAdsrB));
   ENDUIBLOCK(width, height);
   mWidth = MAX(width, mWidth);
   mHeight = MAX(height, mHeight);

   //shape (cross-modulation + warp, shared between both oscillators)
   UIBLOCK(kShapeX, kContentTop, kColWidthShape);
   DROPDOWN(mModTypeDropdown, "mod", &mModType, kColWidthShape - 3);
   UIBLOCK_NEWLINE();
   FLOATSLIDER(mModAmountSlider, "mod amount", &mVoiceParams.mModAmount, 0, 1);
   DROPDOWN(mWarpTypeDropdown, "warp", &mWarpType, kColWidthShape - 3);
   UIBLOCK_NEWLINE();
   FLOATSLIDER(mWarpAmountSlider, "warp amount", &mVoiceParams.mWarpAmount, 0, 1);
   ENDUIBLOCK(width, height);
   mWidth = MAX(width, mWidth);
   mHeight = MAX(height, mHeight);

   mHeight += 4; //small bottom margin so the last row isn't flush against the module edge

   mFilterCutoffMaxSlider->SetMaxValueDisplay("none");
   mFilterCutoffMaxSlider->SetMode(FloatSlider::kSquare);
   mFilterCutoffMinSlider->SetMode(FloatSlider::kSquare);
   mFilterQSlider->SetMode(FloatSlider::kSquare);

   mFilterCutoffMaxBSlider->SetMaxValueDisplay("none");
   mFilterCutoffMaxBSlider->SetMode(FloatSlider::kSquare);
   mFilterCutoffMinBSlider->SetMode(FloatSlider::kSquare);
   mFilterQBSlider->SetMode(FloatSlider::kSquare);

   mFilterCutoffMaxSlider->SetControlVisualizer(this);
   mFilterCutoffMinSlider->SetControlVisualizer(this);
   mFilterCutoffMaxBSlider->SetControlVisualizer(this);
   mFilterCutoffMinBSlider->SetControlVisualizer(this);

   const auto& builtIns = WavetableTables::GetBuiltInTables();
   for (int i = 0; i < (int)builtIns.size(); ++i)
   {
      mTableADropdown->AddLabel(builtIns[i]->GetName().c_str(), i);
      mTableBDropdown->AddLabel(builtIns[i]->GetName().c_str(), i);
   }
   mTableADropdown->AddLabel("drag sample \xE2\x86\x92", kSampleTableChoice);
   mTableBDropdown->AddLabel("drag sample \xE2\x86\x92", kSampleTableChoice);

   mModTypeDropdown->AddLabel("none", (int)WavetableModType::None);
   mModTypeDropdown->AddLabel("fm", (int)WavetableModType::FM);
   mModTypeDropdown->AddLabel("pd", (int)WavetableModType::PD);
   mModTypeDropdown->AddLabel("am", (int)WavetableModType::AM);
   mModTypeDropdown->AddLabel("rm", (int)WavetableModType::RM);

   mWarpTypeDropdown->AddLabel("none", (int)WavetableWarpType::None);
   mWarpTypeDropdown->AddLabel("bend +", (int)WavetableWarpType::BendPlus);
   mWarpTypeDropdown->AddLabel("bend -", (int)WavetableWarpType::BendMinus);
   mWarpTypeDropdown->AddLabel("bend +/-", (int)WavetableWarpType::BendBoth);
   mWarpTypeDropdown->AddLabel("asym +", (int)WavetableWarpType::AsymPlus);
   mWarpTypeDropdown->AddLabel("asym -", (int)WavetableWarpType::AsymMinus);
   mWarpTypeDropdown->AddLabel("asym +/-", (int)WavetableWarpType::AsymBoth);
   mWarpTypeDropdown->AddLabel("flip", (int)WavetableWarpType::Flip);
   mWarpTypeDropdown->AddLabel("mirror", (int)WavetableWarpType::Mirror);
   mWarpTypeDropdown->AddLabel("quantize", (int)WavetableWarpType::Quantize);
   mWarpTypeDropdown->AddLabel("odd only", (int)WavetableWarpType::OddOnly);
   mWarpTypeDropdown->AddLabel("even only", (int)WavetableWarpType::EvenOnly);
}

Wavetable::~Wavetable()
{
}

void Wavetable::Process(double time)
{
   PROFILER(Wavetable);

   IAudioReceiver* target = GetTarget();

   if (!mEnabled || target == nullptr)
      return;

   mNoteInputBuffer.Process(time);

   ComputeSliders(0);

   int bufferSize = target->GetBuffer()->BufferSize();
   assert(bufferSize == gBufferSize);

   mWriteBuffer.Clear();
   mPolyMgr.Process(time, &mWriteBuffer, bufferSize);

   SyncOutputBuffer(mWriteBuffer.NumActiveChannels());
   for (int ch = 0; ch < mWriteBuffer.NumActiveChannels(); ++ch)
   {
      GetVizBuffer()->WriteChunk(mWriteBuffer.GetChannel(ch), mWriteBuffer.BufferSize(), ch);
      Add(target->GetBuffer()->GetChannel(ch), mWriteBuffer.GetChannel(ch), gBufferSize);
   }
}

void Wavetable::PlayNote(NoteMessage note)
{
   if (!mEnabled)
      return;

   if (!NoteInputBuffer::IsTimeWithinFrame(note.time) && GetTarget())
   {
      mNoteInputBuffer.QueueNote(note);
      return;
   }

   if (note.velocity > 0)
   {
      mPolyMgr.Start(note.time, note.pitch, note.velocity / 127.0f, note.voiceIdx, note.modulation);
      mVoiceParams.mAdsr.Start(note.time, 1); //for visualization
      mVoiceParams.mFilterAdsr.Start(note.time, 1); //for visualization
      mVoiceParams.mAdsrB.Start(note.time, 1); //osc B env visualization
      mVoiceParams.mFilterAdsrB.Start(note.time, 1); //osc B filter env visualization
   }
   else
   {
      mPolyMgr.Stop(note.time, note.pitch, note.voiceIdx);
      mVoiceParams.mAdsr.Stop(note.time, false); //for visualization
      mVoiceParams.mFilterAdsr.Stop(note.time, false); //for visualization
      mVoiceParams.mAdsrB.Stop(note.time, false); //osc B env visualization
      mVoiceParams.mFilterAdsrB.Stop(note.time, false); //osc B filter env visualization
   }

   if (mDrawDebug)
   {
      mDebugLines[mDebugLinesPos].text = "PlayNote(" + ofToString(note.time / 1000) + ", " + ofToString(note.pitch) + ", " + ofToString(note.velocity) + ", " + ofToString(note.voiceIdx) + ")";
      if (note.velocity > 0)
         mDebugLines[mDebugLinesPos].color = ofColor::lime;
      else
         mDebugLines[mDebugLinesPos].color = ofColor::red;
      ofLog() << mDebugLines[mDebugLinesPos].text;
      mDebugLinesPos = (mDebugLinesPos + 1) % (int)mDebugLines.size();
   }
}

void Wavetable::SetEnabled(bool enabled)
{
   mEnabled = enabled;
}

void Wavetable::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   //oscillator waveform previews - each shows the live single-cycle shape at that oscillator's scan
   //position with the shared warp applied. Each dims when its oscillator is switched off.
   DrawOscWaveform(ofRectangle(kOscAX, kOscAWaveTop, kColWidthOsc, kWaveformHeight),
                   mVoiceParams.mTableA, mVoiceParams.mPositionA, ofColor(120, 200, 255), mVoiceParams.mUseOscA);
   DrawOscWaveform(ofRectangle(kOscBX, kOscBWaveTop, kColWidthOsc, kWaveformHeight),
                   mVoiceParams.mTableB, mVoiceParams.mPositionB, ofColor(255, 185, 120), mVoiceParams.mUseOscB);

   //osc A controls
   mUseOscACheckbox->Draw();
   DrawTextNormal("on", kOscAX + 14, kContentTop + 11); //manual label (checkbox text is hidden - see CreateUIControls)
   mTableADropdown->Draw();
   mPositionASlider->Draw();
   mVolSlider->Draw();
   mDetuneSlider->Draw();
   mUnisonSlider->Draw();
   mUnisonWidthSlider->Draw();

   //osc A filter + amp env
   mFilterADSRDisplay->Draw();
   mFilterCutoffMaxSlider->Draw();
   mFilterCutoffMinSlider->Draw();
   mFilterQSlider->Draw();
   if (mVoiceParams.mFilterCutoffMax == WAVETABLE_NO_CUTOFF)
   {
      ofPushStyle();
      ofSetColor(0, 0, 0, 100);
      ofFill();
      ofRect(mFilterADSRDisplay->GetRect(true).grow(1));
      ofRect(mFilterCutoffMinSlider->GetRect(true));
      ofRect(mFilterQSlider->GetRect(true));
      ofPopStyle();
   }
   mADSRDisplay->Draw();

   //osc B controls - always fully drawn (never hidden), so the module footprint is stable
   mUseOscBCheckbox->Draw();
   mSyncBCheckbox->Draw();
   mTableBDropdown->Draw();
   mPositionBSlider->Draw();
   mVolBSlider->Draw();
   mDetuneBSlider->Draw();
   mUnisonBSlider->Draw();
   mUnisonWidthBSlider->Draw();

   //osc B filter + amp env
   mFilterADSRDisplayB->Draw();
   mFilterCutoffMaxBSlider->Draw();
   mFilterCutoffMinBSlider->Draw();
   mFilterQBSlider->Draw();
   if (mVoiceParams.mFilterCutoffMaxB == WAVETABLE_NO_CUTOFF)
   {
      ofPushStyle();
      ofSetColor(0, 0, 0, 100);
      ofFill();
      ofRect(mFilterADSRDisplayB->GetRect(true).grow(1));
      ofRect(mFilterCutoffMinBSlider->GetRect(true));
      ofRect(mFilterQBSlider->GetRect(true));
      ofPopStyle();
   }
   mADSRDisplayB->Draw();

   //shape
   mModTypeDropdown->Draw();
   mModAmountSlider->Draw();
   mWarpTypeDropdown->Draw();
   mWarpAmountSlider->Draw();

   //column header labels
   DrawTextNormal("osc a", kOscAX + 2, 12);
   DrawTextNormal("osc b", kOscBX + 2, 12);
   DrawTextNormal("shape", kShapeX + 2, 12);

   //per-osc filter headers (dim when that osc's filter is off) + "env" sub-labels above each amp env
   ofPushStyle();
   if (mVoiceParams.mFilterCutoffMax == WAVETABLE_NO_CUTOFF)
      ofSetColor(100, 100, 100);
   DrawTextNormal("filter a", kFiltAX + 2, 12);
   ofPopStyle();
   ofPushStyle();
   if (mVoiceParams.mFilterCutoffMaxB == WAVETABLE_NO_CUTOFF)
      ofSetColor(100, 100, 100);
   DrawTextNormal("filter b", kFiltBX + 2, 12);
   ofPopStyle();

   ofVec2f ampAPos = mADSRDisplay->GetPosition(true);
   DrawTextNormal("env a", ampAPos.x, ampAPos.y - 2, 10);
   ofVec2f ampBPos = mADSRDisplayB->GetPosition(true);
   DrawTextNormal("env b", ampBPos.x, ampBPos.y - 2, 10);

   std::string importedName;
   if (mTableChoiceA == kSampleTableChoice && mImportedTableA)
      importedName = mImportedTableA->GetName();
   else if (mTableChoiceB == kSampleTableChoice && mImportedTableB)
      importedName = mImportedTableB->GetName();
   if (!importedName.empty())
      DrawTextNormal("loaded: " + importedName, kOscAX, mHeight - 4, 10);
}

void Wavetable::DrawModuleUnclipped()
{
   if (mDrawDebug)
   {
      mPolyMgr.DrawDebug(mWidth + 3, 0);
      float y = mHeight + 15;
      for (size_t i = 0; i < mDebugLines.size(); ++i)
      {
         const DebugLine& line = mDebugLines[(mDebugLinesPos + i) % mDebugLines.size()];
         ofSetColor(line.color);
         DrawTextNormal(line.text, 0, y);
         y += 15;
      }
   }
}

float Wavetable::GetDrawValue(float phase)
{
   if (mVoiceParams.mSyncMode != Oscillator::SyncMode::None)
   {
      phase = FloatWrap(phase, FTWO_PI);
      if (mVoiceParams.mSyncMode == Oscillator::SyncMode::Frequency)
         phase *= mVoiceParams.mSyncFreq / 200;
      if (mVoiceParams.mSyncMode == Oscillator::SyncMode::Ratio)
         phase *= mVoiceParams.mSyncRatio;
   }
   return WavetableTables::ReadWarped(mVoiceParams.mTableA.get(), mVoiceParams.mPositionA, phase, mVoiceParams.mWarpType, mVoiceParams.mWarpAmount);
}

void Wavetable::DrawOscWaveform(ofRectangle rect, const std::shared_ptr<WavetableFrameSet>& table, float position, ofColor color, bool active)
{
   ofPushStyle();

   //panel background + border
   ofSetColor(12, 14, 18, 200);
   ofFill();
   ofRect(rect, 3);
   ofSetColor(color.r, color.g, color.b, active ? 90 : 40);
   ofNoFill();
   ofRect(rect, 3);

   //zero-crossing centre line
   const float midY = rect.y + rect.height * 0.5f;
   ofSetColor(255, 255, 255, 22);
   ofSetLineWidth(1);
   ofLine(rect.x + 2, midY, rect.getMaxX() - 2, midY);

   //the single-cycle waveform itself, sampled across the box width at the current scan
   //position with the shared warp applied (matches what the voices actually play)
   if (table != nullptr)
   {
      ofSetColor(color.r, color.g, color.b, active ? 230 : 70);
      ofNoFill();
      ofSetLineWidth(active ? 1.5f : 1.0f);
      ofBeginShape();
      const int steps = MAX(2, (int)rect.width - 2);
      for (int i = 0; i <= steps; ++i)
      {
         float phase = ((float)i / steps) * FTWO_PI;
         float v = WavetableTables::ReadWarped(table.get(), position, phase, mVoiceParams.mWarpType, mVoiceParams.mWarpAmount);
         v = ofClamp(v, -1.0f, 1.0f);
         float x = rect.x + 1 + ((float)i / steps) * (rect.width - 2);
         float y = ofMap(v, 1.0f, -1.0f, rect.y + 3, rect.getMaxY() - 3);
         ofVertex(x, y);
      }
      ofEndShape();
   }

   ofPopStyle();
}

void Wavetable::DrawVisualizationToScreen(AbletonMoveLCD* screen, IUIControl* control)
{
   if (control == mFilterCutoffMinSlider || control == mFilterCutoffMaxSlider)
   {
      float minVal = mFilterCutoffMinSlider->GetMin();
      float maxVal = mFilterCutoffMaxSlider->GetMax();
      int minX = ofMap(mVoiceParams.mFilterCutoffMin, minVal, maxVal, 6, AbletonMoveLCD::kMoveDisplayWidth - 6);
      int maxX = ofMap(mVoiceParams.mFilterCutoffMax, minVal, maxVal, 6, AbletonMoveLCD::kMoveDisplayWidth - 6);
      screen->DrawRect(minX, 27, 1, 4, LCDDrawMode::Outline);
      screen->DrawRect(maxX, 27, 1, 4, LCDDrawMode::Outline);
      screen->DrawRect(minX, 28, maxX - minX, 2, LCDDrawMode::Outline);
   }
   else
   {
      int lastY = -1;
      for (float x = 0; x < AbletonMoveLCD::kMoveDisplayWidth; ++x)
      {
         float phase = x / AbletonMoveLCD::kMoveDisplayWidth * FTWO_PI;
         phase += gTime * .005f;
         float value = GetDrawValue(phase);
         int newY = int(ofMap(value, -1, 1, 10, AbletonMoveLCD::kMoveDisplayHeight - 10));
         if (lastY == -1)
            lastY = newY;
         if (lastY <= newY)
         {
            for (int y = lastY; y <= newY; ++y)
               screen->DrawPixel(x, y, LCDDrawMode::Toggle);
         }
         else
         {
            for (int y = lastY; y >= newY; --y)
               screen->DrawPixel(x, y, LCDDrawMode::Toggle);
         }
         lastY = newY;
      }
   }
}

void Wavetable::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadInt("voicelimit", moduleInfo, -1, -1, kNumVoices);
   mModuleSaveData.LoadBool("mono", moduleInfo, false);

   SetUpFromSaveData();
}

void Wavetable::SetUpFromSaveData()
{
   SetTarget(TheSynth->FindModule(mModuleSaveData.GetString("target")));

   int voiceLimit = mModuleSaveData.GetInt("voicelimit");
   if (voiceLimit > 0)
      mPolyMgr.SetVoiceLimit(voiceLimit);
   else
      mPolyMgr.SetVoiceLimit(kNumVoices);

   bool mono = mModuleSaveData.GetBool("mono");
   mWriteBuffer.SetNumActiveChannels(mono ? 1 : 2);
}

void Wavetable::SetTableChoice(bool isB, int choice)
{
   const auto& builtIns = WavetableTables::GetBuiltInTables();
   std::shared_ptr<WavetableFrameSet> table;
   if (choice == kSampleTableChoice)
      table = isB ? mImportedTableB : mImportedTableA;
   if (table == nullptr)
      table = (choice >= 0 && choice < (int)builtIns.size()) ? builtIns[choice] : builtIns[0];

   if (isB)
      mVoiceParams.mTableB = table;
   else
      mVoiceParams.mTableA = table;
}

void Wavetable::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   if (list == mTableADropdown)
      SetTableChoice(false, mTableChoiceA);
   if (list == mTableBDropdown)
      SetTableChoice(true, mTableChoiceB);
   if (list == mModTypeDropdown)
      mVoiceParams.mModType = (WavetableModType)mModType;
   if (list == mWarpTypeDropdown)
      mVoiceParams.mWarpType = (WavetableWarpType)mWarpType;
}

void Wavetable::RadioButtonUpdated(RadioButton* list, int oldVal, double time)
{
}

void Wavetable::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void Wavetable::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
}

void Wavetable::CheckboxUpdated(Checkbox* checkbox, double time)
{
   if (checkbox == mEnabledCheckbox)
      mPolyMgr.KillAll();
}

void Wavetable::ButtonClicked(ClickButton* button, double time)
{
}

void Wavetable::SampleDropped(int x, int y, Sample* sample)
{
   if (sample == nullptr || sample->LengthInSamples() <= 0)
      return;

   int numSamples = sample->LengthInSamples();
   ChannelBuffer* data = sample->Data();
   int numChannels = sample->NumChannels();

   std::vector<float> mono(numSamples);
   if (numChannels <= 1)
   {
      float* ch0 = data->GetChannel(0);
      for (int i = 0; i < numSamples; ++i)
         mono[i] = ch0[i];
   }
   else
   {
      float* ch0 = data->GetChannel(0);
      float* ch1 = data->GetChannel(1);
      for (int i = 0; i < numSamples; ++i)
         mono[i] = (ch0[i] + ch1[i]) * 0.5f;
   }

   auto table = WavetableTables::BuildFromSample(mono.data(), numSamples, sample->Name());

   //drop onto oscillator A's column (or anything left of the midpoint between
   //the two columns) to load it into A, oscillator B's column (or anything to
   //the right) to load it into B
   float threshold = kOscBX - kGap / 2;
   bool targetB = (x > threshold);

   if (targetB)
   {
      mImportedTableB = table;
      mTableChoiceB = kSampleTableChoice;
      mVoiceParams.mTableB = table;
   }
   else
   {
      mImportedTableA = table;
      mTableChoiceA = kSampleTableChoice;
      mVoiceParams.mTableA = table;
   }
}

void Wavetable::SaveImportedTable(FileStreamOut& out, const std::shared_ptr<WavetableFrameSet>& table)
{
   bool hasTable = (table != nullptr);
   out << hasTable;
   if (!hasTable)
      return;

   out << table->GetName();
   int frameCount = table->GetFrameCount();
   out << frameCount;
   for (int f = 0; f < frameCount; ++f)
   {
      const float* frameData = table->GetFrameData(f, false, false);
      for (int s = 0; s < WavetableFrameSet::kTableSize; ++s)
         out << frameData[s];
   }
}

std::shared_ptr<WavetableFrameSet> Wavetable::LoadImportedTable(FileStreamIn& in)
{
   bool hasTable;
   in >> hasTable;
   if (!hasTable)
      return nullptr;

   std::string name;
   in >> name;
   int frameCount;
   in >> frameCount;

   auto table = std::make_shared<WavetableFrameSet>(name);
   for (int f = 0; f < frameCount; ++f)
   {
      std::vector<float> frame(WavetableFrameSet::kTableSize);
      for (int s = 0; s < WavetableFrameSet::kTableSize; ++s)
         in >> frame[s];
      table->AddFrame(frame);
   }
   table->FinalizeHarmonicCaches();
   return table;
}

void Wavetable::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   SaveImportedTable(out, mImportedTableA);
   SaveImportedTable(out, mImportedTableB);
}

void Wavetable::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   LoadStateValidate(rev <= GetModuleSaveStateRev());

   if (rev >= 1)
   {
      mImportedTableA = LoadImportedTable(in);
      mImportedTableB = LoadImportedTable(in);

      //dropdowns were already restored by IDrawableModule::LoadState above (they're
      //ordinary UI controls); re-point the actual playback table pointers now that
      //any imported table data has been read back in
      SetTableChoice(false, mTableChoiceA);
      SetTableChoice(true, mTableChoiceB);
      mVoiceParams.mModType = (WavetableModType)mModType;
      mVoiceParams.mWarpType = (WavetableWarpType)mWarpType;
   }
}
