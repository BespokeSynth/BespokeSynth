/**
    bespoke synth - MovieOut

    A standalone "record the visuals to mp4" module, inspired by TouchDesigner's Movie File Out.
    Bespoke cables can't carry video, so instead of receiving pixels over a cable this module points
    a cable at any target module and re-renders THAT module into an offscreen framebuffer each frame,
    reads the pixels back, and (on stop) encodes them to an mp4 on the Desktop with ffmpeg.
    Works with any of the visualizers - or in fact any drawable module - with no changes to them.

    Experimental / macOS-focused GL code. Requires ffmpeg (brew install ffmpeg).
**/

#pragma once

#include "IDrawableModule.h"
#include "ClickButton.h"
#include "Checkbox.h"
#include "Slider.h"
#include "DropdownList.h"
#include "VizRecorder.h"

class PatchCableSource;

class MovieOut : public IDrawableModule, public IButtonListener, public IIntSliderListener, public IDropdownListener
{
public:
   MovieOut();
   virtual ~MovieOut();
   static IDrawableModule* Create() { return new MovieOut(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;

   void SetEnabled(bool enabled) override { mEnabled = enabled; }
   bool IsEnabled() const override { return mEnabled; }

   //IDrawableModule
   void Poll() override;
   void PostRepatch(PatchCableSource* cableSource, bool fromUserClick) override;

   void ButtonClicked(ClickButton* button, double time) override;
   void IntSliderUpdated(IntSlider* slider, int oldVal, double time) override {}
   void DropdownUpdated(DropdownList* list, int oldVal, double time) override {}

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;

private:
   void DrawModule() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }

   IDrawableModule* GetTarget() const;
   void ComputeFrameSize(float tw, float th, int& fw, int& fh) const;

   VizRecorder mRecorder;
   PatchCableSource* mTargetCable{ nullptr };

   int mScale{ 2 }; //output scale multiplier applied to the target's on-screen size (1..4)
   int mAspect{ 0 }; //0=source, 1=16:9, 2=1:1, 3=9:16
   int mFrameW{ 0 };
   int mFrameH{ 0 };
   ClickButton* mRecordButton{ nullptr };
   IntSlider* mScaleSlider{ nullptr };
   DropdownList* mAspectSelector{ nullptr };
   std::string mNote; //transient UI note (e.g. "visualizers only")

   float mWidth{ 210 };
   float mHeight{ 116 };
};
