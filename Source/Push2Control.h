/*
  ==============================================================================

    Push2Control.h
    Created: 24 Feb 2020 8:57:57pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

#include "push2/push2/JuceToPush2DisplayBridge.h"
#include "IDrawableModule.h"
#include "MidiDevice.h"
#include "MidiController.h"

class NVGcontext;
class NVGLUframebuffer;
class IUIControl;

class Push2Control : public IDrawableModule, public MidiDeviceListener
{
public:
   Push2Control();
   virtual ~Push2Control();
   static IDrawableModule* Create() { return new Push2Control(); }
   
   string GetTitleLabel() override { return "push 2 control"; }
   void CreateUIControls() override;
   
   void OnMidiNote(MidiNote& note) override;
   void OnMidiControl(MidiControl& control) override;
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   
   static bool sDrawingPush2Display;
   
private:
   //IDrawableModule
   void DrawModule() override;
   void DrawModuleUnclipped() override;
   void PostRender() override;
   bool Enabled() const override { return true; }
   void GetModuleDimensions(int& width, int& height) override { width = mWidth; height = mHeight; }
   void OnClicked(int x, int y, bool right) override;
   
   NBase::Result Initialize();
   void DrawToFramebuffer(NVGcontext* vg, NVGLUframebuffer* fb, float t, float pxRatio);
   void RenderPush2Display();
   
   void SetDisplayModule(IDrawableModule* module);
   void DrawControls(vector<IUIControl*> controls, float yPos);
   void UpdateControlList();
   void AddFavoriteControl(IUIControl* control);
   void RemoveFavoriteControl(IUIControl* control);
   void SetLed(MidiMessageType type, int index, int color, int channel = 1);
   int GetColorForType(ModuleType type);
   bool GetGridIndex(int gridX, int gridY, int& gridIndex) { gridIndex = gridX + gridY * 8; return gridX >= 0 && gridX < 8 && gridY >= 0 && gridY < 8; }
   bool IsIgnorableModule(IDrawableModule* module);
   vector<IDrawableModule*> SortModules(vector<IDrawableModule*> modules);
   void AddModuleChain(IDrawableModule* module, vector<IDrawableModule*>& modules, vector<IDrawableModule*>& output);
   void DrawDisplayModuleRect(ofRectangle rect);
   
   bool mDisplayInitialized;
   
   ableton::Push2DisplayBridge bridge_;      /* The bridge allowing to use juce::graphics for push */
   ableton::Push2Display push2Display_;      /* The low-level push2 class */
   NVGcontext* mVG;
   NVGLUframebuffer* mFB;
   unsigned char* mPixels;
   int mPixelRatio;
   
   const float kColumnSpacing = 121;
   
   int mFontHandle;
   int mFontHandleBold;
   
   int mWidth;
   int mHeight;
   
   IDrawableModule* mDisplayModule;
   IDrawableModule* mLastModuleSetFromHover;
   vector<IUIControl*> mSliderControls;
   vector<IUIControl*> mButtonControls;
   int mModuleColumnOffset;
   float mModuleColumnOffsetSmoothed;
   
   vector<IDrawableModule*> mModules;
   float mModuleListOffset;
   float mModuleListOffsetSmoothed;
   IDrawableModule* mModuleGrid[8*8];
   
   vector<IUIControl*> mFavoriteControls;
   bool mNewButtonHeld;
   bool mDeleteButtonHeld;
   IDrawableModule* mHeldModule;
   bool mAllowRepatch;
   
   int mLedState[128*2];  //bottom 128 are notes, top 128 are CCs
   
   MidiDevice mDevice;
};
