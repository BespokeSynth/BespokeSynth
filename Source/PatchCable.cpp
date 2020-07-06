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

PatchCable* PatchCable::sActivePatchCable = nullptr;

PatchCable::PatchCable(PatchCableSource* owner)
: mHovered(false)
, mDragging(false)
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
   int lineAlpha = 100;
   
   bool fatCable = false;
   
   if (TheSynth->GetLastClickedModule() == GetOwningModule() ||
       TheSynth->GetLastClickedModule() == mTarget)
      fatCable = true;
   
   if (TheSynth->ShouldAccentuateActiveModules())  //only draw if we're making noise
   {
      if (GetConnectionType() == kConnectionType_Note ||
          GetConnectionType() == kConnectionType_Grid ||
          GetConnectionType() == kConnectionType_Pulse ||
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
      lineAlpha = 255;
   }
   
   if (mHovered || mDragging)
      plugWidth = 6;
   
   ofPushMatrix();
   ofPushStyle();
   
   ConnectionType type = mOwner->GetConnectionType();
   ofColor lineColor = mOwner->GetColor();
   ofColor lineColorAlphaed = lineColor;
   lineColorAlphaed.a = lineAlpha;
   
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
         
         ofSetColor(lineColor);
         
         float lastElapsed = 0;
         for (int i=0; i<NoteHistory::kHistorySize; ++i)
         {
            const NoteHistoryEvent& event = mOwner->GetHistory().GetHistoryEvent(i);
            float elapsed = float(gTime - event.mTime) / NOTE_HISTORY_LENGTH;
            ofSetLineWidth(lineWidth * (4 + ofClamp(1 - elapsed * .7f, 0, 1) * 5 + cos((gTime - event.mTime) * PI * 8 / TheTransport->MsPerBar()) * .3f));
            if (elapsed > 1)
               elapsed = 1;
            if (event.mOn)
            {
               ofBeginShape();
               ofVec2f pos;
               for (int j=lastElapsed*wireLength; j<elapsed*wireLength; ++j)
               {
                  pos = MathUtils::Bezier(ofClamp(j/wireLength, 0, 1), cable.start, bezierControl1, bezierControl2, cable.plug);
                  ofVertex(pos.x,pos.y);
               }
               ofEndShape();
               
               /*if (elapsed < 1)
               {
                  ofPushStyle();
                  ofFill();
                  ofSetLineWidth(1);
                  ofCircle(pos.x, pos.y, 4);
                  ofPopStyle();
               }*/
            }
            lastElapsed = elapsed;
            
            if (elapsed >= 1)
               break;
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
               pos += sampleOffsetDir * 15 * sample;
               ofVertex(pos.x + offset.x,pos.y + offset.y);
            }
            ofVertex(cable.plug.x + offset.x,cable.plug.y + offset.y);
            ofEndShape();
         }
         
         ofSetLineWidth(plugWidth);
         ofSetColor(lineColor);
         ofLine(cable.plug.x,cable.plug.y,cable.end.x,cable.end.y);
         
         bool warn = false;
         
         if (vizBuff->NumChannels() > 1 && mAudioReceiverTarget && mAudioReceiverTarget->GetInputMode() == IAudioReceiver::kInputMode_Mono)
            warn = true; //warn that the multichannel audio is being crunched to mono
         
         if (vizBuff->NumChannels() == 1 && mAudioReceiverTarget && mAudioReceiverTarget->GetBuffer()->RecentNumActiveChannels() > 1)
            warn = true; //warn that the target expects multichannel audio but we're not filling all of the channels
         
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
   if (GetConnectionType() == kConnectionType_UIControl) //no repatching UI control cables by the plug
      return false;
   
   x = TheSynth->GetMouseX();
   y = TheSynth->GetMouseY();
   
   PatchCablePos cable = GetPatchCablePos();
   mHovered = DistSqToLine(ofVec2f(x,y),cable.plug,cable.end) < 25;
   
   return false;
}

void PatchCable::MouseReleased()
{
   if (mDragging)
   {
      ofVec2f mousePos(ofGetMouseX(), ofGetMouseY());
      if ((mousePos - mGrabPos).distanceSquared() > 3)
      {
         PatchCablePos cable = GetPatchCablePos();
         IClickable* potentialTarget = TheSynth->GetModuleAt(cable.end.x, cable.end.y);
         if (potentialTarget && (GetConnectionType() == kConnectionType_UIControl || GetConnectionType() == kConnectionType_Grid))
         {
            const auto& uicontrols = ((IDrawableModule*)potentialTarget)->GetUIControls();
            for (auto uicontrol : uicontrols)
            {
               if (uicontrol->IsShowing() == false || !IsValidTarget(uicontrol))
                  continue;
               
               float x,y,w,h;
               uicontrol->GetPosition(x, y);
               uicontrol->GetDimensions(w, h);
               if (cable.end.x >= x && cable.end.y >= y && cable.end.x < x+w && cable.end.y < y+h)
               {
                  potentialTarget = uicontrol;
                  break;
               }
            }
         }
         if (mOwner->IsValidTarget(potentialTarget))
            mOwner->SetPatchCableTarget(this, potentialTarget, true);
         
         mDragging = false;
         mHovered = false;
         if (sActivePatchCable == this)
            sActivePatchCable = nullptr;
         
         if (mTarget == nullptr)
            Destroy();
      }
   }
}

bool PatchCable::TestClick(int x, int y, bool right, bool testOnly /* = false */)
{
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
   ofVec2f start = mOwner->GetPosition();
   
   float wThat,hThat,xThat,yThat;
   
   int yThatAdjust = 0;
   IDrawableModule* targetModule = dynamic_cast<IDrawableModule*>(mTarget);
   if (targetModule && targetModule->HasTitleBar() && !mDragging)
      yThatAdjust = IDrawableModule::TitleBarHeight();
   
   if (mDragging)
   {
      int mouseX = TheSynth->GetMouseX();
      int mouseY = TheSynth->GetMouseY();
      wThat = 0;
      hThat = 0;
      xThat = mouseX;
      yThat = mouseY;
   }
   else if (mTarget)
   {
      mTarget->GetDimensions(wThat,hThat);
      mTarget->GetPosition(xThat,yThat);
      
      IDrawableModule* targetModuleParent = mTarget->GetModuleParent();
      if (targetModuleParent && (targetModuleParent->Minimized() || mTarget->IsShowing() == false))
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
   
   ofVec2f startDirection;
   if (mOwner->HasOverrideCableDir())
   {
      startDirection = mOwner->GetOverrideCableDir();
   }
   else
   {
      switch (mOwner->GetCableSide())
      {
         case PatchCableSource::kBottom:
            startDirection = ofVec2f(0,1);
            break;
         case PatchCableSource::kRight:
            startDirection = ofVec2f(1,0);
            break;
         case PatchCableSource::kLeft:
            startDirection = ofVec2f(-1,0);
            break;
         default:
            startDirection = ofVec2f(0,0);
            break;
      }
   }
   
   ofVec2f endDirection;
   ofVec2f end = FindClosestSide(xThat,yThat-yThatAdjust,wThat,hThat+yThatAdjust, start, startDirection, endDirection);
   
   if (mTargetRadioButton && mUIControlConnection && !mDragging)
   {
      mTarget->GetDimensions(wThat,hThat);
      mTarget->GetPosition(xThat,yThat);
      end = mTargetRadioButton->GetOptionPosition(mUIControlConnection->mValue);
   }
   
   float plugDirDistanceToStart = endDirection.dot(start-end);
   float plugLength = ofClamp(plugDirDistanceToStart - 20, 5, 14);
   ofVec2f plug = end + endDirection * plugLength;
   
   PatchCablePos cable;
   cable.start = start;
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
      mGrabPos.set(ofGetMouseX(), ofGetMouseY());
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

void PatchCable::Destroy()
{
   mOwner->RemovePatchCable(this);
}

ConnectionType PatchCable::GetConnectionType() const
{
   return mOwner->GetConnectionType();
}
