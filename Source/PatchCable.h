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
//  PatchCable.h
//  Bespoke
//
//  Created by Ryan Challinor on 12/12/15.
//
//

#ifndef __Bespoke__PatchCable__
#define __Bespoke__PatchCable__

#include "IClickable.h"

class RollingBuffer;
class PatchCableSource;
class IAudioReceiver;
class RadioButton;
class UIControlConnection;

struct PatchCablePos
{
   ofVec2f start;
   ofVec2f end;
   ofVec2f plug;
   ofVec2f startDirection;
};

enum ConnectionType
{
   kConnectionType_Note,
   kConnectionType_Audio,
   kConnectionType_UIControl,
   kConnectionType_Grid,
   kConnectionType_Special,
   kConnectionType_Pulse,
   kConnectionType_Modulator
};

class PatchCable : public IClickable
{
   friend class PatchCableSource;
public:
   PatchCable(PatchCableSource* owner);
   virtual ~PatchCable();
   
   void Render() override;
   bool TestClick(int x, int y, bool right, bool testOnly = false) override;
   bool MouseMoved(float x, float y) override;
   void MouseReleased() override;
   void GetDimensions(float& width, float& height) override { width = 10; height = 10; }
   
   IDrawableModule* GetOwningModule() const;
   IClickable* GetTarget() const { return mTarget; }
   ConnectionType GetConnectionType() const;
   bool IsDragging() const { return mDragging; }
   void SetHoveringOnSource(bool hovering) { mHoveringOnSource = hovering; }
   void SetSourceIndex(int index) { mSourceIndex = index; }
   
   void Grab();
   bool IsValidTarget(IClickable* target) const;
   void Destroy(bool fromUserClick);
   
   void SetUIControlConnection(UIControlConnection* conn) { mUIControlConnection = conn; }
   
   static PatchCable* sActivePatchCable;
   
protected:
   void OnClicked(int x, int y, bool right) override;
private:
   void SetTarget(IClickable* target);
   PatchCablePos GetPatchCablePos();
   ofVec2f FindClosestSide(int x, int y, int w, int h, ofVec2f start, ofVec2f startDirection, ofVec2f& endDirection);
   IClickable* GetDropTarget();
   
   PatchCableSource* mOwner;
   IClickable* mTarget;
   RadioButton* mTargetRadioButton;
   UIControlConnection* mUIControlConnection;
   IAudioReceiver* mAudioReceiverTarget;

   bool mHovered;
   bool mDragging;
   ofVec2f mGrabPos;
   bool mHoveringOnSource;
   int mSourceIndex;
};

#endif /* defined(__Bespoke__PatchCable__) */
