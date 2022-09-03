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
/*
  ==============================================================================

    Push2Control.h
    Created: 24 Feb 2020 8:57:57pm
    Author:  Ryan Challinor

  ==============================================================================
*/

#pragma once

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


   void CreateUIControls() override;
   void Poll() override;

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
   static IUIControl* sBindToUIControl;

private:
   //IDrawableModule
   void DrawModule() override;
   void DrawModuleUnclipped() override;
   void PostRender() override;
   bool Enabled() const override { return true; }
   void GetModuleDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight;
   }
   void OnClicked(float x, float y, bool right) override;

   bool Initialize();
   void DrawToFramebuffer(NVGcontext* vg, NVGLUframebuffer* fb, float t, float pxRatio);
   void RenderPush2Display();

   void SetModuleGridLights();
   void DrawDisplayModuleControls();
   void DrawLowerModuleSelector();
   void SetDisplayModule(IDrawableModule* module, bool addToHistory);
   void DrawControls(std::vector<IUIControl*> controls, bool sliders, float yPos);
   void UpdateControlList();
   void AddFavoriteControl(IUIControl* control);
   void RemoveFavoriteControl(IUIControl* control);
   void BookmarkModuleToSlot(int slotIndex, IDrawableModule* module);
   void SwitchToBookmarkedModule(int slotIndex);
   int GetPadColorForType(ModuleCategory type);
   bool GetGridIndex(int gridX, int gridY, int& gridIndex)
   {
      gridIndex = gridX + gridY * 8;
      return gridX >= 0 && gridX < 8 && gridY >= 0 && gridY < 8;
   }
   bool IsIgnorableModule(IDrawableModule* module);
   std::vector<IDrawableModule*> SortModules(std::vector<IDrawableModule*> modules);
   void AddModuleChain(IDrawableModule* module, std::vector<IDrawableModule*>& modules, std::vector<IDrawableModule*>& output, int depth);
   void DrawDisplayModuleRect(ofRectangle rect);
   std::string GetModuleTypeToSpawn();
   ModuleCategory GetModuleTypeForSpawnList(IUIControl* control);
   ofColor GetSpawnGridColor(int index, ModuleCategory moduleType) const;
   int GetSpawnGridPadColor(int index, ModuleCategory moduleType) const;

   unsigned char* mPixels;
   const int kPixelRatio = 1;

   const float kColumnSpacing = 121;

   int mFontHandle;
   int mFontHandleBold;

   float mWidth;
   float mHeight;

   IDrawableModule* mDisplayModule;
   std::vector<IUIControl*> mSliderControls;
   std::vector<IUIControl*> mButtonControls;
   std::vector<IUIControl*> mDisplayedControls;
   int mModuleColumnOffset;
   float mModuleColumnOffsetSmoothed;

   std::vector<IDrawableModule*> mModules;
   float mModuleListOffset;
   float mModuleListOffsetSmoothed;
   IDrawableModule* mModuleGrid[8 * 8];
   ofRectangle mModuleGridRect;

   std::vector<IUIControl*> mFavoriteControls;
   std::vector<IUIControl*> mSpawnModuleControls;
   bool mNewButtonHeld;
   bool mDeleteButtonHeld;
   bool mModulationButtonHeld;
   bool mAddModuleBookmarkButtonHeld;
   std::array<bool, 128> mNoteHeldState;
   IDrawableModule* mHeldModule;
   bool mAllowRepatch;
   std::vector<IDrawableModule*> mModuleHistory;
   int mModuleHistoryPosition;
   std::vector<IDrawableModule*> mBookmarkSlots;
   bool mInMidiControllerBindMode;

   enum class ScreenDisplayMode
   {
      kNormal,
      kAddModule
   };
   ScreenDisplayMode mScreenDisplayMode;

   IPush2GridController* mGridControlModule;
   bool mDisplayModuleCanControlGrid;

   int mLedState[128 * 2]; //bottom 128 are notes, top 128 are CCs

   MidiDevice mDevice;

   SpawnListManager mSpawnLists;
   int mPendingSpawnPitch;
   int mSelectedGridSpawnListIndex;
   std::string mPushBridgeInitErrMsg;
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
