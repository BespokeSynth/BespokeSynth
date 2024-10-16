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
//  PatchCableSource.h
//  Bespoke
//
//  Created by Ryan Challinor on 12/13/15.
//
//

#pragma once

#include "PatchCable.h"
#include "IClickable.h"
#include "SynthGlobals.h"
#include "IDrawableModule.h"

class IAudioReceiver;
class INoteReceiver;
class IPulseReceiver;
class IModulator;

enum DefaultPatchBehavior
{
   kDefaultPatchBehavior_Repatch,
   kDefaultPatchBehavior_Add
};

enum PatchCableDrawMode
{
   kPatchCableDrawMode_Normal,
   kPatchCableDrawMode_CablesOnHoverOnly,
   kPatchCableDrawMode_SourceOnHoverOnly
};

#define NOTE_HISTORY_LENGTH 250

struct NoteHistoryEvent
{
   bool mOn{ false };
   double mTime{ 0 };
   int mData{ 0 };
};

class NoteHistory
{
public:
   void AddEvent(double time, bool on, int data);
   bool CurrentlyOn();
   double GetLastOnEventTime() { return mLastOnEventTime; }
   const NoteHistoryEvent& GetHistoryEvent(int ago) const;
   static const int kHistorySize = 100;

private:
   NoteHistoryEvent mHistory[kHistorySize]{};
   int mHistoryPos{ 0 };
   double mLastOnEventTime{ -999 };
};

class PatchCableSource : public IClickable
{
public:
   enum class Side
   {
      kNone,
      kBottom,
      kLeft,
      kRight
   };

   PatchCableSource(IDrawableModule* owner, ConnectionType type);
   virtual ~PatchCableSource();
   PatchCable* AddPatchCable(IClickable* target);
   const std::vector<PatchCable*>& GetPatchCables() const { return mPatchCables; }
   void Clear();
   void FindValidTargets();
   bool IsValidTarget(IClickable* target) const;
   void CableGrabbed();
   ConnectionType GetConnectionType() const { return mType; }
   void SetConnectionType(ConnectionType type);
   IDrawableModule* GetOwner() const { return mOwner; }
   void SetOverrideVizBuffer(RollingBuffer* viz) { mOverrideVizBuffer = viz; }
   RollingBuffer* GetOverrideVizBuffer() const { return mOverrideVizBuffer; }
   void UpdatePosition(bool parentMinimized);
   void SetManualPosition(int x, int y)
   {
      mManualPositionX = x;
      mManualPositionY = y;
      mAutomaticPositioning = false;
   }
   void RemovePatchCable(PatchCable* cable, bool fromUserAction = false);
   void ClearPatchCables();
   void SetPatchCableTarget(PatchCable* cable, IClickable* target, bool fromUserClick);
   const std::vector<INoteReceiver*>& GetNoteReceivers() const { return mNoteReceivers; }
   const std::vector<IPulseReceiver*>& GetPulseReceivers() const { return mPulseReceivers; }
   IAudioReceiver* GetAudioReceiver() const { return mAudioReceiver; }
   IClickable* GetTarget() const;
   void SetTarget(IClickable* target);
   void SetAllowMultipleTargets(bool allow) { mAllowMultipleTargets = allow; }
   void SetDefaultPatchBehavior(DefaultPatchBehavior beh) { mDefaultPatchBehavior = beh; }
   void SetPatchCableDrawMode(PatchCableDrawMode mode) { mPatchCableDrawMode = mode; }
   void SetColor(ofColor color) { mColor = color; }
   ofColor GetColor() const;
   void SetEnabled(bool enabled) { mEnabled = enabled; }
   bool Enabled() const;
   void AddTypeFilter(std::string type) { mTypeFilter.push_back(type); }
   void ClearTypeFilter() { mTypeFilter.clear(); }
   void SetManualSide(Side side) { mManualSide = side; }
   void SetClickable(bool clickable) { mClickable = clickable; }
   bool TestHover(float x, float y) const;
   void SetOverrideCableDir(ofVec2f dir, Side side)
   {
      mHasOverrideCableDir = true;
      mOverrideCableDir = dir;
      mManualSide = side;
   }
   ofVec2f GetCableStart(int index) const;
   ofVec2f GetCableStartDir(int index, ofVec2f dest) const;
   void SetModulatorOwner(IModulator* modulator) { mModulatorOwner = modulator; }
   IModulator* GetModulatorOwner() const { return mModulatorOwner; }
   void SetIsPartOfCircularDependency(bool set) { mIsPartOfCircularDependency = set; }
   bool GetIsPartOfCircularDependency() const { return mIsPartOfCircularDependency; }

   void AddHistoryEvent(double time, bool on, int data = 0)
   {
      mNoteHistory.AddEvent(time, on, data);
      if (on)
      {
         mLastOnEventTime = time;
      }
   }
   NoteHistory& GetHistory() { return mNoteHistory; }
   double GetLastOnEventTime() const { return mLastOnEventTime; }

   void DrawSource();
   void DrawCables(bool parentMinimized);
   void Render() override;
   bool TestClick(float x, float y, bool right, bool testOnly = false) override;
   bool MouseMoved(float x, float y) override;
   void MouseReleased() override;
   void GetDimensions(float& width, float& height) override
   {
      width = 10;
      height = 10;
   }
   void KeyPressed(int key, bool isRepeat);
   bool IsHovered() const { return mHoverIndex != -1; }

   void SaveState(FileStreamOut& out);
   void LoadState(FileStreamIn& in);

   static bool sAllowInsert;
   static bool sIsLoadingModulePreset;

protected:
   void OnClicked(float x, float y, bool right) override;

private:
   bool InAddCableMode() const;
   int GetHoverIndex(float x, float y) const;

   std::vector<PatchCable*> mPatchCables;
   int mHoverIndex{ -1 }; //-1 = not hovered
   ConnectionType mType{ ConnectionType::kConnectionType_Audio };
   bool mAllowMultipleTargets{ true };
   DefaultPatchBehavior mDefaultPatchBehavior{ DefaultPatchBehavior::kDefaultPatchBehavior_Repatch };
   PatchCableDrawMode mPatchCableDrawMode{ PatchCableDrawMode::kPatchCableDrawMode_Normal };
   IDrawableModule* mOwner{ nullptr };
   RollingBuffer* mOverrideVizBuffer{ nullptr };
   bool mAutomaticPositioning{ true };
   int mManualPositionX{ 0 };
   int mManualPositionY{ 0 };
   ofColor mColor;
   bool mEnabled{ true };
   bool mClickable{ true };
   Side mSide{ Side::kNone };
   Side mManualSide{ Side::kNone };
   bool mHasOverrideCableDir{ false };
   ofVec2f mOverrideCableDir;
   bool mIsPartOfCircularDependency{ false };

   std::vector<INoteReceiver*> mNoteReceivers;
   std::vector<IPulseReceiver*> mPulseReceivers;
   IAudioReceiver* mAudioReceiver{ nullptr };

   std::vector<std::string> mTypeFilter;
   std::vector<IClickable*> mValidTargets;

   NoteHistory mNoteHistory;
   double mLastOnEventTime{ -9999 };

   IModulator* mModulatorOwner{ nullptr };

   enum class DrawPass
   {
      kSource,
      kCables
   };
   DrawPass mDrawPass{ DrawPass::kSource };
   bool mParentMinimized{ false };
   IDrawableModule* mLastSeenAutopatchableModule{ nullptr };
};
