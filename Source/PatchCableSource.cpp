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

namespace
{
   const int patchCableSourceRadius = 5;
}

PatchCableSource::PatchCableSource(IDrawableModule* owner, ConnectionType type)
: mOwner(owner)
, mType(type)
, mHovered(false)
, mOverrideVizBuffer(NULL)
, mAutomaticPositioning(true)
, mAudioReceiver(NULL)
, mDefaultPatchBehavior(kDefaultPatchBehavior_Repatch)
, mPatchCableDrawMode(kPatchCableDrawMode_Normal)
, mEnabled(true)
{
   mAllowMultipleTargets = (mType == kConnectionType_Note);
   
   if (mType == kConnectionType_Note)
      mColor = IDrawableModule::GetColor(kModuleType_Note);
   else if (mType == kConnectionType_Audio)
      mColor = IDrawableModule::GetColor(kModuleType_Audio);
   else
      mColor = IDrawableModule::GetColor(kModuleType_Other);
   mColor.setBrightness(mColor.getBrightness() * .8f);
}

PatchCableSource::~PatchCableSource()
{
   for (auto cable : mPatchCables)
      delete cable;
}

PatchCable* PatchCableSource::AddPatchCable(IClickable* target)
{
   for (auto cable : mPatchCables)
   {
      if (cable->GetTarget() == target)
         return NULL;
   }
   
   PatchCable* cable = new PatchCable(this);
   mPatchCables.push_back(cable);
   
   if (target)
      SetPatchCableTarget(cable, target);
   
   return cable;
}

void PatchCableSource::SetPatchCableTarget(PatchCable* cable, IClickable* target)
{
   IClickable* oldTarget = cable->GetTarget();
   
   mOwner->PreRepatch(this);
   
   if (cable->GetTarget())
   {
      mAudioReceiver = NULL;
      RemoveFromVector(dynamic_cast<INoteReceiver*>(cable->GetTarget()), mNoteReceivers);
   }
   
   cable->SetTarget(target);
   
   INoteReceiver* noteReceiver = dynamic_cast<INoteReceiver*>(target);
   if (noteReceiver)
      mNoteReceivers.push_back(noteReceiver);
   IAudioReceiver* audioReceiver = dynamic_cast<IAudioReceiver*>(target);
   if (audioReceiver)
   {
      mAudioReceiver = audioReceiver;
      TheSynth->ArrangeAudioSourceDependencies();
   }
   
   mOwner->PostRepatch(this);
   
   //insert
   if (ofGetKeyPressed('i'))
   {
      static bool allowInsert = true;
      if (allowInsert)  //avoid cascade on the next set
      {
         allowInsert = false;
         IDrawableModule* targetModule = dynamic_cast<IDrawableModule*>(target);
         if (targetModule)
            targetModule->SetTarget(oldTarget);
         allowInsert = true;
      }
   }
}

void PatchCableSource::Clear()
{
   for (auto cable : mPatchCables)
      delete cable;
   mPatchCables.clear();
}

void PatchCableSource::UpdatePosition()
{
   if (mAutomaticPositioning)
   {
      int x,y,w,h;
      mOwner->GetPosition(x, y);
      mOwner->GetDimensions(w, h);
      
      enum Side
      {
         kBottom,
         kLeft,
         kRight
      };
      Side mSide = kBottom;
      
      ofVec2f centerOfMass;
      int count = 0;
      for (auto cable : mPatchCables)
      {
         if (cable->GetTarget())
         {
            int targetX,targetY,targetW,targetH;
            cable->GetTarget()->GetPosition(targetX, targetY);
            cable->GetTarget()->GetDimensions(targetW, targetH);
            centerOfMass.x += targetX + targetW / 2;
            centerOfMass.y += targetY + targetH / 2;
            ++count;
         }
      }
      centerOfMass.x /= count;
      centerOfMass.y /= count;
      
      if (count > 0)
      {
         if (centerOfMass.y > y+h || (centerOfMass.x > x && centerOfMass.x < x+w))
            mSide = kBottom;
         else if (centerOfMass.x < x+w/2)
            mSide = kLeft;
         else
            mSide = kRight;
      }
      
      if (mSide == kBottom)
      {
         mX = x+w/2;
         mY = y+h+3;
      }
      else if (mSide == kLeft)
      {
         mX = x-3;
         mY = y+h/2;
      }
      else if (mSide == kRight)
      {
         mX = x+w+3;
         mY = y+h/2;
      }
   }
   else
   {
      int x,y;
      mOwner->GetPosition(x, y);
      mX = mManualPositionX + x;
      mY = mManualPositionY + y;
   }
}

void PatchCableSource::Render()
{
   if (!Enabled())
      return;
   
   if (mPatchCableDrawMode == kPatchCableDrawMode_Normal ||
       (mPatchCableDrawMode == kPatchCableDrawMode_HoverOnly && mHovered))
   for (auto cable : mPatchCables)
      cable->Draw();
   
   ofPushMatrix();
   ofPushStyle();
   
   ofPushStyle();
   ofSetLineWidth(0);
   ofSetColor(mColor);
   ofFill();
   ofCircle(mX,mY,patchCableSourceRadius);
   if (mHovered)
   {
      ofSetColor(ofColor::white);
      ofCircle(mX,mY,patchCableSourceRadius-2);
   }
   ofPopStyle();
   
   if (mHovered && InAddCableMode())
   {
      ofPushStyle();
      ofSetColor(0,0,0);
      ofSetLineWidth(3);
      ofLine(mX,mY-(patchCableSourceRadius-1),mX,mY+(patchCableSourceRadius-1));
      ofLine(mX-(patchCableSourceRadius-1),mY,mX+(patchCableSourceRadius-1),mY);
      ofPopStyle();
   }
   
   ofPopStyle();
   ofPopMatrix();
}

bool PatchCableSource::MouseMoved(float x, float y)
{
   if (!Enabled())
      return false;
   
   x = TheSynth->GetMouseX();
   y = TheSynth->GetMouseY();
   
   mHovered = ofDistSquared(x, y, mX, mY) < patchCableSourceRadius * patchCableSourceRadius;
   
   for (auto cable : mPatchCables)
      cable->NotifyMouseMoved(x, y);
   
   return false;
}

void PatchCableSource::MouseReleased()
{
   for (auto cable : mPatchCables)
      cable->MouseReleased();
}

bool PatchCableSource::TestClick(int x, int y, bool right, bool testOnly /* = false */)
{
   if (!Enabled())
      return false;
   
   if (mHovered)
   {
      if (!testOnly)
      {
         if (mPatchCables.empty() || InAddCableMode())
         {
            PatchCable* newCable = AddPatchCable(NULL);
            newCable->Grab();
         }
         else
         {
            mPatchCables[mPatchCables.size()-1]->Grab();
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
   return VectorContains(target, mValidTargets);
}

void PatchCableSource::FindValidTargets()
{
   mValidTargets.clear();
   vector<IDrawableModule*> allModules;
   TheSynth->GetAllModules(allModules);
   for (auto module : allModules)
   {
      if (module == mOwner)
         continue;
      if (module == mOwner->GetParent())
         continue;
      if (mTypeFilter.empty() == false && !VectorContains(module->GetTypeName(), mTypeFilter))
         continue;
      if (mType == kConnectionType_Audio && dynamic_cast<IAudioReceiver*>(module))
         mValidTargets.push_back(module);
      if (mType == kConnectionType_Note && dynamic_cast<INoteReceiver*>(module))
         mValidTargets.push_back(module);
      if (mType == kConnectionType_Grid && dynamic_cast<IGridControllerListener*>(module))
         mValidTargets.push_back(module);
      if (mType == kConnectionType_UIControl && module != TheTitleBar)
      {
         for (auto uicontrol : module->GetUIControls())
         {
            if (uicontrol->IsShowing() && dynamic_cast<ADSRDisplay*>(uicontrol) == NULL)
               mValidTargets.push_back(uicontrol);
         }
      }
      if (mType == kConnectionType_Special)
      {
         mValidTargets.push_back(module);
      }
   }
}

void PatchCableSource::CableGrabbed()
{
   FindValidTargets();
}

void PatchCableSource::KeyPressed(int key, bool isRepeat)
{
   if (key == OF_KEY_BACKSPACE)
   {
      for (auto cable : mPatchCables)
      {
         if (cable == PatchCable::sActivePatchCable)
         {
            RemovePatchCable(cable);
            break;
         }
      }
   }
}

void PatchCableSource::RemovePatchCable(PatchCable* cable)
{
   mOwner->PreRepatch(this);
   mAudioReceiver = NULL;
   RemoveFromVector(dynamic_cast<INoteReceiver*>(cable->GetTarget()), mNoteReceivers);
   RemoveFromVector(cable, mPatchCables);
   mOwner->PostRepatch(this);
   delete cable;
}

void PatchCableSource::ClearPatchCables()
{
   while (mPatchCables.empty() == false)
      RemovePatchCable(mPatchCables[0]);
}

void PatchCableSource::SetTarget(IClickable* target)
{
   if (target != NULL)
   {
      if (mPatchCables.empty())
         AddPatchCable(target);
      else
         SetPatchCableTarget(mPatchCables[0], target);
   }
   else
   {
      mAudioReceiver = NULL;
      mNoteReceivers.clear();
   }
}

IClickable* PatchCableSource::GetTarget() const
{
   if (mPatchCables.empty())
      return NULL;
   else
      return mPatchCables[0]->GetTarget();
}

void PatchCableSource::SaveState(FileStreamOut& out)
{
   out << (int)mPatchCables.size();
   
   for (int i=0; i<mPatchCables.size(); ++i)
   {
      string path = "";
      IClickable* target = mPatchCables[i]->GetTarget();
      if (target)
         path = target->Path();
      out << path;
   }
}

void PatchCableSource::LoadState(FileStreamIn& in)
{
   assert(mPatchCables.size() == 0);
   
   int size;
   in >> size;
   mPatchCables.resize(size);
   
   for (int i=0; i<mPatchCables.size(); ++i)
   {
      string path;
      in >> path;
      IClickable* target = TheSynth->FindModule(path, false);
      if (target == NULL)
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
      mPatchCables[i]->SetTarget(target);
   }
}
