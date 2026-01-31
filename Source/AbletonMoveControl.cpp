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
/*
  ==============================================================================

    AbletonMoveControl.cpp
    Created: 22 Apr 2025
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "AbletonMoveControl.h"
#include "SynthGlobals.h"
#include "OpenFrameworksPort.h"
#include "UIControlMacros.h"
#include "ModuleSaveDataPanel.h"
#include "QuickSpawnMenu.h"
#include "PatchCableSource.h"
#include "DropdownList.h"
#include "FloatSliderLFOControl.h"
#include "UserPrefsEditor.h"
#include "Snapshots.h"
#include "ADSRDisplay.h"
#include "Looper.h"
#include "NoteLooper.h"
#include "ControlRecorder.h"
#include "push2/JuceToPush2DisplayBridge.h"
#include "push2/Push2-Bitmap.h"
#include "push2/../../Push2-Display.h"
#include "AbletonDeviceShared.h"
#include "TrackOrganizer.h"
#include "IInputRecordable.h"
#include "Amplifier.h"
#include "AudioSend.h"
#include "SongBuilder.h"
#include "LooperRecorder.h"
#include "AudioSend.h"

using namespace AbletonDevice;

IUIControl* AbletonMoveControl::sBindToUIControl = nullptr;
namespace
{
   ableton::Push2DisplayBridge ThePushBridge; // The bridge allowing to use juce::graphics for push
}

AbletonMoveControl::AbletonMoveControl()
: IDrawableModule(100, 100)
, mDevice(this)
{
   Initialize();
}

AbletonMoveControl::~AbletonMoveControl()
{
}

void AbletonMoveControl::Exit()
{
   for (int i = 0; i < 128; ++i)
   {
      SetLed(i, 0);
      SetLed(i, 0);
   }

   mLCD.Clear();

   mDevice.DisconnectInput();
   mDevice.DisconnectOutput();
}

void AbletonMoveControl::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   ofColor cableColor = IDrawableModule::GetColor(kModuleCategory_Other);
   cableColor.a *= .3f;

   for (size_t i = 0; i < mTrackCables.size(); ++i)
   {
      mTrackCables[i] = new PatchCableSource(this, kConnectionType_Special);
      mTrackCables[i]->AddTypeFilter("trackorganizer");
      mTrackCables[i]->SetColor(cableColor);
      mTrackCables[i]->SetManualPosition(8, (int)i * 12 + 10);
      AddPatchCableSource(mTrackCables[i]);
   }

   int cableX = 30;
   int cableY = 10;
   ofVec2f controlModuleCablePos(cableX, cableY);
   for (size_t i = 0; i < mGlobalControlModuleCables.size(); ++i)
   {
      mGlobalControlModuleCables[i] = new PatchCableSource(this, kConnectionType_Special);
      mGlobalControlModuleCables[i]->SetColor(cableColor);
      mGlobalControlModuleCables[i]->SetManualPosition(cableX, cableY);
      AddPatchCableSource(mGlobalControlModuleCables[i]);
      cableX += 12;
   }

   cableX = 30;
   cableY += 30;

   mSongBuilderCable = new PatchCableSource(this, kConnectionType_Special);
   mSongBuilderCable->AddTypeFilter("songbuilder");
   mSongBuilderCable->SetColor(cableColor);
   mSongBuilderCable->SetManualPosition(cableX, cableY);
   AddPatchCableSource(mSongBuilderCable);

   cableX += 20;

   mOutputGainCable = new PatchCableSource(this, kConnectionType_Special);
   mOutputGainCable->AddTypeFilter("gain");
   mOutputGainCable->SetColor(cableColor);
   mOutputGainCable->SetManualPosition(cableX, cableY);
   AddPatchCableSource(mOutputGainCable);

   for (size_t i = 0; i < mGlobalGridInterfaceCables.size(); ++i)
   {
      mGlobalGridInterfaceCables[i] = new PatchCableSource(this, kConnectionType_Special);
      mGlobalGridInterfaceCables[i]->SetPredicateFilter(IsOfType<IAbletonGridController*>);
      mGlobalGridInterfaceCables[i]->SetColor(cableColor);
      mGlobalGridInterfaceCables[i]->SetManualPosition(controlModuleCablePos.x + i * 12, controlModuleCablePos.y + 12);
      AddPatchCableSource(mGlobalGridInterfaceCables[i]);
   }
}

void AbletonMoveControl::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   if (!ThePushBridge.IsInitialized())
   {
      ofSetColor(255, 0, 0, gModuleDrawAlpha);
      DrawTextNormal(mPushBridgeInitErrMsg, 3, 15);
   }

   if (mGlobalModuleIndex >= 0 && mGlobalModuleIndex < mGlobalControlModuleCables.size())
   {
      ofPushStyle();
      ofNoFill();
      ofSetColor(ofColor(255, 255, 255, 50));
      const auto* cableSource = mGlobalControlModuleCables[mGlobalModuleIndex];
      ofVec2f cablePos = cableSource->GetManualPosition();
      ofRect(cablePos.x - 6, cablePos.y - 6, 12, 24);
      ofPopStyle();
   }
}

void AbletonMoveControl::DrawModuleUnclipped()
{
   if (mDisplayModule != nullptr && !mDisplayModule->IsDeleted() &&
       mDisplayModule->GetOwningContainer() != nullptr && mDisplayModule->IsShowing())
   {
      ofPushMatrix();
      ofPushStyle();
      ofVec2f pos = GetPosition();
      ofTranslate(-pos.x, -pos.y);
      DrawDisplayModuleRect(mDisplayModule->GetRect(), 3);
      ofPopMatrix();
      ofPopStyle();
   }

   std::string tooltip = "";
   const PatchCableSource* hoverCable = nullptr;

   for (int i = 0; i < (size_t)mTrackCables.size(); ++i)
   {
      if (mTrackCables[i]->IsHovered())
      {
         hoverCable = mTrackCables[i];
         tooltip = "track " + ofToString(i);
      }
   }

   for (int i = 0; i < (size_t)mGlobalControlModuleCables.size(); ++i)
   {
      if (mGlobalControlModuleCables[i]->IsHovered())
      {
         hoverCable = mGlobalControlModuleCables[i];
         tooltip = "global module " + ofToString(i);
      }
   }

   for (int i = 0; i < (size_t)mGlobalGridInterfaceCables.size(); ++i)
   {
      if (mGlobalGridInterfaceCables[i]->IsHovered())
      {
         hoverCable = mGlobalGridInterfaceCables[i];
         tooltip = "grid interface " + ofToString(i);
      }
   }

   if (mSongBuilderCable->IsHovered())
   {
      hoverCable = mSongBuilderCable;
      tooltip = "song builder";
   }

   if (mOutputGainCable->IsHovered())
   {
      hoverCable = mOutputGainCable;
      tooltip = "output gain";
   }

   if (hoverCable != nullptr)
   {
      IClickable* target = hoverCable->GetTarget();
      if (target != nullptr)
         tooltip += std::string(": ") + target->Name();

      float width = GetStringWidth(tooltip);

      const ofVec2f kTooltipOffset(15, 8);
      ofVec2f pos = hoverCable->GetPosition() - GetPosition() + kTooltipOffset;

      ofFill();
      ofSetColor(50, 50, 50);
      ofRect(pos.x, pos.y, width + 10, 15);

      ofNoFill();
      ofSetColor(255, 255, 255);
      ofRect(pos.x, pos.y, width + 10, 15);

      ofSetColor(255, 255, 255);
      DrawTextNormal(tooltip, pos.x + 5, pos.y + 12);
   }

   if (mShowLCDOnScreen)
   {
      ofPushMatrix();
      // get to screen space
      ofTranslate(-GetPosition().x, -GetPosition().y);
      ofTranslate(-TheSynth->GetDrawOffset().x, -TheSynth->GetDrawOffset().y);
      ofScale(1 / gDrawScale, 1 / gDrawScale, 1 / gDrawScale);

      ofPushStyle();
      ofFill();
      const float kLcdX = 30;
      const float kLcdY = 75;
      const float kLcdPixelW = 4;
      const float kLcdPixelH = 4;
      const float kPixelSpacing = 0;
      for (int col = 0; col < mLCD.kMoveDisplayWidth; ++col)
      {
         for (int row = 0; row < mLCD.kMoveDisplayHeight; ++row)
         {
            if (mLCD.GetPixel(col, row) > 0)
               ofSetColor(255, 255, 255, 150);
            else
               ofSetColor(0, 0, 0, 150);
            ofRect(kLcdX + kLcdPixelW * col, kLcdY + kLcdPixelH * row, kLcdPixelW - kPixelSpacing, kLcdPixelH - kPixelSpacing, 0);
         }
      }
      ofPopStyle();

      ofPopMatrix();
   }
}

void AbletonMoveControl::DrawDisplayModuleRect(ofRectangle rect, float thickness)
{
   if (mDisplayModule->HasTitleBar())
   {
      rect.y -= IDrawableModule::TitleBarHeight();
      rect.height += IDrawableModule::TitleBarHeight();
   }
   ofSetColor(255, 255, 255, ofMap(sin(gTime / 1000 * PI * 2), -1, 1, 60, 100));
   ofSetLineWidth(thickness);
   ofNoFill();
   ofRect(rect.x - 3, rect.y - 3, rect.width + 6, rect.height + 6, 6);
}

void AbletonMoveControl::PostRender()
{
   if (ThePushBridge.IsInitialized())
      RenderPush2Display();
}

void AbletonMoveControl::KeyPressed(int key, bool isRepeat)
{
   IDrawableModule::KeyPressed(key, isRepeat);
}

void AbletonMoveControl::OnClicked(float x, float y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);

   if (!ThePushBridge.IsInitialized())
   {
      Initialize();
   }

   SendLeds(true);
}

void AbletonMoveControl::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadBool("show_lcd_on_screen", moduleInfo, false);

   SetUpFromSaveData();
}

void AbletonMoveControl::SetUpFromSaveData()
{
   mShowLCDOnScreen = mModuleSaveData.GetBool("show_lcd_on_screen");
}

void AbletonMoveControl::SaveLayout(ofxJSONElement& moduleInfo)
{
}

void AbletonMoveControl::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   out << mAutoZoomToTrack;
}

void AbletonMoveControl::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   LoadStateValidate(rev <= GetModuleSaveStateRev());

   if (rev >= 3)
      in >> mAutoZoomToTrack;
}

bool AbletonMoveControl::Initialize()
{
   mLCD.Init();

   if (!ThePushBridge.IsInitialized())
   {
      if (auto result = ThePushBridge.Init(ableton::DeviceType::Move); result.Failed())
      {
         mPushBridgeInitErrMsg = result.GetDescription();
         ofLog() << mPushBridgeInitErrMsg;
         return false;
      }
      ofLog() << "ableton move connected";
   }

   const std::vector<std::string>& devices = mDevice.GetPortList(false);
   for (int i = 0; i < devices.size(); ++i)
   {
#if JUCE_WINDOWS
      if (strcmp(devices[i].c_str(), "Ableton Move MIDI") == 0)
#else
      if (strcmp(devices[i].c_str(), "Ableton Move Live Port") == 0)
#endif
      {
         mDevice.ConnectOutput(i);
         mDevice.ConnectInput(devices[i].c_str());
         break;
      }
   }

   return true;
}

int AbletonMoveControl::GetDisplayKnobIndex()
{
   if (mLastAdjustedKnobTime > gTime - 200)
      return mLastAdjustedKnobIndex;

   if (mMostRecentlyTouchedKnobIndex != -1 && GetButtonState(kMainEncoderTouchSection + mMostRecentlyTouchedKnobIndex))
      return mMostRecentlyTouchedKnobIndex;

   for (int i = 0; i < kNumMainEncoders; ++i)
   {
      if (GetButtonState(kMainEncoderTouchSection + i))
         return i;
   }

   return -1;
}

bool AbletonMoveControl::ShouldDisplayMixer()
{
   for (int i = 0; i < kNumTrackButtons; ++i)
   {
      if (GetButtonState(kTrackButtonSection + kNumTrackButtons - 1 - i))
      {
         if (GetButtonState(kVolumeEncoderTouch))
         {
            int trackIndex = i + mTrackRowOffset;
            mLastGainAdjustTrackIndex = trackIndex;
            mLastGainAdjustTrackTime = gTime;
            return true;
         }
      }
   }

   return mSelectedTrackRow == kTrackRowMixer || mLastGainAdjustTrackTime > gTime - 300;
}

bool AbletonMoveControl::ShouldDisplaySnapshotView()
{
   if (GetButtonState(kDotButton))
      return true;
   if (mSelectedTrackRow == kTrackRowMixer)
      return true;
   if (GetButtonState(kClickyEncoderButton) && !mShowSoundSelector)
      return true;
   return false;
}

float AbletonMoveControl::GetModuleViewOffset() const
{
   if (mSelectedTrackRow == kTrackRowGlobal)
      return mGlobalModuleViewOffset;
   if (mSelectedTrackRow >= 0 && mSelectedTrackRow < mTrackCables.size())
   {
      if (const auto* trackRow = dynamic_cast<TrackOrganizer*>(mTrackCables[mSelectedTrackRow]->GetTarget()))
         return trackRow->GetModuleViewOffset();
   }

   return 0.0f;
}

void AbletonMoveControl::SetModuleViewOffset(float offset)
{
   if (mSelectedTrackRow == kTrackRowGlobal)
   {
      mGlobalModuleViewOffset = offset;
   }
   else if (mSelectedTrackRow >= 0 && mSelectedTrackRow < mTrackCables.size())
   {
      if (auto* trackRow = dynamic_cast<TrackOrganizer*>(mTrackCables[mSelectedTrackRow]->GetTarget()))
         trackRow->SetModuleViewOffset(offset);
   }
}

void AbletonMoveControl::DrawToFramebuffer()
{
   if (gTime < mScreenOverrideTimeout)
      return;

   mLCD.Clear();

   bool needToDraw = true;

   if (needToDraw && mShowSoundSelector)
   {
      TrackOrganizer* trackRow = GetActiveTrackRow();
      IUIControl* soundSelector = trackRow ? trackRow->GetSoundSelector() : nullptr;
      if (soundSelector != nullptr)
      {
         const int kMaxDisplayEntries = 5;
         int numValues = soundSelector->GetNumValues();
         int displayOffset = std::clamp(mSoundSelectorIndex - 2, 0, std::max(numValues - kMaxDisplayEntries, 0));
         int y = 12;
         for (int i = displayOffset; i < numValues && i < displayOffset + kMaxDisplayEntries; ++i)
         {
            float normalized = float(i) / (numValues - 1);
            int value = soundSelector->GetValueForMidiCC(normalized);
            std::string label = soundSelector->GetDisplayValue(value);
            mLCD.DrawLCDText(label.c_str(), 13, y, value == soundSelector->GetValue() ? LCDFONT_STYLE_UNDERLINE : LCDFONT_STYLE_REGULAR);
            if (i == mSoundSelectorIndex)
               mLCD.DrawArrow(9, y - 4, 4, false, true);
            y += 12;
         }
      }
      needToDraw = false;
   }

   if (needToDraw && mGridControlInterface != nullptr && mGridControlInterface->HasHighPriorityAbletonMoveScreenUpdate(this))
   {
      if (mGridControlInterface->UpdateAbletonMoveScreen(this, &mLCD))
         needToDraw = false;
   }

   //scroll module offset
   int trackRowSpacingY = 8;
   bool holdingModuleSelectInput = GetButtonState(kClickyEncoderTouch) && GetButtonState(kClickyEncoderButton);
   bool moduleIsScrolling = fabsf(mTrackRowOffsetSmoothed - mTrackRowOffset) > .05f;
   if (needToDraw && (holdingModuleSelectInput || moduleIsScrolling))
   {
      mTrackRowOffsetSmoothed = ofLerp(mTrackRowOffsetSmoothed, mTrackRowOffset, 0.3f);
      for (int i = 0; i < (int)mTrackCables.size(); ++i)
      {
         mLCD.DrawRect(5, 2 + i * trackRowSpacingY, 4, 5, i == mSelectedTrackRow);
         TrackOrganizer* trackRow = dynamic_cast<TrackOrganizer*>(mTrackCables[i]->GetTarget());
         if (trackRow != nullptr)
            mLCD.DrawLCDText(trackRow->GetTrackName().c_str(), 10, 1 + i * trackRowSpacingY + 6);
      }
      mLCD.DrawRect(3, 0 + int(mTrackRowOffsetSmoothed * trackRowSpacingY), mLCD.kMoveDisplayWidth - 3, trackRowSpacingY * kNumTrackButtons + 1, !K(filled));

      needToDraw = false;
   }

   bool canScrollModules = (mSelectedTrackRow == kTrackRowGlobal || mSelectedTrackRow >= 0);
   if (needToDraw && ((GetButtonState(kClickyEncoderTouch) && canScrollModules) || gTime < mDisplayModuleSelectTimeout))
   {
      //module select within track/global
      std::string trackName;
      TrackOrganizer* trackRow = GetActiveTrackRow();
      int moduleIndex;
      if (trackRow != nullptr)
      {
         moduleIndex = trackRow->GetModuleIndex();
         trackName = trackRow->GetTrackName();
      }
      else
      {
         moduleIndex = mGlobalModuleIndex;
         trackName = "global";
      }

      mLCD.DrawLCDText((trackName + ":").c_str(), 5, 12, LCDFONT_STYLE_REGULAR);
      for (int i = 0; i < (int)mTrackControlLayout.size(); ++i)
      {
         mLCD.DrawLCDText(mTrackControlLayout[i].mName.c_str(), 14, 20 + i * 10, i == moduleIndex ? LCDFONT_STYLE_UNDERLINE : LCDFONT_STYLE_REGULAR);
         for (int j = 0; j < mTrackControlLayout[i].mPageCount; ++j)
         {
            mLCD.DrawRect(84 + j * 9, 14 + i * 10, 5, 5, i == moduleIndex && int(GetModuleViewOffset()) == j);
         }
      }

      needToDraw = false;
   }

   if (needToDraw && ShouldDisplaySnapshotView() && GetButtonState(kClickyEncoderTouch))
   {
      mLCD.DrawLCDText(("snapshot offset: " + ofToString(mSnapshotOffset)).c_str(), 5, 10, LCDFONT_STYLE_REGULAR);
      needToDraw = false;
   }

   int mixerAdjustTrackIndex = -1;
   float mixerAdjustValue = 0;
   if (needToDraw && ShouldDisplayMixer())
   {
      std::string title = "mixer";
      if (mShiftHeld)
         title = "sends";
      mLCD.DrawLCDText(title.c_str(), 5, 10, LCDFONT_STYLE_UNDERLINE);

      const int kBarPadBelow = 3;
      const int kMaxBarHeight = mLCD.kMoveDisplayHeight - 16;
      for (int i = 0; i < (int)mTrackCables.size(); ++i)
      {
         int x = 5 + i * 16;

         TrackOrganizer* track = dynamic_cast<TrackOrganizer*>(mTrackCables[i]->GetTarget());
         if (track != nullptr)
         {
            Amplifier* gain = track->GetGain();
            AudioSend* send = track->GetSend();
            FloatSlider* adjustSlider = nullptr;
            if (mShiftHeld)
               adjustSlider = send != nullptr ? send->GetAmountSlider() : nullptr;
            else
               adjustSlider = gain != nullptr ? gain->GetGainSlider() : nullptr;
            if (gain != nullptr && adjustSlider != nullptr)
            {
               bool isBeingAdjusted = false;
               if (GetButtonState(kMainEncoderTouchSection + i) ||
                   (i == mLastGainAdjustTrackIndex && mLastGainAdjustTrackTime > gTime - 300))
               {
                  isBeingAdjusted = true;
                  mixerAdjustTrackIndex = i;
                  mixerAdjustValue = adjustSlider != nullptr ? adjustSlider->GetValue() : 0;
               }

               float level, watermarkLevel;
               gain->GetLevel(level, watermarkLevel);
               int sliderPosition = std::clamp(int(adjustSlider->GetMidiValue() * kMaxBarHeight), 0, kMaxBarHeight);
               int levelHeight = std::clamp(int(level * kMaxBarHeight), 0, kMaxBarHeight);
               int watermarkLevelHeight = std::clamp(int(watermarkLevel * kMaxBarHeight), 0, kMaxBarHeight);
               mLCD.DrawRect(x, mLCD.kMoveDisplayHeight - kBarPadBelow - kMaxBarHeight, 4, kMaxBarHeight, !K(filled));
               mLCD.DrawRect(x, mLCD.kMoveDisplayHeight - kBarPadBelow - levelHeight, 4, levelHeight, K(filled));
               mLCD.DrawRect(x + 5, mLCD.kMoveDisplayHeight - kBarPadBelow - watermarkLevelHeight, 2, 1, K(filled));
               //arrow
               int arrowSize = 4;
               bool fillArrow = false;
               if (isBeingAdjusted)
               {
                  arrowSize = 5;
                  fillArrow = true;
               }
               mLCD.DrawArrow(x + 7, mLCD.kMoveDisplayHeight - kBarPadBelow - sliderPosition, arrowSize, true, fillArrow);
               //mLCD.DrawRect(x + 7, mLCD.kMoveDisplayHeight - kBarPadBelow - gainPosition - 3, 4, 6, K(filled));
            }
         }
      }
      if (mixerAdjustTrackIndex != -1)
      {
         TrackOrganizer* track = dynamic_cast<TrackOrganizer*>(mTrackCables[mixerAdjustTrackIndex]->GetTarget());
         if (track != nullptr)
         {
            std::string display = track->GetTrackName();
            display += "  " + ofToString(mixerAdjustValue, 2);
            mLCD.DrawLCDText(display.c_str(), 52, 10, LCDFONT_STYLE_REGULAR);
         }
      }

      needToDraw = false;
   }

   if (needToDraw && mGridControlInterface != nullptr && !mGridControlInterface->HasHighPriorityAbletonMoveScreenUpdate(this))
   {
      if (mGridControlInterface->UpdateAbletonMoveScreen(this, &mLCD))
         needToDraw = false;
   }

   if (needToDraw)
   {
      if (mDisplayModule != nullptr || mGridControlInterface != nullptr)
      {
         std::string display = "";
         if (mDisplayModuleContext != "")
            display = mDisplayModuleContext + ": ";

         IDrawableModule* nameModule = nullptr;
         if (mDisplayModule != nullptr)
            nameModule = mDisplayModule;
         else if (mGridControlInterface != nullptr)
            nameModule = dynamic_cast<IDrawableModule*>(mGridControlInterface);

         if (nameModule != nullptr)
            display += nameModule->Name();

         mLCD.DrawLCDText(display.c_str(), 5, 12, LCDFONT_STYLE_UNDERLINE);
      }

      if (mDisplayModule != nullptr)
      {
         if (GetDisplayKnobIndex() == -1)
         {
            for (int i = 0; i < kNumMainEncoders; ++i)
            {
               int controlIndex = GetControlOffset() + i;
               if (controlIndex < (int)mControls.size())
               {
                  auto* control = mControls[controlIndex];
                  mLCD.DrawLCDText(control->GetDisplayName().c_str(), i * 14 + 4, 26 + (i % 4) * 10);
               }
            }

            const int kModuleViewBarX = 4;
            const int kModuleViewBarY = 58;
            const int kModuleViewBarWidth = 124;
            const int kModuleViewBarHeight = 6;
            mLCD.DrawRect(kModuleViewBarX, kModuleViewBarY, kModuleViewBarWidth, kModuleViewBarHeight, false);
            float numControlsQuantized = ceil((float)mControls.size() / kNumMainEncoders) * kNumMainEncoders;
            float offsetStart = GetControlOffset() / numControlsQuantized;
            float offsetEnd = MIN(GetControlOffset() + kNumMainEncoders, numControlsQuantized) / numControlsQuantized;
            mLCD.DrawRect(kModuleViewBarX + offsetStart * kModuleViewBarWidth, kModuleViewBarY + 2, (offsetEnd - offsetStart) * kModuleViewBarWidth, kModuleViewBarHeight - 4, true);
            if (GetButtonState(kVolumeEncoderTouch))
               mLCD.DrawRect(kModuleViewBarX + (GetModuleViewOffset() * kNumMainEncoders / numControlsQuantized) * kModuleViewBarWidth, kModuleViewBarY, 1, kModuleViewBarHeight, true);
         }
      }

      int displayKnobIndex = GetDisplayKnobIndex();
      if (displayKnobIndex != -1)
      {
         int controlIndex = displayKnobIndex + GetControlOffset();
         if (controlIndex < (int)mControls.size())
         {
            auto* control = mControls[controlIndex];
            mLCD.DrawLCDText(control->GetDisplayName().c_str(), 5, 26);
            mLCD.DrawLCDText(control->GetDisplayValue(control->GetValue()).c_str(), 5, 40);

            int sliderX = 5;
            int sliderY = 42;
            int sliderW = 122;
            int sliderH = 10;
            int sliderMinPos = sliderX + 2;
            int sliderMaxPos = sliderX + sliderW - 3;
            mLCD.DrawRect(sliderX, sliderY, sliderW, sliderH, false);
            mLCD.DrawRect(ofLerp(sliderMinPos, sliderMaxPos, control->GetMidiValue()), sliderY + 2, 1, sliderH - 4, false);

            int numValues = control->GetNumValues();
            if (numValues > 0 && numValues < 20)
            {
               for (int i = 1; i < numValues; ++i)
               {
                  int x = ofLerp(sliderMinPos, sliderMaxPos, (i - 0.5f) / (numValues - 1));
                  mLCD.DrawRect(x, sliderY, 1, 2, false);
                  mLCD.DrawRect(x, sliderY + sliderH - 2, 1, 2, false);
               }
            }
         }
      }
   }

   TrackOrganizer* track = GetActiveTrackRow();

   float measurePos = TheTransport->GetMeasurePos(gTime);
   int wrapLengthMeasures = 4;
   if (track != nullptr)
   {
      if (const auto& recordable = track->GetRecorder())
      {
         int recordableLength = (int)recordable->GetRecordingLengthMeasures();
         if (recordableLength > 0)
            wrapLengthMeasures = recordableLength;
      }
   }
   mLCD.DrawRect(0, 0, ((measurePos + (TheTransport->GetMeasure(gTime) % wrapLengthMeasures)) / wrapLengthMeasures) * mLCD.kMoveDisplayWidth, 1, true);
   float numBeats = TheTransport->GetTimeSigTop();
   int beat = int(measurePos * numBeats);
   float beatWidth = mLCD.kMoveDisplayWidth / numBeats;
   mLCD.DrawRect(beat * beatWidth, 1, beatWidth, 1, true);

   if (!ShouldDisplayMixer())
   {

      if (track != nullptr)
      {
         Amplifier* gain = track->GetGain();
         if (gain != nullptr)
         {
            float level, watermarkLevel;
            gain->GetLevel(level, watermarkLevel);
            mLCD.DrawRect(0, 2, std::clamp(int(level * mLCD.kMoveDisplayWidth), 0, mLCD.kMoveDisplayWidth), 1, true);
            mLCD.DrawRect(std::clamp(int(watermarkLevel * mLCD.kMoveDisplayWidth), 0, mLCD.kMoveDisplayWidth), 2, 1, 2, true);
         }
      }
   }

   mLCD.DrawRect(0, 0, 1, trackRowSpacingY * (int)mTrackCables.size(), K(filled));
   mLCD.DrawRect(0, int(mTrackRowOffsetSmoothed * trackRowSpacingY), 3, trackRowSpacingY * kNumTrackButtons + 1, K(filled));
   if (mSelectedTrackRow >= 0 && int(gTime / 200) % 2 == 0)
   {
      float trackY = trackRowSpacingY * mSelectedTrackRow;
      for (int i = 1; i < trackRowSpacingY - 1; ++i)
         mLCD.TogglePixel(1, trackY + i);
   }

   if (mLissajousDisplayMode == LissajousDisplayMode::Always ||
       (ShouldDisplayMixer() && mLissajousDisplayMode == LissajousDisplayMode::MixerTrack))
   {
      auto* buffer = TheSynth->GetGlobalRecordBuffer();

      TrackOrganizer* trackRowToDisplay = nullptr;
      if (!ShouldDisplayMixer())
         trackRowToDisplay = GetActiveTrackRow();
      else if (mixerAdjustTrackIndex != -1)
         trackRowToDisplay = dynamic_cast<TrackOrganizer*>(mTrackCables[mixerAdjustTrackIndex]->GetTarget());
      if (trackRowToDisplay != nullptr)
      {
         auto* gain = trackRowToDisplay != nullptr ? trackRowToDisplay->GetGain() : nullptr;
         if (gain != nullptr)
            buffer = gain->GetVizBuffer();
      }

      if (mLissajousDisplayMode == LissajousDisplayMode::MixerTrack && trackRowToDisplay == nullptr)
      {
         //don't display, this is the global buffer
      }
      else
      {
         const int kDelaySamps = 90;
         const float kScale = 1.2f;
         const int kCenterX = mLCD.kMoveDisplayWidth / 2;
         const int kCenterY = mLCD.kMoveDisplayHeight / 2;
         bool lissajousPixels[mLCD.kMoveDisplayWidth * mLCD.kMoveDisplayHeight];
         for (int i = 0; i < mLCD.kMoveDisplayWidth * mLCD.kMoveDisplayHeight; ++i)
            lissajousPixels[i] = false;

         int numPoints = MIN(buffer->Size() - kDelaySamps - 1, .02f * gSampleRate);
         for (int i = 100; i < numPoints; ++i)
         {
            int x = kCenterX + buffer->GetSample(i, 0) * kScale * mLCD.kMoveDisplayHeight;
            int y = kCenterY + buffer->GetSample(i + kDelaySamps, 1) * kScale * mLCD.kMoveDisplayHeight;
            if (x >= 0 && x < mLCD.kMoveDisplayWidth && y >= 0 && y < mLCD.kMoveDisplayHeight)
               lissajousPixels[x + y * mLCD.kMoveDisplayWidth] = true;
         }

         for (int i = 0; i < mLCD.kMoveDisplayWidth * mLCD.kMoveDisplayHeight; ++i)
         {
            if (lissajousPixels[i])
               mLCD.TogglePixel(i % mLCD.kMoveDisplayWidth, i / mLCD.kMoveDisplayWidth);
         }
      }
   }

   if (gTime < mTemporaryScreenMessageTimeout)
   {
      mLCD.ClearRect(1, 2, mLCD.kMoveDisplayWidth - 2, 13);
      mLCD.DrawRect(1, 2, mLCD.kMoveDisplayWidth - 2, 13, !K(filled));
      mLCD.DrawLCDText(mTemporaryScreenMessage.c_str(), 4, 12, LCDFONT_STYLE_REGULAR);
   }
}

void AbletonMoveControl::UpdateLeds()
{
   TrackOrganizer* activeTrackRow = GetActiveTrackRow();

   //step buttons
   SongBuilder* songBuilder = dynamic_cast<SongBuilder*>(mSongBuilderCable->GetTarget());
   for (int i = 0; i < kNumStepButtons; ++i)
   {
      int colorIndex = kColorOff;
      int colorIndex2 = -1;
      if (songBuilder != nullptr && ShouldDisplaySnapshotView())
      {
         if (songBuilder->GetQueuedScene() == i)
         {
            colorIndex = kColorGreen;
            colorIndex2 = kColorGrey;
         }
         else if (songBuilder->GetCurrentScene() == i)
         {
            colorIndex = kColorWhite;
         }
         else if (i < songBuilder->GetNumScenes())
         {
            colorIndex = kColorDarkGrey;
         }
      }
      else
      {
         auto* recorder = activeTrackRow != nullptr ? activeTrackRow->GetRecorder() : nullptr;
         if (recorder)
         {
            LooperRecorder* looperRecorder = dynamic_cast<LooperRecorder*>(recorder);
            if (looperRecorder == nullptr)
            {
               Looper* looper = dynamic_cast<Looper*>(recorder);
               if (looper != nullptr)
                  looperRecorder = looper->GetRecorder();
            }
            if (looperRecorder != nullptr)
            {
               if (i < 5)
               {
                  int numBars;
                  if (i == 0)
                     numBars = 16;
                  else if (i == 1)
                     numBars = 8;
                  else if (i == 2)
                     numBars = 4;
                  else if (i == 3)
                     numBars = 2;
                  else
                     numBars = 1;

                  bool flip = (TheTransport->GetMeasure(gTime) / numBars) % 2 == 0;
                  colorIndex = flip ? kColorBlue : kColorLightBlue;
               }
            }
         }
      }

      SetLed(i + kStepButtonSection, colorIndex, colorIndex2);
      SetLed(i + kStepButtonLedSection, 0);
   }

   SetLed(kPlayButton, TheSynth->IsAudioPaused() ? 127 : 120);
   SetLed(kUpButton, 0);
   SetLed(kDownButton, 0);
   SetLed(kLeftButton, 0);
   SetLed(kRightButton, 0);
   if (GetButtonState(kCaptureButton))
   {
      SetLed(kPageLeftButton, 0, 127);
      SetLed(kPageRightButton, 0, 127);
      SetLed(kOctaveUpButton, 0, 127);
      SetLed(kOctaveDownButton, 0, 127);
   }
   else if (GetDisplayKnobIndex() != -1)
   {
      SetLed(kPageLeftButton, 0, 127);
      SetLed(kPageRightButton, 0, 127);
      SetLed(kOctaveUpButton, 0, 127);
      SetLed(kOctaveDownButton, 0, 127);
      SetLed(kMoveUndoButton, 0, 127);
   }
   else
   {
      SetLed(kPageLeftButton, 0);
      SetLed(kPageRightButton, 0);
      SetLed(kOctaveUpButton, mTrackRowOffset > 0 ? 127 : 0);
      SetLed(kOctaveDownButton, mTrackRowOffset < (int)mTrackCables.size() - kNumTrackButtons ? 127 : 0);
      SetLed(kMoveUndoButton, 0);

      Snapshots* snapshots = activeTrackRow != nullptr ? activeTrackRow->GetSnapshots() : nullptr;
      if (snapshots != nullptr)
         SetLed(kDotButton, activeTrackRow->GetColorIndex());
      else
         SetLed(kDotButton, 0);
   }

   SetLed(kMoveDeleteButton, 0);

   if (ShouldDisplaySnapshotView())
   {
      for (int i = 0; i < kNumTrackButtons; ++i)
      {
         const int kNumCols = 8;
         int colors[kNumCols]{ 0, 0, 0, 0, 0, 0, 0, 0 };
         int colors2[kNumCols]{ -1, -1, -1, -1, -1, -1, -1, -1 };
         TrackOrganizer* trackRow = dynamic_cast<TrackOrganizer*>(mTrackCables[i + mTrackRowOffset]->GetTarget());
         if (trackRow != nullptr)
         {
            Snapshots* snapshots = trackRow->GetSnapshots();
            if (snapshots != nullptr)
            {
               for (int col = 0; col < kNumCols; ++col)
               {
                  int snapshotIdx = col + mSnapshotOffset;
                  if (snapshots->GetCurrentSnapshot() == snapshotIdx)
                  {
                     colors[col] = kColorWhite;
                     colors2[col] = kColorDarkGrey;
                  }
                  else if (snapshots->HasSnapshot(snapshotIdx))
                  {
                     if (snapshots->GetLabel(snapshotIdx) == "off" || snapshots->GetLabel(snapshotIdx) == "none" || snapshots->GetLabel(snapshotIdx) == "0")
                        colors[col] = kColorDarkGrey;
                     else
                        colors[col] = trackRow->GetColorIndex();
                  }
               }
            }
         }

         for (int col = 0; col < kNumCols; ++col)
         {
            SetLed(kMovePadsSection + (kNumTrackButtons - 1 - i) * 8 + col, colors[col], colors2[col]);
         }
      }
   }
   else if (mGridControlInterface != nullptr)
   {
      mGridControlInterface->UpdateAbletonGridLeds(this);
   }
   else
   {
      for (int i = kMovePadsSection; i < kMovePadsSection + kNumMovePads; ++i)
         SetLed(i, kColorOff);
   }

   bool soundSelectorAvailable = false;
   TrackOrganizer* trackRow = GetActiveTrackRow();
   IUIControl* soundSelector = trackRow ? trackRow->GetSoundSelector() : nullptr;
   if (soundSelector != nullptr)
      soundSelectorAvailable = true;

   SetLed(kShiftButton, 127, mShiftHeld || mBottomRowMode ? 0 : -1);

   if (mAutoZoomToTrack)
      SetLed(kLedNew, 127);
   else if (mBottomRowMode)
      SetLed(kLedNew, 10);
   else
      SetLed(kLedNew, 0);

   if (mDisplayModule == TheTransport)
      SetLed(kLedTempo, 127, 30);
   else if (mBottomRowMode)
      SetLed(kLedTempo, 10);
   else
      SetLed(kLedTempo, 0);

   if (mShowSoundSelector)
      SetLed(kLedGroove, 127, 30);
   else if (mBottomRowMode)
      SetLed(kLedGroove, soundSelectorAvailable ? 10 : 0);
   else
      SetLed(kLedGroove, 0);

   if (mDisplayModule == TheScale)
      SetLed(kLedScale, 127, 30);
   else if (mBottomRowMode)
      SetLed(kLedScale, 10);
   else
      SetLed(kLedScale, 0);

   if (mLissajousDisplayMode != LissajousDisplayMode::Never)
      SetLed(kLedStar, 127, mBottomRowMode ? 30 : -1);
   else if (mBottomRowMode)
      SetLed(kLedStar, 10);
   else
      SetLed(kLedStar, 0);

   if (mBottomRowMode)
   {
      SetLed(kButtonNew, kColorLightGrey, 0);
      SetLed(kButtonSettings, kColorOff);
      SetLed(kButtonBranch, kColorOff);
      SetLed(kButtonDowel, kColorOff);
      SetLed(kButtonTempo, kColorLightGrey, 0);
      SetLed(kButtonMetronome, kColorOff);
      SetLed(kButtonGroove, soundSelectorAvailable ? kColorLightGrey : kColorOff, 0);
      SetLed(kButtonLayout, kColorOff);
      SetLed(kButtonScale, kColorLightGrey, 0);
      SetLed(kButtonFullVelocity, kColorOff);
      SetLed(kButtonRepeat, kColorOff);
      SetLed(kButtonChord, kColorOff);
      SetLed(kButtonStar, kColorLightGrey, 0);
      SetLed(kButtonAdd, kColorOff);
      SetLed(kButtonDuplicate, kColorOff);
      SetLed(kButtonQuantize, kColorOff);
   }

   //test led colors
   //SetLed(92, (int)mModuleListOffset);
   //ofLog() << (int)mModuleListOffset;

   for (int i = 0; i < (int)mTrackCables.size(); ++i)
   {
      int trackIndex = i + mTrackRowOffset;
      if (trackIndex >= 0 && trackIndex < (int)mTrackCables.size())
      {
         TrackOrganizer* trackRow = dynamic_cast<TrackOrganizer*>(mTrackCables[trackIndex]->GetTarget());
         int buttonNumber = (kTrackButtonSection + kNumTrackButtons - 1) - i;
         if (trackRow != nullptr)
            SetLed(buttonNumber, trackRow->GetColorIndex(), trackIndex == mSelectedTrackRow ? 0 : -1);
         else
            SetLed(buttonNumber, 0);
      }
   }

   SetLed(kBackButton, 127, mSelectedTrackRow == kTrackRowGlobal ? 20 : -1);
   SetLed(kHamburgerButton, 127, mSelectedTrackRow == kTrackRowMixer ? 20 : -1);

   int recordColor1 = kColorOff;
   int recordColor2 = -1;
   if (activeTrackRow)
   {
      auto* recorder = activeTrackRow->GetRecorder();
      if (recorder != nullptr)
      {
         if (recorder->IsRecording())
         {
            if (mShiftHeld)
            {
               recordColor1 = kColorMustard;
               recordColor2 = kColorDarkGrey;
            }
            else
            {
               recordColor1 = kColorRed;
               recordColor2 = kColorDeepRed;
            }
         }
         else
         {
            if (mShiftHeld)
            {
               recordColor1 = kColorBlue;
               recordColor2 = kColorWhite;
            }
            else
            {
               recordColor1 = kColorDarkGrey;
            }
         }
      }
   }
   SetLed(kCircleButton, recordColor1, recordColor2);

   if (mSelectedTrackRow == kTrackRowMixer)
   {
      for (int i = 0; i < kNumMainEncoders; ++i)
      {
         const auto* trackRow = dynamic_cast<TrackOrganizer*>(mTrackCables[i]->GetTarget());
         SetLed(i + kMainEncoderSection, trackRow != nullptr ? trackRow->GetColorIndex() : kColorOff);
      }
   }
   else
   {
      for (int i = 0; i < kNumMainEncoders; ++i)
      {
         int color = kColorLightGrey;
         const auto* trackRow = GetActiveTrackRow();
         if (trackRow != nullptr)
            color = trackRow->GetColorIndex();
         SetLed(i + kMainEncoderSection, i + GetControlOffset() < (int)mControls.size() ? color : kColorOff);
      }
   }

   SetLed(kCaptureButton, 127);
}

void AbletonMoveControl::RenderPush2Display()
{
   DrawToFramebuffer();

   uint8_t* lcdPixels = mLCD.GetPixels();

   uint16_t* pixels = ThePushBridge.GetDisplay()->GetRawBitmap();
   memset(pixels, 0, sizeof(uint16_t) * mLCD.GetNumDisplayPixels());
   const int kPixelBlockRows = 8;
   const int kPixelBlockColumns = 64;
   const int kPixelBlockCellWidth = 2;
   const int kPixelBlockCellHeight = 8;
   for (int row = 0; row < kPixelBlockRows; ++row)
   {
      for (int col = 0; col < kPixelBlockColumns; ++col)
      {
         int cellIndex = col + row * kPixelBlockColumns;
         int pixelXStart = col * kPixelBlockCellWidth;
         int pixelYStart = row * kPixelBlockCellHeight;
         for (int i = 0; i < kPixelBlockCellWidth * kPixelBlockCellHeight; ++i)
         {
            int pixelX = pixelXStart + i / kPixelBlockCellHeight;
            int pixelY = pixelYStart + i % kPixelBlockCellHeight;
            int pixelIndex = (pixelX + pixelY * AbletonMoveLCD::kMoveDisplayWidth) * 4;
            if (lcdPixels[pixelIndex] > 100)
               pixels[cellIndex] |= 1 << i;
         }
      }
   }
}

void AbletonMoveControl::Poll()
{
   bool controlsChanged = false;
   if (mDisplayModule != nullptr && mDisplayModule->HasPush2OverrideControls())
   {
      std::vector<IUIControl*> desiredControls;
      mDisplayModule->GetPush2OverrideControls(desiredControls);
      if (desiredControls.size() != mControls.size())
      {
         controlsChanged = true;
      }
      else
      {
         for (size_t i = 0; i < desiredControls.size(); ++i)
         {
            if (desiredControls[i] != mControls[i])
            {
               controlsChanged = true;
               break;
            }
         }
      }
   }

   if (mDisplayModule != nullptr && mDisplayModule->HasPush2OverrideControls() != mDisplayModuleIsShowingOverrideControls)
   {
      controlsChanged = true;
      mDisplayModuleIsShowingOverrideControls = mDisplayModule->HasPush2OverrideControls();
   }

   if (controlsChanged)
      UpdateControlList();

   UpdateLeds();

   if (GetButtonState(kCaptureButton))
   {
      const float kMovementRate = 600.0f;
      if (GetButtonState(kPageRightButton))
         TheSynth->PanView(-kMovementRate * ofGetLastFrameTime(), 0);
      if (GetButtonState(kPageLeftButton))
         TheSynth->PanView(kMovementRate * ofGetLastFrameTime(), 0);
      if (GetButtonState(kOctaveUpButton))
         TheSynth->PanView(0, kMovementRate * ofGetLastFrameTime());
      if (GetButtonState(kOctaveDownButton))
         TheSynth->PanView(0, -kMovementRate * ofGetLastFrameTime());
   }

   SendLeds(false);
}

void AbletonMoveControl::SetDisplayModule(IDrawableModule* module, bool addToHistory)
{
   mDisplayModule = module;
   mDisplayModuleContext = "";
   UpdateControlList();
}

void AbletonMoveControl::SetDisplayModuleWithContext(IDrawableModule* module, std::string context)
{
   SetDisplayModule(module);
   mDisplayModuleContext = context;
}

void AbletonMoveControl::DisplayScreenMessage(std::string message, float durationMs /*=500*/)
{
   mTemporaryScreenMessage = message;
   mTemporaryScreenMessageTimeout = gTime + durationMs;
}

//static
bool AbletonMoveControl::IsDisplayableControl(IUIControl* control)
{
   const IDrawableModule* owningModule = control->GetModuleParent();
   bool isEnabledCheckbox = owningModule != nullptr && control == owningModule->GetEnabledCheckbox();
   if (!isEnabledCheckbox &&
       (control->IsSliderControl() || control->IsButtonControl()) &&
       (control->GetShouldSaveState() || dynamic_cast<ClickButton*>(control) != nullptr))
      return true;
   return false;
}

void AbletonMoveControl::UpdateControlList()
{
   mControls.clear();
   std::vector<IUIControl*> controls;
   if (mDisplayModule != nullptr)
   {
      if (mDisplayModule->HasPush2OverrideControls())
         mDisplayModule->GetPush2OverrideControls(controls);
      else
         controls = mDisplayModule->GetUIControls();
   }

   for (int i = 0; i < controls.size(); ++i)
   {
      if (IsDisplayableControl(controls[i]))
         mControls.push_back(controls[i]);
   }
}

void AbletonMoveControl::DetermineTrackControlLayout()
{
   std::vector<IDrawableModule*> trackModules;
   std::vector<IAbletonGridController*> gridInterfaces;
   TrackOrganizer* trackRow = GetActiveTrackRow();
   if (trackRow != nullptr)
   {
      trackModules = trackRow->GetControlModules();
      gridInterfaces = trackRow->GetGridInterfaces();
   }
   else
   {
      for (int i = 0; i < (int)mGlobalControlModuleCables.size(); ++i)
         trackModules.push_back(dynamic_cast<IDrawableModule*>(mGlobalControlModuleCables[i]->GetTarget()));
      for (int i = 0; i < (int)mGlobalGridInterfaceCables.size(); ++i)
         gridInterfaces.push_back(dynamic_cast<IAbletonGridController*>(mGlobalGridInterfaceCables[i]->GetTarget()));
   }

   mTrackControlLayout.clear();
   for (int i = 0; i < (int)trackModules.size(); ++i)
   {
      int numControls = 0;
      std::vector<IUIControl*> controls;
      if (trackModules[i] != nullptr)
      {
         if (trackModules[i]->HasPush2OverrideControls())
            trackModules[i]->GetPush2OverrideControls(controls);
         else
            controls = trackModules[i]->GetUIControls();
      }

      for (int j = 0; j < controls.size(); ++j)
      {
         if (IsDisplayableControl(controls[j]))
            ++numControls;
      }

      IDrawableModule* nameModule = trackModules[i];
      if (nameModule == nullptr && gridInterfaces[i] != nullptr)
         nameModule = dynamic_cast<IDrawableModule*>(gridInterfaces[i]);
      if (nameModule == nullptr)
         break; // no control module or grid interface in this slot

      int numPages = ((numControls - 1) / kNumMainEncoders) + 1;
      mTrackControlLayout.push_back(TrackLayoutEntry(nameModule->Name(), numPages));
   }
}

int AbletonMoveControl::GetControlOffset() const
{
   return int(GetModuleViewOffset()) * kNumMainEncoders;
}

void AbletonMoveControl::SetLed(int index, int color, int flashColor /*=-1*/)
{
   assert(index >= 0 && index < 128 * 2);

   mControlState[index].mQueuedLedState = LedState(color, flashColor);
}

void AbletonMoveControl::SetLedFlashColor(int index, int flashColor)
{
   assert(index >= 0 && index < 128 * 2);

   mControlState[index].mQueuedLedState = LedState(mControlState[index].mQueuedLedState.color, flashColor);
}

void AbletonMoveControl::SendLeds(bool force)
{
   for (int index = 0; index < mControlState.size(); ++index)
   {
      if (mControlState[index].mLedState != mControlState[index].mQueuedLedState || force)
      {
         mControlState[index].mLedState = mControlState[index].mQueuedLedState;

         int channel = 1;
         if (mControlState[index].mLedState.flashColor != -1)
            channel = 10;

         //bool isPulse = (channel >= 7 && channel <= 11);
         if (index < 128)
         {
            if (mControlState[index].mLedState.flashColor != -1)
               mDevice.SendNote(gTime, index, mControlState[index].mLedState.flashColor, false, -1);
            mDevice.SendNote(gTime, index, mControlState[index].mLedState.color, false, channel);
         }
         else
         {
            if (mControlState[index].mLedState.flashColor != -1)
               mDevice.SendCC(index - 128, mControlState[index].mLedState.flashColor);
            mDevice.SendCC(index - 128, mControlState[index].mLedState.color, channel);
         }
      }
   }
}

bool AbletonMoveControl::GetButtonState(int index) const
{
   return mControlState[index].mButtonState;
}

void AbletonMoveControl::SetGridControlInterface(IAbletonGridController* controller)
{
   mGridControlInterface = controller;
}

void AbletonMoveControl::OnMidiNote(MidiNote& note)
{
   //ofLog() << "AbletonMoveControl::OnMidiNote() " << note.mPitch << " " << note.mVelocity;

   mControlState[note.mPitch].mButtonState = note.mVelocity > 0;

   bool handled = false;
   if (mBottomRowMode)
   {
      if (note.mPitch == kButtonNew)
      {
         if (note.mVelocity > 0)
            mAutoZoomToTrack = !mAutoZoomToTrack;
         handled = true;
      }
      if (note.mPitch == kButtonTempo)
      {
         if (note.mVelocity > 0)
         {
            SetActiveTrackRow(kTrackRowTransport, false);
         }
         else
         {
            bool wasHold = WasPeekHold(note.mPitch);

            //if we held it, then we were just "peeking" into the transport page, so switch back
            if (wasHold)
               SetActiveTrackRow(mPreviousSelectedTrackRow, false);
         }
         handled = true;
      }
      if (note.mPitch == kButtonGroove)
      {
         if (note.mVelocity > 0)
         {
            mShowSoundSelector = true;
            mSoundSelectorIndex = 0;
            SetDisplayModule(nullptr);

            TrackOrganizer* trackRow = GetActiveTrackRow();
            IUIControl* soundSelector = trackRow ? trackRow->GetSoundSelector() : nullptr;
            if (soundSelector != nullptr)
            {
               int numValues = soundSelector->GetNumValues();
               for (int i = 0; i < numValues; ++i)
               {
                  float normalized = float(i) / (numValues - 1);
                  int value = soundSelector->GetValueForMidiCC(normalized);
                  if (value == soundSelector->GetValue())
                  {
                     mSoundSelectorIndex = i;
                     break;
                  }
               }
            }
         }
         handled = true;
      }
      if (note.mPitch == kButtonScale)
      {
         if (note.mVelocity > 0)
         {
            SetDisplayModule(TheScale);
            mPreviousSelectedTrackRow = mSelectedTrackRow;
            mShowSoundSelector = false;
            //SetActiveTrackRow(kTrackRowScale, false);
         }
         else
         {
            bool wasHold = WasPeekHold(note.mPitch);

            //if we held it, then we were just "peeking" into the scale page, so switch back
            if (wasHold)
               SetActiveTrackRow(mPreviousSelectedTrackRow, false);
         }
         handled = true;
      }
      if (note.mPitch == kButtonStar)
      {
         if (note.mVelocity > 0)
         {
            if (mLissajousDisplayMode == LissajousDisplayMode::Always)
               mLissajousDisplayMode = LissajousDisplayMode::Never;
            else if (mLissajousDisplayMode == LissajousDisplayMode::MixerTrack)
               mLissajousDisplayMode = LissajousDisplayMode::Always;
            else if (mLissajousDisplayMode == LissajousDisplayMode::Never)
               mLissajousDisplayMode = LissajousDisplayMode::MixerTrack;

            if (mLissajousDisplayMode == LissajousDisplayMode::Always)
               DisplayScreenMessage("lissajous: always");
            else if (mLissajousDisplayMode == LissajousDisplayMode::MixerTrack)
               DisplayScreenMessage("lissajous: mixer track");
            else if (mLissajousDisplayMode == LissajousDisplayMode::Never)
               DisplayScreenMessage("lissajous: hide");
         }
         handled = true;
      }

      if (handled)
         mBottomRowMode = false;
   }

   if (!handled)
   {
      if (!ShouldDisplaySnapshotView() && mGridControlInterface != nullptr)
      {
         bool handled = mGridControlInterface->OnAbletonGridControl(this, note.mPitch, note.mVelocity);
         if (handled)
            return;
      }

      if (note.mPitch >= kMainEncoderTouchSection && note.mPitch < kMainEncoderTouchSection + kNumMainEncoders) //main encoders
      {
         int controlIndex = note.mPitch - kMainEncoderTouchSection + GetControlOffset();
         if (controlIndex < mControls.size())
         {
            if (note.mVelocity > 0)
            {
               mControls[controlIndex]->StartBeacon();

               /*if (mLFOButtonHeld)
               {
                  FloatSlider* slider = dynamic_cast<FloatSlider*>(mControls[controlIndex]);
                  if (slider != nullptr)
                  {
                     bool hadLFO = (slider->GetLFO() != nullptr);
                     FloatSliderLFOControl* lfo = slider->AcquireLFO();
                     if (!hadLFO)
                        lfo->SetLFOEnabled(true);
                     SetDisplayModule(lfo, true);
                  }
               }
               else if (mAutomateButtonHeld)
               {
                  if (mControls[controlIndex] != nullptr && mCurrentControlRecorder == nullptr)
                  {
                     std::vector<IDrawableModule*> modules;
                     TheSynth->GetAllModules(modules);
                     for (auto* searchModule : modules)
                     {
                        ControlRecorder* controlRecorderModule = dynamic_cast<ControlRecorder*>(searchModule);
                        if (controlRecorderModule != nullptr)
                        {
                           if (controlRecorderModule->GetPatchCableSource()->GetTarget() == mControls[controlIndex])
                              mCurrentControlRecorder = controlRecorderModule;
                        }
                     }

                     if (mCurrentControlRecorder == nullptr) //couldn't find one, so make one
                     {
                        ModuleFactory::Spawnable spawnable;
                        spawnable.mLabel = "controlrecorder";
                        mCurrentControlRecorder = dynamic_cast<ControlRecorder*>(TheSynth->SpawnModuleOnTheFly(spawnable, mControls[controlIndex]->GetRect().getMaxX() + 40, mControls[controlIndex]->GetPosition().y));
                        mCurrentControlRecorder->SetTarget(mControls[controlIndex]);
                     }

                     mCurrentControlRecorder->SetEnabled(true);
                     mCurrentControlRecorder->SetRecording(true);
                  }
               }
               else if (mInMidiControllerBindMode)
               {
                  sBindToUIControl = mControls[controlIndex];
               }

               if (mHeldModule && AllowRepatch())
               {
                  PatchCableSource* cable = mHeldModule->GetPatchCableSource();
                  if (mHeldModulePatchCableIndex > 0)
                     cable = mHeldModule->GetPatchCableSources()[mHeldModulePatchCableIndex];
                  if (cable != nullptr)
                  {
                     cable->FindValidTargets();
                     if (cable->IsValidTarget(mControls[controlIndex]))
                        cable->SetTarget(mControls[controlIndex]);
                  }
               }*/
            }
            else
            {
               if (sBindToUIControl == mControls[controlIndex])
                  sBindToUIControl = nullptr;
            }
         }

         if (note.mVelocity > 0)
            mMostRecentlyTouchedKnobIndex = note.mPitch - kMainEncoderTouchSection;
      }
      else if (note.mPitch >= kMovePadsSection && note.mPitch < kMovePadsSection + kNumMovePads && ShouldDisplaySnapshotView()) //pads
      {
         int gridIndex = note.mPitch - kMovePadsSection;
         int row = (3 - gridIndex / 8) + mTrackRowOffset;
         int col = gridIndex % 8 + mSnapshotOffset;
         if (row < (int)mTrackCables.size())
         {
            TrackOrganizer* trackRow = dynamic_cast<TrackOrganizer*>(mTrackCables[row]->GetTarget());
            if (trackRow != nullptr)
            {
               Snapshots* snapshots = trackRow->GetSnapshots();
               if (snapshots != nullptr)
               {
                  if (GetButtonState(kShiftButton))
                  {
                     DisplayScreenMessage(snapshots->GetLabel(col));
                  }
                  else if (GetButtonState(kDotButton))
                  {
                     snapshots->StoreSnapshot(col, true);
                  }
                  else if (snapshots->HasSnapshot(col))
                  {
                     snapshots->SetSnapshot(col, gTime);
                     DisplayScreenMessage(snapshots->GetLabel(col));
                  }
               }
            }
         }
      }
      else if (note.mPitch >= kStepButtonSection && note.mPitch < kStepButtonSection + kNumStepButtons)
      {
         if (note.mVelocity > 0)
         {
            if (ShouldDisplaySnapshotView())
            {
               SongBuilder* songBuilder = dynamic_cast<SongBuilder*>(mSongBuilderCable->GetTarget());
               if (songBuilder != nullptr)
               {
                  int scene = note.mPitch - kStepButtonSection;
                  if (scene >= 0 && scene < songBuilder->GetNumScenes())
                     songBuilder->SetScene(scene, NextBufferTime(false));
               }
            }
            else
            {
               auto* activeTrackRow = GetActiveTrackRow();
               auto* recorder = activeTrackRow != nullptr ? activeTrackRow->GetRecorder() : nullptr;
               if (recorder)
               {
                  LooperRecorder* looperRecorder = dynamic_cast<LooperRecorder*>(recorder);
                  Looper* looper = dynamic_cast<Looper*>(recorder);
                  if (looperRecorder == nullptr && looper != nullptr)
                     looperRecorder = looper->GetRecorder();
                  if (looperRecorder != nullptr)
                  {
                     int buttonIndex = note.mPitch - kStepButtonSection;
                     if (buttonIndex < 5)
                     {
                        int numBars;
                        if (buttonIndex == 0)
                           numBars = 16;
                        else if (buttonIndex == 1)
                           numBars = 8;
                        else if (buttonIndex == 2)
                           numBars = 4;
                        else if (buttonIndex == 3)
                           numBars = 2;
                        else
                           numBars = 1;

                        looperRecorder->SetNumBars(numBars);
                        if (looper)
                           looperRecorder->Commit(looper);
                        else
                           looperRecorder->Commit(looperRecorder->GetNextCommitTarget());
                     }
                  }
               }
            }
         }
      }
      else
      {
         //ofLog() << "unhandled note " << note.mPitch << " " << note.mVelocity;
      }
   }

   mControlState[note.mPitch].mLastChangeTime = gTime;
}

void AbletonMoveControl::OnMidiControl(MidiControl& control)
{
   //ofLog() << "AbletonMoveControl::OnMidiControl() " << control.mControl << " " << control.mValue;
   control.mControl += 128;

   mControlState[control.mControl].mButtonState = control.mValue > 0;

   for (int i = 0; i < kNumTrackButtons; ++i)
   {
      if (GetButtonState(kTrackButtonSection + kNumTrackButtons - 1 - i))
      {
         if (control.mControl == kVolumeEncoderTurn)
         {
            int trackIndex = i + mTrackRowOffset;
            if (trackIndex >= 0 && trackIndex < (int)mTrackCables.size())
            {
               TrackOrganizer* track = dynamic_cast<TrackOrganizer*>(mTrackCables[trackIndex]->GetTarget());
               if (track != nullptr)
               {
                  Amplifier* gain = track->GetGain();
                  AudioSend* send = track->GetSend();
                  FloatSlider* adjustSlider = nullptr;
                  if (mShiftHeld)
                     adjustSlider = send != nullptr ? send->GetAmountSlider() : nullptr;
                  else
                     adjustSlider = gain != nullptr ? gain->GetGainSlider() : nullptr;
                  if (adjustSlider != nullptr)
                  {
                     AdjustControlWithEncoder(adjustSlider, control.mValue, K(ignoreShift));
                     mLastGainAdjustTrackIndex = trackIndex;
                     mLastGainAdjustTrackTime = gTime;
                     /*mLCD.Clear();
                     mLCD.DrawRect(5, 4, mLCD.kMoveDisplayWidth - 6, 4, !K(filled));
                     mLCD.DrawRect(5, 4, (mLCD.kMoveDisplayWidth - 6) * ofMap(gainSlider->GetValue(), gainSlider->GetMin(), gainSlider->GetMax(), 0, 1), 4, K(filled));
                     mLCD.DrawText(("track " + ofToString(trackIndex) + " gain: " + ofToString(gainSlider->GetValue())).c_str(), 5, 23, LCDFONT_STYLE_REGULAR);
                     mScreenOverrideTimeout = gTime + 500;*/
                  }
               }
            }
            return;
         }
      }
   }

   if (GetButtonState(kHamburgerButton))
   {
      if (control.mControl == kVolumeEncoderTurn)
      {
         Amplifier* outputGain = dynamic_cast<Amplifier*>(mOutputGainCable->GetTarget());
         FloatSlider* gainSlider = outputGain != nullptr ? outputGain->GetGainSlider() : nullptr;
         if (gainSlider != nullptr)
         {
            AdjustControlWithEncoder(gainSlider, control.mValue);
            mLCD.Clear();
            mLCD.DrawRect(3, 4, mLCD.kMoveDisplayWidth - 6, 4, !K(filled));
            mLCD.DrawRect(3, 4, (mLCD.kMoveDisplayWidth - 6) * gainSlider->GetMidiValue(), 4, K(filled));
            mLCD.DrawLCDText(("output gain: " + ofToString(gainSlider->GetValue(), 2)).c_str(), 5, 23, LCDFONT_STYLE_REGULAR);
            mScreenOverrideTimeout = gTime + 500;
         }
      }
   }

   if (GetButtonState(kCaptureButton))
   {
      if (control.mControl == kVolumeEncoderTurn)
      {
         float increment = control.mValue < 64 ? control.mValue : control.mValue - 128;
         increment *= mShiftHeld ? .0005f : .005f;
         TheSynth->ZoomView(increment, false);
      }
   }

   if (mGridControlInterface != nullptr)
   {
      bool handled = mGridControlInterface->OnAbletonGridControl(this, control.mControl, control.mValue);
      if (handled)
         return;
   }

   int displayKnobIndex = GetDisplayKnobIndex();

   if (control.mControl >= kMainEncoderSection && control.mControl < kMainEncoderSection + kNumMainEncoders) //main encoders
   {
      int controlIndex = control.mControl - kMainEncoderSection + GetControlOffset();

      if (ShouldDisplayMixer())
      {
         if (controlIndex >= 0 && controlIndex < (int)mTrackCables.size())
         {
            TrackOrganizer* track = dynamic_cast<TrackOrganizer*>(mTrackCables[controlIndex]->GetTarget());
            if (track != nullptr)
            {
               Amplifier* gain = track->GetGain();
               AudioSend* send = track->GetSend();
               FloatSlider* adjustSlider = nullptr;
               if (mShiftHeld)
                  adjustSlider = send != nullptr ? send->GetAmountSlider() : nullptr;
               else
                  adjustSlider = gain != nullptr ? gain->GetGainSlider() : nullptr;
               if (adjustSlider != nullptr)
                  AdjustControlWithEncoder(adjustSlider, control.mValue, K(ignoreShift));
            }
         }
      }
      else
      {
         bool justResetParameter = gTime - mLastResetTime < 1000;
         if (controlIndex < mControls.size() && !justResetParameter)
         {
            AdjustControlWithEncoder(mControls[controlIndex], control.mValue);
            mLastAdjustedKnobIndex = control.mControl - kMainEncoderSection;
            mLastAdjustedKnobTime = gTime;
            mMostRecentlyTouchedKnobIndex = mLastAdjustedKnobIndex;
         }
      }
   }
   else if (control.mControl >= kTrackButtonSection && control.mControl < kTrackButtonSection + kNumTrackButtons)
   {
      int index = (kNumTrackButtons - 1) - (control.mControl - kTrackButtonSection) + mTrackRowOffset;
      if (control.mValue > 0)
      {
         SetActiveTrackRow(index, false);
      }
      else
      {
         bool wasHold = WasPeekHold(control.mControl);

         //if we held it, then we were just "peeking" into the other track, so switch back
         if (wasHold)
            SetActiveTrackRow(mPreviousSelectedTrackRow, false);

         //if we quickly pressed a button for a track we were already holding, reset module index within row
         if (!wasHold && index == mPreviousSelectedTrackRow)
            SetActiveTrackRow(index, true);

         if (!wasHold && mAutoZoomToTrack)
            ZoomToTrack(GetActiveTrackRow());
      }
   }
   else if (control.mControl == kBackButton)
   {
      if (control.mValue > 0)
      {
         SetActiveTrackRow(kTrackRowGlobal, false);
      }
      else
      {
         bool wasHold = WasPeekHold(control.mControl);

         //if we held it, then we were just "peeking" into the global page, so switch back
         if (wasHold)
            SetActiveTrackRow(mPreviousSelectedTrackRow, false);

         //if we quickly pressed a button for a track we were already holding, reset global module index within row
         if (!wasHold && mPreviousSelectedTrackRow == kTrackRowGlobal)
            SetActiveTrackRow(kTrackRowGlobal, true);
      }
   }
   else if (control.mControl == kHamburgerButton)
   {
      if (control.mValue > 0)
      {
         SetActiveTrackRow(kTrackRowMixer, false);
      }
      else
      {
         bool wasHold = WasPeekHold(control.mControl);

         //if we held it, then we were just "peeking" into the global page, so switch back
         if (wasHold)
            SetActiveTrackRow(mPreviousSelectedTrackRow, false);
      }
   }
   else if (control.mControl == kClickyEncoderButton)
   {
      if (mShowSoundSelector)
      {
         TrackOrganizer* trackRow = GetActiveTrackRow();
         IUIControl* soundSelector = trackRow ? trackRow->GetSoundSelector() : nullptr;
         if (soundSelector != nullptr)
         {
            int numValues = soundSelector->GetNumValues();
            soundSelector->SetFromMidiCC(float(mSoundSelectorIndex) / (numValues - 1), control.mValue, gTime);
         }
      }
   }
   else if (control.mControl == kClickyEncoderTurn)
   {
      int direction = control.mValue < 64 ? 1 : -1;

      TrackOrganizer* trackRow = GetActiveTrackRow();
      if (mShowSoundSelector)
      {
         IUIControl* soundSelector = trackRow ? trackRow->GetSoundSelector() : nullptr;
         if (soundSelector != nullptr)
         {
            mSoundSelectorIndex = std::clamp(mSoundSelectorIndex + direction, 0, soundSelector->GetNumValues() - 1);
         }
      }
      else if (GetButtonState(kClickyEncoderButton))
      {
         mTrackRowOffset = std::clamp(mTrackRowOffset + direction, 0, (int)mTrackCables.size() - kNumTrackButtons);
      }
      else if (ShouldDisplaySnapshotView())
      {
         const int kNumCols = 8;
         int maxNumSnapshots = 8;
         for (auto* trackCable : mTrackCables)
         {
            TrackOrganizer* trackRow = dynamic_cast<TrackOrganizer*>(trackCable->GetTarget());
            if (trackRow != nullptr)
            {
               Snapshots* snapshots = trackRow->GetSnapshots();
               if (snapshots != nullptr)
               {
                  maxNumSnapshots = std::max(maxNumSnapshots, snapshots->GetSize());
               }
            }
         }
         mSnapshotOffset = std::clamp(mSnapshotOffset + direction, 0, maxNumSnapshots - kNumCols);
      }
      else if (trackRow != nullptr)
      {
         if (mPageWithinModules)
         {
            int numModulePages = 1;
            if (trackRow->GetModuleIndex() < (int)mTrackControlLayout.size())
               numModulePages = mTrackControlLayout[trackRow->GetModuleIndex()].mPageCount;
            if (trackRow->GetModuleViewOffset() + direction < 0 || trackRow->GetModuleViewOffset() + direction >= numModulePages)
            {
               bool changed = trackRow->AdjustModuleIndex(direction);
               if (changed)
               {
                  if (direction > 0)
                     trackRow->SetModuleViewOffset(0);
                  else
                     trackRow->SetModuleViewOffset(trackRow->GetModuleIndex() < (int)mTrackControlLayout.size() ? mTrackControlLayout[trackRow->GetModuleIndex()].mPageCount - 1 : 0);

                  SetDisplayModuleWithContext(trackRow->GetCurrentModule(), trackRow->GetTrackName());
                  SetGridControlInterface(trackRow->GetCurrentGridInterface());
               }
            }
            else
            {
               trackRow->SetModuleViewOffset(int(trackRow->GetModuleViewOffset()) + direction);
            }
         }
         else
         {
            bool changed = trackRow->AdjustModuleIndex(direction);
            if (changed)
            {
               trackRow->SetModuleViewOffset(0);
               SetDisplayModuleWithContext(trackRow->GetCurrentModule(), trackRow->GetTrackName());
               SetGridControlInterface(trackRow->GetCurrentGridInterface());
            }
         }
      }
      else
      {
         int numModulePages = 1;
         if (mGlobalModuleIndex < (int)mTrackControlLayout.size())
            numModulePages = mTrackControlLayout[mGlobalModuleIndex].mPageCount;
         if (mGlobalModuleViewOffset + direction < 0 || mGlobalModuleViewOffset + direction >= numModulePages)
         {
            bool changed = AdjustGlobalModuleIndex(direction);
            if (changed)
            {
               if (direction > 0)
                  mGlobalModuleViewOffset = 0;
               else
                  mGlobalModuleViewOffset = mGlobalModuleIndex < (int)mTrackControlLayout.size() ? mTrackControlLayout[mGlobalModuleIndex].mPageCount - 1 : 0;

               SetDisplayModuleWithContext(GetCurrentGlobalModule(), "global");
               SetGridControlInterface(GetCurrentGlobalGridInterface());
            }
         }
         else
         {
            mGlobalModuleViewOffset = int(mGlobalModuleViewOffset) + direction;
         }
      }
   }
   else if (control.mControl == kVolumeEncoderTurn)
   {
      float increment = control.mValue < 64 ? control.mValue : control.mValue - 128;
      increment *= 0.07f;
      SetModuleViewOffset(ofClamp(GetModuleViewOffset() + increment, 0, MAX(0, (int)mControls.size() / kNumMainEncoders)));
   }
   /*else if (control.mControl == kSetupButton && control.mValue > 0)
   {
      mInMidiControllerBindMode = !mInMidiControllerBindMode;
   }
   else if (control.mControl == kNewButton)
   {
      mNewButtonHeld = control.mValue > 0;
   }
   else if (control.mControl == kDeleteButton)
   {
      mDeleteButtonHeld = control.mValue > 0;
   }
   else if (control.mControl == kFixedLengthButton)
   {
      mLFOButtonHeld = control.mValue > 0;
   }
   else if (control.mControl == kAutomateButton)
   {
      mAutomateButtonHeld = control.mValue > 0;

      if (!mAutomateButtonHeld && mCurrentControlRecorder != nullptr)
      {
         mCurrentControlRecorder->SetRecording(false);
         mCurrentControlRecorder = nullptr;
      }
   }
   else if (control.mControl == kMasterButton)
   {
      mAddModuleBookmarkButtonHeld = control.mValue > 0;
   }
   else if (control.mControl == kTapTempoButton)
   {
      if (gHoveredModule != nullptr && gHoveredModule != mDisplayModule)
         SetDisplayModule(gHoveredModule, true);
   }
   else if (control.mControl == kDeviceButton || control.mControl == kMixButton || control.mControl == kBrowseButton || control.mControl == kClipButton)
   {
      if (control.mValue > 0)
      {
         int index = 0;
         if (control.mControl == kDeviceButton)
            index = 0;
         if (control.mControl == kMixButton)
            index = 1;
         if (control.mControl == kBrowseButton)
            index = 2;
         if (control.mControl == kClipButton)
            index = 3;

         if (mAddTrackHeld)
         {
            if (mDisplayModuleSnapshots == nullptr)
            {
               ModuleFactory::Spawnable spawnable;
               spawnable.mLabel = "snapshots";
               mDisplayModuleSnapshots = dynamic_cast<Snapshots*>(TheSynth->SpawnModuleOnTheFly(spawnable, mDisplayModule->GetRect().getMaxX() + 40, mDisplayModule->GetPosition().y));
               mDisplayModuleSnapshots->AddSnapshotTarget(mDisplayModule);
            }

            if (mDisplayModuleSnapshots != nullptr)
               mDisplayModuleSnapshots->StoreSnapshot(index, true);
         }
         else
         {
            if (mDisplayModuleSnapshots != nullptr)
               mDisplayModuleSnapshots->SetSnapshot(index, gTime);
         }
      }
   }
   else if (control.mControl == kUpButton || control.mControl == kDownButton || control.mControl == kLeftButton || control.mControl == kRightButton)
   {
      if (control.mValue > 0)
      {
         ofVec2f direction;
         if (control.mControl == kUpButton)
            direction.y -= 1;
         if (control.mControl == kDownButton)
            direction.y += 1;
         if (control.mControl == kLeftButton)
            direction.x -= 1;
         if (control.mControl == kRightButton)
            direction.x += 1;

         if (mShiftHeld && mDisplayModule)
         {
            ofVec2f pos = mDisplayModule->GetPosition();
            pos += direction * 50;
            mDisplayModule->SetPosition(pos.x, pos.y);
         }
         else
         {
            TheSynth->PanView(direction.x * -100, direction.y * -100);
         }
      }
   }*/
   else if (control.mControl == kPageLeftButton ||
            control.mControl == kPageRightButton ||
            control.mControl == kOctaveUpButton ||
            control.mControl == kOctaveDownButton ||
            control.mControl == kMoveUndoButton)
   {
      if (GetButtonState(kCaptureButton))
      {
         //view panning happening in Poll()
      }
      else if (displayKnobIndex != -1)
      {
         if (control.mValue > 0)
         {
            int controlIndex = displayKnobIndex + GetControlOffset();
            if (controlIndex < mControls.size())
            {
               if (control.mControl == kPageRightButton)
                  mControls[controlIndex]->Increment(1);
               if (control.mControl == kPageLeftButton)
                  mControls[controlIndex]->Increment(-1);
               if (control.mControl == kOctaveUpButton)
                  mControls[controlIndex]->Double();
               if (control.mControl == kOctaveDownButton)
                  mControls[controlIndex]->Halve();
               if (control.mControl == kMoveUndoButton)
               {
                  mControls[controlIndex]->ResetToOriginal();
                  mLastResetTime = gTime;
               }
            }

            MidiController::sLastActivityUIControl = mControls[controlIndex];
            MidiController::sLastConnectedActivityTime = gTime;
         }
      }
      else
      {
         if (control.mValue > 0)
         {
            if (control.mControl == kOctaveUpButton)
               mTrackRowOffset = std::clamp(mTrackRowOffset - 1, 0, (int)mTrackCables.size() - kNumTrackButtons);
            if (control.mControl == kOctaveDownButton)
               mTrackRowOffset = std::clamp(mTrackRowOffset + 1, 0, (int)mTrackCables.size() - kNumTrackButtons);
         }
      }
   }
   else if (control.mControl == kDotButton)
   {
      if (control.mValue == 0) //on release
      {
         auto* trackRow = GetActiveTrackRow();
         Snapshots* snapshots = trackRow != nullptr ? trackRow->GetSnapshots() : nullptr;
         if (snapshots != nullptr)
            snapshots->StoreSnapshot(snapshots->GetCurrentSnapshot(), false);
      }
   }
   /*else if (control.mControl == kScaleButton)
   {
      if (control.mValue > 0 && mGridControlModule != nullptr)
      {
         SetDisplayModule(mGridControlModule, true);
      }
   }
   else if (control.mControl >= kQuantizeButtonSection && control.mControl < kQuantizeButtonSection + kNumQuantizeButtons) //quantization level buttons to the right of the main grid, used to bookmark modules
   {
      if (control.mValue > 0)
      {
         int i = control.mControl - kQuantizeButtonSection;
         if (mAddModuleBookmarkButtonHeld)
            BookmarkModuleToSlot(i, mDisplayModule);
         if (mDeleteButtonHeld)
            BookmarkModuleToSlot(i, nullptr);
         else
            SwitchToBookmarkedModule(i);
      }
   }*/
   else if (control.mControl == kShiftButton)
   {
      mShiftHeld = control.mValue > 0;

      if (control.mValue > 0)
      {
         if (gTime - mControlState[control.mControl].mLastChangeTime < 200) //double-click
            mBottomRowMode = true;
         else
            mBottomRowMode = false;
      }
   }
   /*else if (control.mControl == kAddTrackButton)
   {
      mAddTrackHeld = control.mValue > 0;
   }
   else if (control.mControl == kSelectButton)
   {
      if (control.mValue > 0 && mDisplayModule != nullptr)
      {
         if (mHeldModule != nullptr && mHeldModule->GetPatchCableSources().size() > 1)
         {
            mHeldModulePatchCableIndex = (mHeldModulePatchCableIndex + 1) % mHeldModule->GetPatchCableSources().size();
            mTextPopup = "setting selected cable output index to " + ofToString(mHeldModulePatchCableIndex);
            mTextPopupTime = gTime;
         }
         else if (mHeldKnobIndex == -1)
         {
            ofRectangle rect = mDisplayModule->GetRect();
            TheSynth->PanTo(rect.getCenter().x, rect.getCenter().y);
         }
         else
         {
            int controlIndex = mHeldKnobIndex + GetControlOffset();
            if (controlIndex < mControls.size() && mScreenDisplayMode == ScreenDisplayMode::kNormal)
            {
               mControls[controlIndex]->ResetToOriginal();
               mLastResetTime = gTime;
            }
         }
      }
   }*/
   else if (control.mControl == kPlayButton)
   {
      if (control.mValue > 0)
      {
         if (TheSynth->IsAudioPaused())
            TheTransport->Reset();
         else
            TheSynth->SetAudioPaused(true);
      }
   }
   else if (control.mControl == kCircleButton)
   {
      if (control.mValue > 0)
      {
         auto* trackRow = GetActiveTrackRow();
         auto* recorder = trackRow != nullptr ? trackRow->GetRecorder() : nullptr;
         if (recorder != nullptr)
         {
            if (recorder->IsRecording())
            {
               if (mShiftHeld)
                  recorder->CancelRecording();
               else
                  recorder->SetRecording(false);
            }
            else
            {
               if (mShiftHeld)
                  recorder->ClearRecording();
               else
                  recorder->SetRecording(true);
            }
         }
      }
   }
   else if (control.mControl == kCaptureButton)
   {
      if (control.mValue > 0 &&
          gTime - mControlState[control.mControl].mLastChangeTime < 200) //double-click
      {
         auto* track = GetActiveTrackRow();
         if (track != nullptr)
            ZoomToTrack(track);
      }
   }
   else
   {
      ofLog() << "unhandled control " << (control.mControl - 128) << " " << control.mValue;
   }

   mControlState[control.mControl].mLastChangeTime = gTime;
}

void AbletonMoveControl::OnMidiPitchBend(MidiPitchBend& pitchBend)
{
   if (mGridControlInterface != nullptr)
   {
      bool handled = mGridControlInterface->OnAbletonGridControl(this, kPitchBendIndex, pitchBend.mValue);
      if (handled)
         return;
   }

   float value = pitchBend.mValue / MidiDevice::kPitchBendMax;
   TheSynth->SetZoomLevel(pow(2, value * 2 - 1) + .1f);

   //ofLog() << "pitchbend " << pitchBend.mChannel << " " << pitchBend.mValue;
}

void AbletonMoveControl::OnMidiPressure(MidiPressure& pressure)
{
   if (mGridControlInterface != nullptr)
   {
      bool handled = mGridControlInterface->OnAbletonGridControl(this, kChannelPressureIndex + pressure.mChannel, pressure.mPressure);
      if (handled)
         return;
   }
}


void AbletonMoveControl::ZoomToTrack(TrackOrganizer* track)
{
   if (track)
   {
      ofRectangle rect = track->GetBoundingRect();
      float ratioX = ofGetWidth() / rect.width;
      float ratioY = ofGetHeight() / rect.height;
      float newScale = std::min(ratioX, ratioY) * .9f;
      ofVec2f offset = (rect.getCenter() * -1) + ofVec2f(ofGetWidth(), ofGetHeight()) / newScale * .5f;
      TheSynth->GetLocationZoomer()->MoveToLocation(newScale, offset);
   }
}


bool AbletonMoveControl::AdjustGlobalModuleIndex(int amount)
{
   int newIndex = mGlobalModuleIndex + amount;
   if (newIndex >= 0 && newIndex < (int)mGlobalControlModuleCables.size() &&
       (mGlobalControlModuleCables[newIndex]->GetTarget() != nullptr || mGlobalGridInterfaceCables[newIndex]->GetTarget() != nullptr))
   {
      mGlobalModuleIndex = newIndex;
      return true;
   }
   return false;
}

void AbletonMoveControl::AdjustControlWithEncoder(IUIControl* control, float midiInputValue, bool ignoreShift)
{
   float currentNormalized = control->GetMidiValue();
   float increment = GetEncoderIncrement(midiInputValue);
   if (mShiftHeld && !ignoreShift)
      increment *= 0.1f;

   FloatSlider* floatSlider = dynamic_cast<FloatSlider*>(control);
   ClickButton* button = dynamic_cast<ClickButton*>(control);
   if (floatSlider && floatSlider->GetModulator() && floatSlider->GetModulator()->Active() && floatSlider->GetModulator()->CanAdjustRange())
   {
      IModulator* modulator = floatSlider->GetModulator();
      float min = floatSlider->GetMin();
      float max = floatSlider->GetMax();
      float modMin = ofMap(modulator->GetMin(), min, max, 0, 1);
      float modMax = ofMap(modulator->GetMax(), min, max, 0, 1);

      modulator->GetMin() = ofMap(modMin - increment, 0, 1, min, max, K(clamp));
      modulator->GetMax() = ofMap(modMax + increment, 0, 1, min, max, K(clamp));
   }
   else if (button)
   {
      static double sLastButtonClickTime = 0.0;
      if (increment > 0 && gTime - sLastButtonClickTime > 400)
      {
         sLastButtonClickTime = gTime;
         button->SetFromMidiCC(1.0f, NextBufferTime(false), false);
      }
   }
   else
   {
      float newValue = std::clamp(currentNormalized + increment, 0.0f, 1.0f);
      control->SetFromMidiCC(newValue, NextBufferTime(false), false);
   }

   MidiController::sLastActivityUIControl = control;
   MidiController::sLastConnectedActivityTime = gTime;
}

IDrawableModule* AbletonMoveControl::GetCurrentGlobalModule() const
{
   if (mGlobalModuleIndex >= 0 && mGlobalModuleIndex < (int)mGlobalControlModuleCables.size())
      return dynamic_cast<IDrawableModule*>(mGlobalControlModuleCables[mGlobalModuleIndex]->GetTarget());
   return nullptr;
}

IAbletonGridController* AbletonMoveControl::GetCurrentGlobalGridInterface() const
{
   IAbletonGridController* ret = nullptr;
   if (mGlobalModuleIndex >= 0 && mGlobalModuleIndex < (int)mGlobalGridInterfaceCables.size())
      ret = dynamic_cast<IAbletonGridController*>(mGlobalGridInterfaceCables[mGlobalModuleIndex]->GetTarget());
   if (ret == nullptr)
      ret = dynamic_cast<IAbletonGridController*>(mGlobalGridInterfaceCables[0]->GetTarget());
   return ret;
}

TrackOrganizer* AbletonMoveControl::GetActiveTrackRow() const
{
   if (mSelectedTrackRow < 0 || mSelectedTrackRow >= (int)mTrackCables.size() || mTrackCables[mSelectedTrackRow] == nullptr)
      return nullptr;
   return dynamic_cast<TrackOrganizer*>(mTrackCables[mSelectedTrackRow]->GetTarget());
}

void AbletonMoveControl::SetActiveTrackRow(int row, bool resetModuleIndex)
{
   mPreviousSelectedTrackRow = mSelectedTrackRow;

   if (resetModuleIndex)
      mDisplayModuleSelectTimeout = gTime + 500.0;

   mSelectedTrackRow = row;
   mShowSoundSelector = false;

   if (row == kTrackRowGlobal)
   {
      if (resetModuleIndex)
      {
         mGlobalModuleIndex = 0;
         mGlobalModuleViewOffset = 0.0f;
      }
      SetDisplayModuleWithContext(GetCurrentGlobalModule(), "global");
      SetGridControlInterface(GetCurrentGlobalGridInterface());
   }
   else if (row == kTrackRowMixer)
   {
      SetGridControlInterface(nullptr);
      SetDisplayModuleWithContext(nullptr, "mixer");
   }
   else if (row == kTrackRowTransport)
   {
      SetDisplayModule(TheTransport);
      SetGridControlInterface(TheTransport);
   }
   else if (row == kTrackRowScale)
   {
      SetDisplayModule(TheScale);
      SetGridControlInterface(nullptr);
   }
   else if (row >= 0 && row < (int)mTrackCables.size())
   {
      TrackOrganizer* trackRow = GetActiveTrackRow();
      if (trackRow != nullptr)
      {
         if (resetModuleIndex)
            trackRow->ResetModuleIndex();
         IDrawableModule* module = trackRow->GetCurrentModule();
         SetDisplayModuleWithContext(module, trackRow->GetTrackName());
      }

      IAbletonGridController* controller = trackRow != nullptr ? trackRow->GetCurrentGridInterface() : nullptr;
      if (controller != mGridControlInterface)
      {
         if (controller != nullptr)
         {
            SetGridControlInterface(controller);

            for (int i = kMovePadsSection; i < kMovePadsSection + kNumMovePads; ++i)
               SetLed(i, 0);
            mGridControlInterface->OnAbletonGridConnect(this);
         }
         else
         {
            SetGridControlInterface(nullptr);
         }
      }
   }

   DetermineTrackControlLayout();
}

int AbletonMoveControl::GetGridControllerOption1Control() const
{
   return kRepeatButton;
}

int AbletonMoveControl::GetGridControllerOption2Control() const
{
   return kAccentButton;
}
