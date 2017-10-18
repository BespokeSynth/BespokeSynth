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
#include "IAudioSource.h"
#include "GridController.h"
#include "ModularSynth.h"
#include "SynthGlobals.h"
#include "PatchCableSource.h"

PatchCable* PatchCable::sActivePatchCable = NULL;

PatchCable::PatchCable(PatchCableSource* owner)
: mHovered(false)
, mDragging(false)
, mTarget(NULL)
{
   mOwner = owner;
   TheSynth->RegisterPatchCable(this);
}

PatchCable::~PatchCable()
{
   if (sActivePatchCable == this)
      sActivePatchCable = NULL;
   TheSynth->UnregisterPatchCable(this);
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
            if (vizBuff == NULL)
               vizBuff = audioSource->GetVizBuffer();
            assert(vizBuff);
            int numSamples = vizBuff->Size();
            bool allZero = true;
            for (int i=0; i<numSamples; ++i)
            {
               if (vizBuff->GetSample(i, 0) != 0)
               {
                  allZero = false;
                  break;
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
      if (type == kConnectionType_Note || type == kConnectionType_Grid)
      {
         INoteSource* noteSource = dynamic_cast<INoteSource*>(mOwner->GetOwner());
         IGridController* grid = dynamic_cast<IGridController*>(mOwner->GetOwner());
         
         ofSetLineWidth(lineWidth);
         ofSetColor(lineColorAlphaed);
         ofLine(cable.start.x,cable.start.y,cable.plug.x,cable.plug.y);
         ofSetColor(lineColor);
         
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
            int lastX = cable.start.x;
            int lastY = cable.start.y;
            for (NoteHistoryList::iterator i = hist.begin(); i != hist.end(); ++i)
            {
               NoteHistoryEvent& note = *i;
               float elapsed = (gTime - note.mTime) / NOTE_HISTORY_LENGTH;
               float diffPlugX = cable.plug.x-cable.start.x;
               float diffPlugY = cable.plug.y-cable.start.y;
               int x = cable.start.x + diffPlugX*elapsed;
               int y = cable.start.y + diffPlugY*elapsed;
               if (elapsed > 1)
               {
                  x = cable.plug.x;
                  y = cable.plug.y;
               }
               if (note.mOn)
               {
                  ofSetLineWidth(lineWidth * 4);
                  ofLine(lastX, lastY, x, y);
               }
               lastX = x;
               lastY = y;
               
               if (elapsed > 1)
                  break;
            }
         }
         
         ofSetLineWidth(plugWidth);
         ofSetColor(lineColor);
         ofLine(cable.plug.x,cable.plug.y,cable.end.x,cable.end.y);
      }
      else if (type == kConnectionType_Audio)
      {
         IAudioSource* audioSource = dynamic_cast<IAudioSource*>(mOwner->GetOwner());
         if (audioSource)
         {
            ofSetLineWidth(plugWidth);
            ofSetColor(lineColor);
            ofLine(cable.plug.x,cable.plug.y,cable.end.x,cable.end.y);
            
            ofSetLineWidth(lineWidth);
            
            RollingBuffer* vizBuff = mOwner->GetOverrideVizBuffer();
            if (vizBuff == NULL)
               vizBuff = audioSource->GetVizBuffer();
            assert(vizBuff);
            int numSamples = vizBuff->Size();
            float wireLength = sqrtf((cable.plug.x - cable.start.x)*(cable.plug.x - cable.start.x) + (cable.plug.y - cable.start.y)*(cable.plug.y - cable.start.y));
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
                  float x = cable.start.x + i*dx;
                  float y = cable.start.y + i*dy;
                  float sample = vizBuff->GetSample((i/wireLength * numSamples), ch);
                  sample = sqrtf(fabsf(sample)) * (sample < 0 ? -1 : 1);
                  sample = ofClamp(sample, -1.0f, 1.0f);
                  x += 20 * sample * -dy;
                  y += 20 * sample * dx;
                  ofVertex(x + offset.x,y + offset.y);
               }
               ofVertex(cable.plug.x + offset.x,cable.plug.y + offset.y);
               ofEndShape();
            }
         }
         else
         {
            ofSetLineWidth(plugWidth);
            ofSetColor(lineColor);
            ofLine(cable.plug.x,cable.plug.y,cable.end.x,cable.end.y);
            
            ofSetLineWidth(lineWidth);
            ofSetColor(lineColorAlphaed);
            ofLine(cable.start.x,cable.start.y,cable.plug.x,cable.plug.y);
         }
      }
      else
      {
         ofSetLineWidth(plugWidth);
         ofSetColor(lineColor);
         ofLine(cable.plug.x,cable.plug.y,cable.end.x,cable.end.y);
         
         ofSetLineWidth(lineWidth);
         ofSetColor(lineColorAlphaed);
         ofLine(cable.start.x,cable.start.y,cable.plug.x,cable.plug.y);
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
         sActivePatchCable = NULL;
      
      if (mTarget == NULL)
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
   
   int startX,startY,endX,endY;
   IDrawableModule::FindClosestSides(xThis,yThis-yThisAdjust,wThis,hThis+yThisAdjust,xThat,yThat-yThatAdjust,wThat,hThat+yThatAdjust, startX,startY,endX,endY);
   
   //use patchcablesource as start position
   startX = mOwner->GetPosition().x;
   startY = mOwner->GetPosition().y;
   
   float diffX = endX-startX;
   float diffY = endY-startY;
   float length = sqrtf(diffX*diffX + diffY*diffY);
   float endCap = MIN(.5f,20/length);
   if (GetConnectionType() == kConnectionType_UIControl)
      endCap = MIN(.5f,8/length);
   int plugX = int((startX-endX)*endCap)+endX;
   int plugY = int((startY-endY)*endCap)+endY;
   
   PatchCablePos cable;
   cable.start.x = startX;
   cable.start.y = startY;
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
