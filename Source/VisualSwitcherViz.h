#pragma once

#include "IDrawableModule.h"
#include "IVisualNode.h"
#include "DropdownList.h"
#include "Transport.h"
#include "PatchCableSource.h"
#include "VizGL.h"

class VisualSwitcherViz : public IDrawableModule, public IDropdownListener, public ITimeListener, public IVisualNode
{
public:
   VisualSwitcherViz();
   virtual ~VisualSwitcherViz();

   static IDrawableModule* Create() { return new VisualSwitcherViz(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }

   bool IsResizable() const override { return true; }
   void Resize(float w, float h) override
   {
      mWidth = MAX(280, w);
      mHeight = MAX(320, h);
      if (mTargetCable)
         mTargetCable->SetManualPosition(mWidth - 12, 12);
   }

   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

   void DropdownUpdated(DropdownList* list, int oldVal, double time) override;

   // ITimeListener
   void OnTimeEvent(double time) override;

   // IVisualNode
   unsigned int GetOutputTexture() override { return VizGL::FboTexture(mOut); }
   int GetOutputWidth() const override { return mCurrentOutputWidth; }
   int GetOutputHeight() const override { return mCurrentOutputHeight; }
   void CookIfNeeded(int frameNum) override
   {
      if (mLastCookFrame != frameNum)
      {
         mLastCookFrame = frameNum;
         Cook();
      }
   }
   void Cook();

   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;

private:
   void UpdateTransportListener();
   bool EnsureShader();

   int mLastCookFrame{ -1 };

   unsigned int mCurrentOutputTexture{ 0 };
   int mCurrentOutputWidth{ 0 };
   int mCurrentOutputHeight{ 0 };

   VizGL::Fbo mOut;
   unsigned int mProgram{ 0 };

   PatchCableSource* mTargetCable{ nullptr };
   DropdownList* mIntervalSelector{ nullptr };

   NoteInterval mInterval{ kInterval_4n };
   int mActiveIndex{ 0 };

   float mWidth{ 280 };
   float mHeight{ 320 };
};
