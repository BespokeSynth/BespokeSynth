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

#ifndef __Bespoke__PatchCableSource__
#define __Bespoke__PatchCableSource__

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
   NoteHistoryEvent() : mOn(false), mTime(0) {}
   bool mOn;
   double mTime;
};

class NoteHistory
{
public:
   NoteHistory() { mHistoryPos = 0; mLastOnEventTime = -999; }
   void AddEvent(double time, bool on);
   bool CurrentlyOn();
   double GetLastOnEventTime() { return mLastOnEventTime; }
   const NoteHistoryEvent& GetHistoryEvent(int ago) const;
   static const int kHistorySize = 100;
private:
   NoteHistoryEvent mHistory[kHistorySize];
   int mHistoryPos;
   double mLastOnEventTime;
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
   void SetManualPosition(int x, int y) { mManualPositionX = x; mManualPositionY = y; mAutomaticPositioning = false; }
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
   ofColor GetColor() const { return mColor; }
   void SetEnabled(bool enabled) { mEnabled = enabled; }
   bool Enabled() const;
   void AddTypeFilter(std::string type) { mTypeFilter.push_back(type); }
   void ClearTypeFilter() { mTypeFilter.clear(); }
   void SetManualSide(Side side) { mManualSide = side; }
   void SetClickable(bool clickable) { mClickable = clickable; }
   bool TestHover(float x, float y) const;
   void SetOverrideCableDir(ofVec2f dir) { mHasOverrideCableDir = true; mOverrideCableDir = dir; }
   ofVec2f GetCableStart(int index) const;
   ofVec2f GetCableStartDir(int index, ofVec2f dest) const;
   void SetModulatorOwner(IModulator* modulator) { mModulatorOwner = modulator; }
   IModulator* GetModulatorOwner() const { return mModulatorOwner; }
   
   void AddHistoryEvent(double time, bool on) { mNoteHistory.AddEvent(time, on); if (on) { mLastOnEventTime = time; } }
   NoteHistory& GetHistory() { return mNoteHistory; }
   double GetLastOnEventTime() const { return mLastOnEventTime; }
   
   void DrawSource();
   void DrawCables(bool parentMinimized);
   void Render() override;
   bool TestClick(int x, int y, bool right, bool testOnly = false) override;
   bool MouseMoved(float x, float y) override;
   void MouseReleased() override;
   void GetDimensions(float& width, float& height) override { width = 10; height = 10; }
   void KeyPressed(int key, bool isRepeat);
   
   void SaveState(FileStreamOut& out);
   void LoadState(FileStreamIn& in);
   
   static bool sAllowInsert;
   
protected:
   void OnClicked(int x, int y, bool right) override;
private:
   bool InAddCableMode() const;
   int GetHoverIndex(float x, float y) const;
   
   std::vector<PatchCable*> mPatchCables;
   int mHoverIndex; //-1 = not hovered
   ConnectionType mType;
   bool mAllowMultipleTargets;
   DefaultPatchBehavior mDefaultPatchBehavior;
   PatchCableDrawMode mPatchCableDrawMode;
   IDrawableModule* mOwner;
   RollingBuffer* mOverrideVizBuffer;
   bool mAutomaticPositioning;
   int mManualPositionX;
   int mManualPositionY;
   ofColor mColor;
   bool mEnabled;
   bool mClickable;
   Side mSide;
   Side mManualSide;
   bool mHasOverrideCableDir;
   ofVec2f mOverrideCableDir;
   
   std::vector<INoteReceiver*> mNoteReceivers;
   std::vector<IPulseReceiver*> mPulseReceivers;
   IAudioReceiver* mAudioReceiver;
   
   std::vector<std::string> mTypeFilter;
   std::vector<IClickable*> mValidTargets;
   
   NoteHistory mNoteHistory;
   double mLastOnEventTime;
   
   IModulator* mModulatorOwner;

   enum class DrawPass
   {
      kSource,
      kCables
   };
   DrawPass mDrawPass;
   bool mParentMinimized;
};

#endif /* defined(__Bespoke__PatchCableSource__) */
