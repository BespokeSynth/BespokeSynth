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
class Snapshots;
class ControlRecorder;

class Push2Control : public IDrawableModule, public MidiDeviceListener, public IDropdownListener
{
public:
   Push2Control();
   virtual ~Push2Control();
   static IDrawableModule* Create() { return new Push2Control(); }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void CreateUIControls() override;
   void Poll() override;
   void Exit() override;
   void KeyPressed(int key, bool isRepeat) override;

   void SetLed(MidiMessageType type, int index, int color, int flashColor = -1);
   void SetDisplayModule(IDrawableModule* module, bool addToHistory = true);
   IDrawableModule* GetDisplayModule() const { return mDisplayModule; }

   void OnMidiNote(MidiNote& note) override;
   void OnMidiControl(MidiControl& control) override;
   void OnMidiPitchBend(MidiPitchBend& pitchBend) override;

   MidiDevice* GetDevice() { return &mDevice; }

   void DropdownUpdated(DropdownList* list, int oldVal, double time) override {}

   void LoadLayout(const ofxJSONElement& moduleInfo) override;
   void SetUpFromSaveData() override;
   void SaveState(FileStreamOut& out) override;
   void LoadState(FileStreamIn& in, int rev) override;
   void SaveLayout(ofxJSONElement& moduleInfo) override;
   int GetModuleSaveStateRev() const override { return 1; }

   int GetGridControllerOption1Control() const;
   int GetGridControllerOption2Control() const;

   static bool sDrawingPush2Display;
   static NVGcontext* sVG;
   static NVGLUframebuffer* sFB;
   static void CreateStaticFramebuffer(); //windows was having trouble creating a nanovg context and fbo on the fly
   static IUIControl* sBindToUIControl;

   bool IsEnabled() const override { return true; }

private:
   //IDrawableModule
   void DrawModule() override;
   void DrawModuleUnclipped() override;
   void PostRender() override;
   void GetModuleDimensions(float& width, float& height) override
   {
      width = mWidth;
      height = mHeight + (mShowManualGrid ? 98 : 0);
   }
   void OnClicked(float x, float y, bool right) override;

   bool Initialize();
   void DrawToFramebuffer(NVGcontext* vg, NVGLUframebuffer* fb, float t, float pxRatio);
   void RenderPush2Display();

   void SetModuleGridLights();
   void DrawDisplayModuleControls();
   void DrawLowerModuleSelector();
   void DrawRoutingDisplay();
   void DrawControls(std::vector<IUIControl*> controls, bool sliders, float yPos);
   void UpdateControlList();
   void AddFavoriteControl(IUIControl* control);
   void RemoveFavoriteControl(IUIControl* control);
   void BookmarkModuleToSlot(int slotIndex, IDrawableModule* module);
   void SwitchToBookmarkedModule(int slotIndex);
   int GetPadColorForType(ModuleCategory type, bool enabled) const;
   bool GetGridIndex(int gridX, int gridY, int& gridIndex) const
   {
      gridIndex = gridX + gridY * 8;
      return gridX >= 0 && gridX < 8 && gridY >= 0 && gridY < 8;
   }
   bool IsIgnorableModule(IDrawableModule* module);
   std::vector<IDrawableModule*> SortModules(std::vector<IDrawableModule*> modules);
   void AddModuleChain(IDrawableModule* module, std::vector<IDrawableModule*>& modules, std::vector<IDrawableModule*>& output, int depth);
   void DrawDisplayModuleRect(ofRectangle rect, float thickness);
   std::string GetModuleTypeToSpawn();
   ModuleCategory GetModuleTypeForSpawnList(IUIControl* control);
   ofColor GetSpawnGridColor(int index, ModuleCategory moduleType) const;
   int GetSpawnGridPadColor(int index, ModuleCategory moduleType) const;
   int GetNumDisplayPixels() const;
   bool AllowRepatch() const;
   void UpdateRoutingModules();
   void SetGridControlInterface(IPush2GridController* controller, IDrawableModule* module);

   unsigned char* mPixels{ nullptr };
   const int kPixelRatio = 1;

   const float kColumnSpacing = 121;

   int mFontHandle{ 0 };
   int mFontHandleBold{ 0 };

   float mWidth{ 100 };
   float mHeight{ 20 };

   IDrawableModule* mDisplayModule{ nullptr };
   Snapshots* mDisplayModuleSnapshots{ nullptr };
   ControlRecorder* mCurrentControlRecorder{ nullptr };
   std::vector<IUIControl*> mSliderControls;
   std::vector<IUIControl*> mButtonControls;
   std::vector<IUIControl*> mDisplayedControls;
   bool mDisplayModuleIsShowingOverrideControls{ false };
   int mModuleViewOffset{ 0 };
   float mModuleViewOffsetSmoothed{ 0 };

   std::vector<IDrawableModule*> mModules;
   float mModuleListOffset{ 0 };
   float mModuleListOffsetSmoothed{ 0 };
   std::array<IDrawableModule*, 8 * 8> mModuleGrid;
   std::array<PatchCableSource*, 8 * 8> mModuleGridManualCables;
   ofRectangle mModuleGridRect;

   enum class ModuleGridLayoutStyle
   {
      Automatic,
      Manual
   };

   ModuleGridLayoutStyle mModuleGridLayoutStyle{ ModuleGridLayoutStyle::Automatic };
   DropdownList* mModuleGridLayoutStyleDropdown{ nullptr };
   bool mShowManualGrid{ false };
   Checkbox* mShowManualGridCheckbox{ nullptr };
   std::vector<IUIControl*> mFavoriteControls;
   std::vector<IUIControl*> mSpawnModuleControls;
   bool mNewButtonHeld{ false };
   bool mDeleteButtonHeld{ false };
   bool mLFOButtonHeld{ false };
   bool mAutomateButtonHeld{ false };
   bool mAddModuleBookmarkButtonHeld{ false };
   std::array<bool, 128> mNoteHeldState;
   IDrawableModule* mHeldModule{ nullptr };
   double mModuleHeldTime{ -1 };
   bool mRepatchedHeldModule{ false };
   std::vector<IDrawableModule*> mModuleHistory;
   int mModuleHistoryPosition{ -1 };
   std::vector<IDrawableModule*> mBookmarkSlots;
   bool mInMidiControllerBindMode{ false };
   bool mShiftHeld{ false };
   bool mAddTrackHeld{ false };
   int mHeldKnobIndex{ -1 };
   double mLastResetTime{ -1 };
   int mHeldModulePatchCableIndex{ 0 };
   std::string mTextPopup;
   double mTextPopupTime{ -1 };

   struct Routing
   {
      Routing(IDrawableModule* module, ofColor connectionColor)
      {
         mModule = module;
         mConnectionColor = connectionColor;
      }
      IDrawableModule* mModule;
      ofColor mConnectionColor;
   };

   std::vector<Routing> mRoutingInputModules;
   std::vector<Routing> mRoutingOutputModules;

   enum class ScreenDisplayMode
   {
      kNormal,
      kAddModule,
      kMap,
      kRouting
   };
   ScreenDisplayMode mScreenDisplayMode{ ScreenDisplayMode::kNormal };

   IPush2GridController* mGridControlInterface{ nullptr };
   IDrawableModule* mGridControlModule{ nullptr };
   bool mDisplayModuleCanControlGrid{ false };

   int mLedState[128 * 2]{}; //bottom 128 are notes, top 128 are CCs

   MidiDevice mDevice;

   SpawnListManager mSpawnLists;
   int mPendingSpawnPitch{ -1 };
   int mSelectedGridSpawnListIndex{ -1 };
   std::string mPushBridgeInitErrMsg;
};

//https://raw.githubusercontent.com/Ableton/push-interface/master/doc/MidiMapping.png
class IPush2GridController
{
public:
   virtual ~IPush2GridController() {}
   virtual void OnPush2Connect() {}
   virtual bool OnPush2Control(Push2Control* push2, MidiMessageType type, int controlIndex, float midiValue) = 0;
   virtual void UpdatePush2Leds(Push2Control* push2) = 0;
};
