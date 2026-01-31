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
//  TrackOrganizer.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 5/14/25.
//
//

#include "TrackOrganizer.h"
#include "SynthGlobals.h"
#include "ModularSynth.h"
#include "PatchCableSource.h"
#include "Snapshots.h"
#include "LaunchpadKeyboard.h"
#include "IInputRecordable.h"
#include "Amplifier.h"
#include "AudioSend.h"
#include "UIControlMacros.h"

#include <cstring>

TrackOrganizer::TrackOrganizer()
: IDrawableModule(200, 80)
{
   mColorIndex = rand() % 27 + 1;
}

TrackOrganizer::~TrackOrganizer()
{
}

void TrackOrganizer::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   UIBLOCK0();
   TEXTENTRY(mNameEntry, "track name", 20, &mTrackName);
   DROPDOWN(mColorSelector, "color", &mColorIndex, 100);
   BUTTON(mSelectModulesButton, "select modules");
   ENDUIBLOCK0();

   for (int i = 0; i < (int)AbletonDevice::kColors.size(); ++i)
      mColorSelector->AddLabel(AbletonDevice::kColors[i].name, i);

   ofColor cableColor = IDrawableModule::GetColor(kModuleCategory_Other);
   cableColor.a *= .3f;

   int cableX = 12;
   int cableY = 60;

   mSnapshotsCable = new PatchCableSource(this, kConnectionType_Special);
   mSnapshotsCable->AddTypeFilter("snapshots");
   mSnapshotsCable->SetColor(cableColor);
   mSnapshotsCable->SetManualPosition(cableX, cableY);
   AddPatchCableSource(mSnapshotsCable);
   cableX += 12;

   // hack to handle serialization with old layout
   PatchCableSource* dummy = new PatchCableSource(this, kConnectionType_Special);
   AddPatchCableSource(dummy);
   dummy->SetShowing(false);

   cableX += 10;

   ofVec2f controlModuleCablePos(cableX, cableY);
   for (size_t i = 0; i < mControlModuleCables.size(); ++i)
   {
      mControlModuleCables[i] = new PatchCableSource(this, kConnectionType_Special);
      mControlModuleCables[i]->SetColor(cableColor);
      mControlModuleCables[i]->SetManualPosition(cableX, cableY);
      AddPatchCableSource(mControlModuleCables[i]);
      cableX += 12;
   }

   cableX += 10;

   mRecorderCable = new PatchCableSource(this, kConnectionType_Special);
   mRecorderCable->SetPredicateFilter(IsOfType<IInputRecordable*>);
   mRecorderCable->SetColor(cableColor);
   mRecorderCable->SetManualPosition(cableX, cableY);
   AddPatchCableSource(mRecorderCable);
   cableX += 12;

   mGainCable = new PatchCableSource(this, kConnectionType_Special);
   mGainCable->AddTypeFilter("gain");
   mGainCable->SetColor(cableColor);
   mGainCable->SetManualPosition(cableX, cableY);
   AddPatchCableSource(mGainCable);
   cableX += 12;

   mSendCable = new PatchCableSource(this, kConnectionType_Special);
   mSendCable->AddTypeFilter("send");
   mSendCable->SetColor(cableColor);
   mSendCable->SetManualPosition(cableX, cableY);
   cableX += 12;

   cableX += 10;

   mOtherTrackModulesCable = new PatchCableSource(this, kConnectionType_Special);
   mOtherTrackModulesCable->SetColor(cableColor);
   mOtherTrackModulesCable->SetManualPosition(cableX, cableY);
   mOtherTrackModulesCable->SetAllowMultipleTargets(true);
   AddPatchCableSource(mOtherTrackModulesCable);

   for (size_t i = 0; i < mGridInterfaceCables.size(); ++i)
   {
      mGridInterfaceCables[i] = new PatchCableSource(this, kConnectionType_Special);
      mGridInterfaceCables[i]->SetPredicateFilter(IsOfType<IAbletonGridController*>);
      mGridInterfaceCables[i]->SetColor(cableColor);
      mGridInterfaceCables[i]->SetManualPosition(controlModuleCablePos.x + i * 12, controlModuleCablePos.y + 12);
      AddPatchCableSource(mGridInterfaceCables[i]);
   }

   ofVec2f snapshotCablePosition = mSnapshotsCable->GetManualPosition();
   cableX = snapshotCablePosition.x;
   cableY = snapshotCablePosition.y;
   cableY += 12;

   mSoundSelectorCable = new PatchCableSource(this, kConnectionType_UIControl);
   mSoundSelectorCable->SetManualPosition(cableX, cableY);
   AddPatchCableSource(mSoundSelectorCable);

   AddPatchCableSource(mSendCable);
}

void TrackOrganizer::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   ofPushStyle();
   ofFill();
   ofSetColor(AbletonDevice::kColors[mColorIndex].color, gModuleDrawAlpha * .3f);
   ofRect(0, -IDrawableModule::TitleBarHeight(), mWidth, mHeight + IDrawableModule::TitleBarHeight());
   ofPopStyle();

   mNameEntry->Draw();
   mColorSelector->Draw();
   mSelectModulesButton->Draw();

   Amplifier* gain = GetGain();
   if (gain)
      gain->DrawLevelMeter(110, 20, 80, 15);

   if (mModuleIndex >= 0 && mModuleIndex < mControlModuleCables.size())
   {
      ofPushStyle();
      ofNoFill();
      ofSetColor(ofColor(255, 255, 255, 50));
      const auto* cableSource = mControlModuleCables[mModuleIndex];
      ofVec2f cablePos = cableSource->GetManualPosition();
      ofRect(cablePos.x - 6, cablePos.y - 6, 12, 24);
      ofPopStyle();
   }
}

std::list<IDrawableModule*> TrackOrganizer::GetAllTrackModules()
{
   std::list<IDrawableModule*> trackModules;
   trackModules.push_back(this);
   for (const auto* source : GetPatchCableSources())
   {
      for (const auto* cable : source->GetPatchCables())
      {
         IDrawableModule* targetedModule = dynamic_cast<IDrawableModule*>(cable->GetTarget());
         if (targetedModule != nullptr && !ListContains(targetedModule, trackModules))
            trackModules.push_back(targetedModule);
      }
   }
   return trackModules;
}

ofRectangle TrackOrganizer::GetBoundingRect()
{
   ofRectangle allModulesRect = GetRect();
   std::list<IDrawableModule*> trackModules = GetAllTrackModules();
   for (auto* module : trackModules)
   {
      if (module != nullptr && !module->IsDeleted() &&
          module->GetOwningContainer() != nullptr && module->IsShowing())
      {
         ofRectangle rect = module->GetRect();

         if (module->HasTitleBar())
         {
            rect.y -= IDrawableModule::TitleBarHeight();
            rect.height += IDrawableModule::TitleBarHeight();
         }

         allModulesRect = ofRectangle::include(allModulesRect, rect);
      }
   }

   allModulesRect.grow(15);
   if (mDrawTrackName)
   {
      allModulesRect.y -= 23;
      allModulesRect.height += 23;
   }

   return allModulesRect;
}

void TrackOrganizer::PreDrawModuleUnclipped()
{
   if (mDrawTrackBounds || mDrawTrackName)
   {
      ofRectangle allModulesRect = GetBoundingRect();
      allModulesRect.x -= mX;
      allModulesRect.y -= mY;

      ofColor color = AbletonDevice::kColors[mColorIndex].color;
      color.a = 20;

      ofPushStyle();

      if (mDrawTrackName)
      {
         ofSetColor(255, 255, 255);
         DrawTextBold(mTrackName, allModulesRect.x + 10, allModulesRect.y + 23, 21);
      }

      if (mDrawTrackBounds)
      {
         ofSetColor(color);
         ofSetLineWidth(3);
         ofFill();
         ofRect(allModulesRect, 20);
      }

      ofPopStyle();
   }
}

void TrackOrganizer::DrawModuleUnclipped()
{
   //colorize connected modules to match track color
   ofPushStyle();
   ofColor color = AbletonDevice::kColors[mColorIndex].color;
   //if we don't have bounds to connect the modules, at make them blink when you click this to make them easier to see
   if (!mDrawTrackBounds && ShouldShowCables())
      color.a = ofMap(sin(gTime / 500 * PI * 2), -1, 1, 50, 255);
   else
      color.a = 100;
   ofSetColor(color);
   ofSetLineWidth(3);
   ofNoFill();
   std::list<IDrawableModule*> trackModules = GetAllTrackModules();
   for (auto* module : trackModules)
   {
      if (module != nullptr && !module->IsDeleted() &&
          module->GetOwningContainer() != nullptr && module->IsShowing())
      {
         ofPushMatrix();

         ofVec2f pos = GetPosition();
         ofTranslate(-pos.x, -pos.y);
         ofRectangle rect = module->GetRect();

         if (module->HasTitleBar())
         {
            rect.y -= IDrawableModule::TitleBarHeight();
            rect.height += IDrawableModule::TitleBarHeight();
         }

         ofRect(rect.x - 3, rect.y - 3, rect.width + 6, rect.height + 6, 6);

         ofPopMatrix();
      }
   }

   ofPopStyle();

   std::string tooltip = "";
   PatchCableSource* hoverCable = nullptr;

   if (mSnapshotsCable->IsHovered())
   {
      hoverCable = mSnapshotsCable;
      tooltip = "snapshots";
   }

   if (mSoundSelectorCable->IsHovered())
   {
      hoverCable = mSoundSelectorCable;
      tooltip = "sound selector";
   }

   for (int i = 0; i < (size_t)mControlModuleCables.size(); ++i)
   {
      if (mControlModuleCables[i]->IsHovered())
      {
         hoverCable = mControlModuleCables[i];
         tooltip = "control module " + ofToString(i + 1);
      }
      if (mGridInterfaceCables[i]->IsHovered())
      {
         hoverCable = mGridInterfaceCables[i];
         tooltip = "grid interface " + ofToString(i + 1);
      }
   }

   if (mRecorderCable->IsHovered())
   {
      hoverCable = mRecorderCable;
      tooltip = "recorder";
   }

   if (mGainCable->IsHovered())
   {
      hoverCable = mGainCable;
      tooltip = "gain";
   }

   if (mSendCable->IsHovered())
   {
      hoverCable = mSendCable;
      tooltip = "send";
   }

   if (mOtherTrackModulesCable->IsHovered())
   {
      hoverCable = mOtherTrackModulesCable;
      tooltip = "other modules in track";
   }

   if (hoverCable != nullptr)
   {
      if (hoverCable != mOtherTrackModulesCable)
      {
         IClickable* target = hoverCable->GetTarget();
         if (target != nullptr)
            tooltip += std::string(": ") + target->Name();
      }

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

   if (gHoveredModule == this && !TheSynth->GetGroupSelectedModules().empty())
   {
      ofPushStyle();
      ofSetColor(50, 50, 50);
      ofFill();
      ofRect(3, 3, 150, 24);
      ofSetColor(255, 255, 255);
      DrawTextNormal("press enter to add\ncurrently selected modules", 5, 12);
      ofPopStyle();
   }
}

void TrackOrganizer::Poll()
{
   IDrawableModule::Poll();

   if (mSelectModulesOnMouseRelease && !TheSynth->IsMouseButtonHeld(1))
   {
      TheSynth->SetGroupSelectedModules(GetAllTrackModules());
      mSelectModulesOnMouseRelease = false;
   }

   bool showCables = ShouldShowCables();

   for (auto* source : GetPatchCableSources())
   {
      source->SetClickable(showCables);
      source->SetPatchCableDrawMode(showCables ? kPatchCableDrawMode_Normal : kPatchCableDrawMode_CablesOnHoverOnly);
   }
}

bool TrackOrganizer::ShouldShowCables() const
{
   return (gHoveredModule == this || TheSynth->GetLastClickedModule() == this) && !Minimized();
}

void TrackOrganizer::PostRepatch(PatchCableSource* cableSource, bool fromUserClick)
{
   if (cableSource == mGainCable)
   {
      Amplifier* gain = GetGain();
      if (gain)
         gain->SetShowLevelMeter(true);
   }
}

void TrackOrganizer::KeyPressed(int key, bool isRepeat)
{
   if (gHoveredModule == this && key == OF_KEY_RETURN)
   {
      GatherModules(TheSynth->GetGroupSelectedModules());
   }
}

void TrackOrganizer::GatherModules(const std::vector<IDrawableModule*>& modulesToAdd)
{
   auto trackModules = GetAllTrackModules();
   for (auto* module : modulesToAdd)
   {
      if (module != this && !ListContains(module, trackModules))
      {
         //TODO(Ryan) smartly determine which category to put each module in, based upon type and chain position
         if (mSnapshotsCable->GetTarget() == nullptr && dynamic_cast<Snapshots*>(module))
            mSnapshotsCable->SetTarget(module);
         else if (mGridInterfaceCables[0]->GetTarget() == nullptr && dynamic_cast<IAbletonGridController*>(module))
            mGridInterfaceCables[0]->SetTarget(module);
         else if (mRecorderCable->GetTarget() == nullptr && dynamic_cast<IInputRecordable*>(module))
            mRecorderCable->SetTarget(module);
         else if (mGainCable->GetTarget() == nullptr && dynamic_cast<Amplifier*>(module))
            mGainCable->SetTarget(module);
         else if (mSendCable->GetTarget() == nullptr && dynamic_cast<AudioSend*>(module))
            mSendCable->SetTarget(module);
         else
            mOtherTrackModulesCable->AddPatchCable(module);
      }
   }
}

void TrackOrganizer::ButtonClicked(ClickButton* button, double time)
{
   if (button == mSelectModulesButton)
   {
      if (TheSynth->GetGroupSelectedModules().size() > 0)
         GatherModules(TheSynth->GetGroupSelectedModules());
      else
         mSelectModulesOnMouseRelease = TheSynth->IsMouseButtonHeld(1);
   }
}

bool TrackOrganizer::AdjustModuleIndex(int amount)
{
   int newIndex = mModuleIndex + amount;
   if (newIndex >= 0 && newIndex < (int)mControlModuleCables.size() &&
       (mControlModuleCables[newIndex]->GetTarget() != nullptr || mGridInterfaceCables[newIndex]->GetTarget() != nullptr))
   {
      mModuleIndex = newIndex;
      return true;
   }
   return false;
}

IDrawableModule* TrackOrganizer::GetCurrentModule() const
{
   if (mModuleIndex >= 0 && mModuleIndex < (int)mControlModuleCables.size())
      return dynamic_cast<IDrawableModule*>(mControlModuleCables[mModuleIndex]->GetTarget());
   return nullptr;
}

IAbletonGridController* TrackOrganizer::GetCurrentGridInterface() const
{
   IAbletonGridController* ret = nullptr;
   if (mModuleIndex >= 0 && mModuleIndex < (int)mGridInterfaceCables.size())
      ret = dynamic_cast<IAbletonGridController*>(mGridInterfaceCables[mModuleIndex]->GetTarget());
   if (ret == nullptr)
      ret = dynamic_cast<IAbletonGridController*>(mGridInterfaceCables[0]->GetTarget());
   return ret;
}

std::vector<IDrawableModule*> TrackOrganizer::GetControlModules() const
{
   std::vector<IDrawableModule*> ret;
   for (const auto* cableSource : mControlModuleCables)
      ret.push_back(dynamic_cast<IDrawableModule*>(cableSource->GetTarget()));
   return ret;
}

std::vector<IAbletonGridController*> TrackOrganizer::GetGridInterfaces() const
{
   std::vector<IAbletonGridController*> ret;
   for (const auto* cableSource : mGridInterfaceCables)
      ret.push_back(dynamic_cast<IAbletonGridController*>(cableSource->GetTarget()));
   return ret;
}

Snapshots* TrackOrganizer::GetSnapshots() const
{
   return dynamic_cast<Snapshots*>(mSnapshotsCable->GetTarget());
}

IUIControl* TrackOrganizer::GetSoundSelector() const
{
   return dynamic_cast<IUIControl*>(mSoundSelectorCable->GetTarget());
}

IInputRecordable* TrackOrganizer::GetRecorder() const
{
   return dynamic_cast<IInputRecordable*>(mRecorderCable->GetTarget());
}

Amplifier* TrackOrganizer::GetGain() const
{
   return dynamic_cast<Amplifier*>(mGainCable->GetTarget());
}

AudioSend* TrackOrganizer::GetSend() const
{
   return dynamic_cast<AudioSend*>(mSendCable->GetTarget());
}

void TrackOrganizer::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadBool("draw_track_bounds", moduleInfo, true);
   mModuleSaveData.LoadBool("draw_track_name", moduleInfo, true);

   SetUpFromSaveData();
}

void TrackOrganizer::SetUpFromSaveData()
{
   mDrawTrackBounds = mModuleSaveData.GetBool("draw_track_bounds");
   mDrawTrackName = mModuleSaveData.GetBool("draw_track_name");
}

void TrackOrganizer::SaveLayout(ofxJSONElement& moduleInfo)
{
}
