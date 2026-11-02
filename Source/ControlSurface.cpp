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

    ControlSurface.cpp
    Created: 24 Oct 2026
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "ControlSurface.h"
#include "SynthGlobals.h"
#include "OpenFrameworksPort.h"
#include "ModuleSaveDataPanel.h"
#include "QuickSpawnMenu.h"
#include "PatchCableSource.h"
#include "FloatSliderLFOControl.h"
#include "UserPrefsEditor.h"
#include "Snapshots.h"
#include "ADSRDisplay.h"
#include "Looper.h"
#include "NoteLooper.h"
#include "ControlRecorder.h"
#include "AbletonDeviceShared.h"
#include "TrackOrganizer.h"
#include "IInputRecordable.h"
#include "Amplifier.h"
#include "AudioSend.h"
#include "SongBuilder.h"
#include "LooperRecorder.h"
#include "IControlVisualizer.h"
#include "AudioSyncer.h"
#include "nanovg/nanovg.h"
#include "Checkbox.h"

#include <mutex>

using namespace AbletonDevice;

IUIControl* ControlSurface::sBindToUIControl = nullptr;

namespace
{
   const int kCableAreaTop = 22;
   const int kLCDOnModuleX = 3;

   //nanovg images can only be deleted on the render thread, in the context that made them, so the images of deleted modules are
   //recycled by the next module that needs one rather than deleted
   std::mutex sFreeLCDImagesMutex;
   std::vector<std::pair<NVGcontext*, int>> sFreeLCDImages;
}

ControlSurface::ControlSurface()
: IDrawableModule(166, 82)
, mDevice(this)
{
   for (int i = 0; i < (int)ControlSurfaceDeviceType::Count; ++i)
      mSurfaces[i] = CreateControlSurfaceDevice((ControlSurfaceDeviceType)i);

   mLCD.Init();
   for (auto& lcd : mKnobLCDs)
   {
      lcd.SetSize(120, 64); //the push 2 screen fits eight of these side by side, one above each knob
      lcd.Init();
   }
   mModuleListLCD.Init();
   mMixerLCD.Init();
   mLissajousLCD.Init();
   mLCDImages.fill(-1);
}

ControlSurface::~ControlSurface()
{
   std::lock_guard<std::mutex> lock(sFreeLCDImagesMutex);
   for (int i = 0; i < (int)mLCDImages.size(); ++i)
   {
      if (mLCDImages[i] != -1)
         sFreeLCDImages.push_back(std::make_pair(gNanoVGRenderContexts[i], mLCDImages[i]));
   }
}

void ControlSurface::Exit()
{
   IDrawableModule::Exit();

   DisconnectDevice();
}

ControlSurfaceDevice* ControlSurface::Surface() const
{
   int index = std::clamp((int)mDeviceType, 0, (int)ControlSurfaceDeviceType::Count - 1);
   return mSurfaces[index].get();
}

int ControlSurface::GetMaxTrackRowOffset() const
{
   if (mSessionOrganizer == nullptr)
      return 0;
   return std::max(0, mSessionOrganizer->GetNumTracks() - NumTrackButtons());
}

int ControlSurface::GetNumSnapshotColumns() const
{
   if (HasGrid())
      return GetGridNumCols();
   return NumTrackButtons();
}

//devices without a pad grid show the active track's snapshots on the track buttons instead
bool ControlSurface::UseTrackButtonsForSnapshots() const
{
   return !HasGrid() && GetButtonState(kDotButton);
}

void ControlSurface::AutoDetectDevice()
{
   for (int i = 0; i < (int)ControlSurfaceDeviceType::Count; ++i)
   {
      if (mSurfaces[i]->IsPresent())
      {
         mDeviceType = (ControlSurfaceDeviceType)i;
         return;
      }
   }
}

void ControlSurface::ConnectDevice()
{
   DisconnectDevice();

   for (auto& state : mControlState)
   {
      state.mLedState = ControlSurfaceLedState();
      state.mButtonState = 0;
   }
   mTrackRowOffset = std::clamp(mTrackRowOffset, 0, GetMaxTrackRowOffset());
   mTrackButtonPressedForSnapshot.fill(false);

   ControlSurfaceDevice* surface = Surface();
   if (surface->Connect(mDevice, mStatusMessage))
   {
      mConnectedSurface = surface;
      mGridControlInterface = nullptr; //so the current grid interface gets OnAbletonGridConnect() for this device
      SetActiveTrackRow(mSelectedTrackRow, false);
      UpdateLeds();
      SendLeds(K(force));
   }
}

void ControlSurface::DisconnectDevice()
{
   ControlSurfaceDevice* surface = mConnectedSurface;
   if (surface == nullptr)
      return;

   for (auto& state : mControlState)
   {
      state.mQueuedLedState = ControlSurfaceLedState(kColorOff, -1, 1);
      state.mLedPriority = LedPriority::None;
   }
   SendLeds(K(force));

   mConnectedSurface = nullptr;
   surface->Disconnect(mDevice);
}

void ControlSurface::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   mDeviceTypeDropdown = new DropdownList(this, "device", 3, 3, (int*)(&mDeviceType), 160);
   for (int i = 0; i < (int)ControlSurfaceDeviceType::Count; ++i)
      mDeviceTypeDropdown->AddLabel(mSurfaces[i]->GetName(), i);

   ofColor cableColor = IDrawableModule::GetColor(kModuleCategory_Other);
   cableColor.a *= .3f;

   mSessionOrganizerCable = new PatchCableSource(this, kConnectionType_Special);
   mSessionOrganizerCable->AddTypeFilter("sessionorganizer");
   mSessionOrganizerCable->SetColor(cableColor);
   mSessionOrganizerCable->SetManualPosition(8, kCableAreaTop + 10);
   AddPatchCableSource(mSessionOrganizerCable);

   int cableX = 30;
   int cableY = kCableAreaTop + 10;
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

void ControlSurface::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   mDeviceTypeDropdown->Draw();

   std::string status = mStatusMessage;
   if (mConnectedSurface != nullptr && status.empty())
   {
      ofSetColor(0, 255, 0, gModuleDrawAlpha);
      status = "connected";
   }
   else
   {
      ofSetColor(255, 0, 0, gModuleDrawAlpha);
      if (status.empty())
         status = "not connected";
   }
   DrawTextNormal(status, 3, mHeight - 4, 10);

   if (ShowsLCDOnModule())
      DrawLCDImage(kLCDOnModuleX, mHeight, mLCD.GetWidth(), mLCD.GetHeight(), gModuleDrawAlpha / 255.0f);

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

void ControlSurface::DrawModuleUnclipped()
{
   IDrawableModule* controlModule = GetCurrentControlModule();
   if (controlModule != nullptr && !controlModule->IsDeleted() &&
       controlModule->GetOwningContainer() != nullptr && controlModule->IsShowing())
   {
      ofPushMatrix();
      ofPushStyle();
      ofVec2f pos = GetPosition();
      ofTranslate(-pos.x, -pos.y);
      DrawDisplayModuleRect(controlModule->GetRect(), 3);
      ofPopMatrix();
      ofPopStyle();
   }

   std::string tooltip = "";
   const PatchCableSource* hoverCable = nullptr;

   if (mSessionOrganizerCable->IsHovered())
   {
      hoverCable = mSessionOrganizerCable;
      tooltip = "sessionorganizer";
   }

   for (int i = 0; i < (int)mGlobalControlModuleCables.size(); ++i)
   {
      if (mGlobalControlModuleCables[i]->IsHovered())
      {
         hoverCable = mGlobalControlModuleCables[i];
         tooltip = "global module " + ofToString(i);
      }
   }

   for (int i = 0; i < (int)mGlobalGridInterfaceCables.size(); ++i)
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

      const float kLcdX = 30;
      const float kLcdY = 75;
      const float kLcdScale = 4;
      DrawLCDImage(kLcdX, kLcdY, mLCD.GetWidth() * kLcdScale, mLCD.GetHeight() * kLcdScale, 150 / 255.0f);

      ofPopMatrix();
   }
}

void ControlSurface::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   if (cableSource == mSessionOrganizerCable)
      mSessionOrganizer = dynamic_cast<SessionOrganizer*>(cableSource->GetTarget());
}

void ControlSurface::DrawDisplayModuleRect(ofRectangle rect, float thickness)
{
   if (GetCurrentControlModule()->HasTitleBar())
   {
      rect.y -= IDrawableModule::TitleBarHeight();
      rect.height += IDrawableModule::TitleBarHeight();
   }
   ofSetColor(255, 255, 255, ofMap(sin(gTime / 1000 * PI * 2), -1, 1, 60, 100));
   ofSetLineWidth(thickness);
   ofNoFill();
   ofRect(rect.x - 3, rect.y - 3, rect.width + 6, rect.height + 6, 6);
}

void ControlSurface::GetModuleDimensions(float& width, float& height)
{
   width = mWidth;
   height = mHeight;
   if (ShowsLCDOnModule())
   {
      width = std::max(width, kLCDOnModuleX * 2.0f + mLCD.GetWidth());
      height += mLCD.GetHeight() + 3;
   }
}

//draws the lcd as a single nearest-neighbor scaled image. must be called on the render thread.
void ControlSurface::DrawLCDImage(float x, float y, float width, float height, float alpha)
{
   int contextIndex = -1;
   for (int i = 0; i < (int)gNanoVGRenderContexts.size(); ++i)
   {
      if (gNanoVGRenderContexts[i] == gNanoVG)
         contextIndex = i;
   }
   if (contextIndex == -1)
      return;

   const int kWidth = mLCD.GetWidth();
   const int kHeight = mLCD.GetHeight();

   //the lcd only reliably fills in the first byte of each pixel, so build a proper rgba image from it
   mLCDImagePixels.resize(kWidth * kHeight * 4);
   for (int py = 0; py < kHeight; ++py)
   {
      for (int px = 0; px < kWidth; ++px)
      {
         unsigned char value = mLCD.GetPixel(px, py) > 0 ? 255 : 0;
         unsigned char* pixel = &mLCDImagePixels[(px + py * kWidth) * 4];
         pixel[0] = value;
         pixel[1] = value;
         pixel[2] = value;
         pixel[3] = 255;
      }
   }

   int& image = mLCDImages[contextIndex];
   if (image == -1)
   {
      std::lock_guard<std::mutex> lock(sFreeLCDImagesMutex);
      for (auto it = sFreeLCDImages.begin(); it != sFreeLCDImages.end(); ++it)
      {
         if (it->first == gNanoVG)
         {
            image = it->second;
            sFreeLCDImages.erase(it);
            break;
         }
      }
   }

   if (image == -1)
      image = nvgCreateImageRGBA(gNanoVG, kWidth, kHeight, NVG_IMAGE_NEAREST, mLCDImagePixels.data());
   else
      nvgUpdateImage(gNanoVG, image, mLCDImagePixels.data());

   if (image == -1)
      return;

   NVGpaint paint = nvgImagePattern(gNanoVG, x, y, width, height, 0, image, alpha);
   nvgBeginPath(gNanoVG);
   nvgRect(gNanoVG, x, y, width, height);
   nvgFillPaint(gNanoVG, paint);
   nvgFill(gNanoVG);
}

void ControlSurface::PostRender()
{
   ControlSurfaceDevice* surface = mConnectedSurface;
   bool sendToDevice = surface != nullptr && surface->HasDisplay() && surface->IsDisplayConnected();
   bool usePanels = sendToDevice && surface->WantsDisplayPanels();
   if (sendToDevice || mShowLCDOnScreen || ShowsLCDOnModule())
      DrawToFramebuffer(usePanels);
   if (usePanels)
   {
      DrawDisplayPanels();
      surface->SendDisplayPanels(mDevice, mDisplayPanels);
   }
   else if (sendToDevice)
   {
      surface->SendDisplay(mDevice, mLCD);
   }
}

//for devices with room for several screens: a knob detail screen per knob, tinted with its track's color, plus the module list,
//the mixer, and the main screen
void ControlSurface::DrawDisplayPanels()
{
   mDisplayPanels.clear();

   bool mixer = ShouldDisplayMixer();
   for (int i = 0; i < (int)mKnobLCDs.size() && i < NumMainEncoders(); ++i)
   {
      AbletonMoveLCD& lcd = mKnobLCDs[i];
      lcd.Clear();
      int controlIndex = i + GetControlOffset();
      int color = kColorOff;
      if (mixer)
      {
         //in the mixer, each knob adjusts a track
         TrackOrganizer* track = mSessionOrganizer != nullptr ? mSessionOrganizer->GetTrack(controlIndex) : nullptr;
         FloatSlider* slider = GetMixerSlider(controlIndex);
         if (track != nullptr && slider != nullptr)
         {
            lcd.DrawLCDText(track->GetTrackName().c_str(), 5, 12, LCDFONT_STYLE_UNDERLINE);
            DrawKnobDisplay(lcd, slider);
            if (mLissajousDisplayMode != LissajousDisplayMode::Never)
               DrawLissajousDisplay(lcd, track);
            color = track->GetColorIndex();
         }
      }
      else if (IUIControl* control = GetControlAt(controlIndex))
      {
         DrawKnobDisplay(lcd, control);
         int flashColor;
         GetKnobColors(controlIndex, color, flashColor);
      }
      const ofColor& tint = kColors[std::clamp(color, 0, (int)kColors.size() - 1)].color;
      mDisplayPanels.push_back(ControlSurfaceDisplayPanel(&lcd, ControlSurfaceDisplayPanel::Role::Knob, tint));
   }

   const auto* trackRow = GetActiveTrackRow();
   ofColor moduleTint = ofColor::white;
   if (trackRow != nullptr)
      moduleTint = kColors[std::clamp(trackRow->GetColorIndex(), 0, (int)kColors.size() - 1)].color;

   mModuleListLCD.Clear();
   if (CanSelectModules())
      DrawModuleList(mModuleListLCD);
   mDisplayPanels.push_back(ControlSurfaceDisplayPanel(&mModuleListLCD, ControlSurfaceDisplayPanel::Role::ModuleList, moduleTint));

   mLissajousLCD.Clear();
   DrawLissajousDisplay(mLissajousLCD, trackRow);
   mDisplayPanels.push_back(ControlSurfaceDisplayPanel(&mLissajousLCD, ControlSurfaceDisplayPanel::Role::Lissajous, moduleTint));

   mMixerLCD.Clear();
   DrawMixer(mMixerLCD);
   mDisplayPanels.push_back(ControlSurfaceDisplayPanel(&mMixerLCD, ControlSurfaceDisplayPanel::Role::Mixer));

   mDisplayPanels.push_back(ControlSurfaceDisplayPanel(&mLCD, ControlSurfaceDisplayPanel::Role::Main));
}

void ControlSurface::OnClicked(float x, float y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);

   if (mConnectedSurface == nullptr)
      mNeedsConnect = true;
   else
      SendLeds(K(force));
}

void ControlSurface::DropdownUpdated(DropdownList* list, int oldVal, double time)
{
   if (list == mDeviceTypeDropdown)
      mNeedsConnect = true;
}

void ControlSurface::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadBool("show_lcd_on_screen", moduleInfo, false);

   SetUpFromSaveData();
}

void ControlSurface::SetUpFromSaveData()
{
   mShowLCDOnScreen = mModuleSaveData.GetBool("show_lcd_on_screen");
}

void ControlSurface::SaveLayout(ofxJSONElement& moduleInfo)
{
}

void ControlSurface::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);

   out << mAutoZoomToTrack;
}

void ControlSurface::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   LoadStateValidate(rev <= GetModuleSaveStateRev());

   if (rev >= 3)
      in >> mAutoZoomToTrack;
}

void ControlSurface::PostLoadState()
{
   //the device type was loaded with the dropdown's value, so connect to that one rather than auto-detecting
   mLoadedFromState = true;
   mNeedsConnect = true;
}

int ControlSurface::GetDisplayKnobIndex()
{
   //without touch sensing, keep showing the knob for a bit longer after it's turned
   double holdMs = Surface()->HasEncoderTouch() ? 200 : 500;
   if (mLastAdjustedKnobTime > gTime - holdMs)
      return mLastAdjustedKnobIndex;

   if (mMostRecentlyTouchedKnobIndex != -1 && GetButtonState(GetMainEncoderTouchControl(mMostRecentlyTouchedKnobIndex)))
      return mMostRecentlyTouchedKnobIndex;

   for (int i = 0; i < NumMainEncoders(); ++i)
   {
      if (GetButtonState(GetMainEncoderTouchControl(i)))
         return i;
   }

   return -1;
}

bool ControlSurface::ShouldDisplayMixer()
{
   for (int i = 0; i < NumTrackButtons(); ++i)
   {
      if (GetButtonState(GetTrackButtonControl(i)))
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

bool ControlSurface::ShouldDisplaySnapshotView() const
{
   if (GetButtonState(kDotButton) || GetButtonState(kStoreSnapshotButton))
      return true;
   if (mSelectedTrackRow == kTrackRowMixer)
      return true;
   if (GetButtonState(kClickyEncoderButton) && !mShowSoundSelector)
      return true;
   return false;
}

float ControlSurface::GetModuleViewOffset() const
{
   if (mSelectedTrackRow == kTrackRowGlobal)
   {
      if (GetCurrentGlobalModule() == mDisplayModule)
         return mGlobalModuleViewOffset;
   }
   if (mSessionOrganizer)
   {
      if (const auto* trackRow = mSessionOrganizer->GetTrack(mSelectedTrackRow))
      {
         if (trackRow->GetCurrentModule() == mDisplayModule)
            return trackRow->GetModuleViewOffset();
      }
   }

   return mModalModuleViewOffset;
}

void ControlSurface::SetModuleViewOffset(float offset)
{
   if (mSelectedTrackRow == kTrackRowGlobal)
   {
      if (GetCurrentGlobalModule() == mDisplayModule)
      {
         mGlobalModuleViewOffset = offset;
         return;
      }
   }

   if (mSessionOrganizer)
   {
      if (auto* trackRow = mSessionOrganizer->GetTrack(mSelectedTrackRow))
      {
         if (trackRow->GetCurrentModule() == mDisplayModule)
         {
            trackRow->SetModuleViewOffset(offset);
            return;
         }
      }
   }

   mModalModuleViewOffset = offset;
}

//separatePanels: the module list and mixer are shown on their own panels, so leave them off the main screen
void ControlSurface::DrawToFramebuffer(bool separatePanels)
{
   if (gTime < mScreenOverrideTimeout)
      return;

   mLCD.Clear();

   bool needToDraw = true;

   if (needToDraw && mShowAddTrackSelector)
   {
      const int kMaxDisplayEntries = 5;
      int numValues = (int)mAddTrackSelectorList.size();
      int displayOffset = std::clamp(mAddTrackSelectorIndex - 2, 0, std::max(numValues - kMaxDisplayEntries, 0));
      int y = 12;
      for (int i = displayOffset; i < numValues && i < displayOffset + kMaxDisplayEntries; ++i)
      {
         juce::File trackFile(mAddTrackSelectorList[i]);
         std::string label = trackFile.getFileNameWithoutExtension().toStdString();
         mLCD.DrawLCDText(label.c_str(), 13, y, LCDFONT_STYLE_REGULAR);
         if (i == mAddTrackSelectorIndex)
            mLCD.DrawArrow(9, y - 4, 4, false, LCDDrawMode::Fill);
         y += 12;
      }

      needToDraw = false;
   }

   if (needToDraw && mCurrentControlRecorder != nullptr)
   {
      int displayKnobIndex = GetDisplayKnobIndex();
      if (displayKnobIndex != -1)
      {
         DrawKnobDisplay(mLCD, GetControlAt(displayKnobIndex + GetControlOffset()));
      }
      needToDraw = false;
   }

   if (needToDraw && mGridControlInterface != nullptr)
   {
      if (mGridControlInterface->UpdateAbletonMoveScreen(this, &mLCD, IAbletonGridController::LCDDrawPass::HighPriority))
         needToDraw = false;
   }

   if (needToDraw && mShowSoundSelector && GetDisplayKnobIndex() == -1)
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
               mLCD.DrawArrow(9, y - 4, 4, false, LCDDrawMode::Fill);
            y += 12;
         }
      }
      needToDraw = false;
   }

   //scroll module offset
   int trackRowSpacingY = 8;
   bool holdingModuleSelectInput = GetButtonState(kClickyEncoderTouch) && GetButtonState(kClickyEncoderButton);
   bool moduleIsScrolling = fabsf(mTrackRowOffsetSmoothed - mTrackRowOffset) > .05f;
   if (needToDraw && (holdingModuleSelectInput || moduleIsScrolling))
   {
      mTrackRowOffsetSmoothed = ofLerp(mTrackRowOffsetSmoothed, mTrackRowOffset, 0.3f);
      for (int i = 0; mSessionOrganizer != nullptr && i < mSessionOrganizer->GetNumTracks(); ++i)
      {
         mLCD.DrawRect(5, 2 + i * trackRowSpacingY, 4, 5, i == mSelectedTrackRow ? LCDDrawMode::Fill : LCDDrawMode::Outline);
         TrackOrganizer* trackRow = mSessionOrganizer->GetTrack(i);
         if (trackRow != nullptr)
            mLCD.DrawLCDText(trackRow->GetTrackName().c_str(), 10, 1 + i * trackRowSpacingY + 6);
      }
      mLCD.DrawRect(3, 0 + int(mTrackRowOffsetSmoothed * trackRowSpacingY), mLCD.GetWidth() - 3, trackRowSpacingY * NumTrackButtons() + 1, LCDDrawMode::Outline);

      needToDraw = false;
   }

   bool canScrollModules = CanSelectModules();
   if (needToDraw && !separatePanels && ((GetButtonState(kClickyEncoderTouch) && canScrollModules) || gTime < mDisplayModuleSelectTimeout))
   {
      DrawModuleList(mLCD);
      needToDraw = false;
   }

   if (needToDraw && ShouldDisplaySnapshotView() && GetButtonState(kClickyEncoderTouch))
   {
      mLCD.DrawLCDText(("snapshot offset: " + ofToString(mSnapshotOffset)).c_str(), 5, 10, LCDFONT_STYLE_REGULAR);
      needToDraw = false;
   }

   int mixerAdjustTrackIndex = GetMixerAdjustTrackIndex();
   if (needToDraw && ShouldDisplayMixer())
   {
      if (!separatePanels)
         DrawMixer(mLCD);
      needToDraw = false;
   }

   if (needToDraw && mGridControlInterface != nullptr)
   {
      if (mGridControlInterface->UpdateAbletonMoveScreen(this, &mLCD, IAbletonGridController::LCDDrawPass::Normal))
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
         if (IDrawableModule* currentControlModule = GetCurrentControlModule())
            nameModule = currentControlModule;

         if (nameModule != nullptr)
            display += nameModule->Name();

         FloatSliderLFOControl* lfo = dynamic_cast<FloatSliderLFOControl*>(GetCurrentControlModule());
         if (lfo != nullptr)
         {
            if (FloatSlider* owningSlider = lfo->GetOwner())
               display = "lfo: " + owningSlider->GetDisplayName();
         }

         mLCD.DrawLCDText(display.c_str(), 5, 12, LCDFONT_STYLE_UNDERLINE);
      }

      if (IDrawableModule* currentControlModule = GetCurrentControlModule())
      {
         if (GetDisplayKnobIndex() == -1)
         {
            //label the current page of controls
            for (int i = 0; i < std::min(NumMainEncoders(), EncoderPageSize()); ++i)
            {
               int controlIndex = GetControlOffset() + i;
               if (controlIndex >= 0 && controlIndex < (int)mControls.size())
               {
                  if (auto* control = mControls[controlIndex])
                  {
                     std::string displayName = control->GetDisplayName();
                     if (control->GetModulator() && control->GetModulator()->Active())
                        displayName = "*" + displayName;
                     if (displayName.length() > 12 && controlIndex + 4 < (int)mControls.size()) // truncate if we might stomp on the control name in the next column
                        displayName = displayName.substr(0, 12);
                     int x = i * 14 + 3;
                     int y = 26 + (i % 4) * 10;
                     mLCD.DrawLCDText(displayName.c_str(), x + 3, y);

                     int pixelHeight = control->GetMidiValue() * 8;
                     mLCD.DrawRect(x, y - pixelHeight, 1, pixelHeight, LCDDrawMode::Outline);
                  }
               }
            }

            if ((int)mControls.size() > NumMainEncoders())
            {
               const int kModuleViewBarX = 4;
               const int kModuleViewBarY = 58;
               const int kModuleViewBarWidth = 124;
               const int kModuleViewBarHeight = 6;
               mLCD.DrawRect(kModuleViewBarX, kModuleViewBarY, kModuleViewBarWidth, kModuleViewBarHeight, LCDDrawMode::Outline);
               float numControlsQuantized = ceil((float)mControls.size() / EncoderPageSize()) * EncoderPageSize();
               float offsetStart = GetControlOffset() / numControlsQuantized;
               float offsetEnd = MIN(GetControlOffset() + NumMainEncoders(), numControlsQuantized) / numControlsQuantized;
               mLCD.DrawRect(kModuleViewBarX + offsetStart * kModuleViewBarWidth, kModuleViewBarY + 2, (offsetEnd - offsetStart) * kModuleViewBarWidth, kModuleViewBarHeight - 4, LCDDrawMode::Fill);
               if (GetButtonState(kVolumeEncoderTouch) && !Surface()->HasControlPageButtons())
                  mLCD.DrawRect(kModuleViewBarX + (GetModuleViewOffset() * EncoderPageSize() / numControlsQuantized) * kModuleViewBarWidth, kModuleViewBarY, 1, kModuleViewBarHeight, LCDDrawMode::Fill);
            }
         }
      }

      int displayKnobIndex = GetDisplayKnobIndex();
      if (displayKnobIndex != -1 && !(separatePanels && ShouldDisplayMixer()))
         DrawKnobDisplay(mLCD, GetControlAt(displayKnobIndex + GetControlOffset()));

      FloatSliderLFOControl* lfo = dynamic_cast<FloatSliderLFOControl*>(GetCurrentControlModule());
      if (lfo != nullptr)
         lfo->DrawToAbletonMoveScreen(&mLCD);
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
   mLCD.DrawRect(0, 0, ((measurePos + (TheTransport->GetMeasure(gTime) % wrapLengthMeasures)) / wrapLengthMeasures) * mLCD.GetWidth(), 1, LCDDrawMode::Fill);
   float numBeats = TheTransport->GetTimeSigTop();
   int beat = int(measurePos * numBeats);
   float beatWidth = mLCD.GetWidth() / numBeats;
   mLCD.DrawRect(beat * beatWidth, 1, beatWidth, 1, LCDDrawMode::Fill);

   if (!ShouldDisplayMixer())
   {
      if (track != nullptr)
      {
         Amplifier* gain = track->GetGain();
         if (gain != nullptr)
         {
            float level, watermarkLevel;
            gain->GetLevel(level, watermarkLevel);
            mLCD.DrawRect(0, 2, std::clamp(int(level * mLCD.GetWidth()), 0, mLCD.GetWidth()), 1, LCDDrawMode::Fill);
            mLCD.DrawRect(std::clamp(int(watermarkLevel * mLCD.GetWidth()), 0, mLCD.GetWidth()), 2, 1, 2, LCDDrawMode::Fill);
         }
      }
   }

   if (mSessionOrganizer != nullptr)
      mLCD.DrawRect(0, 0, 1, trackRowSpacingY * mSessionOrganizer->GetNumTracks(), LCDDrawMode::Fill);
   mLCD.DrawRect(0, int(mTrackRowOffsetSmoothed * trackRowSpacingY), 3, trackRowSpacingY * NumTrackButtons() + 1, LCDDrawMode::Fill);
   if (mSelectedTrackRow >= 0 && int(gTime / 200) % 2 == 0)
   {
      float trackY = trackRowSpacingY * mSelectedTrackRow;
      mLCD.DrawRect(1, trackY, 1, trackRowSpacingY - 1, LCDDrawMode::Toggle);
   }

   if (mLissajousDisplayMode == LissajousDisplayMode::Always ||
       (ShouldDisplayMixer() && mLissajousDisplayMode == LissajousDisplayMode::MixerTrack))
   {
      TrackOrganizer* trackRowToDisplay = nullptr;
      if (!ShouldDisplayMixer())
         trackRowToDisplay = GetActiveTrackRow();
      else if (mixerAdjustTrackIndex != -1 && mSessionOrganizer != nullptr)
         trackRowToDisplay = mSessionOrganizer->GetTrack(mixerAdjustTrackIndex);

      if (mLissajousDisplayMode == LissajousDisplayMode::MixerTrack && trackRowToDisplay == nullptr)
      {
         //don't display, this is the global buffer
      }
      else
      {
         DrawLissajousDisplay(mLCD, trackRowToDisplay);
      }
   }

   if (mGridControlInterface != nullptr)
      mGridControlInterface->UpdateAbletonMoveScreen(this, &mLCD, IAbletonGridController::LCDDrawPass::Overlay);

   if (gTime < mTemporaryScreenMessageTimeout)
   {
      mLCD.DrawRect(1, 2, mLCD.GetWidth() - 2, 13, LCDDrawMode::Clear);
      mLCD.DrawRect(1, 2, mLCD.GetWidth() - 2, 13, LCDDrawMode::Outline);
      mLCD.DrawLCDText(mTemporaryScreenMessage.c_str(), 4, 12, LCDFONT_STYLE_REGULAR);
   }
}

bool ControlSurface::CanSelectModules() const
{
   return mSelectedTrackRow == kTrackRowGlobal || mSelectedTrackRow >= 0;
}

void ControlSurface::DrawModuleList(AbletonMoveLCD& lcd)
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

   lcd.DrawLCDText((trackName + ":").c_str(), 5, 12, LCDFONT_STYLE_REGULAR);
   for (int i = 0; i < (int)mTrackControlLayout.size(); ++i)
   {
      lcd.DrawLCDText(mTrackControlLayout[i].mName.c_str(), 14, 20 + i * 10, i == moduleIndex ? LCDFONT_STYLE_UNDERLINE : LCDFONT_STYLE_REGULAR);
      for (int j = 0; j < mTrackControlLayout[i].mPageCount; ++j)
      {
         lcd.DrawRect(84 + j * 9, 14 + i * 10, 5, 5, i == moduleIndex && int(GetModuleViewOffset()) == j ? LCDDrawMode::Fill : LCDDrawMode::Outline);
      }
   }
}

//the gain (or with shift, send) slider that the mixer adjusts for a track
FloatSlider* ControlSurface::GetMixerSlider(int trackIndex) const
{
   TrackOrganizer* track = mSessionOrganizer != nullptr ? mSessionOrganizer->GetTrack(trackIndex) : nullptr;
   if (track == nullptr)
      return nullptr;
   if (mShiftHeld)
      return track->GetSend() != nullptr ? track->GetSend()->GetAmountSlider() : nullptr;
   return track->GetGain() != nullptr ? track->GetGain()->GetGainSlider() : nullptr;
}

//the track whose gain is being touched or was just adjusted, or -1
int ControlSurface::GetMixerAdjustTrackIndex()
{
   int adjustTrackIndex = -1;
   if (ShouldDisplayMixer())
   {
      for (int i = 0; mSessionOrganizer != nullptr && i < mSessionOrganizer->GetNumTracks(); ++i)
      {
         TrackOrganizer* track = mSessionOrganizer->GetTrack(i);
         if (track == nullptr || track->GetGain() == nullptr || GetMixerSlider(i) == nullptr)
            continue;
         if (GetButtonState(GetMainEncoderTouchControl(i)) ||
             (i == mLastGainAdjustTrackIndex && mLastGainAdjustTrackTime > gTime - 300) ||
             (i == mVolumeKnobTrackIndex && mVolumeKnobTrackTime > gTime - 300))
            adjustTrackIndex = i;
      }
   }
   return adjustTrackIndex;
}

void ControlSurface::DrawMixer(AbletonMoveLCD& lcd)
{
   int mixerAdjustTrackIndex = GetMixerAdjustTrackIndex();
   std::string title = "mixer";
   if (mShiftHeld)
      title = "sends";
   lcd.DrawLCDText(title.c_str(), 5, 10, LCDFONT_STYLE_UNDERLINE);

   const int kBarPadBelow = 3;
   const int kMaxBarHeight = lcd.GetHeight() - 16;
   for (int i = 0; mSessionOrganizer != nullptr && i < mSessionOrganizer->GetNumTracks(); ++i)
   {
      int x = 5 + i * 16;

      TrackOrganizer* track = mSessionOrganizer->GetTrack(i);
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
            bool isBeingAdjusted = (i == mixerAdjustTrackIndex);

            float level, watermarkLevel;
            gain->GetLevel(level, watermarkLevel);
            int sliderPosition = std::clamp(int(adjustSlider->GetMidiValue() * kMaxBarHeight), 0, kMaxBarHeight);
            int levelHeight = std::clamp(int(level * kMaxBarHeight), 0, kMaxBarHeight);
            int watermarkLevelHeight = std::clamp(int(watermarkLevel * kMaxBarHeight), 0, kMaxBarHeight);
            lcd.DrawRect(x, lcd.GetHeight() - kBarPadBelow - kMaxBarHeight, 4, kMaxBarHeight, LCDDrawMode::Outline);
            lcd.DrawRect(x, lcd.GetHeight() - kBarPadBelow - levelHeight, 4, levelHeight, LCDDrawMode::Fill);
            lcd.DrawRect(x + 5, lcd.GetHeight() - kBarPadBelow - watermarkLevelHeight, 2, 1, LCDDrawMode::Fill);
            if (!track->IsTrackEnabled())
            {
               lcd.DrawLCDText("x", x + 7, lcd.GetHeight() - kBarPadBelow - sliderPosition + 3);
            }
            else
            {
               //arrow
               int arrowSize = 4;
               bool fillArrow = false;
               if (isBeingAdjusted)
               {
                  arrowSize = 5;
                  fillArrow = true;
               }
               lcd.DrawArrow(x + 7, lcd.GetHeight() - kBarPadBelow - sliderPosition, arrowSize, true, fillArrow ? LCDDrawMode::Fill : LCDDrawMode::Outline);
            }
            //lcd.DrawRect(x + 7, lcd.GetHeight() - kBarPadBelow - gainPosition - 3, 4, 6, LCDDrawMode::Fill);
         }
      }
   }
   if (mixerAdjustTrackIndex != -1)
   {
      TrackOrganizer* track = mSessionOrganizer->GetTrack(mixerAdjustTrackIndex);
      FloatSlider* adjustSlider = GetMixerSlider(mixerAdjustTrackIndex);
      if (track != nullptr && adjustSlider != nullptr)
      {
         std::string display = track->GetTrackName();
         display += "  " + ofToString(adjustSlider->GetValue(), 2);
         lcd.DrawLCDText(display.c_str(), 52, 10, LCDFONT_STYLE_REGULAR);
      }
   }
}

void ControlSurface::DrawKnobDisplay(AbletonMoveLCD& lcd, IUIControl* control)
{
   if (control != nullptr)
   {
      std::string displayName = control->GetDisplayName();
      if (control->GetModulator() && control->GetModulator()->Active())
         displayName = "*" + displayName;
      lcd.DrawLCDText(displayName.c_str(), 5, 26);
      std::string displayValue = control->GetDisplayValue(control->GetValue());
      if (control->GetModulator() && control->GetModulator()->Active())
      {
         if (mShiftHeld)
            displayValue += "   min: " + control->GetDisplayValue(control->GetModulator()->GetMin());
         else
            displayValue += "   max: " + control->GetDisplayValue(control->GetModulator()->GetMax());
      }
      lcd.DrawLCDText(displayValue.c_str(), 5, 40);

      int sliderX = 5;
      int sliderY = 42;
      int sliderW = lcd.GetWidth() - 6;
      int sliderH = 10;
      int sliderMinPos = sliderX + 2;
      int sliderMaxPos = sliderX + sliderW - 3;
      lcd.DrawRect(sliderX, sliderY, sliderW, sliderH, LCDDrawMode::Outline);
      lcd.DrawRect(ofLerp(sliderMinPos, sliderMaxPos, control->GetMidiValue()), sliderY + 2, 1, sliderH - 4, LCDDrawMode::Outline);

      int numValues = control->GetNumValues();
      if (numValues > 0 && numValues < 20)
      {
         for (int i = 1; i < numValues; ++i)
         {
            int x = ofLerp(sliderMinPos, sliderMaxPos, (i - 0.5f) / (numValues - 1));
            lcd.DrawRect(x, sliderY, 1, 2, LCDDrawMode::Outline);
            lcd.DrawRect(x, sliderY + sliderH - 2, 1, 2, LCDDrawMode::Outline);
         }
      }

      IModulator* modulator = control->GetModulator();
      if (modulator != nullptr)
      {
         if (modulator == mCurrentControlRecorder)
         {
            lcd.DrawLCDText("* RECORDING *", 5, 12, LCDFONT_STYLE_UNDERLINE);
         }
         else if (modulator->Active())
         {
            float sliderMin = 0;
            float sliderMax = 1;
            FloatSlider* slider = dynamic_cast<FloatSlider*>(control);
            if (slider)
            {
               sliderMin = slider->GetMin();
               sliderMax = slider->GetMax();
            }
            float modMin = ofMap(modulator->GetMin(), sliderMin, sliderMax, 0, 1);
            float modMax = ofMap(modulator->GetMax(), sliderMin, sliderMax, 0, 1);
            int min = ofLerp(sliderMinPos, sliderMaxPos, modMin);
            int max = ofLerp(sliderMinPos, sliderMaxPos, modMax);
            if (min > max) //swap
            {
               float temp = min;
               min = max;
               max = temp;
            }
            lcd.DrawRect(min, sliderY + 4, max - min, sliderH - 8, LCDDrawMode::Fill);
         }
      }

      ControlRecorder* controlRecorder = dynamic_cast<ControlRecorder*>(modulator);
      if (controlRecorder != nullptr && (modulator->Active() || modulator == mCurrentControlRecorder))
      {
         int progressPixel = -1;
         if (controlRecorder->Active())
            progressPixel = int(controlRecorder->GetPlaybackTime(gTime) / controlRecorder->GetLength() * lcd.GetWidth());

         int lastY = -1;
         for (float x = 0; x < lcd.GetWidth(); ++x)
         {
            float percent = x / lcd.GetWidth();
            int newY = int(ofMap(controlRecorder->GetValue(percent),
                                 modulator->GetMin(), modulator->GetMax(),
                                 lcd.GetHeight() - 10, 10));
            if (lastY == -1)
               lastY = newY;
            if (lastY <= newY)
            {
               for (int y = lastY; y <= newY; ++y)
                  lcd.DrawPixel(x, y, LCDDrawMode::Toggle);
            }
            else
            {
               for (int y = lastY; y >= newY; --y)
                  lcd.DrawPixel(x, y, LCDDrawMode::Toggle);
            }

            if (x == progressPixel)
               lcd.DrawRect(x - 1, newY - 1, 3, 3, LCDDrawMode::Outline);

            lastY = newY;
         }
      }

      if (IControlVisualizer* visualizer = control->GetControlVisualizer())
         visualizer->DrawVisualizationToScreen(&lcd, control);
   }
}

void ControlSurface::DrawLissajousDisplay(AbletonMoveLCD& lcd, const TrackOrganizer* track)
{
   RollingBuffer* buffer = nullptr;

   if (track != nullptr)
   {
      auto* gain = track->GetGain();
      if (gain != nullptr)
         buffer = gain->GetVizBuffer();
   }
   else
   {
      buffer = TheSynth->GetGlobalRecordBuffer();
   }

   if (buffer != nullptr)
   {
      const int kDelaySamps = 90;
      const float kScale = 1.2f;
      const int kCenterX = lcd.GetWidth() / 2;
      const int kCenterY = lcd.GetHeight() / 2;
      std::vector<bool> lissajousPixels(lcd.GetWidth() * lcd.GetHeight(), false);

      int numPoints = MIN(buffer->Size() - kDelaySamps - 1, .02f * gSampleRate);
      for (int i = 100; i < numPoints; ++i)
      {
         int x = kCenterX + buffer->GetSample(i, 0) * kScale * lcd.GetHeight();
         int y = kCenterY + buffer->GetSample(i + kDelaySamps, 1) * kScale * lcd.GetHeight();
         if (x >= 0 && x < lcd.GetWidth() && y >= 0 && y < lcd.GetHeight())
            lissajousPixels[x + y * lcd.GetWidth()] = true;
      }

      for (int i = 0; i < lcd.GetWidth() * lcd.GetHeight(); ++i)
      {
         if (lissajousPixels[i])
            lcd.DrawPixel(i % lcd.GetWidth(), i / lcd.GetWidth(), LCDDrawMode::Toggle);
      }
   }
}

//the color a knob's light (and its push 2 screen) uses for the control at controlIndex
void ControlSurface::GetKnobColors(int controlIndex, int& color, int& flashColor) const
{
   color = kColorLightGrey;
   flashColor = -1;

   const auto* trackRow = GetActiveTrackRow();
   if (trackRow != nullptr)
      color = trackRow->GetColorIndex();

   if (dynamic_cast<FloatSliderLFOControl*>(GetCurrentControlModule()))
      color = kColorPurple;

   IUIControl* control = GetControlAt(controlIndex);
   if (control != nullptr && control->GetModulator() != nullptr &&
       (control->GetModulator()->Active() || control->GetModulator() == mCurrentControlRecorder))
   {
      FloatSlider* slider = dynamic_cast<FloatSlider*>(control);
      if (slider != nullptr && slider->GetLFO()) // flash purple for LFO
      {
         color = kColorBrightMagenta;
         flashColor = kColorPaleMagenta;
      }
      else // flash red for other modulator
      {
         color = kColorRed;
         flashColor = kColorSoftRed;
      }
   }
}

IUIControl* ControlSurface::GetControlAt(int controlIndex) const
{
   if (controlIndex >= 0 && controlIndex < (int)mControls.size())
      return mControls[controlIndex];
   return nullptr;
}

void ControlSurface::UpdateLeds()
{
   TrackOrganizer* activeTrackRow = GetActiveTrackRow();

   for (auto& state : mControlState)
      state.mLedPriority = LedPriority::None;

   //step buttons
   SongBuilder* songBuilder = dynamic_cast<SongBuilder*>(mSongBuilderCable->GetTarget());
   for (int i = 0; i < NumStepButtons(); ++i)
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
            IInputRecordable* retroactiveRecorder = recorder->IsInRetroactiveRecorderMode() ? recorder : nullptr;
            if (retroactiveRecorder == nullptr)
            {
               Looper* looper = dynamic_cast<Looper*>(recorder);
               if (looper != nullptr)
                  retroactiveRecorder = looper->GetRecorder();
            }
            if (retroactiveRecorder != nullptr && recorder->IsInRetroactiveRecorderMode())
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

   if (MuteButtonMutesTracks())
   {
      //flashing while the selected track is off. high priority so grid interfaces that light mute for their own use don't hide it
      TrackOrganizer* track = GetActiveTrackRow();
      if (track != nullptr && track->GetEnabledControl())
         SetLed(kMoveMuteButton, track->GetColorIndex(), track->IsTrackEnabled() ? -1 : kColorOff, LedPriority::High);
      else
         SetLed(kMoveMuteButton, kColorWhite, -1, LedPriority::High);
   }
   else
   {
      SetLed(kMoveMuteButton, ShouldDisplayMixer() ? 127 : 0);
   }

   //only grid interfaces use copy, so it's off unless the current one lights it
   SetLed(kCopyButton, 0);

   bool soundSelectorAvailable = false;
   TrackOrganizer* trackRow = GetActiveTrackRow();
   IUIControl* soundSelector = trackRow ? trackRow->GetSoundSelector() : nullptr;
   if (soundSelector != nullptr)
      soundSelectorAvailable = true;

   SetLed(kUpButton, 0);
   SetLed(kDownButton, 0);
   SetLed(kLeftButton, 0);
   SetLed(kRightButton, 0);
   if (GetButtonState(kShiftButton))
   {
      if (mTrackRowOffset != GetMaxTrackRowOffset())
         SetLed(kOctaveDownButton, 127, 10, LedPriority::High);
      else
         SetLed(kOctaveDownButton, 0, -1, LedPriority::High);

      if (mTrackRowOffset != 0)
         SetLed(kOctaveUpButton, 127, 10, LedPriority::High);
      else
         SetLed(kOctaveUpButton, 0, -1, LedPriority::High);
   }
   else if (GetButtonState(kCaptureButton))
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
      SetLed(kPageLeftButton, 0, -1, LedPriority::Low);
      SetLed(kPageRightButton, 0, -1, LedPriority::Low);
      SetLed(kOctaveUpButton, mTrackRowOffset > 0 ? 127 : 0, -1, LedPriority::Low);
      SetLed(kOctaveDownButton, mTrackRowOffset < GetMaxTrackRowOffset() ? 127 : 0, -1, LedPriority::Low);
      SetLed(kMoveUndoButton, 0);

      Snapshots* snapshots = activeTrackRow != nullptr ? activeTrackRow->GetSnapshots() : nullptr;
      if (snapshots != nullptr)
      {
         if (mShiftHeld && soundSelectorAvailable)
            SetLed(kDotButton, activeTrackRow->GetColorIndex(), 0);
         else
            SetLed(kDotButton, activeTrackRow->GetColorIndex());
      }
      else
      {
         SetLed(kDotButton, 0);
      }
   }

   if (mSelectedTrackRow == kTrackRowTransport)
   {
      SetLed(kMoveDeleteButton, 20, TheSynth->IsAudioPaused() ? 127 : -1, LedPriority::High);
   }
   else
   {
      TrackOrganizer* trackRow = GetActiveTrackRow();
      bool trackButtonHeld = false;
      for (int i = 0; i < NumTrackButtons(); ++i)
      {
         if (GetButtonState(GetTrackButtonControl(i)))
            trackButtonHeld = true;
      }

      if (trackRow != nullptr && trackButtonHeld)
      {
         if (trackRow->GetEnabledControl())
            SetLed(kMoveDeleteButton, 127, trackRow->IsTrackEnabled() ? -1 : 0, LedPriority::High);
         else
            SetLed(kMoveDeleteButton, 0, -1, LedPriority::High);
      }
      else
      {
         SetLed(kMoveDeleteButton, 0);
      }
   }

   SetLed(kBackButton, 127, mSelectedTrackRow == kTrackRowGlobal ? 20 : -1);
   SetLed(kHamburgerButton, kColorWhite, mSelectedTrackRow == kTrackRowMixer ? 20 : -1); //white rather than 127, which is red on the launch control 3
   SetLed(kPlayButton, TheSynth->IsAudioPaused() ? kColorRed : kColorGreen, mSelectedTrackRow == kTrackRowTransport ? kColorBrightBlue : -1);

   int recordColor1 = kColorOff;
   int recordColor2 = -1;
   if (activeTrackRow)
   {
      auto* recorder = activeTrackRow->GetRecorder();
      if (recorder != nullptr)
      {
         if (GetButtonState(kMoveDeleteButton))
         {
            recordColor1 = kColorBlue;
            recordColor2 = kColorWhite;
         }
         else if (recorder->IsRecording())
         {
            if (GetButtonState(kShiftButton))
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
            recordColor1 = kColorDarkGrey;
         }
      }
   }
   SetLed(kCircleButton, recordColor1, recordColor2);

   if (ShouldDisplaySnapshotView() && mSessionOrganizer != nullptr && HasGrid())
   {
      const int numCols = GetGridNumCols();
      if (GetButtonState(kMoveMuteButton))
      {
         //one pad per track, reading left to right from the top row. flashing means the track is disabled
         for (int i = GetGridStartIndex(); i < GetGridStartIndex() + GetGridNumPads(); ++i)
         {
            int col = (i - GetGridStartIndex()) % numCols;
            int row = GetGridNumRows() - 1 - (i - GetGridStartIndex()) / numCols;
            int trackIndex = col + row * numCols;
            if (TrackOrganizer* track = mSessionOrganizer->GetTrack(trackIndex))
               SetLed(i, track->GetColorIndex(), track->IsTrackEnabled() ? -1 : 127);
            else
               SetLed(i, kColorOff);
         }
      }
      else
      {
         //a row of pads per visible track, a column per snapshot
         for (int i = 0; i < GetGridNumRows(); ++i)
         {
            std::vector<int> colors(numCols, kColorOff);
            std::vector<int> colors2(numCols, -1);
            TrackOrganizer* trackRow = i < NumTrackButtons() ? mSessionOrganizer->GetTrack(i + mTrackRowOffset) : nullptr;
            if (trackRow != nullptr)
            {
               Snapshots* snapshots = trackRow->GetSnapshots();
               if (snapshots != nullptr)
               {
                  for (int col = 0; col < numCols; ++col)
                  {
                     int snapshotIdx = col + mSnapshotOffset;
                     if (snapshots->GetCurrentSnapshot() == snapshotIdx)
                     {
                        colors[col] = IsStoreSnapshotHeld() ? kColorRed : kColorWhite;
                        if (IUIControl::IsInactiveValue(snapshots->GetLabel(snapshotIdx)))
                           colors2[col] = kColorDarkGrey;
                        else
                           colors2[col] = trackRow->GetColorIndex();
                     }
                     else if (snapshots->HasSnapshot(snapshotIdx))
                     {
                        if (IUIControl::IsInactiveValue(snapshots->GetLabel(snapshotIdx)))
                           colors[col] = kColorDarkGrey;
                        else
                           colors[col] = trackRow->GetColorIndex();
                     }
                  }
               }
            }

            for (int col = 0; col < numCols; ++col)
               SetLed(GetGridStartIndex() + (GetGridNumRows() - 1 - i) * numCols + col, colors[col], colors2[col]);
         }
      }
   }
   else if (mGridControlInterface != nullptr)
   {
      mGridControlInterface->UpdateAbletonGridLeds(this);
   }
   else
   {
      for (int i = GetGridStartIndex(); i < GetGridStartIndex() + GetGridNumPads(); ++i)
         SetLed(i, kColorOff);
   }

   SetLed(kShiftButton, 127, mShiftHeld || mBottomRowMode ? 0 : -1);

   if (mAutoZoomToTrack)
      SetLed(kLedNew, 127);
   else if (mBottomRowMode)
      SetLed(kLedNew, 10);
   else
      SetLed(kLedNew, 0);

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

   if (mShowAddTrackSelector)
      SetLed(kLedAdd, 127, mBottomRowMode ? 30 : -1);
   else if (mBottomRowMode)
      SetLed(kLedAdd, 10);
   else
      SetLed(kLedAdd, 0);

   //dedicated buttons: bright while active, dim while available
   bool knobTouched = mMostRecentlyTouchedKnobIndex != -1 && GetButtonState(GetMainEncoderTouchControl(mMostRecentlyTouchedKnobIndex));
   auto* activeRecorder = GetActiveTrackRow() != nullptr ? GetActiveTrackRow()->GetRecorder() : nullptr;
   SetLed(kClickyEncoderButton, mShowAddTrackSelector || mShowSoundSelector ? 127 : 0); //lit when there's a choice to confirm (push 2's tap tempo)
   SetLed(kAddTrackSelectorButton, mShowAddTrackSelector ? 127 : 20);
   SetLed(kRandomizeButton, IsRandomizeChordHeld() ? 127 : 20);
   SetLed(kStoreSnapshotButton, GetButtonState(kStoreSnapshotButton) ? 127 : 20);
   SetLed(kSoundSelectorButton, mShowSoundSelector ? 127 : (soundSelectorAvailable ? 20 : 0));
   SetLed(kScalePageButton, mDisplayModule == TheScale ? 127 : 20);
   SetLed(kAutoZoomButton, mAutoZoomToTrack ? 127 : 20);
   SetLed(kLFOButton, GetButtonState(kLFOButton) || knobTouched ? 127 : 20);
   SetLed(kBindButton, sBindToUIControl != nullptr ? 127 : 20);
   SetLed(kCancelRecordingButton, activeRecorder != nullptr && activeRecorder->IsRecording() ? kColorRed : kColorOff);

   if (mBottomRowMode)
   {
      SetLed(kButtonNew, kColorLightGrey, 0);
      SetLed(kButtonSettings, kColorOff);
      SetLed(kButtonBranch, kColorOff);
      SetLed(kButtonDowel, kColorOff);
      SetLed(kButtonTempo, kColorOff, 0);
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

   if (UseTrackButtonsForSnapshots())
   {
      Snapshots* snapshots = activeTrackRow != nullptr ? activeTrackRow->GetSnapshots() : nullptr;
      for (int i = 0; i < NumTrackButtons(); ++i)
      {
         int color = kColorOff;
         int flashColor = -1;
         int snapshotIdx = i + mSnapshotOffset;
         if (snapshots != nullptr && snapshots->HasSnapshot(snapshotIdx))
         {
            if (IUIControl::IsInactiveValue(snapshots->GetLabel(snapshotIdx)))
               color = kColorDarkGrey;
            else
               color = activeTrackRow->GetColorIndex();
            if (snapshots->GetCurrentSnapshot() == snapshotIdx)
               flashColor = kColorWhite;
         }
         SetLed(GetTrackButtonControl(i), color, flashColor);
      }
   }
   else
   {
      for (int i = 0; i < NumTrackButtons(); ++i)
      {
         int trackIndex = i + mTrackRowOffset;
         TrackOrganizer* trackRow = mSessionOrganizer != nullptr ? mSessionOrganizer->GetTrack(trackIndex) : nullptr;
         int buttonNumber = GetTrackButtonControl(i);
         if (trackRow != nullptr)
         {
            bool isActiveTrack = (trackIndex == mSelectedTrackRow);
            bool isEnabled = trackRow->IsTrackEnabled();
            int colorIndex = trackRow->GetColorIndex();
            if (!isEnabled)
            {
               if (isActiveTrack)
                  SetLed(buttonNumber, kColorDimRed, 0);
               else
                  SetLed(buttonNumber, colorIndex, kColorDimRed);
            }
            else
            {
               if (isActiveTrack)
                  SetLed(buttonNumber, colorIndex, 0);
               else
                  SetLed(buttonNumber, colorIndex);
            }
         }
         else
         {
            SetLed(buttonNumber, 0);
         }
      }
   }

   //devices that support it show each encoder's value as its led brightness
   const auto GetLedBrightness = [](const IUIControl* control)
   {
      return ofMap(control != nullptr ? control->GetMidiValue() : 1.0f, 0.0f, 1.0f, 0.05f, 1.0f, K(clamp));
   };

   if (mSelectedTrackRow == kTrackRowMixer)
   {
      for (int i = 0; i < NumMainEncoders(); ++i)
      {
         const auto* trackRow = mSessionOrganizer != nullptr ? mSessionOrganizer->GetTrack(i) : nullptr;
         Amplifier* gain = trackRow != nullptr ? trackRow->GetGain() : nullptr;
         SetLedWithBrightness(GetMainEncoderControl(i), trackRow != nullptr ? trackRow->GetColorIndex() : kColorOff, -1, GetLedBrightness(gain != nullptr ? gain->GetGainSlider() : nullptr));
      }
   }
   else
   {
      for (int i = 0; i < NumMainEncoders(); ++i)
      {
         int controlIndex = i + GetControlOffset();
         if (controlIndex >= 0 && controlIndex < (int)mControls.size())
         {
            int color, flashColor;
            GetKnobColors(controlIndex, color, flashColor);
            SetLedWithBrightness(GetMainEncoderControl(i), color, flashColor, GetLedBrightness(mControls[controlIndex]));
         }
         else
         {
            SetLed(GetMainEncoderControl(i), kColorOff);
         }
      }
   }

   SetLed(kCaptureButton, 127, GetBindButton() == kCaptureButton && sBindToUIControl != nullptr ? 0 : -1); //flashing while waiting for a midi controller to bind

   //navigation buttons, lit when they can do something
   bool canSelectModule = mSelectedTrackRow == kTrackRowGlobal || GetActiveTrackRow() != nullptr;
   //white rather than 127, which is red on devices that read these as palette colors (launch control 3)
   SetLed(kPrevModuleButton, canSelectModule ? kColorWhite : kColorOff);
   SetLed(kNextModuleButton, canSelectModule ? kColorWhite : kColorOff);
   SetLed(kPrevControlPageButton, int(GetModuleViewOffset()) > 0 ? kColorWhite : kColorOff);
   SetLed(kNextControlPageButton, int(GetModuleViewOffset()) < GetNumControlPages() - 1 ? kColorWhite : kColorOff);

   if (IsRandomizeChordHeld())
   {
      for (int i = GetGridStartIndex(); i < GetGridStartIndex() + GetGridNumPads(); ++i)
         SetLed(i, kColorBrightMagenta, kColorBrightGreen, LedPriority::High);
   }
   else if (mBottomRowMode)
   {
      for (int i = GetGridStartIndex(); i < GetGridStartIndex() + GetGridNumPads(); ++i)
         SetLed(i, kColorDarkGrey, -1, LedPriority::High);
   }
}

void ControlSurface::Poll()
{
   if (mNeedsConnect)
   {
      mNeedsConnect = false;
      if (!mLoadedFromState && mConnectedSurface == nullptr)
      {
         mLoadedFromState = true; //only auto-detect when the module is first created
         AutoDetectDevice();
      }
      ConnectDevice();
   }

   ControlInput input;
   while (mQueuedInputs.consume(input))
   {
      if (input.index < 128)
         OnNoteInput(input.index, input.value);
      else
         OnControlInput(input.index, input.value);
   }

   MidiPitchBend pitchBend;
   while (mQueuedPitchBendMessages.consume(pitchBend))
      OnPitchBendInput(pitchBend);

   MidiPressure pressureMessage;
   while (mQueuedPressureMessages.consume(pressureMessage))
      OnPressureInput(pressureMessage);

   bool controlsChanged = false;
   IDrawableModule* currentControlModule = GetCurrentControlModule();
   if (currentControlModule != nullptr && currentControlModule->HasPush2OverrideControls())
   {
      std::vector<IUIControl*> desiredControls;
      currentControlModule->GetPush2OverrideControls(desiredControls);
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

   if (currentControlModule != nullptr && currentControlModule->HasPush2OverrideControls() != mDisplayModuleIsShowingOverrideControls)
   {
      controlsChanged = true;
      SetModuleViewOffset(0);
      mDisplayModuleIsShowingOverrideControls = currentControlModule->HasPush2OverrideControls();
   }

   if (controlsChanged)
      UpdateControlList();

   UpdateLeds();
   UpdateHoldHints();

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

   ControlSurfaceDevice* surface = mConnectedSurface;
   SendLeds(surface != nullptr && surface->ConsumeLedRefreshRequest());

   for (int i = 0; i < NumMainEncoders(); ++i)
   {
      if (GetButtonState(GetMainEncoderTouchControl(i)))
         mLastTouchedKnobTime[i] = gTime;
   }
}

void ControlSurface::SetDisplayModule(IDrawableModule* module, bool addToHistory)
{
   mDisplayModule = module;
   mDisplayModuleContext = "";
   mModalModuleViewOffset = 0.0;
   UpdateControlList();
}

void ControlSurface::SetDisplayModuleWithContext(IDrawableModule* module, std::string context)
{
   SetDisplayModule(module);
   mDisplayModuleContext = context;
}

void ControlSurface::DisplayScreenMessage(std::string message, float durationMs /*=500*/)
{
   mTemporaryScreenMessage = message;
   mTemporaryScreenMessageTimeout = gTime + durationMs;
   mTemporaryScreenMessageIsHint = false;
}

//a screen message that's refreshed every frame while it applies. real messages take precedence
void ControlSurface::DisplayHint(const std::string& hint)
{
   if (gTime < mTemporaryScreenMessageTimeout && !mTemporaryScreenMessageIsHint)
      return;
   mTemporaryScreenMessage = hint;
   mTemporaryScreenMessageTimeout = gTime + 100;
   mTemporaryScreenMessageIsHint = true;
}

//remembers which held buttons have been used as modifiers since they were pressed, for telling taps from holds and for hold hints
void ControlSurface::TrackModifierUse(int index, int value)
{
   if (value <= 0 || index < 0 || index >= (int)mControlState.size())
      return;

   for (int i = 0; i < (int)mControlState.size(); ++i)
   {
      if (i != index && mControlState[i].mButtonState)
         mControlState[i].mUsedSincePress = true;
   }
   mControlState[index].mUsedSincePress = false;
   mControlState[index].mPressTime = gTime;
}

//held for a moment without using anything else with it
bool ControlSurface::IsHeldOnItsOwn(int index) const
{
   return GetButtonState(index) && !mControlState[index].mUsedSincePress && gTime - mControlState[index].mPressTime > 300;
}

//explains what to do next while a modifier button is held on its own
void ControlSurface::UpdateHoldHints()
{
   std::string hint;
   int storeSnapshotButton = HasDedicatedButton(kStoreSnapshotButton) ? kStoreSnapshotButton : kDotButton;
   if (sBindToUIControl != nullptr && VectorContains(sBindToUIControl, mControls))
      hint = "move a MIDI control";
   else if (IsHeldOnItsOwn(GetBindButton()))
      hint = "touch a knob to bind";
   else if (IsHeldOnItsOwn(kLFOButton))
      hint = "touch a knob: add LFO";
   else if (IsHeldOnItsOwn(kLoopButton))
      hint = "touch knob: record";
   else if (IsRandomizeChordHeld() && (IsHeldOnItsOwn(kRandomizeButton) || IsHeldOnItsOwn(kMoveUndoButton)))
      hint = "touch knob: randomize";
   else if (HasGrid() && !GetButtonState(kShiftButton) && IsHeldOnItsOwn(storeSnapshotButton))
      hint = "tap a pad to store";
   else if (MuteButtonMutesTracks() && IsHeldOnItsOwn(kMoveMuteButton))
      hint = "press scene: mute track";

   if (!hint.empty())
      DisplayHint(hint);
}

//static
bool ControlSurface::IsDisplayableControl(IUIControl* control)
{
   const IDrawableModule* owningModule = control->GetModuleParent();

   if (owningModule == TheTransport && control->Name() == std::string("tempo"))
      return false;

   if (dynamic_cast<const FloatSliderLFOControl*>(owningModule) != nullptr && control->Name() == std::string("pin"))
      return false;

   bool isEnabledCheckbox = owningModule != nullptr && control == owningModule->GetEnabledCheckbox();
   if (!isEnabledCheckbox &&
       (control->IsSliderControl() || control->IsButtonControl()) &&
       (control->GetShouldSaveState() || dynamic_cast<ClickButton*>(control) != nullptr))
      return true;
   return false;
}

void ControlSurface::UpdateControlList()
{
   mControls.clear();
   std::vector<IUIControl*> controls;
   if (IDrawableModule* currentControlModule = GetCurrentControlModule())
   {
      if (currentControlModule->HasPush2OverrideControls())
         currentControlModule->GetPush2OverrideControls(controls);
      else
         controls = currentControlModule->GetUIControls();
   }

   for (int i = 0; i < controls.size(); ++i)
   {
      if (IsDisplayableControl(controls[i]))
      {
         mControls.push_back(controls[i]);
      }
   }
}

void ControlSurface::DetermineTrackControlLayout()
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

      int numPages = ((numControls - 1) / EncoderPageSize()) + 1;
      mTrackControlLayout.push_back(TrackLayoutEntry(nameModule->Name(), numPages));
   }
}

void ControlSurface::ShowSoundSelector()
{
   mShowSoundSelector = true;
   mSoundSelectorIndex = 0;

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

void ControlSurface::ShowAddTrackSelector()
{
   mShowAddTrackSelector = true;
   mAddTrackSelectorIndex = 0;

   mAddTrackSelectorList.clear();
   for (const auto& entry : juce::RangedDirectoryIterator{ juce::File{ ofToDataPath("trackorganizer") }, false, "*.pfb" })
   {
      const auto& file = entry.getFile();
      mAddTrackSelectorList.push_back(file.getFullPathName().toStdString());
   }
}

void ControlSurface::OnScalePageButton(int controlIndex, int velocity)
{
   if (velocity > 0)
   {
      SetActiveTrackRow(kTrackRowScale, false);
   }
   else
   {
      bool wasHold = WasPeekHold(controlIndex);

      //if we held it, then we were just "peeking" into the scale page, so switch back
      if (wasHold)
         SetActiveTrackRow(mPreviousSelectedTrackRow, false);
      else
         OnTrackRowExited(kTrackRowScale, mPreviousSelectedTrackRow);
   }
}

void ControlSurface::ToggleAutoZoomToTrack()
{
   mAutoZoomToTrack = !mAutoZoomToTrack;
   DisplayScreenMessage(mAutoZoomToTrack ? "auto-zoom to track: on" : "auto-zoom to track: off");
}

void ControlSurface::ToggleTrackEnabled(int trackIndex)
{
   if (mSessionOrganizer == nullptr)
      return;
   if (TrackOrganizer* track = mSessionOrganizer->GetTrack(trackIndex))
      track->SetTrackEnabled(!track->IsTrackEnabled());
}

void ControlSurface::AddLFOToControl(int controlIndex)
{
   if (controlIndex < 0 || controlIndex >= (int)mControls.size())
      return;

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

//touch a knob, then press the bind button: the next midi controller that's moved gets bound to the knob's control.
//(or hold the bind button, then touch a knob, see OnNoteInput().) returns false if no knob is being touched
bool ControlSurface::BindTouchedKnob()
{
   if (mMostRecentlyTouchedKnobIndex == -1 || !GetButtonState(GetMainEncoderTouchControl(mMostRecentlyTouchedKnobIndex)))
      return false;

   int controlIndex = mMostRecentlyTouchedKnobIndex + GetControlOffset();
   if (controlIndex >= 0 && controlIndex < (int)mControls.size() && mControls[controlIndex] != nullptr)
      sBindToUIControl = mControls[controlIndex];
   return true;
}

//on devices with a dedicated store button, the dot button only shows the snapshot view, and tapping a pad there recalls
bool ControlSurface::IsStoreSnapshotHeld() const
{
   if (HasDedicatedButton(kStoreSnapshotButton))
      return GetButtonState(kStoreSnapshotButton);
   return GetButtonState(kDotButton);
}

//buttons for functions that the move reaches through combos, on devices that have buttons to spare. returns true if handled
bool ControlSurface::HandleDedicatedButton(int controlIndex, int velocity)
{
   if (controlIndex == kAddTrackSelectorButton)
   {
      if (velocity > 0)
      {
         if (mShowAddTrackSelector)
            mShowAddTrackSelector = false;
         else
            ShowAddTrackSelector();
      }
      return true;
   }

   if (controlIndex == kSoundSelectorButton)
   {
      if (velocity > 0)
      {
         if (mShowSoundSelector)
            mShowSoundSelector = false;
         else
            ShowSoundSelector();
      }
      return true;
   }

   if (controlIndex == kScalePageButton)
   {
      OnScalePageButton(controlIndex, velocity);
      return true;
   }

   if (controlIndex == kAutoZoomButton)
   {
      if (velocity > 0)
         ToggleAutoZoomToTrack();
      return true;
   }

   if (controlIndex == kLFOButton)
   {
      //touch a knob, then press. (or hold, then touch a knob, see OnNoteInput())
      if (velocity > 0 && mMostRecentlyTouchedKnobIndex != -1 && GetButtonState(GetMainEncoderTouchControl(mMostRecentlyTouchedKnobIndex)))
      {
         AddLFOToControl(mMostRecentlyTouchedKnobIndex + GetControlOffset());
         MarkUsedSincePress(kLFOButton);
      }
      return true;
   }

   if (controlIndex == kCancelRecordingButton)
   {
      if (velocity > 0)
      {
         auto* trackRow = GetActiveTrackRow();
         auto* recorder = trackRow != nullptr ? trackRow->GetRecorder() : nullptr;
         if (recorder != nullptr && recorder->IsRecording())
            recorder->CancelRecording();
      }
      return true;
   }

   if (controlIndex == kBindButton)
   {
      if (velocity > 0)
         BindTouchedKnob();
      return true;
   }

   //held as modifiers, see IsRandomizeChordHeld() and IsStoreSnapshotHeld()
   if (controlIndex == kRandomizeButton || controlIndex == kStoreSnapshotButton)
      return true;

   return false;
}

int ControlSurface::GetControlOffset() const
{
   return int(GetModuleViewOffset()) * EncoderPageSize();
}

int ControlSurface::GetNumControlPages() const
{
   return std::max(1, ((int)mControls.size() + EncoderPageSize() - 1) / EncoderPageSize());
}

void ControlSurface::StepControlPage(int direction)
{
   SetModuleViewOffset(std::clamp(int(GetModuleViewOffset()) + direction, 0, GetNumControlPages() - 1));
}

void ControlSurface::StartControlRecorder(int controlIndex)
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
         mCurrentControlRecorder->SetMinimized(true);
      }

      mCurrentControlRecorder->SetEnabled(true);
      mCurrentControlRecorder->SetRecording(true);
   }
}

void ControlSurface::SetLed(int index, int color, int flashColor /*=-1*/, LedPriority priority /*= LedPriority::Normal*/)
{
   SetLedWithBrightness(index, color, flashColor, 1.0f, priority);
}

void ControlSurface::SetLedWithBrightness(int index, int color, int flashColor, float brightness, LedPriority priority /*= LedPriority::Normal*/)
{
   if (index < 0 || index >= (int)mControlState.size())
      return;

   if (!Surface()->SupportsLedBrightness())
      brightness = 1.0f;

   if (priority <= mControlState[index].mLedPriority)
   {
      mControlState[index].mQueuedLedState = ControlSurfaceLedState(color, flashColor, brightness);
      mControlState[index].mLedPriority = priority;
   }
}

void ControlSurface::SetLedFlashColor(int index, int flashColor)
{
   if (index < 0 || index >= (int)mControlState.size())
      return;

   mControlState[index].mQueuedLedState.flashColor = flashColor;
}

void ControlSurface::SendLeds(bool force)
{
   ControlSurfaceDevice* surface = mConnectedSurface;
   if (surface == nullptr)
      return;

   //for devices that can't flash leds themselves, alternate between the colors in eighth notes
   bool softwareFlash = !surface->SupportsLedFlashing();
   bool flashPhase = int(TheTransport->GetMeasurePos(gTime) * TheTransport->GetTimeSigTop() * 2) % 2 == 1;

   for (int index = 0; index < (int)mControlState.size(); ++index)
   {
      if (!surface->HasLed(index))
         continue;

      ControlSurfaceLedState state = mControlState[index].mQueuedLedState;
      if (softwareFlash && state.flashColor != -1)
      {
         if (flashPhase)
            state.color = state.flashColor;
         state.flashColor = -1;
      }

      if (mControlState[index].mLedState != state || force)
      {
         mControlState[index].mLedState = state;
         surface->SendLed(mDevice, index, state);
      }
   }
}

bool ControlSurface::GetButtonState(int index) const
{
   if (index < 0 || index >= (int)mControlState.size())
      return false;
   return mControlState[index].mButtonState;
}

void ControlSurface::SetGridControlInterface(IAbletonGridController* controller)
{
   mGridControlInterface = controller;
}

void ControlSurface::OnMidi(const juce::MidiMessage& message)
{
   if (ControlSurfaceDevice* surface = mConnectedSurface)
      surface->OnMidi(message);
}

void ControlSurface::OnMidiNote(MidiNote& note)
{
   HandleInput_InputThread(note.mPitch, note.mVelocity);
}

void ControlSurface::OnMidiControl(MidiControl& control)
{
   HandleInput_InputThread(control.mControl + 128, control.mValue);
}

void ControlSurface::HandleInput_InputThread(int rawIndex, int value)
{
   ControlSurfaceDevice* surface = mConnectedSurface;
   if (surface == nullptr)
      return;

   //translate from the device's own midi into the move layout
   int index = surface->TranslateInput(rawIndex, value);
   if (index == -1)
      return;
   value = surface->ConvertInputValue(index, value);

   mControlState[index].mButtonState = value > 0;
   TrackModifierUse(index, value); //before grid interfaces get a chance to consume the input here

   if (!ShouldDisplaySnapshotView() && !mBottomRowMode && mGridControlInterface != nullptr)
   {
      bool handled = mGridControlInterface->OnAbletonGridControl_InputThread(this, index, value);
      if (handled)
         return;
   }

   // queue up the message to be handled in Poll()
   mQueuedInputs.produce(ControlInput(index, value));
}

void ControlSurface::OnNoteInput(int pitch, int velocity)
{
   if (pitch == kPrevModuleButton || pitch == kNextModuleButton)
   {
      //step like the jog wheel does
      if (velocity > 0)
         OnControlInput(kClickyEncoderTurn, pitch == kNextModuleButton ? 1 : 127);
      mControlState[pitch].mLastChangeTime = gTime;
      return;
   }

   if (pitch == kPrevControlPageButton || pitch == kNextControlPageButton)
   {
      if (velocity > 0)
         StepControlPage(pitch == kNextControlPageButton ? 1 : -1);
      mControlState[pitch].mLastChangeTime = gTime;
      return;
   }

   if (HandleDedicatedButton(pitch, velocity))
   {
      mControlState[pitch].mLastChangeTime = gTime;
      return;
   }

   int touchedEncoderIndex = GetMainEncoderTouchIndex(pitch);
   if (touchedEncoderIndex >= NumMainEncoders())
      touchedEncoderIndex = -1;

   if (IsRandomizeChordHeld())
   {
      if (touchedEncoderIndex != -1 && velocity > 0)
      {
         int controlIndex = touchedEncoderIndex + GetControlOffset();
         if (controlIndex >= 0 && controlIndex < mControls.size())
         {
            if (auto* control = mControls[controlIndex])
               control->Randomize();
         }
      }

      if (pitch == kClickyEncoderTouch && velocity > 0)
      {
         if (IDrawableModule* currentControlModule = GetCurrentControlModule())
            currentControlModule->RandomizeModule();
      }

      return;
   }

   bool handled = false;
   if (mBottomRowMode)
   {
      if (pitch == kButtonNew)
      {
         if (velocity > 0)
            ToggleAutoZoomToTrack();
         handled = true;
      }
      if (pitch == kButtonGroove)
      {
         if (velocity > 0)
         {
            ShowSoundSelector();
         }
         handled = true;
      }
      if (pitch == kButtonScale)
      {
         OnScalePageButton(pitch, velocity);
         handled = true;
      }
      if (pitch == kButtonStar)
      {
         if (velocity > 0)
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
      if (pitch == kButtonAdd)
      {
         if (velocity > 0)
            ShowAddTrackSelector();
         handled = true;
      }

      if (handled && velocity == 0)
         mBottomRowMode = false;
   }

   if (!handled)
   {
      if (!ShouldDisplaySnapshotView() && mGridControlInterface != nullptr)
      {
         bool handled = mGridControlInterface->OnAbletonGridControl(this, pitch, velocity);
         if (handled)
            return;
      }

      if (touchedEncoderIndex != -1) //main encoders
      {
         int controlIndex = touchedEncoderIndex + GetControlOffset();
         if (controlIndex < mControls.size())
         {
            if (auto* control = mControls[controlIndex])
            {
               if (velocity > 0)
               {
                  control->StartBeacon();

                  if (GetButtonState(kLoopButton))
                  {
                     StartControlRecorder(controlIndex);
                  }
                  else if (GetButtonState(kMoveDeleteButton))
                  {
                     FloatSlider* slider = dynamic_cast<FloatSlider*>(control);
                     if (slider != nullptr && slider->GetLFO() != nullptr)
                     {
                        slider->GetLFO()->SetLFOEnabled(false);
                     }
                     else if (control->GetModulator() != nullptr)
                     {
                        IDrawableModule* modulatorModule = dynamic_cast<IDrawableModule*>(control->GetModulator());
                        if (modulatorModule != nullptr)
                           modulatorModule->SetEnabled(false);
                     }
                  }
                  else if (GetButtonState(kLFOButton))
                  {
                     AddLFOToControl(controlIndex);
                  }
                  else if (GetButtonState(GetBindButton()))
                  {
                     sBindToUIControl = mControls[controlIndex];
                  }

                  /*if (mHeldModule && AllowRepatch())
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
         }

         if (velocity > 0)
         {
            mMostRecentlyTouchedKnobIndex = touchedEncoderIndex;
            mLastTouchedKnobTime[mMostRecentlyTouchedKnobIndex] = gTime;
         }
      }
      else if (pitch >= GetGridStartIndex() && pitch < GetGridStartIndex() + GetGridNumPads() && ShouldDisplaySnapshotView()) //pads
      {
         if (velocity > 0 && mSessionOrganizer != nullptr)
         {
            int gridIndex = pitch - GetGridStartIndex();
            int rowFromTop = GetGridNumRows() - 1 - gridIndex / GetGridNumCols();
            int colFromLeft = gridIndex % GetGridNumCols();
            if (GetButtonState(kMoveMuteButton))
            {
               //one pad per track, toggle it on/off
               ToggleTrackEnabled(colFromLeft + rowFromTop * GetGridNumCols());
            }
            else if (rowFromTop < NumTrackButtons())
            {
               int row = rowFromTop + mTrackRowOffset;
               int col = colFromLeft + mSnapshotOffset;
               TrackOrganizer* trackRow = mSessionOrganizer->GetTrack(row);
               if (trackRow != nullptr)
               {
                  Snapshots* snapshots = trackRow->GetSnapshots();
                  if (snapshots != nullptr)
                  {
                     if (GetButtonState(kShiftButton))
                     {
                        DisplayScreenMessage(snapshots->GetLabel(col));
                     }
                     else if (IsStoreSnapshotHeld())
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
      }
      else if (pitch >= kStepButtonSection && pitch < kStepButtonSection + NumStepButtons())
      {
         if (velocity > 0)
         {
            if (ShouldDisplaySnapshotView())
            {
               SongBuilder* songBuilder = dynamic_cast<SongBuilder*>(mSongBuilderCable->GetTarget());
               if (songBuilder != nullptr)
               {
                  int scene = pitch - kStepButtonSection;
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
                  IInputRecordable* retroactiveRecorder = recorder->IsInRetroactiveRecorderMode() ? recorder : nullptr;
                  Looper* looper = dynamic_cast<Looper*>(recorder);
                  if (retroactiveRecorder == nullptr)
                  {
                     if (looper != nullptr)
                        retroactiveRecorder = looper->GetRecorder();
                  }
                  if (retroactiveRecorder != nullptr && recorder->IsInRetroactiveRecorderMode())
                  {
                     int buttonIndex = pitch - kStepButtonSection;
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

                        if (looper)
                        {
                           if (LooperRecorder* looperRecorder = looper->GetRecorder())
                           {
                              looperRecorder->SetNumBars(numBars);
                              looperRecorder->Commit(looper);
                           }
                        }
                        else
                        {
                           retroactiveRecorder->DoRetroactiveRecord(numBars);
                        }
                     }
                  }
               }
            }
         }
      }
      else
      {
         //ofLog() << "unhandled note " << pitch << " " << velocity;
      }
   }

   mControlState[pitch].mLastChangeTime = gTime;
}

void ControlSurface::OnControlInput(int controlId, int value)
{
   if (controlId == kMoveMuteButton && value > 0)
   {
      //pressing mute while touching a knob isn't a tap either
      for (int i = 0; i < NumMainEncoders(); ++i)
      {
         if (GetButtonState(GetMainEncoderTouchControl(i)))
            MarkUsedSincePress(kMoveMuteButton);
      }
   }

   if (IsRandomizeChordHeld())
   {
      if (mShowSoundSelector)
      {
         if (value > 0)
         {
            TrackOrganizer* trackRow = GetActiveTrackRow();
            IUIControl* soundSelector = trackRow ? trackRow->GetSoundSelector() : nullptr;
            if (soundSelector != nullptr)
            {
               mSoundSelectorIndex = gRandom() % soundSelector->GetNumValues();
               int numValues = soundSelector->GetNumValues();
               soundSelector->SetFromMidiCC(float(mSoundSelectorIndex) / (numValues - 1), NextBufferTime(true), SetValueMethod::Direct);
            }
         }
      }
      return;
   }

   if (GetButtonState(kShiftButton))
   {
      if (GetButtonState(kOctaveUpButton))
      {
         mTrackRowOffset = 0;
         return;
      }
      if (GetButtonState(kOctaveDownButton))
      {
         mTrackRowOffset = GetMaxTrackRowOffset();
         return;
      }
   }

   for (int i = 0; i < NumTrackButtons(); ++i)
   {
      if (GetButtonState(GetTrackButtonControl(i)))
      {
         int trackIndex = i + mTrackRowOffset;
         if (controlId == kVolumeEncoderTurn)
         {
            if (mSessionOrganizer != nullptr)
            {
               TrackOrganizer* track = mSessionOrganizer->GetTrack(trackIndex);
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
                     AdjustControlWithEncoder(adjustSlider, value, K(ignoreShift));
                     mLastGainAdjustTrackIndex = trackIndex;
                     mLastGainAdjustTrackTime = gTime;
                     /*mLCD.Clear();
                     mLCD.DrawRect(5, 4, mLCD.GetWidth() - 6, 4, LCDDrawMode::Outline);
                     mLCD.DrawRect(5, 4, (mLCD.GetWidth() - 6) * ofMap(gainSlider->GetValue(), gainSlider->GetMin(), gainSlider->GetMax(), 0, 1), 4, LCDDrawMode::Fill);
                     mLCD.DrawText(("track " + ofToString(trackIndex) + " gain: " + ofToString(gainSlider->GetValue())).c_str(), 5, 23, LCDFONT_STYLE_REGULAR);
                     mScreenOverrideTimeout = gTime + 500;*/
                  }
               }
            }
            return;
         }
         if (controlId == kMoveDeleteButton)
         {
            if (value > 0)
            {
               ToggleTrackEnabled(trackIndex);
            }
         }
      }
   }

   if (GetButtonState(kHamburgerButton))
   {
      if (controlId == kVolumeEncoderTurn)
      {
         Amplifier* outputGain = dynamic_cast<Amplifier*>(mOutputGainCable->GetTarget());
         FloatSlider* gainSlider = outputGain != nullptr ? outputGain->GetGainSlider() : nullptr;
         if (gainSlider != nullptr)
         {
            AdjustControlWithEncoder(gainSlider, value);
            mLCD.Clear();
            mLCD.DrawRect(3, 4, mLCD.GetWidth() - 6, 4, LCDDrawMode::Outline);
            mLCD.DrawRect(3, 4, (mLCD.GetWidth() - 6) * gainSlider->GetMidiValue(), 4, LCDDrawMode::Fill);
            mLCD.DrawLCDText(("output gain: " + ofToString(gainSlider->GetValue(), 2)).c_str(), 5, 23, LCDFONT_STYLE_REGULAR);
            mScreenOverrideTimeout = gTime + 500;
         }
         return;
      }
   }

   if (GetButtonState(kCaptureButton))
   {
      if (controlId == kVolumeEncoderTurn)
      {
         float increment = value < 64 ? value : value - 128;
         increment *= mShiftHeld ? .0005f : .005f;
         TheSynth->ZoomView(increment, false);
         return;
      }

      if (controlId == kPageLeftButton ||
          controlId == kPageRightButton ||
          controlId == kOctaveUpButton ||
          controlId == kOctaveDownButton)
      {
         //view panning happening in Poll()
         return;
      }
   }

   //if we're touching a knob
   if (mMostRecentlyTouchedKnobIndex != -1 && GetButtonState(GetMainEncoderTouchControl(mMostRecentlyTouchedKnobIndex)))
   {
      if (controlId == kMoveDeleteButton)
      {
         if (value > 0)
         {
            int controlIndex = mMostRecentlyTouchedKnobIndex + GetControlOffset();
            if (controlIndex >= 0 && controlIndex < (int)mControls.size() && mControls[controlIndex] != nullptr)
            {
               FloatSlider* slider = dynamic_cast<FloatSlider*>(mControls[controlIndex]);
               if (slider != nullptr && slider->GetLFO() != nullptr)
               {
                  slider->GetLFO()->SetLFOEnabled(false);
               }
               else if (mControls[controlIndex]->GetModulator() != nullptr)
               {
                  IDrawableModule* modulatorModule = dynamic_cast<IDrawableModule*>(mControls[controlIndex]->GetModulator());
                  if (modulatorModule != nullptr)
                     modulatorModule->SetEnabled(false);
               }
            }
         }
         return;
      }

      if (controlId == kMoveMuteButton && !MuteButtonMutesTracks())
      {
         if (value > 0)
            AddLFOToControl(mMostRecentlyTouchedKnobIndex + GetControlOffset());
         return;
      }
   }

   if (mGridControlInterface != nullptr)
   {
      bool handled = mGridControlInterface->OnAbletonGridControl(this, controlId, value);
      if (handled)
         return;
   }

   int displayKnobIndex = GetDisplayKnobIndex();

   int encoderIndex = GetMainEncoderIndex(controlId);
   if (encoderIndex >= NumMainEncoders())
      encoderIndex = -1;
   int trackButtonIndex = GetTrackButtonIndex(controlId);
   if (trackButtonIndex >= NumTrackButtons())
      trackButtonIndex = -1;

   if (encoderIndex != -1) //main encoders
   {
      int knobIndex = encoderIndex;
      int controlIndex = knobIndex + GetControlOffset();

      bool requireTouch = mRequireTouchForKnobAdjustment && Surface()->HasEncoderTouch();
      if (!requireTouch || GetButtonState(GetMainEncoderTouchControl(knobIndex)) || gTime < mLastTouchedKnobTime[knobIndex] + 300)
      {
         if (ShouldDisplayMixer())
         {
            if (mSessionOrganizer != nullptr)
            {
               TrackOrganizer* track = mSessionOrganizer->GetTrack(controlIndex);
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
                     AdjustControlWithEncoder(adjustSlider, value, K(ignoreShift));
               }
            }
         }
         else
         {
            bool justResetParameter = gTime - mLastResetTime < 1000;
            if (controlIndex < mControls.size() && !justResetParameter)
            {
               AdjustControlWithEncoder(mControls[controlIndex], value);
               mLastAdjustedKnobIndex = knobIndex;
               mLastAdjustedKnobTime = gTime;
               mMostRecentlyTouchedKnobIndex = mLastAdjustedKnobIndex;
            }
         }
      }
   }
   else if (trackButtonIndex != -1 && MuteButtonMutesTracks() && (value > 0 ? GetButtonState(kMoveMuteButton) : mTrackButtonPressedForMute[trackButtonIndex]))
   {
      //hold mute and press a track button to turn that track on or off
      mTrackButtonPressedForMute[trackButtonIndex] = value > 0;
      if (value > 0)
         ToggleTrackEnabled(trackButtonIndex + mTrackRowOffset);
   }
   else if (trackButtonIndex != -1 && (UseTrackButtonsForSnapshots() || mTrackButtonPressedForSnapshot[trackButtonIndex]))
   {
      //track buttons are recalling the active track's snapshots
      if (value > 0)
      {
         mTrackButtonPressedForSnapshot[trackButtonIndex] = true;
         TrackOrganizer* trackRow = GetActiveTrackRow();
         Snapshots* snapshots = trackRow != nullptr ? trackRow->GetSnapshots() : nullptr;
         int snapshotIdx = trackButtonIndex + mSnapshotOffset;
         if (snapshots != nullptr && snapshots->HasSnapshot(snapshotIdx))
         {
            snapshots->SetSnapshot(snapshotIdx, gTime);
            DisplayScreenMessage(snapshots->GetLabel(snapshotIdx));
         }
      }
      else
      {
         mTrackButtonPressedForSnapshot[trackButtonIndex] = false;
      }
   }
   else if (trackButtonIndex != -1)
   {
      int index = trackButtonIndex + mTrackRowOffset;
      if (value > 0)
      {
         SetActiveTrackRow(index, false);
      }
      else
      {
         bool wasHold = WasPeekHold(controlId);

         //if we held it, then we were just "peeking" into the other track, so switch back
         if (wasHold)
            SetActiveTrackRow(mPreviousSelectedTrackRow, false);
         else
            OnTrackRowExited(index, mPreviousSelectedTrackRow);

         //if we quickly pressed a button for a track we were already holding, reset module index within row
         if (!wasHold && index == mPreviousSelectedTrackRow)
            SetActiveTrackRow(index, true);

         if (!wasHold && mAutoZoomToTrack)
            ZoomToTrack(GetActiveTrackRow());
      }
   }
   else if (controlId == kBackButton)
   {
      if (value > 0)
      {
         SetActiveTrackRow(kTrackRowGlobal, false);
      }
      else
      {
         bool wasHold = WasPeekHold(controlId);

         //if we held it, then we were just "peeking" into the global page, so switch back
         if (wasHold)
            SetActiveTrackRow(mPreviousSelectedTrackRow, false);
         else
            OnTrackRowExited(kTrackRowGlobal, mPreviousSelectedTrackRow);

         //if we quickly pressed a button for a track we were already holding, reset global module index within row
         if (!wasHold && mPreviousSelectedTrackRow == kTrackRowGlobal)
            SetActiveTrackRow(kTrackRowGlobal, true);
      }
   }
   else if (controlId == kHamburgerButton)
   {
      if (value > 0)
      {
         SetActiveTrackRow(kTrackRowMixer, false);
      }
      else
      {
         bool wasHold = WasPeekHold(controlId);

         //if we held it, then we were just "peeking" into the global page, so switch back
         if (wasHold)
            SetActiveTrackRow(mPreviousSelectedTrackRow, false);
         else
            OnTrackRowExited(kTrackRowMixer, mPreviousSelectedTrackRow);

         if (!wasHold && mAutoZoomToTrack)
            ZoomToModule(mSessionOrganizer);
      }
   }
   else if (controlId == kPlayButton)
   {
      if (value > 0)
      {
         SetActiveTrackRow(kTrackRowTransport, false);
      }
      else
      {
         bool wasHold = WasPeekHold(controlId);

         //if we held it, then we were just "peeking" into the global page, so switch back
         if (wasHold)
            SetActiveTrackRow(mPreviousSelectedTrackRow, false);
         else
            OnTrackRowExited(kTrackRowTransport, mPreviousSelectedTrackRow);
      }
   }
   else if (controlId == kClickyEncoderButton)
   {
      if (mShowAddTrackSelector)
      {
         if (value > 0)
         {
            if (mSessionOrganizer)
            {
               std::string path = "";

               if (mAddTrackSelectorIndex >= 0 && mAddTrackSelectorIndex < mAddTrackSelectorList.size())
                  path = mAddTrackSelectorList[mAddTrackSelectorIndex];

               if (path != "")
                  mSessionOrganizer->LoadTrackPrefab(path);
            }
            mShowAddTrackSelector = false;
         }
      }
      else if (mShowSoundSelector)
      {
         if (value > 0)
         {
            TrackOrganizer* trackRow = GetActiveTrackRow();
            IUIControl* soundSelector = trackRow ? trackRow->GetSoundSelector() : nullptr;
            if (soundSelector != nullptr)
            {
               int numValues = soundSelector->GetNumValues();
               soundSelector->SetFromMidiCC(float(mSoundSelectorIndex) / (numValues - 1), NextBufferTime(true), SetValueMethod::Direct);
            }
         }
      }
      else
      {
         if (value > 0)
         {
            mLastClickyEncoderPressTime = gTime;
         }
         else
         {
            bool wasTap = gTime - mLastClickyEncoderPressTime < 300;
            if (wasTap)
            {
               if (mTrackRowOffset == 0)
                  mTrackRowOffset = std::min(NumTrackButtons(), GetMaxTrackRowOffset());
               else
                  mTrackRowOffset = 0;
            }
         }
      }
   }
   else if (controlId == kClickyEncoderTurn)
   {
      int direction = value < 64 ? 1 : -1;

      TrackOrganizer* trackRow = GetActiveTrackRow();
      if (mShowAddTrackSelector)
      {
         mAddTrackSelectorIndex = std::clamp(mAddTrackSelectorIndex + direction, 0, int(mAddTrackSelectorList.size()) - 1);
      }
      else if (mShowSoundSelector)
      {
         IUIControl* soundSelector = trackRow ? trackRow->GetSoundSelector() : nullptr;
         if (soundSelector != nullptr)
         {
            mSoundSelectorIndex = std::clamp(mSoundSelectorIndex + direction, 0, soundSelector->GetNumValues() - 1);
         }
      }
      else if (GetButtonState(kClickyEncoderButton))
      {
         mTrackRowOffset = std::clamp(mTrackRowOffset + direction, 0, GetMaxTrackRowOffset());
      }
      else if (ShouldDisplaySnapshotView())
      {
         const int numCols = GetNumSnapshotColumns();
         int maxNumSnapshots = numCols;
         if (mSessionOrganizer != nullptr)
         {
            for (int i = 0; i < mSessionOrganizer->GetNumTracks(); ++i)
            {
               TrackOrganizer* trackRow = mSessionOrganizer->GetTrack(i);
               if (trackRow != nullptr)
               {
                  Snapshots* snapshots = trackRow->GetSnapshots();
                  if (snapshots != nullptr)
                  {
                     maxNumSnapshots = std::max(maxNumSnapshots, snapshots->GetSize());
                  }
               }
            }
         }
         mSnapshotOffset = std::clamp(mSnapshotOffset + direction, 0, std::max(0, maxNumSnapshots - numCols));
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
   else if (controlId == kVolumeEncoderTurn)
   {
      if (Surface()->HasControlPageButtons())
      {
         //devices with buttons for paging through controls use this knob for the current track's volume (shift: send) instead
         TrackOrganizer* track = GetActiveTrackRow();
         FloatSlider* slider = track != nullptr ? GetMixerSlider(mSelectedTrackRow) : nullptr;
         if (slider != nullptr)
         {
            AdjustControlWithEncoder(slider, value, K(ignoreShift));
            mVolumeKnobTrackIndex = mSelectedTrackRow;
            mVolumeKnobTrackTime = gTime;
            DisplayScreenMessage(track->GetTrackName() + (mShiftHeld ? " send " : " gain ") + ofToString(slider->GetValue(), 2));
         }
      }
      else
      {
         float increment = value < 64 ? value : value - 128;
         increment *= 0.03f;
         SetModuleViewOffset(ofClamp(GetModuleViewOffset() + increment, 0, MAX(0, ((int)mControls.size() + EncoderPageSize() - 1) / EncoderPageSize()) - .01f));
      }
   }
   /*else if (controlId == kSetupButton && value > 0)
   {
      mInMidiControllerBindMode = !mInMidiControllerBindMode;
   }
   else if (controlId == kNewButton)
   {
      mNewButtonHeld = value > 0;
   }
   else if (controlId == kDeleteButton)
   {
      mDeleteButtonHeld = value > 0;
   }
   else if (controlId == kFixedLengthButton)
   {
      mLFOButtonHeld = value > 0;
   }*/
   else if (controlId == kLoopButton)
   {
      if (value > 0)
      {
         if (mMostRecentlyTouchedKnobIndex != -1 && GetButtonState(GetMainEncoderTouchControl(mMostRecentlyTouchedKnobIndex)))
         {
            StartControlRecorder(mMostRecentlyTouchedKnobIndex);
            MarkUsedSincePress(kLoopButton);
         }
      }
      else if (value == 0)
      {
         if (mCurrentControlRecorder != nullptr)
         {
            mCurrentControlRecorder->SetRecording(false);
            mCurrentControlRecorder = nullptr;
         }
      }
   }
   /*else if (controlId == kMasterButton)
   {
      mAddModuleBookmarkButtonHeld = value > 0;
   }
   else if (controlId == kTapTempoButton)
   {
      if (gHoveredModule != nullptr && gHoveredModule != mDisplayModule)
         SetDisplayModule(gHoveredModule, true);
   }
   else if (controlId == kDeviceButton || controlId == kMixButton || controlId == kBrowseButton || controlId == kClipButton)
   {
      if (value > 0)
      {
         int index = 0;
         if (controlId == kDeviceButton)
            index = 0;
         if (controlId == kMixButton)
            index = 1;
         if (controlId == kBrowseButton)
            index = 2;
         if (controlId == kClipButton)
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
   else if (controlId == kUpButton || controlId == kDownButton || controlId == kLeftButton || controlId == kRightButton)
   {
      if (value > 0)
      {
         ofVec2f direction;
         if (controlId == kUpButton)
            direction.y -= 1;
         if (controlId == kDownButton)
            direction.y += 1;
         if (controlId == kLeftButton)
            direction.x -= 1;
         if (controlId == kRightButton)
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
   else if (controlId == kPageLeftButton ||
            controlId == kPageRightButton ||
            controlId == kOctaveUpButton ||
            controlId == kOctaveDownButton ||
            controlId == kMoveUndoButton)
   {
      if (displayKnobIndex != -1)
      {
         if (value > 0)
         {
            int controlIndex = displayKnobIndex + GetControlOffset();
            if (controlIndex >= 0 && controlIndex < mControls.size() && mControls[controlIndex] != nullptr)
            {
               if (controlId == kPageRightButton)
                  mControls[controlIndex]->Increment(1);
               if (controlId == kPageLeftButton)
                  mControls[controlIndex]->Increment(-1);
               if (controlId == kOctaveUpButton)
                  mControls[controlIndex]->Double();
               if (controlId == kOctaveDownButton)
                  mControls[controlIndex]->Halve();
               if (controlId == kMoveUndoButton)
               {
                  mControls[controlIndex]->ResetToDefault();
                  mLastResetTime = gTime;
               }
            }

            MidiController::sLastActivityUIControl = mControls[controlIndex];
            MidiController::sLastConnectedActivityTime = gTime;
         }
      }
      else
      {
         if (value > 0)
         {
            if (controlId == kOctaveUpButton)
               mTrackRowOffset = std::clamp(mTrackRowOffset - 1, 0, GetMaxTrackRowOffset());
            if (controlId == kOctaveDownButton)
               mTrackRowOffset = std::clamp(mTrackRowOffset + 1, 0, GetMaxTrackRowOffset());
         }
      }
   }
   else if (controlId == kDotButton)
   {
      if (value > 0 && mShiftHeld)
         ShowSoundSelector();
   }
   /*else if (controlId == kScaleButton)
   {
      if (value > 0 && mGridControlModule != nullptr)
      {
         SetDisplayModule(mGridControlModule, true);
      }
   }
   else if (controlId >= kQuantizeButtonSection && controlId < kQuantizeButtonSection + kNumQuantizeButtons) //quantization level buttons to the right of the main grid, used to bookmark modules
   {
      if (value > 0)
      {
         int i = controlId - kQuantizeButtonSection;
         if (mAddModuleBookmarkButtonHeld)
            BookmarkModuleToSlot(i, mDisplayModule);
         if (mDeleteButtonHeld)
            BookmarkModuleToSlot(i, nullptr);
         else
            SwitchToBookmarkedModule(i);
      }
   }*/
   else if (controlId == kShiftButton)
   {
      mShiftHeld = value > 0;

      if (value > 0)
      {
         if (gTime - mControlState[controlId].mLastChangeTime < 200 && NumStepButtons() > 0) //double-click
            mBottomRowMode = true;
         else
            mBottomRowMode = false;
      }
   }
   /*else if (controlId == kAddTrackButton)
   {
      mAddTrackHeld = value > 0;
   }
   else if (controlId == kSelectButton)
   {
      if (value > 0 && mDisplayModule != nullptr)
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
               mControls[controlIndex]->ResetToDefault();
               mLastResetTime = gTime;
            }
         }
      }
   }*/
   else if (controlId == kMoveDeleteButton)
   {
      if (mSelectedTrackRow == kTrackRowTransport)
      {
         if (value > 0)
         {
            if (TheSynth->IsAudioPaused())
               TheTransport->Reset();
            else
               TheSynth->SetAudioPaused(true);
         }
      }
   }
   else if (controlId == kMoveMuteButton)
   {
      //tap mute to turn the selected track on or off
      if (value == 0 && MuteButtonMutesTracks() && !mControlState[kMoveMuteButton].mUsedSincePress && !WasPeekHold(controlId))
         ToggleTrackEnabled(mSelectedTrackRow);
   }
   else if (controlId == kCircleButton)
   {
      if (value > 0)
      {
         auto* trackRow = GetActiveTrackRow();
         auto* recorder = trackRow != nullptr ? trackRow->GetRecorder() : nullptr;
         if (recorder != nullptr)
         {
            if (GetButtonState(kMoveDeleteButton))
            {
               recorder->ClearRecording();
            }
            else if (recorder->IsRecording())
            {
               if (GetButtonState(kShiftButton))
                  recorder->CancelRecording();
               else
                  recorder->SetRecording(false);
            }
            else
            {
               recorder->SetRecording(true);
            }
         }
      }
   }
   else if (controlId == kCaptureButton)
   {
      if (value > 0 && GetBindButton() == kCaptureButton && BindTouchedKnob())
      {
         //binding instead of zooming
      }
      else if (value > 0)
      {
         if (GetButtonState(kShiftButton))
         {
            IDrawableModule* zoomToModule = GetCurrentControlModule();
            if (zoomToModule == TheTransport)
            {
               std::vector<IDrawableModule*> allModules;
               TheSynth->GetAllModules(allModules);
               for (int i = 0; i < (int)allModules.size(); ++i)
               {
                  if (dynamic_cast<AudioSyncer*>(allModules[i]))
                  {
                     zoomToModule = allModules[i];
                     break;
                  }
               }
            }

            if (zoomToModule != nullptr)
               ZoomToModule(zoomToModule);
         }
         if (gTime - mControlState[controlId].mLastChangeTime < 200) //double-click
         {
            auto* track = GetActiveTrackRow();
            if (track != nullptr)
               ZoomToTrack(track);
            else if (mSessionOrganizer)
               ZoomToModule(mSessionOrganizer);
         }
      }
   }
   else
   {
      ofLog() << "unhandled control " << (controlId - 128) << " " << value;
   }

   mControlState[controlId].mLastChangeTime = gTime;
}

bool ControlSurface::IsRandomizeChordHeld() const
{
   return (GetButtonState(kShiftButton) && GetButtonState(kMoveUndoButton)) || GetButtonState(kRandomizeButton);
}

IDrawableModule* ControlSurface::GetCurrentControlModule() const
{
   if (mDisplayModule == nullptr || (mGridControlInterface && mGridControlInterface->IsInAbletonGridFocusMode()))
      return dynamic_cast<IDrawableModule*>(mGridControlInterface);
   return mDisplayModule;
}

void ControlSurface::OnMidiPitchBend(MidiPitchBend& pitchBend)
{
   if (mConnectedSurface == nullptr)
      return;

   if (mGridControlInterface != nullptr)
   {
      bool handled = mGridControlInterface->OnAbletonGridControl_InputThread(this, kPitchBendIndex, pitchBend.mValue);
      if (handled)
         return;
   }

   mQueuedPitchBendMessages.produce(pitchBend);
}

void ControlSurface::OnPitchBendInput(MidiPitchBend& pitchBend)
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

void ControlSurface::OnMidiPressure(MidiPressure& pressure)
{
   if (mConnectedSurface == nullptr)
      return;

   if (mGridControlInterface != nullptr)
   {
      bool handled = mGridControlInterface->OnAbletonGridControl_InputThread(this, kChannelPressureIndex + pressure.mChannel, pressure.mPressure);
      if (handled)
         return;
   }

   mQueuedPressureMessages.produce(pressure);
}

void ControlSurface::OnPressureInput(MidiPressure& pressure)
{
   if (mGridControlInterface != nullptr)
   {
      bool handled = mGridControlInterface->OnAbletonGridControl(this, kChannelPressureIndex + pressure.mChannel, pressure.mPressure);
      if (handled)
         return;
   }
}

void ControlSurface::ZoomToTrack(TrackOrganizer* track)
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

void ControlSurface::ZoomToModule(IDrawableModule* module)
{
   if (module)
   {
      ofRectangle rect = module->GetRect();
      float ratioX = ofGetWidth() / rect.width;
      float ratioY = ofGetHeight() / rect.height;
      float newScale = std::min(ratioX, ratioY) * .9f;
      ofVec2f offset = (rect.getCenter() * -1) + ofVec2f(ofGetWidth(), ofGetHeight()) / newScale * .5f;
      TheSynth->GetLocationZoomer()->MoveToLocation(newScale, offset);
   }
}

bool ControlSurface::AdjustGlobalModuleIndex(int amount)
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

void ControlSurface::AdjustControlWithEncoder(IUIControl* control, float midiInputValue, bool ignoreShift)
{
   if (control == nullptr)
      return;

   float currentNormalized = control->GetMidiValue();
   float increment = GetEncoderIncrement(midiInputValue) * Surface()->GetEncoderSensitivity();
   if (mShiftHeld && !ignoreShift)
      increment *= 0.1f;

   FloatSlider* floatSlider = dynamic_cast<FloatSlider*>(control);
   ClickButton* button = dynamic_cast<ClickButton*>(control);
   if (floatSlider && floatSlider->GetModulator() && floatSlider->GetModulator()->Active() && floatSlider->GetModulator()->CanAdjustRange())
   {
      IModulator* modulator = floatSlider->GetModulator();
      float min = floatSlider->GetMin();
      float max = floatSlider->GetMax();
      if (mShiftHeld)
      {
         increment = GetEncoderIncrement(midiInputValue) * Surface()->GetEncoderSensitivity();
         float modMin = ofMap(modulator->GetMin(), min, max, 0, 1);
         modulator->GetMin() = ofMap(modMin + increment, 0, 1, min, max, K(clamp));
      }
      else
      {
         float modMax = ofMap(modulator->GetMax(), min, max, 0, 1);
         modulator->GetMax() = ofMap(modMax + increment, 0, 1, min, max, K(clamp));
      }
   }
   else if (button)
   {
      static double sLastButtonClickTime = 0.0;
      if (increment > 0 && gTime - sLastButtonClickTime > 400)
      {
         sLastButtonClickTime = gTime;
         button->SetFromMidiCC(1.0f, NextBufferTime(false), SetValueMethod::Direct);
      }
   }
   else
   {
      float newValue = std::clamp(currentNormalized + increment, 0.0f, 1.0f);
      control->SetFromMidiCC(newValue, NextBufferTime(false), SetValueMethod::Increment);
   }

   MidiController::sLastActivityUIControl = control;
   MidiController::sLastConnectedActivityTime = gTime;
}

IDrawableModule* ControlSurface::GetCurrentGlobalModule() const
{
   if (mGlobalModuleIndex >= 0 && mGlobalModuleIndex < (int)mGlobalControlModuleCables.size())
      return dynamic_cast<IDrawableModule*>(mGlobalControlModuleCables[mGlobalModuleIndex]->GetTarget());
   return nullptr;
}

IAbletonGridController* ControlSurface::GetCurrentGlobalGridInterface() const
{
   IAbletonGridController* ret = nullptr;
   if (mGlobalModuleIndex >= 0 && mGlobalModuleIndex < (int)mGlobalGridInterfaceCables.size())
      ret = dynamic_cast<IAbletonGridController*>(mGlobalGridInterfaceCables[mGlobalModuleIndex]->GetTarget());
   if (ret == nullptr)
      ret = dynamic_cast<IAbletonGridController*>(mGlobalGridInterfaceCables[0]->GetTarget());
   return ret;
}

TrackOrganizer* ControlSurface::GetActiveTrackRow() const
{
   if (mSessionOrganizer == nullptr)
      return nullptr;
   return mSessionOrganizer->GetTrack(mSelectedTrackRow);
}

void ControlSurface::SetActiveTrackRow(int row, bool resetModuleIndex)
{
   mPreviousSelectedTrackRow = mSelectedTrackRow;

   if (resetModuleIndex)
      mDisplayModuleSelectTimeout = gTime + 500.0;

   mSelectedTrackRow = row;
   mShowSoundSelector = false;
   mShowAddTrackSelector = false;

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
      SetGridControlInterface(TheScale);
   }
   else if (mSessionOrganizer != nullptr && row >= 0 && row < mSessionOrganizer->GetNumTracks())
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

            for (int i = GetGridStartIndex(); i < GetGridStartIndex() + GetGridNumPads(); ++i)
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

void ControlSurface::OnTrackRowExited(int newRow, int oldRow)
{
   if (mSessionOrganizer != nullptr)
   {
      TrackOrganizer* oldTrackRow = mSessionOrganizer->GetTrack(oldRow);
      auto* recorder = oldTrackRow != nullptr ? oldTrackRow->GetRecorder() : nullptr;
      if (recorder != nullptr)
         recorder->SetRecording(false);
   }
}

int ControlSurface::GetGridControllerOption1Control() const
{
   return Surface()->GetGridControllerOption1Control();
}

int ControlSurface::GetGridControllerOption2Control() const
{
   return Surface()->GetGridControllerOption2Control();
}
