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
#include "TitleBar.h"
#include "DropdownList.h"

class NVGcontext;
class NVGLUframebuffer;
class IUIControl;
class IPush2GridController;

class Push2Control : public IDrawableModule, public MidiDeviceListener, public IDropdownListener
{
public:
   Push2Control();
   virtual ~Push2Control();
   static IDrawableModule* Create() { return new Push2Control(); }
   
   string GetTitleLabel() override { return "push 2 control"; }
   void CreateUIControls() override;
   
   void SetLed(MidiMessageType type, int index, int color, int flashColor = -1);
   
   void OnMidiNote(MidiNote& note) override;
   void OnMidiControl(MidiControl& control) override;
   void OnMidiPitchBend(MidiPitchBend& pitchBend) override;
   
   void DropdownUpdated(DropdownList* list, int oldVal) override {}
   
   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   
   static bool sDrawingPush2Display;
   static NVGcontext* sVG;
   static NVGLUframebuffer* sFB;
   static void CreateStaticFramebuffer(); //windows was having trouble creating a nanovg context and fbo on the fly
   
private:
   //IDrawableModule
   void DrawModule() override;
   void DrawModuleUnclipped() override;
   void PostRender() override;
   bool Enabled() const override { return true; }
   void GetModuleDimensions(float& width, float& height) override { width = mWidth; height = mHeight; }
   void OnClicked(int x, int y, bool right) override;
   
   NBase::Result Initialize();
   void DrawToFramebuffer(NVGcontext* vg, NVGLUframebuffer* fb, float t, float pxRatio);
   void RenderPush2Display();
   
   void SetModuleGridLights();
   void DrawDisplayModuleControls();
   void DrawLowerModuleSelector();
   void SetDisplayModule(IDrawableModule* module);
   void DrawControls(vector<IUIControl*> controls, bool sliders, float yPos);
   void UpdateControlList();
   void AddFavoriteControl(IUIControl* control);
   void RemoveFavoriteControl(IUIControl* control);
   int GetColorForType(ModuleType type);
   bool GetGridIndex(int gridX, int gridY, int& gridIndex) { gridIndex = gridX + gridY * 8; return gridX >= 0 && gridX < 8 && gridY >= 0 && gridY < 8; }
   bool IsIgnorableModule(IDrawableModule* module);
   vector<IDrawableModule*> SortModules(vector<IDrawableModule*> modules);
   void AddModuleChain(IDrawableModule* module, vector<IDrawableModule*>& modules, vector<IDrawableModule*>& output);
   void DrawDisplayModuleRect(ofRectangle rect);
   
   bool mDisplayInitialized;
   
   ableton::Push2DisplayBridge bridge_;      /* The bridge allowing to use juce::graphics for push */
   ableton::Push2Display push2Display_;      /* The low-level push2 class */
   unsigned char* mPixels;
   const int kPixelRatio = 1;
   
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
   ofRectangle mModuleGridRect;
   
   vector<IUIControl*> mFavoriteControls;
   vector<IUIControl*> mSpawnModuleControls;
   bool mNewButtonHeld;
   bool mDeleteButtonHeld;
   bool mNoteHeldState[128];
   IDrawableModule* mHeldModule;
   bool mAllowRepatch;
   
   enum class ScreenDisplayMode
   {
      kNormal,
      kAddModule
   };
   ScreenDisplayMode mScreenDisplayMode;
   
   IPush2GridController* mGridControlModule;
   bool mDisplayModuleCanControlGrid;
   
   int mLedState[128*2];  //bottom 128 are notes, top 128 are CCs
   
   MidiDevice mDevice;
   
   SpawnListManager mSpawnLists;
};

//https://raw.githubusercontent.com/Ableton/push-interface/master/doc/MidiMapping.png
class IPush2GridController
{
public:
   virtual ~IPush2GridController() {}
   virtual void OnPush2Connect() {}
   virtual bool OnPush2Control(MidiMessageType type, int controlIndex, float midiValue) = 0;
   virtual void UpdatePush2Leds(Push2Control* push2) = 0;
};
