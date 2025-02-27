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

#pragma once

#include "IClickable.h"

class RollingBuffer;
class PatchCableSource;
class IAudioReceiver;
class RadioButton;
class UIControlConnection;

struct PatchCablePos
{
   ofVec2d start;
   ofVec2d end;
   ofVec2d plug;
   ofVec2d startDirection;
};

enum ConnectionType
{
   kConnectionType_Note,
   kConnectionType_Audio,
   kConnectionType_UIControl,
   kConnectionType_Grid,
   kConnectionType_Special,
   kConnectionType_Pulse,
   kConnectionType_Modulator,
   kConnectionType_ValueSetter //for modulator-type that don't have a continuous connection to the control, and just set values as one-offs
};

enum class CableDropBehavior
{
   ShowQuickspawn,
   DoNothing,
   DisconnectCable
};

class PatchCable : public IClickable
{
   friend class PatchCableSource;

public:
   PatchCable(PatchCableSource* owner);
   virtual ~PatchCable();

   void Render() override;
   bool TestClick(double x, double y, bool right, bool testOnly = false) override;
   bool MouseMoved(double x, double y) override;
   void MouseReleased() override;
   void GetDimensions(double& width, double& height) override
   {
      width = 10;
      height = 10;
   }

   IDrawableModule* GetOwningModule() const;
   IClickable* GetTarget() const { return mTarget; }
   ConnectionType GetConnectionType() const;
   bool IsDragging() const { return mDragging; }
   void SetHoveringOnSource(bool hovering) { mHoveringOnSource = hovering; }
   void SetSourceIndex(int index) { mSourceIndex = index; }
   PatchCableSource* GetOwner() const { return mOwner; }
   void Grab();
   void Release();
   bool IsValidTarget(IClickable* target) const;
   void Destroy(bool fromUserClick);
   void SetTempDrawTarget(IClickable* target) { mTempDrawTarget = target; }
   void ShowQuickspawnForCable();
   IClickable* GetDropTarget();

   void SetUIControlConnection(UIControlConnection* conn) { mUIControlConnection = conn; }

   static PatchCable* sActivePatchCable;

protected:
   void OnClicked(double x, double y, bool right) override;

private:
   void SetCableTarget(IClickable* target);
   PatchCablePos GetPatchCablePos();
   ofVec2d FindClosestSide(double x, double y, double w, double h, ofVec2d start, ofVec2d startDirection, ofVec2d& endDirection);

   PatchCableSource* mOwner{ nullptr };
   IClickable* mTarget{ nullptr };
   IClickable* mTempDrawTarget{ nullptr };
   RadioButton* mTargetRadioButton{ nullptr };
   UIControlConnection* mUIControlConnection{ nullptr };
   IAudioReceiver* mAudioReceiverTarget{ nullptr };

   bool mHovered{ false };
   bool mDragging{ false };
   ofVec2d mGrabPos;
   bool mHoveringOnSource{ false };
   int mSourceIndex{ 0 };
};
