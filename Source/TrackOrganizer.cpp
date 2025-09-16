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
#include "UIControlMacros.h"

#include <cstring>

TrackOrganizer::TrackOrganizer()
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

   mGridInterfaceCable = new PatchCableSource(this, kConnectionType_Special);
   mGridInterfaceCable->SetPredicateFilter(IsOfType<IAbletonGridController*>);
   mGridInterfaceCable->SetColor(cableColor);
   mGridInterfaceCable->SetManualPosition(cableX, cableY);
   AddPatchCableSource(mGridInterfaceCable);
   cableX += 12;

   cableX += 10;

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

   cableX += 10;

   mOtherTrackModulesCable = new PatchCableSource(this, kConnectionType_Special);
   mOtherTrackModulesCable->SetColor(cableColor);
   mOtherTrackModulesCable->SetManualPosition(cableX, cableY);
   mOtherTrackModulesCable->SetAllowMultipleTargets(true);
   AddPatchCableSource(mOtherTrackModulesCable);
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

void TrackOrganizer::PreDrawModuleUnclipped()
{
   if (mDrawTrackBounds)
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
      allModulesRect.x -= mX;
      allModulesRect.y -= mY;

      ofColor color = AbletonDevice::kColors[mColorIndex].color;
      color.a = 20;

      ofPushStyle();

      ofSetColor(color);
      ofSetLineWidth(3);

      ofFill();
      ofRect(allModulesRect, 20);

      //ofNoFill();
      //ofRect(allModulesRect, 20);

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
   ofVec2f pos;
   const ofVec2f kTooltipOffset(15, 8);

   if (mSnapshotsCable->IsHovered())
   {
      tooltip = "snapshots";
      pos = mSnapshotsCable->GetPosition() - GetPosition() + kTooltipOffset;
   }

   if (mGridInterfaceCable->IsHovered())
   {
      tooltip = "grid interface";
      pos = mGridInterfaceCable->GetPosition() - GetPosition() + kTooltipOffset;
   }

   for (const auto* cableSource : mControlModuleCables)
   {
      if (cableSource->IsHovered())
      {
         tooltip = "control module";
         pos = cableSource->GetPosition() - GetPosition() + kTooltipOffset;
      }
   }

   if (mRecorderCable->IsHovered())
   {
      tooltip = "recorder";
      pos = mRecorderCable->GetPosition() - GetPosition() + kTooltipOffset;
   }

   if (mGainCable->IsHovered())
   {
      tooltip = "gain";
      pos = mGainCable->GetPosition() - GetPosition() + kTooltipOffset;
   }

   if (mOtherTrackModulesCable->IsHovered())
   {
      tooltip = "other modules in track";
      pos = mOtherTrackModulesCable->GetPosition() - GetPosition() + kTooltipOffset;
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

void TrackOrganizer::Poll()
{
   IDrawableModule::Poll();

   if (mSelectModulesOnMouseRelease && !TheSynth->IsMouseButtonHeld(1))
   {
      TheSynth->SetGroupSelectedModules(GetAllTrackModules());
      mSelectModulesOnMouseRelease = false;
   }

   bool showCables = ShouldShowCables();
   ofColor cableColor = IDrawableModule::GetColor(kModuleCategory_Other);
   if (showCables)
      cableColor.a *= .3f;
   else
      cableColor.a *= 0;

   for (auto* source : GetPatchCableSources())
   {
      source->SetColor(cableColor);
      source->SetClickable(showCables);
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

void TrackOrganizer::ButtonClicked(ClickButton* button, double time)
{
   if (button == mSelectModulesButton)
      mSelectModulesOnMouseRelease = TheSynth->IsMouseButtonHeld(1);
}

void TrackOrganizer::GetModuleDimensions(float& w, float& h)
{
   w = mWidth;
   h = mHeight;
}

void TrackOrganizer::AdjustModuleIndex(int amount)
{
   int newIndex = mModuleIndex + amount;
   if (newIndex >= 0 && newIndex < (int)mControlModuleCables.size() && mControlModuleCables[newIndex]->GetTarget() != nullptr)
      mModuleIndex = newIndex;
}

IDrawableModule* TrackOrganizer::GetCurrentModule() const
{
   if (mModuleIndex >= 0 && mModuleIndex < (int)mControlModuleCables.size())
      return dynamic_cast<IDrawableModule*>(mControlModuleCables[mModuleIndex]->GetTarget());
   return nullptr;
}

Snapshots* TrackOrganizer::GetSnapshots() const
{
   return dynamic_cast<Snapshots*>(mSnapshotsCable->GetTarget());
}

IAbletonGridController* TrackOrganizer::GetGridInterface() const
{
   return dynamic_cast<IAbletonGridController*>(mGridInterfaceCable->GetTarget());
}

std::vector<IDrawableModule*> TrackOrganizer::GetModuleList() const
{
   std::vector<IDrawableModule*> moduleList;
   for (int i = 0; i < (int)mControlModuleCables.size(); ++i)
   {
      IDrawableModule* module = dynamic_cast<IDrawableModule*>(mControlModuleCables[i]->GetTarget());
      if (module != nullptr)
         moduleList.push_back(module);
   }
   return moduleList;
}

IInputRecordable* TrackOrganizer::GetRecorder() const
{
   return dynamic_cast<IInputRecordable*>(mRecorderCable->GetTarget());
}

Amplifier* TrackOrganizer::GetGain() const
{
   return dynamic_cast<Amplifier*>(mGainCable->GetTarget());
}

void TrackOrganizer::LoadLayout(const ofxJSONElement& moduleInfo)
{
   mModuleSaveData.LoadBool("draw_track_bounds", moduleInfo, true);

   SetUpFromSaveData();
}

void TrackOrganizer::SetUpFromSaveData()
{
   mDrawTrackBounds = mModuleSaveData.GetBool("draw_track_bounds");
}

void TrackOrganizer::SaveLayout(ofxJSONElement& moduleInfo)
{
}
