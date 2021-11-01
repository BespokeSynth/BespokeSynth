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
//  PatchCable.cpp
//  Bespoke
//
//  Created by Ryan Challinor on 12/12/15.
//
//

#include "PatchCable.h"
#include "IDrawableModule.h"
#include "INoteSource.h"
#include "IAudioReceiver.h"
#include "IAudioSource.h"
#include "GridController.h"
#include "ModularSynth.h"
#include "SynthGlobals.h"
#include "PatchCableSource.h"
#include "MathUtils.h"
#include "IPulseReceiver.h"
#include "RadioButton.h"
#include "MidiController.h"
#include "IModulator.h"

PatchCable* PatchCable::sActivePatchCable = nullptr;

PatchCable::PatchCable(PatchCableSource* owner)
: mHovered(false)
, mDragging(false)
, mHoveringOnSource(false)
, mSourceIndex(0)
, mTarget(nullptr)
, mTargetRadioButton(nullptr)
, mUIControlConnection(nullptr)
, mAudioReceiverTarget(nullptr)
{
   mOwner = owner;
   TheSynth->RegisterPatchCable(this);
}

PatchCable::~PatchCable()
{
   if (sActivePatchCable == this)
      sActivePatchCable = nullptr;
   TheSynth->UnregisterPatchCable(this);
}

void PatchCable::SetTarget(IClickable* target)
{
   mTarget = target;
   mTargetRadioButton = dynamic_cast<RadioButton*>(target);
   mAudioReceiverTarget = dynamic_cast<IAudioReceiver*>(target);
}
   
void PatchCable::Render()
{
   PatchCablePos cable = GetPatchCablePos();
   mX = cable.start.x;
   mY = cable.start.y;
   
   float lineWidth = 1;
   float plugWidth = 4;
   float lineAlpha = .4f;
   
   bool fatCable = false;
   
   if (TheSynth->GetLastClickedModule() == GetOwningModule() ||
       TheSynth->GetLastClickedModule() == mTarget)
      fatCable = true;
   
   if (TheSynth->ShouldAccentuateActiveModules())  //only draw if we're making noise
   {
      if (GetConnectionType() == kConnectionType_Note ||
          GetConnectionType() == kConnectionType_Grid ||
          GetConnectionType() == kConnectionType_Pulse ||
          GetConnectionType() == kConnectionType_Modulator ||
          GetConnectionType() == kConnectionType_UIControl)
      {
         bool hasNote = false;
         const NoteHistoryEvent& event = mOwner->GetHistory().GetHistoryEvent(0);
         float elapsed = float(gTime - event.mTime) / NOTE_HISTORY_LENGTH;
         if (event.mOn || elapsed <= 1)
            hasNote = true;
         
         if (!hasNote)
            return;
      }
      else if (GetConnectionType() == kConnectionType_Audio)
      {
         IAudioSource* audioSource = dynamic_cast<IAudioSource*>(GetOwningModule());
         if (audioSource)
         {
            RollingBuffer* vizBuff = mOwner->GetOverrideVizBuffer();
            if (vizBuff == nullptr)
               vizBuff = audioSource->GetVizBuffer();
            assert(vizBuff);
            int numSamples = vizBuff->Size();
            bool allZero = true;
            for (int ch=0; ch<vizBuff->NumChannels(); ++ch)
            {
               for (int i=0; i<numSamples && allZero; ++i)
               {
                  if (vizBuff->GetSample(i, ch) != 0)
                     allZero = false;
               }
            }
            
            if (allZero)
               return;
         }
         else
         {
            return;
         }
      }
      else
      {
         return;
      }
      
      fatCable = true;
   }

   
   if (fatCable)
   {
      lineWidth = 1;
      plugWidth = 5;
      lineAlpha = 1;
   }
   
   if (mHovered || mDragging)
      plugWidth = 6;
   
   ofPushMatrix();
   ofPushStyle();
   ofNoFill();

   IClickable* dropTarget = GetDropTarget();
   if (dropTarget)
   {
      ofPushStyle();

      ofSetColor(255, 255, 255, 100);
      ofSetLineWidth(.5f);
      ofFill();
      ofRectangle rect = dropTarget->GetRect();

      IDrawableModule* dropTargetModule = dynamic_cast<IDrawableModule*>(dropTarget);
      if (dropTargetModule && dropTargetModule->HasTitleBar())
      {
         rect.y -= IDrawableModule::TitleBarHeight();
         rect.height += IDrawableModule::TitleBarHeight();
      }

      ofRect(rect);

      ofPopStyle();
   }
   
   ConnectionType type = mOwner->GetConnectionType();
   ofColor lineColor = mOwner->GetColor();
   if (mHoveringOnSource && sActivePatchCable == nullptr && !TheSynth->IsGroupSelecting())
      lineColor = ofColor::lerp(lineColor, ofColor::white, .5f);
   ofColor lineColorAlphaed = lineColor;
   lineColorAlphaed.a *= lineAlpha;
   
   float wThis,hThis,xThis,yThis;
   GetDimensions(wThis,hThis);
   GetPosition(xThis,yThis);
   
   bool isInsideSelf = cable.end.x >= xThis && cable.end.x <= (xThis + wThis) && cable.end.y >= yThis && cable.end.y <= (yThis + hThis);
   if (!isInsideSelf)
   {
      IAudioSource* audioSource = nullptr;
      ofVec2f wireLineMag = cable.plug - cable.start;
      wireLineMag.x = MAX(50,fabsf(wireLineMag.x));
      wireLineMag.y = MAX(50,fabsf(wireLineMag.y));
      ofVec2f endDirection = MathUtils::Normal(cable.plug - cable.end);
      ofVec2f bezierControl1 = cable.start + MathUtils::ScaleVec(cable.startDirection, wireLineMag * .5f);
      ofVec2f bezierControl2 = cable.plug + MathUtils::ScaleVec(endDirection, wireLineMag * .5f);
      float wireLength = sqrtf((cable.plug - cable.start).lengthSquared());
      
      if (type == kConnectionType_Note ||
          type == kConnectionType_Grid ||
          type == kConnectionType_Pulse ||
          type == kConnectionType_Modulator ||
          type == kConnectionType_UIControl)
      {
         ofSetLineWidth(lineWidth);
         ofSetColor(lineColorAlphaed);
         ofBeginShape();
         ofVertex(cable.start.x,cable.start.y);
         for (int i=1; i<wireLength-1; ++i)
         {
            ofVec2f pos = MathUtils::Bezier(i/wireLength, cable.start, bezierControl1, bezierControl2, cable.plug);
            ofVertex(pos.x,pos.y);
         }
         ofVertex(cable.plug.x,cable.plug.y);
         ofEndShape();
         
         IModulator* modulator = mOwner->GetModulatorOwner();
         if (modulator != nullptr)
         {
            float range = abs(modulator->GetMax() - modulator->GetMin());
            if (range > .00001f)
            {
               float delta = ofClamp(modulator->GetRecentChange() / range, -1, 1);
               ofColor color = ofColor::lerp(ofColor::blue, ofColor::red, delta * .5f + .5f);
               color.a = abs(1 - ((1-delta)*(1-delta))) * 150;
               ofSetColor(color);
               ofSetLineWidth(3);
               
               ofBeginShape();
               ofVertex(cable.start.x,cable.start.y);
               for (int i=1; i<wireLength-1; ++i)
               {
                  ofVec2f pos = MathUtils::Bezier(i/wireLength, cable.start, bezierControl1, bezierControl2, cable.plug);
                  ofVertex(pos.x,pos.y);
               }
               ofVertex(cable.plug.x,cable.plug.y);
               ofEndShape();
               
               //change plug color
               if (delta > 0)
                  lineColor = ofColor::lerp(lineColor, ofColor::red, delta);
               else
                  lineColor = ofColor::lerp(lineColor, ofColor::blue, -delta);
            }
         }
         else
         {
            ofSetColor(lineColor);
            
            float lastElapsed = 0;
            for (int i=0; i<NoteHistory::kHistorySize; ++i)
            {
               const NoteHistoryEvent& event = mOwner->GetHistory().GetHistoryEvent(i);
               float elapsed = float(gTime - event.mTime) / NOTE_HISTORY_LENGTH;
               float clampedElapsed = MIN(elapsed, 1);
               if (event.mOn)
               {
                  ofSetLineWidth(lineWidth * (4 + ofClamp(1 - elapsed * .7f, 0, 1) * 5 + cos((gTime - event.mTime) * PI * 8 / TheTransport->MsPerBar()) * .3f));
                  ofBeginShape();
                  ofVec2f pos;
                  for (int j=lastElapsed*wireLength; j< clampedElapsed*wireLength; ++j)
                  {
                     pos = MathUtils::Bezier(ofClamp(j/wireLength, 0, 1), cable.start, bezierControl1, bezierControl2, cable.plug);
                     ofVertex(pos.x,pos.y);
                  }
                  ofEndShape();
                  
                  /*if (clampedElapsed < 1)
                  {
                     ofPushStyle();
                     ofFill();
                     ofSetLineWidth(1);
                     ofCircle(pos.x, pos.y, 4);
                     ofPopStyle();
                  }*/
               }
               lastElapsed = clampedElapsed;
               
               if (clampedElapsed >= 1)
                  break;
            }
         }
         
         ofSetLineWidth(plugWidth);
         ofSetColor(lineColor);
         ofLine(cable.plug.x,cable.plug.y,cable.end.x,cable.end.y);
      }
      else if (type == kConnectionType_Audio &&
               (audioSource = dynamic_cast<IAudioSource*>(mOwner->GetOwner())) != nullptr)
      {
         ofSetLineWidth(lineWidth);
         
         RollingBuffer* vizBuff = mOwner->GetOverrideVizBuffer();
         if (vizBuff == nullptr)
            vizBuff = audioSource->GetVizBuffer();
         assert(vizBuff);
         int numSamples = vizBuff->Size();
         float dx = (cable.plug.x - cable.start.x) / wireLength;
         float dy = (cable.plug.y - cable.start.y) / wireLength;
         
         for (int ch=0; ch<vizBuff->NumChannels(); ++ch)
         {
            ofSetColor(lineColorAlphaed);
            if (ch != 0)
               ofSetColor(lineColorAlphaed.g, lineColorAlphaed.r, lineColorAlphaed.b, lineColorAlphaed.a);
            ofVec2f offset((ch - (vizBuff->NumChannels()-1)*.5f) * 2 * dy, (ch - (vizBuff->NumChannels()-1) * .5f) * 2 * -dx);
            ofBeginShape();
            ofVertex(cable.start.x + offset.x,cable.start.y + offset.y);
            for (int i=1; i<wireLength-1; ++i)
            {
               ofVec2f pos = MathUtils::Bezier(i/wireLength, cable.start, bezierControl1, bezierControl2, cable.plug);
               float sample = vizBuff->GetSample((i/wireLength * numSamples), ch);
               sample = sqrtf(fabsf(sample)) * (sample < 0 ? -1 : 1);
               sample = ofClamp(sample, -1.0f, 1.0f);
               ofVec2f sampleOffsetDir = MathUtils::BezierPerpendicular(i/wireLength, cable.start, bezierControl1, bezierControl2, cable.plug);
               pos += sampleOffsetDir * 10 * sample;
               ofVertex(pos.x + offset.x,pos.y + offset.y);
            }
            ofVertex(cable.plug.x + offset.x,cable.plug.y + offset.y);
            ofEndShape();
         }
         
         ofSetLineWidth(plugWidth);
         ofSetColor(lineColor);
         ofLine(cable.plug.x,cable.plug.y,cable.end.x,cable.end.y);
         
         bool warn = false;
         
         if (!TheSynth->IsAudioPaused())
         {
            if (vizBuff->NumChannels() > 1 && mAudioReceiverTarget && mAudioReceiverTarget->GetInputMode() == IAudioReceiver::kInputMode_Mono)
            {
               warn = true; //warn that the multichannel audio is being crunched to mono
               if (mHovered)
                  TheSynth->SetNextDrawTooltip("warning: multichannel audio is being squashed to mono");
            }
            
            if (vizBuff->NumChannels() == 1 && mAudioReceiverTarget && mAudioReceiverTarget->GetBuffer()->RecentNumActiveChannels() > 1)
            {
               warn = true; //warn that the target expects multichannel audio but we're not filling all of the channels
               if (mHovered)
                  TheSynth->SetNextDrawTooltip("warning: target expects multichannel audio, but is only getting mono");
            }
         }
         
         if (warn)
         {
            ofFill();
            ofSetColor(255, 255, 0);
            ofCircle(cable.plug.x, cable.plug.y, 6);
            ofSetColor(0, 0, 0);
            DrawTextBold("!", cable.plug.x-2, cable.plug.y+5,17);
         }
      }
      else
      {
         ofSetLineWidth(lineWidth);
         ofSetColor(lineColorAlphaed);
         ofBeginShape();
         ofVertex(cable.start.x,cable.start.y);
         for (int i=1; i<wireLength-1; ++i)
         {
            ofVec2f pos = MathUtils::Bezier(i/wireLength, cable.start, bezierControl1, bezierControl2, cable.plug);
            ofVertex(pos.x,pos.y);
         }
         ofVertex(cable.plug.x,cable.plug.y);
         ofEndShape();
         
         ofSetLineWidth(plugWidth);
         ofSetColor(lineColor);
         ofLine(cable.plug.x,cable.plug.y,cable.end.x,cable.end.y);
      }
   }

   ofPopStyle();
   ofPopMatrix();
}

bool PatchCable::MouseMoved(float x, float y)
{
   if (GetConnectionType() == kConnectionType_Modulator || GetConnectionType() == kConnectionType_UIControl) //no repatching UI control cables by the plug
      return false;
   
   x = TheSynth->GetMouseX(GetOwningModule()->GetOwningContainer());
   y = TheSynth->GetMouseY(GetOwningModule()->GetOwningContainer());
   
   PatchCablePos cable = GetPatchCablePos();
   mHovered = DistSqToLine(ofVec2f(x,y),cable.plug,cable.end) < 25;
   
   return false;
}

void PatchCable::MouseReleased()
{
   if (mDragging)
   {
      ofVec2f mousePos(TheSynth->GetRawMouseX(), TheSynth->GetRawMouseY());
      if ((mousePos - mGrabPos).distanceSquared() > 3)
      {
         IClickable* target = GetDropTarget();
         if (target)
            mOwner->SetPatchCableTarget(this, target, true);
         
         mDragging = false;
         mHovered = false;
         if (sActivePatchCable == this)
            sActivePatchCable = nullptr;
         
         if (mTarget == nullptr)
            Destroy(true);
      }
   }
}

IClickable* PatchCable::GetDropTarget()
{
   if (mDragging)
   {
      PatchCablePos cable = GetPatchCablePos();
      IClickable* potentialTarget = TheSynth->GetRootContainer()->GetModuleAt(cable.end.x, cable.end.y);
      if (potentialTarget && (GetConnectionType() == kConnectionType_Modulator || GetConnectionType() == kConnectionType_Grid || GetConnectionType() == kConnectionType_UIControl))
      {
         const auto& uicontrols = (static_cast<IDrawableModule*>(potentialTarget))->GetUIControls();
         for (auto uicontrol : uicontrols)
         {
            if (uicontrol->IsShowing() == false || !IsValidTarget(uicontrol))
               continue;

            float x, y, w, h;
            uicontrol->GetPosition(x, y);
            uicontrol->GetDimensions(w, h);
            if (cable.end.x >= x && cable.end.y >= y && cable.end.x < x + w && cable.end.y < y + h)
            {
               potentialTarget = uicontrol;
               break;
            }
         }
      }
      if (mOwner->IsValidTarget(potentialTarget))
         return potentialTarget;
   }

   return nullptr;
}

bool PatchCable::TestClick(int x, int y, bool right, bool testOnly /* = false */)
{
   if (right)
      return false;

   if (mHovered)
   {
      if (!testOnly)
         Grab();
      return true;
   }
   return false;
}

void PatchCable::OnClicked(int x, int y, bool right)
{
   
}

PatchCablePos PatchCable::GetPatchCablePos()
{
   ofVec2f start = mOwner->GetCableStart(mSourceIndex);
   
   float wThat,hThat,xThat,yThat;
   
   int yThatAdjust = 0;
   IDrawableModule* targetModule = dynamic_cast<IDrawableModule*>(mTarget);
   IClickable* target = mTarget;

   if (targetModule != nullptr && targetModule->IsDeleted())
   {
      targetModule = nullptr;
      target = nullptr;
   }

   if (targetModule && targetModule->HasTitleBar() && !mDragging)
      yThatAdjust = IDrawableModule::TitleBarHeight();
   
   if (mDragging)
   {
      int mouseX = TheSynth->GetMouseX(GetOwningModule()->GetOwningContainer());
      int mouseY = TheSynth->GetMouseY(GetOwningModule()->GetOwningContainer());
      wThat = 0;
      hThat = 0;
      xThat = mouseX;
      yThat = mouseY;
   }
   else if (target)
   {
      target->GetDimensions(wThat,hThat);
      target->GetPosition(xThat,yThat);
      
      IDrawableModule* targetModuleParent = dynamic_cast<IDrawableModule*>(target->GetParent());
      ModuleContainer* targetModuleParentContainer = targetModuleParent ? targetModuleParent->GetOwningContainer() : nullptr;
      IDrawableModule* targetModuleParentContainerModule = targetModuleParentContainer ? targetModuleParentContainer->GetOwner() : nullptr;
      if (targetModuleParentContainerModule && targetModuleParentContainerModule->Minimized())
         targetModuleParent = targetModuleParentContainerModule;
      if (targetModuleParent && (targetModuleParent->Minimized() || target->IsShowing() == false))
      {
         targetModuleParent->GetPosition(xThat, yThat);
         targetModuleParent->GetDimensions(wThat, hThat);
         if (targetModuleParent->HasTitleBar() && !mDragging)
            yThatAdjust = IDrawableModule::TitleBarHeight();
      }
   }
   else
   {
      wThat = 0;
      hThat = 0;
      xThat = start.x;
      yThat = start.y;
   }
   
   ofVec2f startDirection = mOwner->GetCableStartDir(mSourceIndex, ofVec2f(xThat, yThat));   
   ofVec2f endDirection;
   ofVec2f end = FindClosestSide(xThat,yThat-yThatAdjust,wThat,hThat+yThatAdjust, start, startDirection, endDirection);
   
   //update direction to match found side
   startDirection = mOwner->GetCableStartDir(mSourceIndex, end);
   
   if (mTargetRadioButton && mUIControlConnection && !mDragging)
   {
      target->GetDimensions(wThat,hThat);
      target->GetPosition(xThat,yThat);
      end = mTargetRadioButton->GetOptionPosition(mUIControlConnection->mValue);
   }
   
   float plugDirDistanceToStart = endDirection.dot(start-end);
   float plugLength = ofClamp(plugDirDistanceToStart - 20, 5, 14);
   ofVec2f plug = end + endDirection * plugLength;
   
   PatchCablePos cable;
   cable.start = start + startDirection * 4;
   cable.startDirection = startDirection;
   cable.end = end;
   cable.plug = plug;
   
   return cable;
}

ofVec2f PatchCable::FindClosestSide(int x, int y, int w, int h, ofVec2f start, ofVec2f startDirection, ofVec2f& endDirection)
{
   ofVec2f dirs[4];
   dirs[0].set(-1,0);
   dirs[1].set(1,0);
   dirs[2].set(0,-1);
   dirs[3].set(0,1);
   
   ofVec2f sides[4];
   sides[0].set(x,y+h/2);  //left
   sides[1].set(x+w,y+h/2);  //right
   sides[2].set(x+w/2,y);  //top
   sides[3].set(x+w/2,y+h);  //bottom
   
   float best = -FLT_MAX;
   int bestIndex = 0;
   for (int i=0; i<4; ++i)
   {
      if (i == 3) //skip bottom
         continue;
      
      float score = MathUtils::Normal(start - sides[i]).dot(dirs[i]);
      if (score > best)
      {
         best = score;
         bestIndex = i;
      }
   }
   
   if (w == 0 && h == 0)
      endDirection = MathUtils::Normal(start - ofVec2f(x,y));
   else
      endDirection = dirs[bestIndex];
   return sides[bestIndex];
}

void PatchCable::Grab()
{
   if (!mDragging)
   {
      mDragging = true;
      sActivePatchCable = this;
      gHoveredUIControl = nullptr;
      mGrabPos.set(TheSynth->GetRawMouseX(), TheSynth->GetRawMouseY());
      mOwner->CableGrabbed();
   }
}

IDrawableModule* PatchCable::GetOwningModule() const
{
   return mOwner->GetOwner();
}

bool PatchCable::IsValidTarget(IClickable* target) const
{
   return mOwner->IsValidTarget(target);
}

void PatchCable::Destroy(bool fromUserClick)
{
   mOwner->RemovePatchCable(this, fromUserClick);
}

ConnectionType PatchCable::GetConnectionType() const
{
   return mOwner->GetConnectionType();
}
