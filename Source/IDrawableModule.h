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
//  IDrawableModule.h
//  modularSynth
//
//  Created by Ryan Challinor on 11/25/12.
//
//

#pragma once

#include "IClickable.h"
#include "IPollable.h"
#include "ModuleSaveData.h"
#include "IPatchable.h"

class Checkbox;
class IUIControl;
class FileStreamIn;
class FileStreamOut;
class FloatSlider;
class RollingBuffer;
class ofxJSONElement;
class Sample;
class PatchCable;
class PatchCableSource;
class ModuleContainer;
class UIGrid;

enum ModuleCategory
{
   kModuleCategory_Note,
   kModuleCategory_Synth,
   kModuleCategory_Audio,
   kModuleCategory_Instrument,
   kModuleCategory_Processor,
   kModuleCategory_Modulator,
   kModuleCategory_Pulse,
   kModuleCategory_Other,
   kModuleCategory_Unknown
};

struct PatchCableOld
{
   ofVec2f start;
   ofVec2f end;
   ofVec2f plug;
};

class IDrawableModule : public IClickable, public IPollable, public virtual IPatchable
{
public:
   IDrawableModule();
   virtual ~IDrawableModule();
   static bool CanCreate() { return true; }
   static bool AcceptsAudio() { return false; }
   static bool AcceptsNotes() { return false; }
   static bool AcceptsPulses() { return false; }

   void Render() override;
   void RenderUnclipped();
   virtual void PostRender() {}
   void DrawFrame(float width, float height, bool drawModule, float& titleBarHeight, float& highlight);
   void DrawPatchCables(bool parentMinimized, bool inFront);
   bool CheckNeedsDraw() override;
   virtual bool AlwaysOnTop() { return false; }
   void ToggleMinimized();
   void SetMinimized(bool minimized, bool animate = true)
   {
      if (!HasTitleBar())
         return;
      mMinimized = minimized;
      if (!animate)
         mMinimizeAnimation = minimized ? 1 : 0;
   }
   void TogglePinned();
   void SetPinned(bool pinned);
   bool Pinned() const { return mPinned; }
   virtual void KeyPressed(int key, bool isRepeat);
   virtual void KeyReleased(int key);
   void DrawConnection(IClickable* target);
   void AddUIControl(IUIControl* control);
   void RemoveUIControl(IUIControl* control, bool cleanUpReferences = true);
   void AddUIGrid(UIGrid* grid);
   IUIControl* FindUIControl(const char* name, bool fail = true) const;
   std::vector<IUIControl*> GetUIControls() const;
   std::vector<UIGrid*> GetUIGrids() const;
   virtual void OnUIControlRequested(const char* name) {}
   void AddChild(IDrawableModule* child);
   void RemoveChild(IDrawableModule* child);
   IDrawableModule* FindChild(const std::string name, bool fail) const;
   void GetDimensions(float& width, float& height) override;
   virtual void GetModuleDimensions(float& width, float& height)
   {
      width = 10;
      height = 10;
   }
   virtual void Init();
   virtual void Exit();
   bool IsInitialized() const { return mInitialized; }
   bool Minimized() const { return mMinimizeAnimation > 0; }
   bool WasMinimizeAreaClicked() const { return mWasMinimizeAreaClicked; }
   virtual void MouseReleased() override;
   virtual void FilesDropped(std::vector<std::string> files, int x, int y) {}
   virtual std::string GetTitleLabel() const { return Name(); }
   virtual bool HasTitleBar() const { return true; }
   static float TitleBarHeight() { return mTitleBarHeight; }
   static ofColor GetColor(ModuleCategory type);
   virtual void SetEnabled(bool enabled) {}
   virtual bool IsEnabled() const { return true; }
   virtual bool CanMinimize() { return true; }
   virtual void SampleDropped(int x, int y, Sample* sample) {}
   virtual bool CanDropSample() const { return false; }
   void BasePoll(); //calls poll, using this to guarantee base poll is always called
   bool IsWithinRect(const ofRectangle& rect);
   bool IsVisible();
   std::vector<IDrawableModule*> GetChildren() const { return mChildren; }
   virtual bool IsResizable() const { return false; }
   virtual void Resize(float width, float height) { assert(false); }
   bool IsHoveringOverResizeHandle() const { return mHoveringOverResizeHandle; }
   void SetTypeName(std::string type, ModuleCategory category)
   {
      mTypeName = type;
      mModuleCategory = category;
   }
   void SetTarget(IClickable* target);
   void SetUpPatchCables(std::string targets);
   void AddPatchCableSource(PatchCableSource* source);
   void RemovePatchCableSource(PatchCableSource* source);
   bool TestClick(float x, float y, bool right, bool testOnly = false) override;
   std::string GetTypeName() const { return mTypeName; }
   ModuleCategory GetModuleCategory() const { return mModuleCategory; }
   virtual bool IsSingleton() const { return false; }
   virtual bool CanBeDeleted() const { return (IsSingleton() ? false : true); }
   virtual bool HasSpecialDelete() const { return false; }
   virtual void DoSpecialDelete() {}
   void ComputeSliders(int samplesIn);
   void SetOwningContainer(ModuleContainer* container) { mOwningContainer = container; }
   ModuleContainer* GetOwningContainer() const { return mOwningContainer; }
   virtual ModuleContainer* GetContainer() { return nullptr; }
   void SetShouldDrawOutline(bool should) { mShouldDrawOutline = should; }
   ofVec2f GetMinimumDimensions();
   bool HasEnabledCheckbox() const { return mEnabledCheckbox != nullptr; }
   Checkbox* GetEnabledCheckbox() const { return mEnabledCheckbox; }
   void MarkAsDeleted() { mDeleted = true; }
   bool IsDeleted() const { return mDeleted; }
   virtual bool ShouldClipContents() { return true; }
   bool CanReceiveAudio() { return mCanReceiveAudio; }
   bool CanReceiveNotes() { return mCanReceiveNotes; }
   bool CanReceivePulses() { return mCanReceivePulses; }
   virtual bool ShouldSuppressAutomaticOutputCable() { return false; }
   virtual bool ShouldSerializeForSnapshot() { return false; }

   virtual void CheckboxUpdated(Checkbox* checkbox, double time) {}

   virtual void LoadBasics(const ofxJSONElement& moduleInfo, std::string typeName);
   virtual void CreateUIControls();
   void LoadLayoutBase(const ofxJSONElement& moduleInfo);
   void SaveLayoutBase(ofxJSONElement& moduleInfo);
   void SetUpFromSaveDataBase();
   virtual bool IsSaveable() { return true; }
   ModuleSaveData& GetSaveData() { return mModuleSaveData; }
   virtual void SaveState(FileStreamOut& out);
   virtual void LoadState(FileStreamIn& in, int rev);
   int LoadModuleSaveStateRev(FileStreamIn& in);
   virtual int GetModuleSaveStateRev() const { return -1; }
   virtual void PostLoadState() {}
   virtual std::vector<IUIControl*> ControlsToNotSetDuringLoadState() const;
   virtual std::vector<IUIControl*> ControlsToIgnoreInSaveState() const;
   virtual void UpdateOldControlName(std::string& oldName) {}
   virtual bool LoadOldControl(FileStreamIn& in, std::string& oldName) { return false; }
   virtual bool CanModuleTypeSaveState() const { return true; }
   bool IsSpawningOnTheFly(const ofxJSONElement& moduleInfo);
   virtual bool HasDebugDraw() const { return false; }
   virtual bool HasPush2OverrideControls() const { return false; }
   virtual void GetPush2OverrideControls(std::vector<IUIControl*>& controls) const {}
   virtual bool DrawToPush2Screen() { return false; }

   //IPatchable
   PatchCableSource* GetPatchCableSource(int index = 0) override
   {
      if (index < mPatchCableSources.size())
         return mPatchCableSources[index];
      return nullptr;
   }
   std::vector<PatchCableSource*> GetPatchCableSources() { return mPatchCableSources; }

   static void FindClosestSides(float xThis, float yThis, float wThis, float hThis, float xThat, float yThat, float wThat, float hThat, float& startX, float& startY, float& endX, float& endY, bool sidesOnly = false);

   static float sHueNote;
   static float sHueAudio;
   static float sHueInstrument;
   static float sHueNoteSource;
   static float sSaturation;
   static float sBrightness;

   bool mDrawDebug{ false };

   static constexpr int kMaxOutputsPerPatchCableSource = 32;

protected:
   void Poll() override {}
   void OnClicked(float x, float y, bool right) override;
   bool MouseMoved(float x, float y) override;

   void AddDebugLine(std::string text, int maxLines);

   ModuleSaveData mModuleSaveData;
   Checkbox* mEnabledCheckbox{ nullptr };
   bool mEnabled{ true };
   ModuleCategory mModuleCategory{ ModuleCategory::kModuleCategory_Unknown };
   std::string mDebugDisplayText;

private:
   virtual void PreDrawModule() {}
   virtual void DrawModule() = 0;
   virtual void DrawModuleUnclipped() {}
   float GetMinimizedWidth();
   PatchCableOld GetPatchCableOld(IClickable* target);
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) {}
   virtual void SaveLayout(ofxJSONElement& moduleInfo) {}
   virtual void SetUpFromSaveData() {}
   virtual bool ShouldSavePatchCableSources() const { return true; }
   void ForcePosition();

   std::vector<IUIControl*> mUIControls;
   std::vector<IDrawableModule*> mChildren;
   std::vector<FloatSlider*> mFloatSliders;
   std::vector<UIGrid*> mUIGrids;
   static const int mTitleBarHeight = 12;
   std::string mTypeName;
   static const int sResizeCornerSize = 8;
   ModuleContainer* mOwningContainer{ nullptr };

   bool mMinimized{ false };
   bool mPinned{ false };
   ofVec2f mPinnedPosition{ 0, 0 };
   bool mWasMinimizeAreaClicked{ false };
   float mMinimizeAnimation{ 0 };
   bool mUIControlsCreated{ false };
   bool mInitialized{ false };
   std::string mLastTitleLabel;
   float mTitleLabelWidth{ 0 };
   bool mShouldDrawOutline{ true };
   bool mHoveringOverResizeHandle{ false };
   bool mDeleted{ false };
   bool mCanReceiveAudio{ false };
   bool mCanReceiveNotes{ false };
   bool mCanReceivePulses{ false };
   IKeyboardFocusListener* mKeyboardFocusListener{ nullptr };

   ofMutex mSliderMutex;

   std::vector<PatchCableSource*> mPatchCableSources;
};
