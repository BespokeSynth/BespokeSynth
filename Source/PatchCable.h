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
   kConnectionType_Pulse
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
   
   void Grab();
   bool IsValidTarget(IClickable* target) const;
   void Destroy();
   
   void SetUIControlConnection(UIControlConnection* conn) { mUIControlConnection = conn; }
   
   static PatchCable* sActivePatchCable;
   
protected:
   void OnClicked(int x, int y, bool right) override;
private:
   void SetTarget(IClickable* target);
   PatchCablePos GetPatchCablePos();
   bool IsOverStart(int x, int y);
   bool IsOverEnd(int x, int y);
   ofVec2f FindClosestSide(int x, int y, int w, int h, ofVec2f start, ofVec2f startDirection, ofVec2f& endDirection);
   
   PatchCableSource* mOwner;
   IClickable* mTarget;
   RadioButton* mTargetRadioButton;
   UIControlConnection* mUIControlConnection;
   IAudioReceiver* mAudioReceiverTarget;

   bool mHovered;
   bool mDragging;
   ofVec2f mGrabPos;
};

#endif /* defined(__Bespoke__PatchCable__) */
