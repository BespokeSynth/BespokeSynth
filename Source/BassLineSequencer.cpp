/**
    bespoke synth, a software modular synthesizer
    Copyright (C) 2025 Ryan Challinor (contact: awwbees@gmail.com)

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
//  BassLineSequencer.cpp
//  BespokeSynth
//
//  Created by Ryan Challinor on 1/15/25.
//
//

#include "BassLineSequencer.h"
#include "SynthGlobals.h"
#include "UIControlMacros.h"
#include "FileStream.h"
#include "Scale.h"
#include "ModularSynth.h"
#include "NoteStepSequencer.h"

BassLineSequencer::BassLineSequencer()
{
}

void BassLineSequencer::Init()
{
   IDrawableModule::Init();

   mTransportListenerInfo = TheTransport->AddListener(this, mInterval, OffsetInfo(0, true), true);
   TheScale->AddListener(this);

   mModulation.pitchBend = &mPitchBend;
}

void BassLineSequencer::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   DROPDOWN(mIntervalSelector, "interval", (int*)(&mInterval), 40);
   UIBLOCK_SHIFTRIGHT();
   INTSLIDER(mLengthSlider, "length", &mLength, 1, kMaxSteps);
   UIBLOCK_SHIFTRIGHT();
   INTSLIDER(mOctaveSlider, "octave", &mOctave, 0, 7);
   UIBLOCK_SHIFTRIGHT();
   DROPDOWN(mNoteModeSelector, "notemode", (int*)(&mNoteMode), 80);
   UIBLOCK_SHIFTRIGHT();
   FLOATSLIDER(mGlideSlider, "glide", &mGlideTime, 0, 250);
   UIBLOCK_SHIFTRIGHT();
   BUTTON(mClearButton, "clear");
   UIBLOCK_SHIFTRIGHT();
   BUTTON(mShiftLeftButton, "<");
   UIBLOCK_SHIFTRIGHT();
   BUTTON(mShiftRightButton, ">");
   UIBLOCK_SHIFTRIGHT();
   DROPDOWN(mGlideModeSelector, "glidemode", (int*)(&mGlideMode), 80);
   UIBLOCK_NEWLINE();
   INTSLIDER(mEditPageSlider, "edit page", &mEditPage, 0, GetPageCount() - 1);
   UIBLOCK_SHIFTRIGHT();
   BUTTON(mRandomizeButton, "randomize");
   UIBLOCK_SHIFTRIGHT();
   FLOATSLIDER(mRandomDensitySlider, "r density", &mRandomDensity, 0, 1);
   UIBLOCK_SHIFTRIGHT();
   INTSLIDER(mRandomVarietySlider, "r variety", &mRandomVariety, 1, mNoteRange);
   UIBLOCK_SHIFTRIGHT();
   FLOATSLIDER(mRandomAccentsSlider, "r accents", &mRandomAccents, 0, 1);
   UIBLOCK_SHIFTRIGHT();
   FLOATSLIDER(mRandomTiesSlider, "r ties", &mRandomTies, 0, 1);
   mNoteDisplayY = UIBLOCKHEIGHT();
   ENDUIBLOCK0();

   mGlideSlider->SetMode(FloatSlider::kSquare);

   UIBLOCK2(3, 100);
   UIBLOCK_PUSHSLIDERWIDTH(35);
   int i = 0;
   for (auto& stepControl : mStepControls)
   {
      std::string suffix = ofToString(i);

      if (i == kEditStepControlIndex)
      {
         UIBLOCK_SHIFTX(20);
         UIBLOCK_SHIFTY(-40);
         UIBLOCK_PUSHSLIDERWIDTH(70);
         INTSLIDER(mEditStepControlSlider, "edit step", &mEditStepControl, -1, kMaxStepControls - 1);
         suffix = "_edit";
         UIBLOCK_NEWCOLUMN();
         UIBLOCK_PUSHSLIDERWIDTH(35);
      }

      stepControl.xPos = xPos;
      stepControl.yPos = yPos;

      UICONTROL_CUSTOM(stepControl.mGridSquare, new UIGrid(UICONTROL_BASICS(("gridsquare" + suffix).c_str()), 20, 20, 1, 1));
      DROPDOWN(stepControl.mToneDropdown, ("tone" + suffix).c_str(), &stepControl.mTone, 35);
      stepControl.xMax = xMax;
      CHECKBOX(stepControl.mTieCheckbox, ("tie" + suffix).c_str(), &stepControl.mTie);

      stepControl.mGridSquare->SetListener(this);
      stepControl.mGridSquare->SetStrength(kVelocityNormal);
      stepControl.mGridSquare->SetGridMode(UIGrid::kMultisliderGrow);
      stepControl.mGridSquare->SetRequireShiftForMultislider(true);
      stepControl.mGridSquare->SetCanBeUIControlTarget(true);

      stepControl.mToneDropdown->SetDrawTriangle(false);

      stepControl.mTieCheckbox->SetDisplayText(false);

      UIBLOCK_NEWCOLUMN();

      stepControl.yMax = yMax;

      ++i;
   }
   mWidth = UIBLOCKWIDTH();
   mHeight = UIBLOCKHEIGHT();
   ENDUIBLOCK(mWidth, mHeight);

   UpdatePitchLabels();

   mIntervalSelector->AddLabel("4", kInterval_4);
   mIntervalSelector->AddLabel("3", kInterval_3);
   mIntervalSelector->AddLabel("2", kInterval_2);
   mIntervalSelector->AddLabel("1n", kInterval_1n);
   mIntervalSelector->AddLabel("2n", kInterval_2n);
   mIntervalSelector->AddLabel("4n", kInterval_4n);
   mIntervalSelector->AddLabel("4nt", kInterval_4nt);
   mIntervalSelector->AddLabel("8n", kInterval_8n);
   mIntervalSelector->AddLabel("8nt", kInterval_8nt);
   mIntervalSelector->AddLabel("16n", kInterval_16n);
   mIntervalSelector->AddLabel("16nt", kInterval_16nt);
   mIntervalSelector->AddLabel("32n", kInterval_32n);
   mIntervalSelector->AddLabel("64n", kInterval_64n);
   mIntervalSelector->AddLabel("none", kInterval_None);

   mNoteModeSelector->AddLabel("scale", NoteStepSequencer::kNoteMode_Scale);
   mNoteModeSelector->AddLabel("chromatic", NoteStepSequencer::kNoteMode_Chromatic);
   mNoteModeSelector->AddLabel("pentatonic", NoteStepSequencer::kNoteMode_Pentatonic);
   mNoteModeSelector->AddLabel("5ths", NoteStepSequencer::kNoteMode_Fifths);

   mGlideModeSelector->AddLabel("pitchbend", (int)GlideMode::PitchBend);
   mGlideModeSelector->AddLabel("legato", (int)GlideMode::Legato);
   mGlideModeSelector->AddLabel("slideCC", (int)GlideMode::SlideCC);
}

BassLineSequencer::~BassLineSequencer()
{
   TheTransport->RemoveListener(this);
   TheScale->RemoveListener(this);
}

void BassLineSequencer::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   float displayWidth = GetDisplayWidth();

   ofPushMatrix();
   ofTranslate(kDisplayX, mNoteDisplayY);
   ofPushStyle();
   ofFill();
   ofSetColor(100, 100, 100, 0.5f * gModuleDrawAlpha);
   ofRect(0, 0, displayWidth, kDisplayHeight);
   float noteWidth = displayWidth / mLength;
   for (int i = 0; i < mLength; ++i)
   {
      if (i % 2 == 0)
      {
         if (i % 8 == 0)
            ofSetColor(120, 120, 120, 0.5f * gModuleDrawAlpha);
         else
            ofSetColor(80, 80, 80, 0.5f * gModuleDrawAlpha);
         ofRect(i * noteWidth, 0, noteWidth, kDisplayHeight);
      }
   }

   float toneHeight = kDisplayHeight / mNoteRange;
   /*ofSetColor(255, 255, 255, gModuleDrawAlpha * .2f);
   ofSetLineWidth(0.5f);
   for (int i = 1; i < toneRange; ++i)
      ofLine(0, toneHeight * i, displayWidth, toneHeight * i);*/

   float noteHeight = kDisplayHeight / mNoteRange;
   float lineWidth = MIN(noteHeight, 5);
   ofPushStyle();
   ofSetLineWidth(lineWidth);
   float playingVelocity = mSteps[0].mVelocity;
   for (int i = 0; i < mLength; ++i)
   {
      if (mSteps[i].mVelocity > 0)
      {
         bool isAccent = mSteps[i].mVelocity > kVelocityNormal;

         bool tiePrevious = i > 0 && (mSteps[i - 1].mTie && mSteps[i - 1].mVelocity > 0);
         if (tiePrevious)
         {
            ofLine(GetNoteDrawPos(i - 1, displayWidth, kDisplayHeight, lineWidth, true),
                   GetNoteDrawPos(i, displayWidth, kDisplayHeight, lineWidth, false));
         }

         if (!tiePrevious || mSteps[i].mVelocity > playingVelocity || isAccent)
         {
            playingVelocity = mSteps[i].mVelocity;

            ofVec2f center = GetNoteDrawPos(i, displayWidth, kDisplayHeight, lineWidth, false);
            if (playingVelocity > kVelocityNormal)
            {
               ofSetColor(255, 255, 255, gModuleDrawAlpha);
               ofRect(center.x - lineWidth - 1, center.y - lineWidth - 1, lineWidth * 2 + 2, lineWidth * 2 + 2, 0);
            }
            else if (playingVelocity > kVelocityGhost)
            {
               ofSetColor(200, 200, 200, gModuleDrawAlpha);
               ofCircle(center.x, center.y, lineWidth + 1);
            }
            else
            {
               ofSetColor(150, 150, 150, gModuleDrawAlpha);
            }
         }

         ofLine(GetNoteDrawPos(i, displayWidth, kDisplayHeight, lineWidth, false),
                GetNoteDrawPos(i, displayWidth, kDisplayHeight, lineWidth, true));
      }
   }
   ofPopStyle();
   ofSetColor(0, 255, 0, gModuleDrawAlpha * .2f);
   ofRect(mStepIdx * noteWidth, 0, noteWidth, kDisplayHeight);
   if (mHighlightDisplayStepIdx != -1)
   {
      ofSetColor(255, 255, 255, ofMap(sin(gTime / 500 * PI * 2), -1, 1, .2f, .6f) * gModuleDrawAlpha);
      ofRect((mHighlightDisplayStepIdx + mEditPage * mNumVisibleStepControls) * noteWidth, 0, noteWidth, kDisplayHeight);
   }
   ofPopStyle();
   ofPopMatrix();

   ofPushStyle();
   ofNoFill();
   ofSetColor(200, 200, 200, gModuleDrawAlpha);
   int numNotesOnPage = mNumVisibleStepControls;
   if (mEditPage == GetPageCount() - 1 && mLength % mNumVisibleStepControls != 0)
      numNotesOnPage = mLength % mNumVisibleStepControls;
   float editX = kDisplayX + mEditPage * mNumVisibleStepControls * noteWidth;
   float editW = numNotesOnPage * noteWidth;
   ofSetLineWidth(0.5f);
   ofRect(editX, mNoteDisplayY, editW, kDisplayHeight);
   if (mEditStepControl != -1)
   {
      ofPushStyle();
      ofSetColor(255, 255, 255, gModuleDrawAlpha * ofMap(sin(gTime / 500 * PI * 2), -1, 1, .2f, .8f));
      float editStepControlX = kDisplayX + (mEditPage * mNumVisibleStepControls + mEditStepControl) * noteWidth;
      ofRect(editStepControlX + 1, mNoteDisplayY + 1, noteWidth - 2, kDisplayHeight - 2);
      ofPopStyle();
   }
   ofFill();
   for (int i = 0; i < numNotesOnPage; ++i)
   {
      if (i % 2 == 0)
         ofSetColor(120, 120, 120, gModuleDrawAlpha * .3f);
      else
         ofSetColor(80, 80, 80, gModuleDrawAlpha * .3f);
      ofBeginShape();
      ofVertex(editX + i * noteWidth, mNoteDisplayY + kDisplayHeight);
      ofVertex(mStepControls[i].xPos, mStepControls[i].yPos - 2);
      ofVertex(mStepControls[i].xMax, mStepControls[i].yPos - 2);
      ofVertex(editX + (i + 1) * noteWidth, mNoteDisplayY + kDisplayHeight);
      ofEndShape();
   }
   ofPopStyle();

   if (mHighlightStepControlIdx != -1 && mHighlightStepControlIdx < (int)mStepControls.size())
   {
      ofPushStyle();
      ofFill();
      ofSetColor(255, 255, 255, ofMap(sin(gTime / 500 * PI * 2), -1, 1, .1f, .3f) * gModuleDrawAlpha);
      auto& stepControl = mStepControls[mHighlightStepControlIdx];
      ofRect(stepControl.xPos, stepControl.yPos, stepControl.xMax - stepControl.xPos, stepControl.yMax - stepControl.yPos);
      ofPopStyle();
   }

   mGlideSlider->SetShowing(mGlideMode == GlideMode::PitchBend);

   mIntervalSelector->Draw();
   mLengthSlider->Draw();
   mOctaveSlider->Draw();
   mNoteModeSelector->Draw();
   mGlideSlider->Draw();
   mClearButton->Draw();
   mShiftLeftButton->Draw();
   mShiftRightButton->Draw();
   mRandomizeButton->Draw();
   mRandomDensitySlider->Draw();
   mRandomVarietySlider->Draw();
   mRandomAccentsSlider->Draw();
   mRandomTiesSlider->Draw();
   mEditPageSlider->Draw();
   mEditStepControlSlider->Draw();
   mGlideModeSelector->Draw();

   for (int i = 0; i < (int)mStepControls.size(); ++i)
   {
      bool visible = (i < mNumVisibleStepControls && i + mEditPage * mNumVisibleStepControls < mLength);
      if (i == kEditStepControlIndex)
         visible = true;
      mStepControls[i].mGridSquare->SetShowing(visible);
      mStepControls[i].mToneDropdown->SetShowing(visible);
      mStepControls[i].mTieCheckbox->SetShowing(visible);

      if (visible)
      {
         mStepControls[i].mGridSquare->Draw();
         mStepControls[i].mToneDropdown->Draw();
         mStepControls[i].mTieCheckbox->Draw();
         if (mStepControls[i].mVelocity == 0)
         {
            ofRectangle greyOut = ofRectangle::include(mStepControls[i].mToneDropdown->GetRect(K(local)), mStepControls[i].mTieCheckbox->GetRect(K(local)));
            ofPushStyle();
            ofFill();
            ofSetColor(0, 0, 0, 100);
            ofRect(greyOut);
            ofPopStyle();
         }

         if (i % 4 == 0 && i != (int)mStepControls.size() - 1)
         {
            ofPushStyle();
            ofSetColor(255, 255, 255, gModuleDrawAlpha);
            ofSetLineWidth(0.5f);
            ofLine(mStepControls[i].xPos - 2, mStepControls[i].yPos, mStepControls[i].xPos - 2, mStepControls[i].yMax);
            ofPopStyle();
         }
      }
   }
}

ofVec2f BassLineSequencer::GetNoteDrawPos(int stepIdx, float displayWidth, float displayHeight, float lineWidth, bool end)
{
   float noteHeight = displayHeight / mNoteRange;
   float noteWidth = displayWidth / mLength;
   int radiusScootch = lineWidth;
   int x = stepIdx * noteWidth;
   if (end)
      x += noteWidth;
   int y = (mNoteRange - mSteps[stepIdx].mTone - 1) * noteHeight;
   if (end)
   {
      if (!mSteps[stepIdx].mTie)
         x -= radiusScootch;
      return ofVec2f(x, y + radiusScootch);
   }
   else
   {
      bool tiePrevious = stepIdx > 0 && (mSteps[stepIdx - 1].mTie && mSteps[stepIdx - 1].mVelocity > 0);
      if (tiePrevious)
      {
         float glideDuration = MIN(mGlideTime / TheTransport->GetDuration(mInterval), 1.0f);
         x += glideDuration * noteWidth;
      }
      else
      {
         x += radiusScootch;
      }
      return ofVec2f(x, y + radiusScootch);
   }
}

void BassLineSequencer::PlayNote(NoteMessage note)
{
   if (note.velocity == 0 && note.pitch == mHeldInputPitch)
      mHeldInputPitch = -1;
   if (note.velocity > 0)
   {
      if (mHeldInputPitch == -1)
         mWriteNewNotePitch = note.pitch;
      mHeldInputPitch = note.pitch;
      mLastInputVelocity = note.velocity / 127.0f;
   }
}

void BassLineSequencer::OnTimeEvent(double time)
{
   if (mHasExternalPulseSource)
      return;

   StepBy(time, 1, kPulseFlag_SyncToTransport);
}

void BassLineSequencer::OnPulse(double time, float velocity, int flags)
{
   mHasExternalPulseSource = true;
   StepBy(time, velocity, flags);
}

void BassLineSequencer::StepBy(double time, float velocity, int flags)
{
   mStepIdx = (mStepIdx + 1) % mLength;

   if (flags & kPulseFlag_Reset)
      ResetStep();

   if (flags & kPulseFlag_SyncToTransport)
   {
      mStepIdx = TheTransport->GetSyncedStep(time, this, mTransportListenerInfo, mLength);
   }

   if (mEnabled)
   {
      if (mHeldInputPitch != -1 || mWriteNewNotePitch != -1)
      {
         int pitch;
         if (mWriteNewNotePitch != -1)
            pitch = mWriteNewNotePitch;
         else
            pitch = mHeldInputPitch;

         int tone = -1;
         for (int i = 0; i < mNoteRange; ++i)
         {
            if (pitch == NoteStepSequencer::RowToPitch(mNoteMode, i, mOctave, 0))
            {
               tone = i;
               break;
            }
         }

         if (tone != -1)
         {
            if (mWriteNewNotePitch != -1)
            {
               mWriteNewNotePitch = -1;
            }
            else
            {
               int previousStep = (mStepIdx - 1 + mLength) % mLength;
               mSteps[previousStep].mTie = true;
               mLastWasTied = true;

               mLastInputVelocity = MIN(mLastInputVelocity, kVelocityNormal);
            }

            mSteps[mStepIdx].mTone = tone;
            mSteps[mStepIdx].mVelocity = mLastInputVelocity;
            mSteps[mStepIdx].mTie = false;

            UpdateStepControls();
         }
      }

      bool isAccent = mSteps[mStepIdx].mVelocity > kVelocityNormal;
      int outputVelocity = mSteps[mStepIdx].mVelocity * velocity * 127;

      if (mPlayingPitch != -1 && (!mLastWasTied || outputVelocity == 0))
      {
         PlayNoteOutput(NoteMessage(time, mPlayingPitch, 0, -1, mModulation));
         mPlayingPitch = -1;
      }

      if (outputVelocity > 0)
      {
         int pitch = NoteStepSequencer::RowToPitch(mNoteMode, mSteps[mStepIdx].mTone, mOctave, 0);
         if (mLastWasTied)
         {
            float bendDistance = pitch - mPlayingPitch;
            bool retrigger = outputVelocity > mLastVelocity || isAccent;
            if (mGlideMode == GlideMode::PitchBend)
            {
               if (retrigger)
               {
                  PlayNoteOutput(NoteMessage(time, mPlayingPitch, 0, -1, mModulation));
                  PlayNoteOutput(NoteMessage(time, pitch, outputVelocity, -1, mModulation));
                  mPlayingPitch = pitch;
                  if (bendDistance != 0)
                     mPitchBend.RampValue(time, -bendDistance, 0, mGlideTime);
               }
               else
               {
                  if (bendDistance != 0)
                     mPitchBend.RampValue(time, mPitchBend.GetIndividualValue(0), bendDistance, mGlideTime);
               }
            }
            else if (mGlideMode == GlideMode::Legato || mGlideMode == GlideMode::SlideCC)
            {
               if (mGlideMode == GlideMode::SlideCC)
                  SendCCOutput(102, 127);

               PlayNoteOutput(NoteMessage(time, pitch, outputVelocity, -1, mModulation));
               PlayNoteOutput(NoteMessage(time, mPlayingPitch, 0, -1, mModulation));
               mPlayingPitch = pitch;
            }
         }
         else
         {
            if (mGlideMode == GlideMode::SlideCC)
               SendCCOutput(102, 0);
            mPitchBend.SetValue(0);
            PlayNoteOutput(NoteMessage(time, pitch, outputVelocity, -1, mModulation));
            mPlayingPitch = pitch;
         }
         mLastVelocity = outputVelocity;
         mLastWasTied = mSteps[mStepIdx].mTie;
      }
      else
      {
         mPlayingPitch = -1;
         mLastVelocity = 0;
         mLastWasTied = false;
      }
   }
   else if (mPlayingPitch >= 0)
   {
      PlayNoteOutput(NoteMessage(time, mPlayingPitch, 0, -1, mModulation));
      mPlayingPitch = -1;
   }

   for (int i = 0; i < kMaxStepControls; ++i)
      mStepControls[i].mGridSquare->SetHighlightCol(time, mEnabled && mStepIdx == (i + mEditPage * mNumVisibleStepControls) ? 0 : -1);
}

void BassLineSequencer::ResetStep()
{
   mStepIdx = 0;
}

void BassLineSequencer::UpdateStepControls()
{
   for (int i = 0; i < (int)mStepControls.size(); ++i)
   {
      mStepControls[i].mTone = mSteps[i + mEditPage * mNumVisibleStepControls].mTone;
      mStepControls[i].mVelocity = mSteps[i + mEditPage * mNumVisibleStepControls].mVelocity;
      mStepControls[i].mTie = mSteps[i + mEditPage * mNumVisibleStepControls].mTie;
      mStepControls[i].mGridSquare->SetVal(0, 0, mStepControls[i].mVelocity);
   }
}

void BassLineSequencer::OnEditStepUpdated()
{
   mStepControls[mEditStepControl].mTone = mStepControls[kEditStepControlIndex].mTone;
   mStepControls[mEditStepControl].mVelocity = mStepControls[kEditStepControlIndex].mVelocity;
   mStepControls[mEditStepControl].mTie = mStepControls[kEditStepControlIndex].mTie;
   mStepControls[mEditStepControl].mGridSquare->SetVal(0, 0, mStepControls[kEditStepControlIndex].mVelocity);
}

void BassLineSequencer::OnScaleChanged()
{
   UpdatePitchLabels();
}

void BassLineSequencer::UpdatePitchLabels()
{
   if (TheSynth->IsLoadingModule())
      return;

   for (auto& stepControl : mStepControls)
   {
      stepControl.mToneDropdown->Clear();
      for (int j = mNoteRange - 1; j >= 0; --j)
         stepControl.mToneDropdown->AddLabel(NoteName(NoteStepSequencer::RowToPitch(mNoteMode, j, mOctave, 0), false, true), j);
   }
}

void BassLineSequencer::GetModuleDimensions(float& width, float& height)
{
   width = mWidth;
   height = mHeight;
}

void BassLineSequencer::OnClicked(float x, float y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);

   for (auto& stepControl : mStepControls)
      stepControl.mGridSquare->TestClick(x, y, right);

   if (x >= kDisplayX && x < kDisplayX + GetDisplayWidth() && y >= mNoteDisplayY && y < mNoteDisplayY + kDisplayHeight)
   {
      float noteWidth = GetDisplayWidth() / mLength;
      int numNotesOnPage = mNumVisibleStepControls;
      if (mEditPage == GetPageCount() - 1 && mLength % mNumVisibleStepControls != 0)
         numNotesOnPage = mLength % mNumVisibleStepControls;
      float editW = numNotesOnPage * noteWidth;
      for (int i = 0; i < GetPageCount(); ++i)
      {
         float editX = kDisplayX + i * mNumVisibleStepControls * noteWidth;
         if (x >= editX && x < editX + editW)
         {
            if (i == mEditPage)
            {
               int stepIdx = int((x - kDisplayX) / noteWidth);
               if (stepIdx >= 0 && stepIdx < mLength)
               {
                  if (right)
                  {
                     mSteps[stepIdx].mVelocity = 0;
                  }
                  else
                  {
                     mSteps[stepIdx].mTone = int((1 - (y - mNoteDisplayY) / kDisplayHeight) * mNoteRange);
                     if (mSteps[stepIdx].mVelocity == 0)
                        mSteps[stepIdx].mVelocity = kVelocityNormal;
                  }
                  UpdateStepControls();
               }
               break;
            }
            else
            {
               mEditPage = i;
               UpdateStepControls();
               break;
            }
         }
      }
   }
}

void BassLineSequencer::MouseReleased()
{
   IDrawableModule::MouseReleased();
   for (auto& stepControl : mStepControls)
      stepControl.mGridSquare->MouseReleased();
}

bool BassLineSequencer::MouseMoved(float x, float y)
{
   IDrawableModule::MouseMoved(x, y);
   for (auto& stepControl : mStepControls)
      stepControl.mGridSquare->NotifyMouseMoved(x, y);

   mHighlightDisplayStepIdx = -1;
   for (int i = 0; i < (int)mStepControls.size(); ++i)
   {
      auto& stepControl = mStepControls[i];
      if (x >= stepControl.xPos && x < stepControl.xMax && y >= stepControl.yPos && y < stepControl.yMax)
         mHighlightDisplayStepIdx = i;
   }

   mHighlightStepControlIdx = -1;
   if (x >= kDisplayX && x < kDisplayX + GetDisplayWidth() && y >= mNoteDisplayY && y < mNoteDisplayY + kDisplayHeight)
   {
      float noteWidth = GetDisplayWidth() / mLength;
      for (int i = 0; i < mLength; ++i)
      {
         float noteX = kDisplayX + i * noteWidth;
         if (x >= noteX && x < noteX + noteWidth)
         {
            int controlOffset = mNumVisibleStepControls * mEditPage;
            if (i >= controlOffset && i < controlOffset + mNumVisibleStepControls)
               mHighlightStepControlIdx = i - controlOffset;
         }
      }
   }

   mMouseHoverPos.set(x, y);

   return false;
}

void BassLineSequencer::KeyPressed(int key, bool isRepeat)
{
   bool consumed = false;

   float x = mMouseHoverPos.x;
   float y = mMouseHoverPos.y;

   if (key == OF_KEY_UP || key == OF_KEY_DOWN)
   {
      for (int i = 0; i < (int)mStepControls.size(); ++i)
      {
         if (mStepControls[i].mGridSquare->GetRect(K(local)).contains(x, y))
         {
            float newVelocity = -1;

            if (key == OF_KEY_UP)
            {
               for (int j = 0; j < (int)gStepVelocityLevels.size(); ++j)
               {
                  if (mStepControls[i].mVelocity < gStepVelocityLevels[j])
                  {
                     newVelocity = gStepVelocityLevels[j];
                     break;
                  }
               }
            }

            if (key == OF_KEY_DOWN)
            {
               for (int j = (int)gStepVelocityLevels.size() - 1; j >= 0; --j)
               {
                  if (mStepControls[i].mVelocity > gStepVelocityLevels[j])
                  {
                     newVelocity = gStepVelocityLevels[j];
                     break;
                  }
               }
            }

            if (newVelocity != -1)
            {
               mStepControls[i].mVelocity = newVelocity;
               mStepControls[i].mGridSquare->SetVal(0, 0, newVelocity);
               mSteps[i + mEditPage * mNumVisibleStepControls].mVelocity = newVelocity;
            }
         }
      }

      consumed = true;
   }

   if (key == OF_KEY_UP ||
       key == OF_KEY_DOWN ||
       key == OF_KEY_LEFT ||
       key == OF_KEY_RIGHT ||
       key == OF_KEY_RETURN ||
       key == juce::KeyPress::backspaceKey ||
       key == juce::KeyPress::deleteKey ||
       key == '\\')
   {
      if (x >= kDisplayX && x < kDisplayX + GetDisplayWidth() && y >= mNoteDisplayY && y < mNoteDisplayY + kDisplayHeight)
      {
         float noteWidth = GetDisplayWidth() / mLength;
         for (int i = 0; i < mLength; ++i)
         {
            float noteX = kDisplayX + i * noteWidth;
            if (x >= noteX && x < noteX + noteWidth)
            {
               int adjustToneAmount = 0;

               if (key == OF_KEY_UP)
                  adjustToneAmount = 1;

               if (key == OF_KEY_DOWN)
                  adjustToneAmount = -1;

               if (GetKeyModifiers() == kModifier_Shift)
               {
                  if (mNoteMode == NoteStepSequencer::kNoteMode_Scale)
                     adjustToneAmount *= TheScale->NumTonesInScale();
                  else if (mNoteMode == NoteStepSequencer::kNoteMode_Chromatic)
                     adjustToneAmount *= TheScale->GetPitchesPerOctave();
                  else if (mNoteMode == NoteStepSequencer::kNoteMode_Pentatonic)
                     adjustToneAmount *= 5;
                  else if (mNoteMode == NoteStepSequencer::kNoteMode_Fifths)
                     adjustToneAmount *= 2;
               }

               bool shouldUpdate = false;
               if (adjustToneAmount != 0 && mSteps[i].mTone + adjustToneAmount >= 0 && mSteps[i].mTone + adjustToneAmount < mNoteRange)
               {
                  mSteps[i].mTone = mSteps[i].mTone + adjustToneAmount;
                  shouldUpdate = true;
               }

               if (GetKeyModifiers() == kModifier_Shift)
               {
                  int dir = 0;
                  if (key == OF_KEY_RIGHT)
                     dir = 1;

                  if (key == OF_KEY_LEFT)
                     dir = -1;

                  if (dir != 0 && mSteps[i].mVelocity > 0)
                  {
                     int newStep = (i + dir + mLength) % mLength;
                     mSteps[newStep].mTone = mSteps[i].mTone;
                     mSteps[newStep].mVelocity = mSteps[i].mVelocity;
                     mSteps[newStep].mTie = mSteps[i].mTie;
                     mSteps[i].mVelocity = 0;
                     shouldUpdate = true;
                  }
               }
               else
               {
                  if (key == OF_KEY_RIGHT)
                  {
                     mSteps[i].mTie = true;
                     shouldUpdate = true;
                  }

                  if (key == OF_KEY_LEFT)
                  {
                     mSteps[i].mTie = false;
                     shouldUpdate = true;
                  }
               }

               if (key == OF_KEY_RETURN)
               {
                  if (mSteps[i].mVelocity != kVelocityNormal)
                     mSteps[i].mVelocity = kVelocityNormal;
                  else
                     mSteps[i].mVelocity = kVelocityAccent;
                  shouldUpdate = true;
               }

               if (key == juce::KeyPress::backspaceKey || key == juce::KeyPress::deleteKey)
               {
                  mSteps[i].mVelocity = 0;
                  shouldUpdate = true;
               }

               if (key == '\\')
               {
                  if (i < (int)mSteps.size() - 1)
                  {
                     mSteps[i].mTie = true;
                     mSteps[i + 1].mTone = mSteps[i].mTone;
                     mSteps[i + 1].mVelocity = MIN(mSteps[i].mVelocity, kVelocityNormal);
                     shouldUpdate = true;
                  }
               }

               if (shouldUpdate)
               {
                  UpdateStepControls();
                  consumed = true;
               }
            }
         }
      }
   }

   if (!consumed)
      IDrawableModule::KeyPressed(key, isRepeat);
}

void BassLineSequencer::GridUpdated(UIGrid* grid, int col, int row, float value, float oldValue)
{
   for (int i = 0; i < (int)mStepControls.size(); ++i)
   {
      if (grid == mStepControls[i].mGridSquare)
      {
         if (i == kEditStepControlIndex)
         {
            if (mEditStepControl >= 0)
            {
               mStepControls[kEditStepControlIndex].mVelocity = value;
               mSteps[mEditStepControl + mEditPage * mNumVisibleStepControls].mVelocity = value;
               OnEditStepUpdated();
            }
         }
         else
         {
            mStepControls[i].mVelocity = value;
            mSteps[i + mEditPage * mNumVisibleStepControls].mVelocity = value;
         }
      }
   }
}

void BassLineSequencer::ButtonClicked(ClickButton* button, double time)
{
   if (button == mClearButton)
   {
      for (auto& step : mSteps)
      {
         step.mVelocity = 0;
         step.mTie = false;
      }

      UpdateStepControls();
   }
   if (button == mShiftLeftButton || button == mShiftRightButton)
   {
      int shift = (button == mShiftRightButton) ? 1 : -1;

      int start = (shift == 1) ? mLength - 1 : 0;
      int end = (shift == 1) ? 0 : mLength - 1;
      Step startVal = mSteps[start];
      for (int col = start; col != end; col -= shift)
         mSteps[col] = mSteps[col - shift];
      mSteps[end] = startVal;

      UpdateStepControls();
   }
   if (button == mRandomizeButton)
   {
      juce::Array<int> tones;
      for (int i = 0; i < mNoteRange; ++i)
         tones.add(i);
      for (int i = 0; i < (mNoteRange - mRandomVariety) && tones.size() > 1; ++i)
         tones.remove((gRandom() % (tones.size() - 1)) + 1); //always keep first entry (root note)

      for (auto& step : mSteps)
      {
         step.mTone = tones[gRandom() % tones.size()];
         if (ofRandom(1.0f) < mRandomDensity)
         {
            if (ofRandom(1.0f) <= mRandomAccents)
               step.mVelocity = kVelocityAccent;
            else
               step.mVelocity = ofRandom(1.0f) <= 0.3f ? kVelocityGhost : kVelocityNormal;
         }
         else
         {
            step.mVelocity = 0;
         }

         step.mTie = ofRandom(1.0f) <= mRandomTies ? true : false;
      }

      for (size_t i = 1; i < mSteps.size(); ++i)
      {
         if (mSteps[i].mVelocity == 0)
            mSteps[i - 1].mTie = false;
         else if (mSteps[i - 1].mTie && ofRandom(1.0f) < 0.5f)
            mSteps[i].mTone = mSteps[i - 1].mTone;
      }
   }
   UpdateStepControls();
}

void BassLineSequencer::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   if (list == mIntervalSelector)
   {
      TransportListenerInfo* transportListenerInfo = TheTransport->GetListenerInfo(this);
      if (transportListenerInfo != nullptr)
         transportListenerInfo->mInterval = mInterval;
   }
   if (list == mNoteModeSelector)
   {
      UpdatePitchLabels();
   }
   for (int i = 0; i < (int)mStepControls.size(); ++i)
   {
      if (list == mStepControls[i].mToneDropdown)
      {
         if (i == kEditStepControlIndex)
         {
            if (mEditStepControl >= 0 && mEditStepControl < (int)mSteps.size())
            {
               mSteps[mEditStepControl + mEditPage * mNumVisibleStepControls].mTone = mStepControls[i].mTone;
               OnEditStepUpdated();
            }
         }
         else
         {
            mSteps[i + mEditPage * mNumVisibleStepControls].mTone = mStepControls[i].mTone;
         }
      }
   }
}

void BassLineSequencer::IntSliderUpdated(IntSlider* slider, int oldVal, double time)
{
   if (slider == mLengthSlider)
   {
      mEditPageSlider->SetExtents(0, GetPageCount() - 1);
      if (mEditPage >= GetPageCount())
         mEditPage = GetPageCount() - 1;
      UpdateStepControls();
   }
   if (slider == mOctaveSlider)
   {
      UpdatePitchLabels();
   }
   if (slider == mEditPageSlider)
   {
      UpdateStepControls();
   }
   if (slider == mEditStepControlSlider || slider == mEditPageSlider)
   {
      if (mEditStepControl >= 0)
      {
         mStepControls[kEditStepControlIndex].mTone = mSteps[mEditStepControl + mEditPage * mNumVisibleStepControls].mTone;
         mStepControls[kEditStepControlIndex].mVelocity = mSteps[mEditStepControl + mEditPage * mNumVisibleStepControls].mVelocity;
         mStepControls[kEditStepControlIndex].mTie = mSteps[mEditStepControl + mEditPage * mNumVisibleStepControls].mTie;
         mStepControls[kEditStepControlIndex].mGridSquare->SetVal(0, 0, mSteps[mEditStepControl + mEditPage * mNumVisibleStepControls].mVelocity);
      }
   }
}

void BassLineSequencer::FloatSliderUpdated(FloatSlider* slider, float oldVal, double time)
{
}

void BassLineSequencer::CheckboxUpdated(Checkbox* checkbox, double time)
{
   for (int i = 0; i < (int)mStepControls.size(); ++i)
   {
      if (checkbox == mStepControls[i].mTieCheckbox)
      {
         if (i == kEditStepControlIndex)
         {
            if (mEditStepControl >= 0 && mEditStepControl < (int)mSteps.size())
            {
               mSteps[mEditStepControl + mEditPage * mNumVisibleStepControls].mTie = mStepControls[i].mTie;
               OnEditStepUpdated();
            }
         }
         else
         {
            mSteps[i + mEditPage * mNumVisibleStepControls].mTie = mStepControls[i].mTie;
         }
      }
   }
}

void BassLineSequencer::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadString("target", moduleInfo);
   mModuleSaveData.LoadInt("num_step_controls", moduleInfo, 16, 1, kMaxStepControls, K(isTextField));
   mModuleSaveData.LoadInt("note_range", moduleInfo, 22, 5, 64, K(isTextField));

   SetUpFromSaveData();
}

void BassLineSequencer::SetUpFromSaveData()
{
   SetUpPatchCables(mModuleSaveData.GetString("target"));
   mNumVisibleStepControls = mModuleSaveData.GetInt("num_step_controls");
   int newNoteRange = mModuleSaveData.GetInt("note_range");

   if (newNoteRange != mNoteRange)
   {
      mNoteRange = newNoteRange;
      UpdatePitchLabels();
      mRandomVarietySlider->SetExtents(1, mNoteRange);
   }

   mEditPageSlider->SetExtents(0, GetPageCount() - 1);
   if (mEditPage >= GetPageCount())
      mEditPage = GetPageCount() - 1;
}

void BassLineSequencer::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   for (auto& step : mSteps)
   {
      out << step.mTone;
      out << step.mVelocity;
      out << step.mTie;
   }
   out << mHasExternalPulseSource;
}

void BassLineSequencer::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   for (auto& step : mSteps)
   {
      in >> step.mTone;
      in >> step.mVelocity;
      in >> step.mTie;
   }

   in >> mHasExternalPulseSource;

   UpdateStepControls();
}
