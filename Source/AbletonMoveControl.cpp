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
#include "TitleBar.h"
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
#include "LaunchpadKeyboard.h"
#include "IInputRecordable.h"
#include "Amplifier.h"
#include "SongBuilder.h"
#include "LooperRecorder.h"

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
   for (int i = 0; i < 128 * 2; ++i)
      mButtonState[i] = 0;
}

AbletonMoveControl::~AbletonMoveControl()
{
}

void AbletonMoveControl::Exit()
{
   for (int i = 0; i < 128; ++i)
   {
      SetLed(kMidiMessage_Note, i, 0);
      SetLed(kMidiMessage_Control, i, 0);
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
   for (size_t i = 0; i < mGlobalModuleCables.size(); ++i)
   {
      mGlobalModuleCables[i] = new PatchCableSource(this, kConnectionType_Special);
      mGlobalModuleCables[i]->SetColor(cableColor);
      mGlobalModuleCables[i]->SetManualPosition(cableX, 10);
      AddPatchCableSource(mGlobalModuleCables[i]);
      cableX += 12;
   }

   mSongBuilderCable = new PatchCableSource(this, kConnectionType_Special);
   mSongBuilderCable->AddTypeFilter("songbuilder");
   mSongBuilderCable->SetColor(cableColor);
   mSongBuilderCable->SetManualPosition(30, 22);
   AddPatchCableSource(mSongBuilderCable);
}

void AbletonMoveControl::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   if (!ThePushBridge.IsInitialized())
   {
      ofSetColor(255, 0, 0, gModuleDrawAlpha);
      DrawTextNormal(mPushBridgeInitErrMsg, 3, 15);

      for (auto& cable : mTrackCables)
         cable->SetShowing(false);
   }
   else
   {
      for (auto& cable : mTrackCables)
         cable->SetShowing(true);
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
   ofVec2f pos;
   const ofVec2f kTooltipOffset(15, 8);

   for (const auto* cableSource : mTrackCables)
   {
      if (cableSource->IsHovered())
      {
         tooltip = "track";
         pos = cableSource->GetPosition() - GetPosition() + kTooltipOffset;
      }
   }

   for (const auto* cableSource : mGlobalModuleCables)
   {
      if (cableSource->IsHovered())
      {
         tooltip = "global module";
         pos = cableSource->GetPosition() - GetPosition() + kTooltipOffset;
      }
   }

   if (mSongBuilderCable->IsHovered())
   {
      tooltip = "song builder";
      pos = mSongBuilderCable->GetPosition() - GetPosition() + kTooltipOffset;
   }

   if (tooltip != "")
   {
      float width = GetStringWidth(tooltip);

      ofFill();
      ofSetColor(50, 50, 50);
      ofRect(pos.x, pos.y, width + 10, 15);

      ofNoFill();
      ofSetColor(255, 255, 255);
      ofRect(pos.x, pos.y, width + 10, 15);

      ofSetColor(255, 255, 255);
      DrawTextNormal(tooltip, pos.x + 5, pos.y + 12);
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
}

void AbletonMoveControl::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void AbletonMoveControl::SetUpFromSaveData()
{
}

void AbletonMoveControl::SaveLayout(ofxJSONElement& moduleInfo)
{
}

void AbletonMoveControl::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);
}

void AbletonMoveControl::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   LoadStateValidate(rev <= GetModuleSaveStateRev());

   if (rev < 2)
   {
      int numBookmarks;
      in >> numBookmarks;
      for (int i = 0; i < numBookmarks; ++i)
      {
         std::string path;
         in >> path;
      }
   }
}

bool AbletonMoveControl::Initialize()
{
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

   mLCD.Init();

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

   if (mMostRecentlyTouchedKnobIndex != -1 && GetButtonState(kMidiMessage_Note, kMainEncoderTouchSection + mMostRecentlyTouchedKnobIndex))
      return mMostRecentlyTouchedKnobIndex;

   for (int i = 0; i < kNumMainEncoders; ++i)
   {
      if (GetButtonState(kMidiMessage_Note, kMainEncoderTouchSection + i))
         return i;
   }

   return -1;
}

bool AbletonMoveControl::ShouldDisplayMixer()
{
   for (int i = 0; i < kNumTrackButtons; ++i)
   {
      if (GetButtonState(kMidiMessage_Control, kTrackButtonSection + kNumTrackButtons - 1 - i))
      {
         if (GetButtonState(kMidiMessage_Note, kVolumeEncoderTouch))
         {
            int trackIndex = i + mTrackRowOffset;
            mLastGainAdjustTrackIndex = trackIndex;
            mLastGainAdjustTrackTime = gTime;
            return true;
         }
      }
   }

   return mDisplayModule == nullptr || mLastGainAdjustTrackTime > gTime - 300;
}

bool AbletonMoveControl::ShouldDisplaySnapshotView()
{
   if (GetButtonState(kMidiMessage_Control, kDotButton))
      return true;
   if (mSelectedTrackRow == -1)
   {
      if (mGridControlInterface == nullptr)
         return true;
      else
         return false;
   }
   return false;
}

void AbletonMoveControl::DrawToFramebuffer()
{
   if (gTime < mScreenOverrideTimeout)
      return;

   mLCD.Clear();

   bool needToDraw = true;

   if (needToDraw && GetButtonState(kMidiMessage_Note, kClickyEncoderTouch))
   {
      if (mShiftHeld) //scroll module offset
      {
         mTrackRowOffsetSmoothed = ofLerp(mTrackRowOffsetSmoothed, mTrackRowOffset, 0.3f);
         int spacingY = 8;
         for (int i = 0; i < (int)mTrackCables.size(); ++i)
         {
            mLCD.DrawRect(3, 2 + i * spacingY, 4, 5, i == mSelectedTrackRow);
            TrackOrganizer* trackRow = dynamic_cast<TrackOrganizer*>(mTrackCables[i]->GetTarget());
            if (trackRow != nullptr)
               mLCD.DrawText(trackRow->GetTrackName().c_str(), 10, 1 + i * spacingY + 6);
         }
         mLCD.DrawRect(1, 0 + int(mTrackRowOffsetSmoothed * spacingY), mLCD.kMoveDisplayWidth - 2, spacingY * kNumTrackButtons + 1, !K(filled));
      }
      else //module select within track/global
      {
         std::string trackName;
         TrackOrganizer* trackRow = GetActiveTrackRow();
         int moduleIndex;
         std::vector<IDrawableModule*> moduleList;
         if (trackRow != nullptr)
         {
            moduleIndex = trackRow->GetModuleIndex();
            moduleList = trackRow->GetModuleList();
            trackName = trackRow->GetTrackName();
         }
         else
         {
            moduleIndex = mGlobalModuleIndex;
            for (int i = 0; i < (int)mGlobalModuleCables.size(); ++i)
            {
               IDrawableModule* module = dynamic_cast<IDrawableModule*>(mGlobalModuleCables[i]->GetTarget());
               if (module != nullptr)
                  moduleList.push_back(module);
            }
            trackName = "global";
         }

         mLCD.DrawText((trackName + ":").c_str(), 3, 12, LCDFONT_STYLE_REGULAR);
         for (int i = 0; i < (int)moduleList.size(); ++i)
         {
            mLCD.DrawText(moduleList[i]->Name(), 10, 20 + i * 10, i == moduleIndex ? LCDFONT_STYLE_UNDERLINE : LCDFONT_STYLE_REGULAR);
         }
      }

      needToDraw = false;
   }

   if (needToDraw && ShouldDisplayMixer())
   {
      mLCD.DrawText("mixer", 3, 7, LCDFONT_STYLE_UNDERLINE);

      std::string gainAdjustTrackName = "";
      const int kBarPadBelow = 5;
      const int kMaxBarHeight = mLCD.kMoveDisplayHeight - 16;
      for (int i = 0; i < (int)mTrackCables.size(); ++i)
      {
         int x = 3 + i * 16;

         TrackOrganizer* track = dynamic_cast<TrackOrganizer*>(mTrackCables[i]->GetTarget());
         if (track != nullptr)
         {
            Amplifier* gain = track->GetGain();
            FloatSlider* gainSlider = gain != nullptr ? gain->GetGainSlider() : nullptr;
            if (gain != nullptr && gainSlider != nullptr)
            {
               float level, watermarkLevel;
               gain->GetLevel(level, watermarkLevel);
               int gainFillHeight = std::clamp((int)ofMap(gainSlider->GetValue(), gainSlider->GetMin(), gainSlider->GetMax(), 0, kMaxBarHeight), 0, kMaxBarHeight);
               int levelHeight = std::clamp(int(level * kMaxBarHeight), 0, kMaxBarHeight);
               int watermarkLevelHeight = std::clamp(int(watermarkLevel * kMaxBarHeight), 0, kMaxBarHeight);
               mLCD.DrawRect(x, mLCD.kMoveDisplayHeight - kBarPadBelow - kMaxBarHeight, 4, kMaxBarHeight, !K(filled));
               mLCD.DrawRect(x, mLCD.kMoveDisplayHeight - kBarPadBelow - gainFillHeight, 4, gainFillHeight, K(filled));
               mLCD.DrawRect(x + 6, mLCD.kMoveDisplayHeight - kBarPadBelow - levelHeight, 1, levelHeight, K(filled));
               mLCD.DrawRect(x + 5, mLCD.kMoveDisplayHeight - kBarPadBelow - watermarkLevelHeight, 2, 1, K(filled));

               if (GetButtonState(kMidiMessage_Note, kMainEncoderTouchSection + i) ||
                   (i == mLastGainAdjustTrackIndex && mLastGainAdjustTrackTime > gTime - 300))
               {
                  bool filled = int(gTime / 200) % 2 == 0; //blink
                  mLCD.DrawRect(x, mLCD.kMoveDisplayHeight - 4, 4, 4, filled);
                  gainAdjustTrackName = track->GetTrackName();
               }
            }
         }
      }
      mLCD.DrawText(gainAdjustTrackName.c_str(), 50, 7, LCDFONT_STYLE_REGULAR);

      return;
   }

   if (needToDraw && mGridControlInterface != nullptr)
   {
      if (mGridControlInterface->UpdateAbletonMoveScreen(this, &mLCD))
         needToDraw = false;
   }

   if (needToDraw)
   {
      if (mDisplayModule != nullptr)
      {
         std::string display;
         if (mDisplayModuleContext != "")
            display = mDisplayModuleContext + ": " + mDisplayModule->Name();
         else
            display = mDisplayModule->Name();
         mLCD.DrawText(display.c_str(), 3, 12, LCDFONT_STYLE_UNDERLINE);

         if (GetDisplayKnobIndex() == -1)
         {
            for (int i = 0; i < kNumMainEncoders; ++i)
            {
               int controlIndex = GetControlOffset() + i;
               if (controlIndex < (int)mControls.size())
               {
                  auto* control = mControls[controlIndex];
                  mLCD.DrawText(control->GetDisplayName().c_str(), i * 14, 26 + (i % 4) * 10);
               }
            }

            const int kModuleViewBarX = 0;
            const int kModuleViewBarY = 58;
            const int kModuleViewBarWidth = 128;
            const int kModuleViewBarHeight = 6;
            mLCD.DrawRect(kModuleViewBarX, kModuleViewBarY, kModuleViewBarWidth, kModuleViewBarHeight, false);
            float offsetStart = GetControlOffset() / (float)mControls.size();
            float offsetEnd = MIN(GetControlOffset() + kNumMainEncoders, mControls.size()) / (float)mControls.size();
            mLCD.DrawRect(kModuleViewBarX + offsetStart * kModuleViewBarWidth, kModuleViewBarY + 2, (offsetEnd - offsetStart) * kModuleViewBarWidth, kModuleViewBarHeight - 4, true);
            if (GetButtonState(kMidiMessage_Note, kVolumeEncoderTouch))
               mLCD.DrawRect(kModuleViewBarX + (mModuleViewOffset * kNumMainEncoders / (float)mControls.size()) * kModuleViewBarWidth, kModuleViewBarY, 1, kModuleViewBarHeight, true);
         }
      }

      int displayKnobIndex = GetDisplayKnobIndex();
      if (displayKnobIndex != -1)
      {
         int controlIndex = displayKnobIndex + GetControlOffset();
         if (controlIndex < (int)mControls.size())
         {
            auto* control = mControls[controlIndex];
            mLCD.DrawText(control->GetDisplayName().c_str(), 3, 26);
            mLCD.DrawText(control->GetDisplayValue(control->GetValue()).c_str(), 3, 40);

            int sliderX = 3;
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
   if (track != nullptr)
   {
      Amplifier* gain = track->GetGain();
      if (gain != nullptr)
      {
         float level, watermarkLevel;
         gain->GetLevel(level, watermarkLevel);
         mLCD.DrawRect(0, 0, std::clamp(int(level * mLCD.kMoveDisplayWidth), 0, mLCD.kMoveDisplayWidth), 1, true);
         mLCD.DrawRect(std::clamp(int(watermarkLevel * mLCD.kMoveDisplayWidth), 0, mLCD.kMoveDisplayWidth), 0, 1, 2, true);
      }
   }
}

void AbletonMoveControl::UpdateLeds()
{
   TrackOrganizer* activeTrackRow = GetActiveTrackRow();

   SetLed(kMidiMessage_Control, kPlayButton, TheSynth->IsAudioPaused() ? 127 : 120);
   SetLed(kMidiMessage_Control, kUpButton, 0);
   SetLed(kMidiMessage_Control, kDownButton, 0);
   SetLed(kMidiMessage_Control, kLeftButton, 0);
   SetLed(kMidiMessage_Control, kRightButton, 0);
   if (GetDisplayKnobIndex() != -1)
   {
      SetLed(kMidiMessage_Control, kPageLeftButton, 0, 127);
      SetLed(kMidiMessage_Control, kPageRightButton, 0, 127);
      SetLed(kMidiMessage_Control, kOctaveUpButton, 0, 127);
      SetLed(kMidiMessage_Control, kOctaveDownButton, 0, 127);
      SetLed(kMidiMessage_Control, kDotButton, 0, 127);
   }
   else
   {
      SetLed(kMidiMessage_Control, kPageLeftButton, 0);
      SetLed(kMidiMessage_Control, kPageRightButton, 0);
      SetLed(kMidiMessage_Control, kOctaveUpButton, 0);
      SetLed(kMidiMessage_Control, kOctaveDownButton, 0);

      Snapshots* snapshots = activeTrackRow != nullptr ? activeTrackRow->GetSnapshots() : nullptr;
      if (snapshots != nullptr)
         SetLed(kMidiMessage_Control, kDotButton, activeTrackRow->GetColorIndex());
      else
         SetLed(kMidiMessage_Control, kDotButton, 0);
   }

   SetLed(kMidiMessage_Control, kShiftButton, 127, mShiftHeld ? 0 : -1);

   if (mDisplayModule == TheTransport)
      SetLed(kMidiMessage_Control, kLedTempo, 127, 30);
   else if (mShiftHeld)
      SetLed(kMidiMessage_Control, kLedTempo, 10);
   else
      SetLed(kMidiMessage_Control, kLedTempo, 0);

   if (mDisplayModule == TheScale)
      SetLed(kMidiMessage_Control, kLedScale, 127, 30);
   else if (mShiftHeld)
      SetLed(kMidiMessage_Control, kLedScale, 10);
   else
      SetLed(kMidiMessage_Control, kLedScale, 0);

   //test led colors
   //SetLed(kMidiMessage_Note, 92, (int)mModuleListOffset);
   //ofLog() << (int)mModuleListOffset;

   for (int i = 0; i < (int)mTrackCables.size(); ++i)
   {
      int trackIndex = i + mTrackRowOffset;
      if (trackIndex >= 0 && trackIndex < (int)mTrackCables.size())
      {
         TrackOrganizer* trackRow = dynamic_cast<TrackOrganizer*>(mTrackCables[trackIndex]->GetTarget());
         int buttonNumber = (kTrackButtonSection + kNumTrackButtons - 1) - i;
         if (trackRow != nullptr)
            SetLed(kMidiMessage_Control, buttonNumber, trackRow->GetColorIndex(), trackIndex == mSelectedTrackRow ? 0 : -1);
         else
            SetLed(kMidiMessage_Control, buttonNumber, 0);
      }
   }

   SetLed(kMidiMessage_Control, kHamburgerButton, 127, mSelectedTrackRow == -1 ? 20 : -1);

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
   SetLed(kMidiMessage_Control, kCircleButton, recordColor1, recordColor2);

   for (int i = 0; i < kNumMainEncoders; ++i)
   {
      SetLed(kMidiMessage_Control, i + kMainEncoderSection, i + GetControlOffset() < (int)mControls.size() ? 121 : 0);
   }

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

      SetLed(kMidiMessage_Note, i + AbletonDevice::kStepButtonSection, colorIndex, colorIndex2);
   }

   SetLed(kMidiMessage_Control, AbletonDevice::kMoveDeleteButton, 0);
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
      if (desiredControls.size() != mDisplayedControls.size())
      {
         controlsChanged = true;
      }
      else
      {
         for (size_t i = 0; i < desiredControls.size(); ++i)
         {
            if (desiredControls[i] != mDisplayedControls[i])
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
                  if (snapshots->GetCurrentSnapshot() == col)
                  {
                     colors[col] = kColorWhite;
                     colors2[col] = kColorDarkGrey;
                  }
                  else if (snapshots->HasSnapshot(col))
                  {
                     colors[col] = trackRow->GetColorIndex();
                  }
               }
            }
         }

         for (int col = 0; col < kNumCols; ++col)
         {
            SetLed(kMidiMessage_Note, kMovePadsSection + (kNumTrackButtons - 1 - i) * 8 + col, colors[col], colors2[col]);
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
         SetLed(kMidiMessage_Note, i, kColorOff);
   }

   SendLeds();
}

void AbletonMoveControl::SetDisplayModule(IDrawableModule* module, bool addToHistory)
{
   mDisplayModule = module;
   mModuleViewOffset = 0;
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
   mLCD.Clear();
   mLCD.DrawText(message.c_str(), 3, 12, LCDFONT_STYLE_REGULAR);
   mScreenOverrideTimeout = gTime + durationMs;
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
      bool isEnabledCheckbox = mDisplayModule != nullptr && controls[i] == mDisplayModule->GetEnabledCheckbox();
      if (!isEnabledCheckbox &&
          (controls[i]->IsSliderControl() || controls[i]->IsButtonControl()) &&
          (controls[i]->GetShouldSaveState() || dynamic_cast<ClickButton*>(controls[i]) != nullptr))
         mControls.push_back(controls[i]);
   }
   mDisplayedControls = controls;
}

int AbletonMoveControl::GetControlOffset() const
{
   return int(mModuleViewOffset) * kNumMainEncoders;
}

void AbletonMoveControl::SetLed(MidiMessageType type, int index, int color, int flashColor /*=-1*/)
{
   if (type == kMidiMessage_Control)
      index += 128;
   assert(index >= 0 && index < 128 * 2);

   mQueuedLedState[index] = LedState(color, flashColor);
}

void AbletonMoveControl::SendLeds()
{
   for (int index = 0; index < mQueuedLedState.size(); ++index)
   {
      if (mLedState[index] != mQueuedLedState[index])
      {
         mLedState[index] = mQueuedLedState[index];

         int channel = 1;
         if (mLedState[index].flashColor != -1)
            channel = 10;

         //bool isPulse = (channel >= 7 && channel <= 11);
         if (index < 128)
         {
            if (mLedState[index].flashColor != -1)
               mDevice.SendNote(gTime, index, mLedState[index].flashColor, false, -1);
            mDevice.SendNote(gTime, index, mLedState[index].color, false, channel);
         }
         else
         {
            if (mLedState[index].flashColor != -1)
               mDevice.SendCC(index - 128, mLedState[index].flashColor);
            mDevice.SendCC(index - 128, mLedState[index].color, channel);
         }
      }
   }
}

bool AbletonMoveControl::GetButtonState(MidiMessageType type, int index) const
{
   if (type == kMidiMessage_Control)
      index += 128;
   assert(index >= 0 && index < 128 * 2);

   return mButtonState[index];
}

void AbletonMoveControl::SetGridControlInterface(IAbletonGridController* controller, IDrawableModule* module)
{
   mGridControlInterface = controller;
   SetLed(kMidiMessage_Control, GetGridControllerOption1Control(), 0);
   SetLed(kMidiMessage_Control, GetGridControllerOption2Control(), 0);
}

void AbletonMoveControl::OnMidiNote(MidiNote& note)
{
   //ofLog() << "AbletonMoveControl::OnMidiNote() " << note.mPitch << " " << note.mVelocity;

   mButtonState[note.mPitch] = note.mVelocity > 0;

   if (mShiftHeld)
   {
      if (note.mPitch == kLedTempo)
      {
         if (note.mVelocity > 0)
            SetDisplayModule(TheTransport);
         return;
      }
      if (note.mPitch == kLedScale)
      {
         if (note.mVelocity > 0)
            SetDisplayModule(TheScale);
         return;
      }
   }

   if (!ShouldDisplaySnapshotView() && mGridControlInterface != nullptr)
   {
      bool handled = mGridControlInterface->OnAbletonGridControl(this, kMidiMessage_Note, note.mPitch, note.mVelocity);
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
      int col = gridIndex % 8;
      if (row < (int)mTrackCables.size())
      {
         TrackOrganizer* trackRow = dynamic_cast<TrackOrganizer*>(mTrackCables[row]->GetTarget());
         if (trackRow != nullptr)
         {
            Snapshots* snapshots = trackRow->GetSnapshots();
            if (snapshots != nullptr)
            {
               if (GetButtonState(kMidiMessage_Control, kDotButton))
                  snapshots->StoreSnapshot(col, true);
               else if (snapshots->HasSnapshot(col))
                  snapshots->SetSnapshot(col, gTime);
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

void AbletonMoveControl::OnMidiControl(MidiControl& control)
{
   //ofLog() << "AbletonMoveControl::OnMidiControl() " << control.mControl << " " << control.mValue;

   mButtonState[control.mControl + 128] = control.mValue > 0;

   for (int i = 0; i < kNumTrackButtons; ++i)
   {
      if (GetButtonState(kMidiMessage_Control, kTrackButtonSection + kNumTrackButtons - 1 - i))
      {
         if (control.mControl == kVolumeEncoder)
         {
            int trackIndex = i + mTrackRowOffset;
            if (trackIndex >= 0 && trackIndex < (int)mTrackCables.size())
            {
               TrackOrganizer* track = dynamic_cast<TrackOrganizer*>(mTrackCables[trackIndex]->GetTarget());
               if (track != nullptr)
               {
                  Amplifier* gain = track->GetGain();
                  FloatSlider* gainSlider = gain != nullptr ? gain->GetGainSlider() : nullptr;
                  if (gainSlider != nullptr)
                  {
                     AdjustControlWithEncoder(gainSlider, control.mValue);
                     mLastGainAdjustTrackIndex = trackIndex;
                     mLastGainAdjustTrackTime = gTime;
                     /*mLCD.Clear();
                     mLCD.DrawRect(3, 4, mLCD.kMoveDisplayWidth - 6, 4, !K(filled));
                     mLCD.DrawRect(3, 4, (mLCD.kMoveDisplayWidth - 6) * ofMap(gainSlider->GetValue(), gainSlider->GetMin(), gainSlider->GetMax(), 0, 1), 4, K(filled));
                     mLCD.DrawText(("track " + ofToString(trackIndex) + " gain: " + ofToString(gainSlider->GetValue())).c_str(), 3, 23, LCDFONT_STYLE_REGULAR);
                     mScreenOverrideTimeout = gTime + 500;*/
                  }
               }
            }
            return;
         }
      }
   }

   if (mGridControlInterface != nullptr)
   {
      bool handled = mGridControlInterface->OnAbletonGridControl(this, kMidiMessage_Control, control.mControl, control.mValue);
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
               FloatSlider* gainSlider = gain != nullptr ? gain->GetGainSlider() : nullptr;
               if (gainSlider != nullptr)
                  AdjustControlWithEncoder(gainSlider, control.mValue);
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
      if (control.mValue > 0)
      {
         int index = (kNumTrackButtons - 1) - (control.mControl - kTrackButtonSection) + mTrackRowOffset;
         SetActiveTrackRow(index);
      }
      else
      {
         //if we held it, then we were just "peeking" into the other track, so switch back
         if (gTime - mLastTrackSelectButtonPressTime > 300)
            SetActiveTrackRow(mPreviousSelectedTrackRow);
      }
   }
   else if (control.mControl == kHamburgerButton)
   {
      if (control.mValue > 0)
      {
         SetActiveTrackRow(-1);
      }
      else
      {
         //if we held it, then we were just "peeking" into the other track, so switch back
         if (gTime - mLastTrackSelectButtonPressTime > 300)
            SetActiveTrackRow(mPreviousSelectedTrackRow);
      }
   }
   else if (control.mControl == kClickyEncoder)
   {
      int direction = control.mValue < 64 ? 1 : -1;

      TrackOrganizer* trackRow = GetActiveTrackRow();
      if (mShiftHeld)
      {
         mTrackRowOffset = std::clamp(mTrackRowOffset + direction, 0, (int)mTrackCables.size() - kNumTrackButtons);
      }
      else if (trackRow != nullptr)
      {
         trackRow->AdjustModuleIndex(direction);
         SetDisplayModuleWithContext(trackRow->GetCurrentModule(), trackRow->GetTrackName());
         if (dynamic_cast<IAbletonGridController*>(mDisplayModule) != nullptr)
            SetGridControlInterface(dynamic_cast<IAbletonGridController*>(mDisplayModule), mDisplayModule);
         else
            SetGridControlInterface(trackRow->GetGridInterface(), dynamic_cast<IDrawableModule*>(trackRow->GetGridInterface()));
      }
      else
      {
         AdjustGlobalModuleIndex(direction);
         SetDisplayModuleWithContext(GetCurrentGlobalModule(), "global");
         if (dynamic_cast<IAbletonGridController*>(GetCurrentGlobalModule()) != nullptr)
            SetGridControlInterface(dynamic_cast<IAbletonGridController*>(GetCurrentGlobalModule()), GetCurrentGlobalModule());
         else
            SetGridControlInterface(nullptr, nullptr);
      }
   }
   else if (control.mControl == kVolumeEncoder)
   {
      float increment = control.mValue < 64 ? control.mValue : control.mValue - 128;
      increment *= 0.07f;
      mModuleViewOffset = ofClamp(mModuleViewOffset + increment, 0, MAX(0, (int)mControls.size() / kNumMainEncoders));
   }
   else if (control.mControl == kBackButton)
   {
      if (control.mValue > 0)
         SetDisplayModule(nullptr);
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
   else if (displayKnobIndex != -1 &&
            (control.mControl == kPageLeftButton ||
             control.mControl == kPageRightButton ||
             control.mControl == kOctaveUpButton ||
             control.mControl == kOctaveDownButton ||
             control.mControl == kDotButton))
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
            if (control.mControl == kDotButton)
            {
               mControls[controlIndex]->ResetToOriginal();
               mLastResetTime = gTime;
            }
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
   /*
   else if (control.mControl == kCaptureButton)
   {
      if (control.mValue > 0 && mDisplayModuleCanControlGrid)
      {
         SetGridControlInterface(dynamic_cast<IAbletonGridController*>(mDisplayModule), mDisplayModule);
      }
   }*/
   else
   {
      ofLog() << "unhandled control " << control.mControl << " " << control.mValue;
   }
}

void AbletonMoveControl::OnMidiPitchBend(MidiPitchBend& pitchBend)
{
   if (mGridControlInterface != nullptr)
   {
      bool handled = mGridControlInterface->OnAbletonGridControl(this, kMidiMessage_PitchBend, pitchBend.mChannel, pitchBend.mValue);
      if (handled)
         return;
   }

   float value = pitchBend.mValue / MidiDevice::kPitchBendMax;
   TheSynth->SetZoomLevel(pow(2, value * 2 - 1) + .1f);

   //ofLog() << "pitchbend " << pitchBend.mChannel << " " << pitchBend.mValue;
}

void AbletonMoveControl::AdjustGlobalModuleIndex(int amount)
{
   int newIndex = mGlobalModuleIndex + amount;
   if (newIndex >= 0 && newIndex < (int)mGlobalModuleCables.size() && mGlobalModuleCables[newIndex]->GetTarget() != nullptr)
      mGlobalModuleIndex = newIndex;
}

void AbletonMoveControl::AdjustControlWithEncoder(IUIControl* control, float midiInputValue)
{
   float currentNormalized = control->GetMidiValue();
   float increment = midiInputValue < 64 ? midiInputValue : midiInputValue - 128;
   increment *= mShiftHeld ? .0005f : .005f;

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
}

IDrawableModule* AbletonMoveControl::GetCurrentGlobalModule() const
{
   if (mGlobalModuleIndex >= 0 && mGlobalModuleIndex < (int)mGlobalModuleCables.size())
      return dynamic_cast<IDrawableModule*>(mGlobalModuleCables[mGlobalModuleIndex]->GetTarget());
   return nullptr;
}

TrackOrganizer* AbletonMoveControl::GetActiveTrackRow() const
{
   if (mSelectedTrackRow < 0 || mSelectedTrackRow >= (int)mTrackCables.size() || mTrackCables[mSelectedTrackRow] == nullptr)
      return nullptr;
   return dynamic_cast<TrackOrganizer*>(mTrackCables[mSelectedTrackRow]->GetTarget());
}

void AbletonMoveControl::SetActiveTrackRow(int row)
{
   mPreviousSelectedTrackRow = mSelectedTrackRow;
   mLastTrackSelectButtonPressTime = gTime;

   //if (row == mSelectedTrackRow)
   //   return;

   if (row == -1)
   {
      mSelectedTrackRow = row;
      SetGridControlInterface(nullptr, nullptr);
      mGlobalModuleIndex = 0;
      SetDisplayModuleWithContext(GetCurrentGlobalModule(), "global");
   }
   else if (row < (int)mTrackCables.size())
   {
      mSelectedTrackRow = row;
      TrackOrganizer* trackRow = GetActiveTrackRow();
      if (trackRow != nullptr)
      {
         trackRow->SetModuleIndex(0);
         IDrawableModule* module = trackRow->GetCurrentModule();
         SetDisplayModuleWithContext(module, trackRow->GetTrackName());
      }

      IAbletonGridController* controller = trackRow != nullptr ? trackRow->GetGridInterface() : nullptr;
      if (controller != mGridControlInterface)
      {
         if (controller != nullptr)
         {
            SetGridControlInterface(controller, mDisplayModule);

            for (int i = kMovePadsSection; i < kMovePadsSection + kNumMovePads; ++i)
               SetLed(kMidiMessage_Note, i, 0);
            mGridControlInterface->OnAbletonGridConnect(this);
         }
         else
         {
            SetGridControlInterface(nullptr, nullptr);
         }
      }
   }
}

int AbletonMoveControl::GetGridControllerOption1Control() const
{
   return kRepeatButton;
}

int AbletonMoveControl::GetGridControllerOption2Control() const
{
   return kAccentButton;
}
