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
//  PatchCableSource.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 12/13/15.
//
//

#include "PatchCableSource.h"
#include "ModularSynth.h"
#include "TitleBar.h"
#include "ADSRDisplay.h"
#include "INoteReceiver.h"
#include "GridController.h"
#include "IPulseReceiver.h"
#include "AudioSend.h"
#include "MacroSlider.h"

#include "juce_gui_basics/juce_gui_basics.h"

namespace
{
   const int kPatchCableSourceRadius = 5;
   const int kPatchCableSourceClickRadius = 7;
   const int kPatchCableSpacing = 8;
}

bool PatchCableSource::sAllowInsert = true;

PatchCableSource::PatchCableSource(IDrawableModule* owner, ConnectionType type)
: mOwner(owner)
, mType(type)
, mHoverIndex(-1)
, mOverrideVizBuffer(nullptr)
, mAutomaticPositioning(true)
, mAudioReceiver(nullptr)
, mDefaultPatchBehavior(kDefaultPatchBehavior_Repatch)
, mPatchCableDrawMode(kPatchCableDrawMode_Normal)
, mEnabled(true)
, mClickable(true)
, mSide(Side::kNone)
, mManualSide(Side::kNone)
, mHasOverrideCableDir(false)
, mLastOnEventTime(-9999)
, mModulatorOwner(nullptr)
, mDrawPass(DrawPass::kSource)
, mParentMinimized(false)
{
   mAllowMultipleTargets = (mType == kConnectionType_Note || mType == kConnectionType_Pulse || mType == kConnectionType_Audio || mType == kConnectionType_Modulator);
   SetConnectionType(type);
}

PatchCableSource::~PatchCableSource()
{
   for (auto cable : mPatchCables)
      delete cable;
}

void PatchCableSource::SetConnectionType(ConnectionType type)
{
   mType = type;

   if (mType == kConnectionType_Note)
      mColor = IDrawableModule::GetColor(kModuleType_Note);
   else if (mType == kConnectionType_Audio)
      mColor = IDrawableModule::GetColor(kModuleType_Audio);
   else if (mType == kConnectionType_Modulator)
      mColor = IDrawableModule::GetColor(kModuleType_Modulator);
   else if (mType == kConnectionType_Pulse)
      mColor = IDrawableModule::GetColor(kModuleType_Pulse);
   else
      mColor = IDrawableModule::GetColor(kModuleType_Other);
   mColor.setBrightness(mColor.getBrightness() * .8f);
}

PatchCable* PatchCableSource::AddPatchCable(IClickable* target)
{
   for (auto cable : mPatchCables)
   {
      if (cable != nullptr && cable->GetTarget() == target)
         return nullptr;
   }
   
   PatchCable* cable = new PatchCable(this);
   mPatchCables.push_back(cable);
   
   if (target)
      SetPatchCableTarget(cable, target, false);
   
   return cable;
}

void PatchCableSource::SetPatchCableTarget(PatchCable* cable, IClickable* target, bool fromUserClick)
{
   IClickable* oldTarget = cable->GetTarget();
   
   mOwner->PreRepatch(this);
   
   if (cable->GetTarget())
   {
      mAudioReceiver = nullptr;
      RemoveFromVector(dynamic_cast<INoteReceiver*>(cable->GetTarget()), mNoteReceivers);
      RemoveFromVector(dynamic_cast<IPulseReceiver*>(cable->GetTarget()), mPulseReceivers);
   }
   
   cable->SetTarget(target);
   
   INoteReceiver* noteReceiver = dynamic_cast<INoteReceiver*>(target);
   if (noteReceiver)
      mNoteReceivers.push_back(noteReceiver);
   IPulseReceiver* pulseReceiver = dynamic_cast<IPulseReceiver*>(target);
   if (pulseReceiver)
      mPulseReceivers.push_back(pulseReceiver);
   IAudioReceiver* audioReceiver = dynamic_cast<IAudioReceiver*>(target);
   if (audioReceiver)
   {
      mAudioReceiver = audioReceiver;
      TheSynth->ArrangeAudioSourceDependencies();
   }
   
   mOwner->PostRepatch(this, fromUserClick);
   
   //insert
   if (GetKeyModifiers() == kModifier_Shift && fromUserClick)
   {
      if (sAllowInsert)  //avoid cascade on the next set
      {
         sAllowInsert = false;
         IDrawableModule* targetModule = dynamic_cast<IDrawableModule*>(target);
         if (targetModule && targetModule->GetPatchCableSource())
         {
            targetModule->GetPatchCableSource()->FindValidTargets();
            if (targetModule->GetPatchCableSource()->IsValidTarget(oldTarget))
               targetModule->SetTarget(oldTarget);
         }
         sAllowInsert = true;
      }
   }
}

void PatchCableSource::Clear()
{
   auto cablesToRemove = mPatchCables; //make copy of list
   for (auto cable : cablesToRemove)
      RemovePatchCable(cable);
   mPatchCables.clear();
}

void PatchCableSource::UpdatePosition(bool parentMinimized)
{
   if ((mAutomaticPositioning || parentMinimized) && mOwner != nullptr)
   {
      float x,y,w,h;
      mOwner->GetPosition(x, y);
      mOwner->GetDimensions(w, h);

      if (parentMinimized)
      {
         mOwner->GetParent()->GetPosition(x, y);
         mOwner->GetParent()->GetDimensions(w, h);
      }
      
      if (mManualSide == Side::kNone || parentMinimized)
      {
         ofVec2f centerOfMass;
         int count = 0;
         for (auto cable : mPatchCables)
         {
            if (cable != nullptr)
            {
               if (cable->IsDragging())
               {
                  centerOfMass.x += TheSynth->GetMouseX(mOwner->GetOwningContainer());
                  centerOfMass.y += TheSynth->GetMouseY(mOwner->GetOwningContainer());
                  ++count;
               }
               else if (cable->GetTarget())
               {
                  float targetX,targetY,targetW,targetH;
                  cable->GetTarget()->GetPosition(targetX, targetY);
                  cable->GetTarget()->GetDimensions(targetW, targetH);
                  centerOfMass.x += targetX + targetW / 2;
                  centerOfMass.y += targetY + targetH / 2;
                  ++count;
               }
            }
         }
         centerOfMass.x /= count;
         centerOfMass.y /= count;
         
         if (count > 0)
         {
            if (centerOfMass.y > y+h)
               mSide = Side::kBottom;
            else if (centerOfMass.x < x+w/2)
               mSide = Side::kLeft;
            else
               mSide = Side::kRight;
         }
         else
         {
            mSide = Side::kBottom;
         }
      }
      else
      {
         mSide = mManualSide;
      }
      
      if (mSide == Side::kBottom)
      {
         mX = x+w/2;
         mY = y+h+3;
      }
      else if (mSide == Side::kLeft)
      {
         mX = x-3;
         mY = y+h/2;
      }
      else if (mSide == Side::kRight)
      {
         mX = x+w+3;
         mY = y+h/2;
      }
   }
   else
   {
      float x,y;
      mOwner->GetPosition(x, y);
      mX = mManualPositionX + x;
      mY = mManualPositionY + y;
      mSide = mManualSide;
   }
}

void PatchCableSource::DrawSource()
{
   mDrawPass = DrawPass::kSource;
   Draw();
}

void PatchCableSource::DrawCables(bool parentMinimized)
{
   mDrawPass = DrawPass::kCables;
   mParentMinimized = parentMinimized;
   Draw();
}

void PatchCableSource::Render()
{
   if (!Enabled() || mOwner->IsDeleted())
      return;

   ofPushStyle();

   float cableX = mX;
   float cableY = mY;
   if (mOwner->GetOwningContainer() != nullptr)
   {
      cableX -= mOwner->GetOwningContainer()->GetOwnerPosition().x;
      cableY -= mOwner->GetOwningContainer()->GetOwnerPosition().y;
   }

   for (int i = 0; i < (int)mPatchCables.size() || i==0; ++i)
   {
      if (i < (int)mPatchCables.size() && mPatchCables[i] != nullptr)
      {
         mPatchCables[i]->SetSourceIndex(i);

         if (mParentMinimized)
         {
            IDrawableModule* owningModule = mPatchCables[i]->GetOwningModule();
            IClickable* target = mPatchCables[i]->GetTarget();
            if (owningModule != nullptr && target != nullptr)
            {
               ModuleContainer* owningModuleContainer = owningModule->GetOwningContainer();
               ModuleContainer* targetContainer = target->GetModuleParent()->GetOwningContainer();
               if (owningModuleContainer == targetContainer)
                  continue;   //this is an internally-patched cable in a minimized module
            }
         }
      }

      if (mDrawPass == DrawPass::kCables && (mPatchCableDrawMode != kPatchCableDrawMode_CablesOnHoverOnly || mHoverIndex != -1))
      {
         if (i < (int)mPatchCables.size() && mPatchCables[i] != nullptr)
            mPatchCables[i]->Draw();
      }

      if (mDrawPass == DrawPass::kSource && (mPatchCableDrawMode != kPatchCableDrawMode_SourceOnHoverOnly || mHoverIndex != -1))
      {
         ofSetLineWidth(0);
         ofSetColor(mColor);
         ofFill();
         ofCircle(cableX, cableY, kPatchCableSourceRadius);

         if (mHoverIndex == i && PatchCable::sActivePatchCable == nullptr && !TheSynth->IsGroupSelecting())
         {
            ofSetColor(ofColor::white);
            ofFill();
            ofCircle(cableX, cableY, kPatchCableSourceRadius - 2);
            if (InAddCableMode())
            {
               ofSetColor(0, 0, 0);
               ofSetLineWidth(2);
               ofLine(cableX, cableY - (kPatchCableSourceRadius - 1), cableX, cableY + (kPatchCableSourceRadius - 1));
               ofLine(cableX - (kPatchCableSourceRadius - 1), cableY, cableX + (kPatchCableSourceRadius - 1), cableY);
            }
         }
         
         if (mType == kConnectionType_Grid)
         {
            ofPushStyle();
            ofNoFill();
            ofSetColor(IDrawableModule::GetColor(kModuleType_Other));
            GridControlTarget::DrawGridIcon(mX + 7, mY - 6);
            ofPopStyle();
         }
      }

      if (mSide == Side::kBottom)
         cableY += kPatchCableSpacing;
      else if (mSide == Side::kLeft)
         cableX -= kPatchCableSpacing;
      else if (mSide == Side::kRight)
         cableX += kPatchCableSpacing;
   }

   ofPopStyle();
}

ofVec2f PatchCableSource::GetCableStart(int index) const
{
   float cableX = mX;
   float cableY = mY;

   if (mSide == Side::kBottom)
      cableY += kPatchCableSpacing * index;
   else if (mSide == Side::kLeft)
      cableX -= kPatchCableSpacing * index;
   else if (mSide == Side::kRight)
      cableX += kPatchCableSpacing * index;

   return ofVec2f(cableX, cableY);
}

ofVec2f PatchCableSource::GetCableStartDir(int index, ofVec2f dest) const
{
   if (mHasOverrideCableDir)
   {
      return mOverrideCableDir;
   }
   else
   {
      enum class Direction
      {
         kUp,
         kDown,
         kLeft,
         kRight,
         kNone
      };

      Direction dir;
      switch (mSide)
      {
      case Side::kBottom: dir = Direction::kDown; break;
      case Side::kLeft: dir = Direction::kLeft; break;
      case Side::kRight: dir = Direction::kRight; break;
      case Side::kNone: dir = Direction::kNone; break;
      }

      if (mPatchCables.size() > 0 && index < mPatchCables.size() - 1)  //not the top of the cable stack
      {
         if (mSide == Side::kBottom)
         {
            if (dest.x < mX)
               dir = Direction::kLeft;
            else
               dir = Direction::kRight;
         }
         else if (mSide == Side::kLeft || mSide == Side::kRight)
         {
            if (dest.y < mY)
               dir = Direction::kUp;
            else
               dir = Direction::kDown;
         }
      }

      switch (dir)
      {
      case Direction::kDown:
         return ofVec2f(0, 1);
      case Direction::kRight:
         return ofVec2f(1, 0);
      case Direction::kLeft:
         return ofVec2f(-1, 0);
      case Direction::kUp:
         return ofVec2f(0, -1);
      default:
         return ofVec2f(0, 0);
      }
   }
}

bool PatchCableSource::MouseMoved(float x, float y)
{
   if (!Enabled())
      return false;
   
   if (!mClickable)
      return false;
   
   x = TheSynth->GetMouseX(mOwner->GetOwningContainer());
   y = TheSynth->GetMouseY(mOwner->GetOwningContainer());
   
   mHoverIndex = GetHoverIndex(x, y);

   if (mHoverIndex != -1 && gHoveredUIControl != nullptr && !gHoveredUIControl->IsMouseDown())
      gHoveredUIControl = nullptr; //if we're hovering over a patch cable, get rid of ui control hover
   
   for (size_t i=0; i<mPatchCables.size(); ++i)
   {
      if (mPatchCables[i] != nullptr)
      {
         mPatchCables[i]->SetHoveringOnSource(i == mHoverIndex);
         mPatchCables[i]->NotifyMouseMoved(x, y);
      }
   }
   
   return false;
}

void PatchCableSource::MouseReleased()
{
   std::vector<PatchCable*> cables = mPatchCables;   //copy, since list might get modified here
   FindValidTargets();
   for (auto cable : cables)
      cable->MouseReleased();
}

bool PatchCableSource::TestClick(int x, int y, bool right, bool testOnly /* = false */)
{
   if (!Enabled())
      return false;
   
   if (!mClickable)
      return false;

   if (right)
      return false;
   
   if (mHoverIndex != -1)
   {
      if (!testOnly)
      {
         if (mPatchCables.empty() || InAddCableMode())
         {
            if (mPatchCables.empty() ||
                mType == kConnectionType_Note ||
                mType == kConnectionType_Pulse)
            {
               PatchCable* newCable = AddPatchCable(nullptr);
               newCable->Grab();
            }
            else if (mType == kConnectionType_Audio)
            {
               ofVec2f spawnOffset(-20, 10);
               AudioSend* send = dynamic_cast<AudioSend*>(TheSynth->SpawnModuleOnTheFly("send", x + spawnOffset.x, y + spawnOffset.y));
               send->SetTarget(GetTarget());
               SetTarget(send);
               send->SetSend(1, false);
               TheSynth->SetMoveModule(send, spawnOffset.x, spawnOffset.y, false);
            }
            else if (mType == kConnectionType_Modulator)
            {
               ofVec2f spawnOffset(-20, 10);
               MacroSlider* macroSlider = dynamic_cast<MacroSlider*>(TheSynth->SpawnModuleOnTheFly("macroslider", x + spawnOffset.x, y + spawnOffset.y));
               IUIControl* currentTarget = dynamic_cast<IUIControl*>(GetTarget());
               SetTarget(macroSlider->GetSlider());
               macroSlider->SetOutputTarget(0, currentTarget);
               TheSynth->SetMoveModule(macroSlider, spawnOffset.x, spawnOffset.y, false);
            }
         }
         else
         {
            if (mPatchCables[mHoverIndex] != nullptr)
               mPatchCables[mHoverIndex]->Grab();
         }
      }
      return true;
   }
   
   //for (auto cable : mPatchCables)
   //{
   //   if (cable->TestClick(x, y, right, testOnly))
   //      return true;
   //}
   
   return false;
}

bool PatchCableSource::TestHover(float x, float y) const
{
   if (!Enabled())
      return false;
   
   if (!mClickable)
      return false;
   
   return GetHoverIndex(x, y) != -1;
}

int PatchCableSource::GetHoverIndex(float x, float y) const
{
   float cableX = mX;
   float cableY = mY;
   for (int i = 0; i < mPatchCables.size() || i == 0; ++i)
   {
      if (ofDistSquared(x, y, cableX, cableY) < kPatchCableSourceClickRadius * kPatchCableSourceClickRadius)
         return i;

      if (mSide == Side::kBottom)
         cableY += kPatchCableSpacing;
      else if (mSide == Side::kLeft)
         cableX -= kPatchCableSpacing;
      else if (mSide == Side::kRight)
         cableX += kPatchCableSpacing;
   }

   return -1;
}

bool PatchCableSource::Enabled() const
{
   return mEnabled && (mAutomaticPositioning || !mOwner->Minimized());
}

void PatchCableSource::OnClicked(int x, int y, bool right)
{
   
}

bool PatchCableSource::InAddCableMode() const
{
   return (GetKeyModifiers() == kModifier_Shift && mAllowMultipleTargets) || mDefaultPatchBehavior == kDefaultPatchBehavior_Add;
}

bool PatchCableSource::IsValidTarget(IClickable* target) const
{
   if (target == nullptr)
      return false;
   return VectorContains(target, mValidTargets);
}

void PatchCableSource::FindValidTargets()
{
   mValidTargets.clear();
   std::vector<IDrawableModule*> allModules;
   TheSynth->GetAllModules(allModules);
   for (auto module : allModules)
   {
      if ((mType == kConnectionType_Modulator || mType == kConnectionType_UIControl || mType == kConnectionType_Grid) && module != TheTitleBar)
      {
         for (auto uicontrol : module->GetUIControls())
         {
            if (uicontrol->IsShowing() &&
                (uicontrol->GetShouldSaveState() || dynamic_cast<ClickButton*>(uicontrol) != nullptr) &&
                uicontrol->CanBeTargetedBy(this) &&
                !uicontrol->GetNoHover())
               mValidTargets.push_back(uicontrol);
         }
      }
      if (module == mOwner)
         continue;
      if (module == mOwner->GetParent())
         continue;
      if (mTypeFilter.empty() == false && !VectorContains(module->GetTypeName(), mTypeFilter))
         continue;
      if (mType == kConnectionType_Audio && module->CanReceiveAudio())
         mValidTargets.push_back(module);
      if (mType == kConnectionType_Note && module->CanReceiveNotes())
         mValidTargets.push_back(module);
      if (mType == kConnectionType_Pulse && module->CanReceivePulses())
         mValidTargets.push_back(module);
      if (mType == kConnectionType_Special)
      {
         mValidTargets.push_back(module);
      }
   }
}

void PatchCableSource::CableGrabbed()
{
   FindValidTargets();
   
   mOwner->OnCableGrabbed(this);
}

void PatchCableSource::KeyPressed(int key, bool isRepeat)
{
   if (key == juce::KeyPress::backspaceKey || key == juce::KeyPress::deleteKey)
   {
      for (auto cable : mPatchCables)
      {
         if (cable != nullptr && cable == PatchCable::sActivePatchCable)
         {
            RemovePatchCable(cable, true);
            break;
         }
      }
   }
}

void PatchCableSource::RemovePatchCable(PatchCable* cable, bool fromUserAction)
{
   mOwner->PreRepatch(this);
   mAudioReceiver = nullptr;
   if (cable != nullptr)
   {
      RemoveFromVector(dynamic_cast<INoteReceiver*>(cable->GetTarget()), mNoteReceivers);
      RemoveFromVector(dynamic_cast<IPulseReceiver*>(cable->GetTarget()), mPulseReceivers);
   }
   RemoveFromVector(cable, mPatchCables);
   mOwner->PostRepatch(this, fromUserAction);
   delete cable;
}

void PatchCableSource::ClearPatchCables()
{
   while (mPatchCables.empty() == false)
      RemovePatchCable(mPatchCables[0]);
}

void PatchCableSource::SetTarget(IClickable* target)
{
   if (target != nullptr)
   {
      if (mPatchCables.empty())
         AddPatchCable(target);
      else if (mPatchCables[0] != nullptr && mPatchCables[0]->GetTarget() != target)
         SetPatchCableTarget(mPatchCables[0], target, false);
   }
   else
   {
      mAudioReceiver = nullptr;
      mNoteReceivers.clear();
      mPulseReceivers.clear();
   }
}

IClickable* PatchCableSource::GetTarget() const
{
   if (mPatchCables.empty() || mPatchCables[0] == nullptr)
      return nullptr;
   else
      return mPatchCables[0]->GetTarget();
}

void PatchCableSource::SaveState(FileStreamOut& out)
{
   out << (int)mPatchCables.size();
   
   for (int i=0; i<mPatchCables.size(); ++i)
   {
      std::string path = "";
      IClickable* target = mPatchCables[i]->GetTarget();
      if (target)
         path = target->Path();
      out << path;
   }
}

void PatchCableSource::LoadState(FileStreamIn& in)
{
   ClearPatchCables();
   
   int size;
   in >> size;
   mPatchCables.resize(size);
   
   for (int i=0; i<mPatchCables.size(); ++i)
   {
      std::string path;
      in >> path;
      IClickable* target = TheSynth->FindModule(path, false);
      if (target == nullptr)
      {
         try
         {
            target = TheSynth->FindUIControl(path);
         }
         catch (UnknownUIControlException& e)
         {
            
         }
      }
      mPatchCables[i] = new PatchCable(this);
      SetPatchCableTarget(mPatchCables[i], target, false);
   }
   
   mOwner->PostRepatch(this, false);
}

void NoteHistory::AddEvent(double time, bool on)
{
   if (on)
      mLastOnEventTime = time;
   
   mHistoryPos = (mHistoryPos + 1) % kHistorySize;
   mHistory[mHistoryPos].mTime = time;
   mHistory[mHistoryPos].mOn = on;
}

bool NoteHistory::CurrentlyOn()
{
   return mHistory[mHistoryPos].mOn;
}

const NoteHistoryEvent& NoteHistory::GetHistoryEvent(int ago) const
{
   assert(ago < kHistorySize);
   int index = (mHistoryPos + kHistorySize - ago) % kHistorySize;
   return mHistory[index];
}
