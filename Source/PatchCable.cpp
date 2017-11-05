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

PatchCable* PatchCable::sActivePatchCable = nullptr;

PatchCable::PatchCable(PatchCableSource* owner)
: mHovered(false)
, mDragging(false)
, mTarget(nullptr)
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
   mAudioReceiverTarget = dynamic_cast<IAudioReceiver*>(target);
}

namespace
{
   inline float Cube(float x)
   {
      return x*x*x;
   }

   inline float Square(float x)
   {
      return x*x;
   }

   float Bezier(float t, float p0, float p1, float p2, float p3)
   {
      return Cube(1-t)*p0 + 3*Square(1-t)*t*p1 + 3 * (1-t) * Square(t) * p2 + Cube(t) * p3;
   }
   
   ofVec2f Bezier(float t, ofVec2f p0, ofVec2f p1, ofVec2f p2, ofVec2f p3)
   {
      return ofVec2f(Bezier(t, p0.x, p1.x, p2.x, p3.x), Bezier(t, p0.y, p1.y, p2.y, p3.y));
      /*if (t < .333f)
         return ofVec2f(ofLerp(p0.x,p1.x, t*3), ofLerp(p0.y,p1.y, t*3));
      else if (t < .666f)
         return ofVec2f(ofLerp(p1.x,p2.x,(t-.333f)*3), ofLerp(p1.y,p2.y,(t-.333f)*3));
      else
         return ofVec2f(ofLerp(p2.x,p3.x, (t-.666f)*3), ofLerp(p2.y,p3.y, (t-.666f)*3));*/
   }
   
   float BezierDerivative(float t, float p0, float p1, float p2, float p3)
   {
      return 3*Square(1-t)*(p1-p0)+6*(1-t)*t*(p2-p1)+3*t*t*(p3-p2);
   }
   
   ofVec2f BezierPerpendicular(float t, ofVec2f p0, ofVec2f p1, ofVec2f p2, ofVec2f p3)
   {
      ofVec2f perp(-BezierDerivative(t, p0.y, p1.y, p2.y, p3.y), BezierDerivative(t, p0.x, p1.x, p2.x, p3.x));
      return perp / sqrt(perp.lengthSquared());
   }
   
   ofVec2f ScaleVec(ofVec2f a, ofVec2f b)
   {
      return ofVec2f(a.x * b.x, a.y * b.y);
   }
}
   
void PatchCable::Render()
{
   PatchCablePos cable = GetPatchCablePos();
   mX = cable.start.x;
   mY = cable.start.y;
   
   float lineWidth = 1;
   float plugWidth = 3;
   int lineAlpha = 100;
   
   bool fatCable = false;
   
   if (TheSynth->GetLastClickedModule() == GetOwningModule() ||
       TheSynth->GetLastClickedModule() == mTarget)
      fatCable = true;
   
   if (TheSynth->ShouldAccentuateActiveModules())  //only draw if we're making noise
   {
      if (GetConnectionType() == kConnectionType_Note || GetConnectionType() == kConnectionType_Grid)
      {
         INoteSource* noteSource = dynamic_cast<INoteSource*>(GetOwningModule());
         IGridController* grid = dynamic_cast<IGridController*>(GetOwningModule());
         
         NoteHistoryList hist;
         if (noteSource)
         {
            noteSource->GetNoteOutput()->GetNoteHistory().Lock("draw lines");
            hist = noteSource->GetNoteOutput()->GetNoteHistory().GetHistory();
            noteSource->GetNoteOutput()->GetNoteHistory().Unlock();
         }
         if (grid)
         {
            grid->GetNoteHistory().Lock("draw lines");
            hist = grid->GetNoteHistory().GetHistory();
            grid->GetNoteHistory().Unlock();
         }
         
         bool hasNote = false;
         if (!hist.empty())
         {
            NoteHistoryEvent& note = *hist.begin();
            float elapsed = (gTime - note.mTime) / NOTE_HISTORY_LENGTH;
            if (note.mOn || elapsed <= 1)
               hasNote = true;
         }
         
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
   
   int wThis,hThis,xThis,yThis;
   GetDimensions(wThis,hThis);
   GetPosition(xThis,yThis);
   
   bool isInsideSelf = cable.end.x >= xThis && cable.end.x <= (xThis + wThis) && cable.end.y >= yThis && cable.end.y <= (yThis + hThis);
   if (!isInsideSelf)
   {
      IAudioSource* audioSource = nullptr;
      ofVec2f wireLineMag = cable.plug - cable.start;
      wireLineMag.x = MAX(50,fabsf(wireLineMag.x));
      wireLineMag.y = MAX(50,fabsf(wireLineMag.y));
      ofVec2f endDirection = (cable.plug - cable.end) / sqrtf((cable.plug-cable.end).lengthSquared());
      ofVec2f bezierControl1 = cable.start + ScaleVec(cable.startDirection, wireLineMag * .5f);
      ofVec2f bezierControl2 = cable.plug + ScaleVec(endDirection, wireLineMag * .5f);
      float wireLength = sqrtf((cable.plug - cable.start).lengthSquared());
      
      if (type == kConnectionType_Note || type == kConnectionType_Grid)
      {
         INoteSource* noteSource = dynamic_cast<INoteSource*>(mOwner->GetOwner());
         IGridController* grid = dynamic_cast<IGridController*>(mOwner->GetOwner());
         
         ofSetLineWidth(lineWidth);
         ofSetColor(lineColorAlphaed);
         ofBeginShape();
         ofVertex(cable.start.x,cable.start.y);
         for (int i=1; i<wireLength-1; ++i)
         {
            ofVec2f pos = Bezier(i/wireLength, cable.start, bezierControl1, bezierControl2, cable.plug);
            ofVertex(pos.x,pos.y);
         }
         ofVertex(cable.plug.x,cable.plug.y);
         ofEndShape();
         
         NoteHistoryList hist;
         if (noteSource)
         {
            noteSource->GetNoteOutput()->GetNoteHistory().Lock("draw lines");
            hist = noteSource->GetNoteOutput()->GetNoteHistory().GetHistory();
            noteSource->GetNoteOutput()->GetNoteHistory().Unlock();
         }
         if (grid)
         {
            grid->GetNoteHistory().Lock("draw lines");
            hist = grid->GetNoteHistory().GetHistory();
            grid->GetNoteHistory().Unlock();
         }
         
         if (!hist.empty())
         {
            ofSetLineWidth(lineWidth * 4);
            ofSetColor(lineColor);
            
            float lastElapsed = 0;
            for (NoteHistoryList::iterator i = hist.begin(); i != hist.end(); ++i)
            {
               NoteHistoryEvent& note = *i;
               float elapsed = (gTime - note.mTime) / NOTE_HISTORY_LENGTH;
               if (elapsed > 1)
                  elapsed = 1;
               if (note.mOn)
               {
                  ofBeginShape();
                  for (int j=lastElapsed*wireLength; j<elapsed*wireLength; ++j)
                  {
                     ofVec2f pos = Bezier(j/wireLength, cable.start, bezierControl1, bezierControl2, cable.plug);
                     ofVertex(pos.x,pos.y);
                  }
                  ofEndShape();
               }
               lastElapsed = elapsed;
               
               if (elapsed >= 1)
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
               ofVec2f pos = Bezier(i/wireLength, cable.start, bezierControl1, bezierControl2, cable.plug);
               float sample = vizBuff->GetSample((i/wireLength * numSamples), ch);
               sample = sqrtf(fabsf(sample)) * (sample < 0 ? -1 : 1);
               sample = ofClamp(sample, -1.0f, 1.0f);
               ofVec2f sampleOffsetDir = BezierPerpendicular(i/wireLength, cable.start, bezierControl1, bezierControl2, cable.plug);
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
            ofVec2f pos = Bezier(i/wireLength, cable.start, bezierControl1, bezierControl2, cable.plug);
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
      PatchCablePos cable = GetPatchCablePos();
      IClickable* potentialTarget = TheSynth->GetModuleAt(cable.end.x, cable.end.y);
      if (potentialTarget && GetConnectionType() == kConnectionType_UIControl)
      {
         const auto& uicontrols = ((IDrawableModule*)potentialTarget)->GetUIControls();
         for (auto uicontrol : uicontrols)
         {
            if (uicontrol->IsShowing() == false || !IsValidTarget(uicontrol))
               continue;
            
            int x,y,w,h;
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
         mOwner->SetPatchCableTarget(this, potentialTarget);
      
      mDragging = false;
      mHovered = false;
      if (sActivePatchCable == this)
         sActivePatchCable = nullptr;
      
      if (mTarget == nullptr)
         Destroy();
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
   int wThis,hThis,xThis,yThis,wThat,hThat,xThat,yThat;
   mOwner->GetDimensions(wThis,hThis);
   mOwner->GetPosition(xThis,yThis);
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
      if (targetModuleParent && targetModuleParent->Minimized())
      {
         wThat = 0;
         hThat = 0;
         targetModuleParent->GetPosition(xThat, yThat);
      }
   }
   else
   {
      wThat = 0;
      hThat = 0;
      xThat = xThis + wThis/2;
      yThat = yThis + hThis + 20;
      //xThat += index * 10;
   }
   
   int yThisAdjust = 0;
   IDrawableModule* parentModule = dynamic_cast<IDrawableModule*>(mOwner);
   if (parentModule && parentModule->HasTitleBar())
      yThisAdjust = IDrawableModule::TitleBarHeight();
   int yThatAdjust = 0;
   IDrawableModule* targetModule = dynamic_cast<IDrawableModule*>(mTarget);
   if (targetModule && targetModule->HasTitleBar() && !mDragging)
      yThatAdjust = IDrawableModule::TitleBarHeight();
   
   float blah,endX,endY;
   IDrawableModule::FindClosestSides(xThis,yThis-yThisAdjust,wThis,hThis+yThisAdjust,xThat,yThat-yThatAdjust,wThat,hThat+yThatAdjust, blah,blah,endX,endY);
   
   //use patchcablesource as start position
   float startX = mOwner->GetPosition().x;
   float startY = mOwner->GetPosition().y;
   ofVec2f startDirection;
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
   
   float diffX = endX-startX;
   float diffY = endY-startY;
   float length = sqrtf(diffX*diffX + diffY*diffY);
   float endCap = MIN(.5f,20/length);
   float plugX,plugY;
   if (wThat == 0)
      plugX = (startX-endX) * endCap + endX;
   else if (endX == xThat)
      plugX = endX - 10;
   else if (endX == xThat + wThat)
      plugX = endX + 10;
   else
      plugX = endX;
   
   if (hThat+yThatAdjust == 0)
      plugY = (startY-endY) * endCap + endY;
   else if (endY == yThat-yThatAdjust)
      plugY = endY - 10;
   else if (endY == yThat+hThat)
      plugY = endY + 10;
   else
      plugY = endY;
   
   PatchCablePos cable;
   cable.start.x = startX;
   cable.start.y = startY;
   cable.startDirection = startDirection;
   cable.end.x = endX;
   cable.end.y = endY;
   cable.plug.x = plugX;
   cable.plug.y = plugY;
   
   return cable;
}

void PatchCable::Grab()
{
   mDragging = true;
   sActivePatchCable = this;
   mOwner->CableGrabbed();
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
