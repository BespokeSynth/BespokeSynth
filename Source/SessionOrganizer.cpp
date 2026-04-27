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

    SessionOrganizer.cpp
    Created: 7 Apr 2026
    Author:  Ryan Challinor

  ==============================================================================
*/

#include "SessionOrganizer.h"
#include "SynthGlobals.h"
#include "OpenFrameworksPort.h"
#include "UIControlMacros.h"
#include "ModuleSaveDataPanel.h"
#include "PatchCableSource.h"
#include "DropdownList.h"
#include "TrackOrganizer.h"
#include "IInputRecordable.h"
#include "Amplifier.h"
#include "AudioSend.h"
#include "IControlVisualizer.h"
#include "Snapshots.h"

namespace
{
   float kPaddingTop = 15;
   float kPaddingBottom = 5;
   float kPaddingLeft = 5;
   float kPaddingBetween = 5;
   float kColumnWidth = 100;
   float kColumnHeight = 250;
}

SessionOrganizer::SessionOrganizer()
: IDrawableModule(100, 100)
{
}

SessionOrganizer::~SessionOrganizer()
{
}

void SessionOrganizer::CreateUIControls()
{
   IDrawableModule::CreateUIControls();

   for (size_t i = 0; i < mTrackCables.size(); ++i)
   {
      mTrackCables[i] = new PatchCableSource(this, kConnectionType_Special);
      mTrackCables[i]->AddTypeFilter("trackorganizer");
      mTrackCables[i]->SetManualPosition(8 + (int)i * 12, 8);
      AddPatchCableSource(mTrackCables[i]);
   }

   for (int i = 0; i < (int)mTrackColumns.size(); ++i)
      mTrackColumns[i].CreateUIControls(this, i);
}

void SessionOrganizer::TrackColumn::CreateUIControls(SessionOrganizer* sessionOrganizer, int index)
{
   mEnabledCheckbox = new Checkbox(sessionOrganizer, ("enabled" + ofToString(index)).c_str(), -1, -1, &mEnabled);
   mSnapshotSelector = new RadioButton(sessionOrganizer, ("snapshot" + ofToString(index)).c_str(), -1, -1, &mDummySnapshot);
   mGainSlider = new FloatSlider(sessionOrganizer, ("gain" + ofToString(index)).c_str(), -1, -1, 80, 15, &mDummyGain, 0, 1);
   mSendSlider = new FloatSlider(sessionOrganizer, ("send" + ofToString(index)).c_str(), -1, -1, 80, 15, &mDummySend, 0, 1);

   mEnabledCheckbox->SetOverrideDisplayName("enabled");
   mGainSlider->SetOverrideDisplayName("gain");
   mSendSlider->SetOverrideDisplayName("send");
}

TrackOrganizer* SessionOrganizer::GetTrack(int index) const
{
   if (index >= 0 && index < mTrackCables.size())
      return dynamic_cast<TrackOrganizer*>(mTrackCables[index]->GetTarget());
   return nullptr;
}

void SessionOrganizer::DrawModule()
{
   if (Minimized() || IsVisible() == false)
      return;

   for (int i = 0; i < (int)mTrackColumns.size(); ++i)
      mTrackColumns[i].Draw(this, i);
}

ofVec2f SessionOrganizer::TrackColumn::GetPosition(int index) const
{
   return ofVec2f(kPaddingLeft + index * (kColumnWidth + kPaddingBetween), kPaddingTop);
}

void SessionOrganizer::TrackColumn::Draw(SessionOrganizer* sessionOrganizer, int index)
{
   if (TrackOrganizer* track = sessionOrganizer->GetTrack(index))
   {
      ofVec2f position = GetPosition(index);

      IUIControl* enabledControl = track->GetEnabledControl();
      Snapshots* snapshots = track->GetSnapshots();
      Amplifier* gain = track->GetGain();
      AudioSend* send = track->GetSend();

      ofPushStyle();

      ofSetColor(track->GetColor(), gModuleDrawAlpha * .3f);
      ofFill();
      ofRect(position.x, position.y, kColumnWidth, kColumnHeight);

      ofSetColor(track->GetColor());
      DrawTextBold(track->GetTrackName(), position.x + 5, position.y + 13);

      mEnabledCheckbox->SetShowing(enabledControl != nullptr);
      mSnapshotSelector->SetShowing(snapshots != nullptr);
      mGainSlider->SetShowing(gain != nullptr);
      mSendSlider->SetShowing(send != nullptr);

      mEnabledCheckbox->SetPosition(position.x + 5, position.y + 20);
      mSnapshotSelector->SetPosition(position.x + 5, position.y + 40);
      mGainSlider->SetPosition(position.x + 5, position.y + kColumnHeight - 60);
      mSendSlider->SetPosition(position.x + 5, position.y + kColumnHeight - 40);

      IUIControl::sCurrentOverrideColor = track->GetColor();
      IUIControl::sUseOverrideColor = true;
      mEnabledCheckbox->Draw();
      mSnapshotSelector->Draw();
      mGainSlider->Draw();
      mSendSlider->Draw();
      IUIControl::sUseOverrideColor = false;

      if (gain != nullptr)
         gain->DrawLevelMeter(position.x + 5, position.y + kColumnHeight - 20, kColumnWidth - 10, 15);

      ofPopStyle();
   }
   else
   {
      mEnabledCheckbox->SetShowing(false);
      mSnapshotSelector->SetShowing(false);
      mGainSlider->SetShowing(false);
      mSendSlider->SetShowing(false);
   }
}

void SessionOrganizer::DrawModuleUnclipped()
{
   std::string tooltip = "";
   const PatchCableSource* hoverCable = nullptr;

   for (int i = 0; i < (int)mTrackCables.size(); ++i)
   {
      if (mTrackCables[i]->IsHovered())
      {
         hoverCable = mTrackCables[i];
         tooltip = "track " + ofToString(i);
      }
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
}

void SessionOrganizer::GetModuleDimensions(float& width, float& height)
{
   int numTracks = 0;
   for (int i = 0; i < mTrackCables.size(); ++i)
   {
      if (mTrackCables[i]->GetTarget() != nullptr)
         numTracks = i + 1;
   }
   width = kPaddingLeft + numTracks * (kColumnWidth + kPaddingBetween);
   height = kPaddingTop + kColumnHeight + kPaddingBottom;
}

void SessionOrganizer::OnClicked(float x, float y, bool right)
{
   IDrawableModule::OnClicked(x, y, right);
}

void SessionOrganizer::RadioButtonUpdated(RadioButton* radio, int oldVal, double time)
{
   for (int i = 0; i < (int)mTrackColumns.size(); ++i)
   {
      if (radio == mTrackColumns[i].mSnapshotSelector)
      {
         if (TrackOrganizer* track = GetTrack(i))
         {
            if (Snapshots* snapshots = track->GetSnapshots())
               snapshots->SetSnapshot(radio->GetValue(), time);
         }
      }
   }
}

void SessionOrganizer::CheckboxUpdated(Checkbox* checkbox, double time)
{
   for (int i = 0; i < (int)mTrackColumns.size(); ++i)
   {
      if (checkbox == mTrackColumns[i].mEnabledCheckbox)
      {
         if (TrackOrganizer* track = GetTrack(i))
            track->SetTrackEnabled(mTrackColumns[i].mEnabled);
      }
   }
}

void SessionOrganizer::LoadLayout(const ofxJSONElement& moduleInfo)
{
   SetUpFromSaveData();
}

void SessionOrganizer::SetUpFromSaveData()
{
}

void SessionOrganizer::SaveState(FileStreamOut& out)
{
   out << GetModuleSaveStateRev();

   IDrawableModule::SaveState(out);
}

void SessionOrganizer::LoadState(FileStreamIn& in, int rev)
{
   IDrawableModule::LoadState(in, rev);

   LoadStateValidate(rev <= GetModuleSaveStateRev());
}

void SessionOrganizer::Poll()
{
   for (int i = 0; i < (int)mTrackColumns.size(); ++i)
   {
      mTrackColumns[i].Poll(this, i);

      if (i < (int)mTrackCables.size())
      {
         ofColor cableColor = IDrawableModule::GetColor(kModuleCategory_Other);
         if (const TrackOrganizer* track = GetTrack(i))
            cableColor = track->GetColor();
         cableColor.a *= .3f;
         mTrackCables[i]->SetColor(cableColor);
      }
   }
}

void SessionOrganizer::TrackColumn::Poll(SessionOrganizer* sessionOrganizer, int index)
{
   if (const TrackOrganizer* track = sessionOrganizer->GetTrack(index))
   {
      ofVec2f position = GetPosition(index);

      Snapshots* snapshots = track->GetSnapshots();
      Amplifier* gain = track->GetGain();
      AudioSend* send = track->GetSend();

      mEnabled = track->IsTrackEnabled();

      if (snapshots)
      {
         if (mSnapshotSelector->GetVar() != snapshots->GetCurrentSnapshotVar())
            mSnapshotSelector->SetVar(snapshots->GetCurrentSnapshotVar());

         //update snapshot selector to match snapshots
         for (int i = 0; i < snapshots->GetSize(); ++i)
         {
            std::string label;
            if (snapshots->HasSnapshot(i))
               label = snapshots->GetLabel(i);

            if (i < mSnapshotSelector->GetNumValues())
               mSnapshotSelector->SetLabel(label.c_str(), i);
            else
               mSnapshotSelector->AddLabel(label.c_str(), i);
         }

         //remove extras
         for (int i = snapshots->GetSize(); i < mSnapshotSelector->GetNumValues(); ++i)
            mSnapshotSelector->RemoveLabel(i);

         mSnapshotSelector->SetForcedWidth(kColumnWidth - 10);
      }

      if (send)
      {
         if (mSendSlider->GetVar() != send->GetAmountSlider()->GetVar())
         {
            mSendSlider->SetVar(send->GetAmountSlider()->GetVar());
            mSendSlider->SetExtents(send->GetAmountSlider()->GetMin(), send->GetAmountSlider()->GetMax());
            mSendSlider->SetMode(send->GetAmountSlider()->GetMode());
         }

         ofVec2f offset = sessionOrganizer->GetPosition() - send->GetPosition();
         const auto& cableSources = send->GetPatchCableSources();
         for (int i = 0; i < (int)cableSources.size(); ++i)
         {
            PatchCableSource* cableSource = cableSources[i];
            cableSource->SetManualPosition(position.x + kColumnWidth / 2 - 10 + i * 20 + offset.x, position.y + kColumnHeight + offset.y);
         }

         mSendSlider->SetDimensions(kColumnWidth - 10, 15);
      }

      if (gain)
      {
         if (mGainSlider->GetVar() != gain->GetGainSlider()->GetVar())
         {
            mGainSlider->SetVar(gain->GetGainSlider()->GetVar());
            mGainSlider->SetExtents(gain->GetGainSlider()->GetMin(), gain->GetGainSlider()->GetMax());
            mGainSlider->SetMode(gain->GetGainSlider()->GetMode());
         }

         if (send)
         {
            // send is handling the output cables
            gain->GetPatchCableSource()->EnableAutomaticPositioning();
         }
         else
         {
            ofVec2f offset = sessionOrganizer->GetPosition() - gain->GetPosition();
            gain->GetPatchCableSource()->SetManualPosition(position.x + kColumnWidth / 2 + offset.x, position.y + kColumnHeight + offset.y);
         }

         mGainSlider->SetDimensions(kColumnWidth - 10, 15);
      }
   }
}
