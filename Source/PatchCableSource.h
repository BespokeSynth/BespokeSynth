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

enum DefaultPatchBehavior
{
   kDefaultPatchBehavior_Repatch,
   kDefaultPatchBehavior_Add
};

enum PatchCableDrawMode
{
   kPatchCableDrawMode_Normal,
   kPatchCableDrawMode_HoverOnly
};

class PatchCableSource : public IClickable
{
public:
   enum Side
   {
      kNone,
      kBottom,
      kLeft,
      kRight
   };
   
   PatchCableSource(IDrawableModule* owner, ConnectionType type);
   virtual ~PatchCableSource();
   PatchCable* AddPatchCable(IClickable* target);
   const vector<PatchCable*>& GetPatchCables() const { return mPatchCables; }
   void Clear();
   bool IsValidTarget(IClickable* target) const;
   void CableGrabbed();
   ConnectionType GetConnectionType() const { return mType; }
   IDrawableModule* GetOwner() const { return mOwner; }
   void SetOverrideVizBuffer(RollingBuffer* viz) { mOverrideVizBuffer = viz; }
   RollingBuffer* GetOverrideVizBuffer() const { return mOverrideVizBuffer; }
   void UpdatePosition();
   void SetManualPosition(int x, int y) { mManualPositionX = x; mManualPositionY = y; mAutomaticPositioning = false; }
   void RemovePatchCable(PatchCable* cable);
   void ClearPatchCables();
   void SetPatchCableTarget(PatchCable* cable, IClickable* target);
   const vector<INoteReceiver*>& GetNoteReceivers() const { return mNoteReceivers; }
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
   void AddTypeFilter(string type) { mTypeFilter.push_back(type); }
   void ClearTypeFilter() { mTypeFilter.clear(); }
   Side GetCableSide() const { return mSide; }
   void SetManualSide(Side side) { mManualSide = side; }
   
   void Render() override;
   bool TestClick(int x, int y, bool right, bool testOnly = false) override;
   bool MouseMoved(float x, float y) override;
   void MouseReleased() override;
   void GetDimensions(int& width, int& height) override { width = 10; height = 10; }
   void KeyPressed(int key, bool isRepeat);
   
   void SaveState(FileStreamOut& out);
   void LoadState(FileStreamIn& in);
   
protected:
   void OnClicked(int x, int y, bool right) override;
private:
   void FindValidTargets();
   bool InAddCableMode() const;
   
   vector<PatchCable*> mPatchCables;
   bool mHovered;
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
   Side mSide;
   Side mManualSide;
   
   vector<INoteReceiver*> mNoteReceivers;
   IAudioReceiver* mAudioReceiver;
   
   vector<string> mTypeFilter;
   vector<IClickable*> mValidTargets;
};

#endif /* defined(__Bespoke__PatchCableSource__) */
