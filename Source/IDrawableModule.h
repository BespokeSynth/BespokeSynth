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

#ifndef modularSynth_IDrawableModule_h
#define modularSynth_IDrawableModule_h

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

enum ModuleType
{
   kModuleType_Note,
   kModuleType_Synth,
   kModuleType_Audio,
   kModuleType_Instrument,
   kModuleType_Processor,
   kModuleType_Modulator,
   kModuleType_Pulse,
   kModuleType_Other,
   kModuleType_Unknown
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
   
   void Render() override;
   void RenderUnclipped();
   virtual void PostRender() {}
   void DrawFrame(float width, float height, bool drawModule, float& titleBarHeight, float& highlight);
   void DrawPatchCables(bool parentMinimized);
   bool CheckNeedsDraw() override;
   virtual bool AlwaysOnTop() { return false; }
   void ToggleMinimized();
   void SetMinimized(bool minimized) { if (HasTitleBar()) mMinimized = minimized; }
   virtual void KeyPressed(int key, bool isRepeat);
   virtual void KeyReleased(int key);
   void DrawConnection(IClickable* target);
   void AddUIControl(IUIControl* control);
   void RemoveUIControl(IUIControl* control);
   IUIControl* FindUIControl(const char* name, bool fail = true) const;
   std::vector<IUIControl*> GetUIControls() const;
   void AddChild(IDrawableModule* child);
   void RemoveChild(IDrawableModule* child);
   IDrawableModule* FindChild(const char* name) const;
   void GetDimensions(float& width, float& height) override;
   virtual void GetModuleDimensions(float& width, float& height) { width = 10; height = 10; }
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
   static ofColor GetColor(ModuleType type);
   virtual void SetEnabled(bool enabled) {}
   virtual bool CanMinimize() { return true; }
   virtual void SampleDropped(int x, int y, Sample* sample) {}
   virtual bool CanDropSample() const { return false; }
   void BasePoll();  //calls poll, using this to guarantee base poll is always called
   bool IsWithinRect(const ofRectangle& rect);
   bool IsVisible();
   std::vector<IDrawableModule*> GetChildren() const { return mChildren; }
   virtual bool IsResizable() const { return false; }
   virtual void Resize(float width, float height) { assert(false); }
   void SetTypeName(std::string type) { mTypeName = type; }
   void SetTarget(IClickable* target);
   void SetUpPatchCables(std::string targets);
   void AddPatchCableSource(PatchCableSource* source);
   void RemovePatchCableSource(PatchCableSource* source);
   bool TestClick(int x, int y, bool right, bool testOnly = false) override;
   std::string GetTypeName() const { return mTypeName; }
   ModuleType GetModuleType() const { return mModuleType; }
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
   bool HasEnableCheckbox() const { return mEnabledCheckbox != nullptr; }
   void MarkAsDeleted() { mDeleted = true; }
   bool IsDeleted() const { return mDeleted; }
   virtual bool ShouldClipContents() { return true; }
   bool CanReceiveAudio() { return mCanReceiveAudio; }
   bool CanReceiveNotes() { return mCanReceiveNotes; }
   bool CanReceivePulses() { return mCanReceivePulses; }
   
   virtual void CheckboxUpdated(Checkbox* checkbox) {}
   
   virtual void LoadBasics(const ofxJSONElement& moduleInfo, std::string typeName);
   virtual void CreateUIControls();
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) {}
   virtual void SaveLayout(ofxJSONElement& moduleInfo);
   virtual void SetUpFromSaveData() {}
   virtual bool IsSaveable() { return true; }
   ModuleSaveData& GetSaveData() { return mModuleSaveData; }
   virtual void SaveState(FileStreamOut& out);
   virtual void LoadState(FileStreamIn& in);
   virtual void PostLoadState() {}
   virtual std::vector<IUIControl*> ControlsToNotSetDuringLoadState() const;
   virtual std::vector<IUIControl*> ControlsToIgnoreInSaveState() const;
   virtual void UpdateOldControlName(std::string& oldName) {}
   virtual bool LoadOldControl(FileStreamIn& in, std::string& oldName) { return false; }
   virtual bool CanSaveState() const { return true; }
   virtual size_t GetExpectedSaveStateNumChildren() const { return mChildren.size(); }
   virtual bool HasDebugDraw() const { return false; }
   virtual bool HasPush2OverrideControls() const { return false; }
   virtual void GetPush2OverrideControls(std::vector<IUIControl*>& controls) const { }
   
   //IPatchable
   PatchCableSource* GetPatchCableSource(int index=0) override { if (index == 0) return mMainPatchCableSource; else return mPatchCableSources[index]; }
   std::vector<PatchCableSource*> GetPatchCableSources() { return mPatchCableSources; }
   
   static void FindClosestSides(float xThis, float yThis, float wThis, float hThis, float xThat, float yThat, float wThat, float hThat, float& startX, float& startY, float& endX, float& endY, bool sidesOnly = false);
   
   static float sHueNote;
   static float sHueAudio;
   static float sHueInstrument;
   static float sHueNoteSource;
   static float sSaturation;
   static float sBrightness;
   
   bool mDrawDebug;

protected:
   virtual void Poll() override {}
   virtual void OnClicked(int x, int y, bool right) override;
   virtual bool MouseMoved(float x, float y) override;
   
   ModuleSaveData mModuleSaveData;
   Checkbox* mEnabledCheckbox;
   bool mEnabled;
   ModuleType mModuleType;

private:
   virtual void PreDrawModule() {}
   virtual void DrawModule() = 0;
   virtual void DrawModuleUnclipped() {}
   virtual bool Enabled() const { return true; }
   float GetMinimizedWidth();
   PatchCableOld GetPatchCableOld(IClickable* target);

   std::vector<IUIControl*> mUIControls;
   std::vector<IDrawableModule*> mChildren;
   std::vector<FloatSlider*> mFloatSliders;
   static const int mTitleBarHeight = 12;
   std::string mTypeName;
   static const int sResizeCornerSize = 8;
   ModuleContainer* mOwningContainer;

   bool mMinimized;
   bool mWasMinimizeAreaClicked;
   float mMinimizeAnimation;
   bool mUIControlsCreated;
   bool mInitialized;
   std::string mLastTitleLabel;
   float mTitleLabelWidth;
   bool mShouldDrawOutline;
   bool mHoveringOverResizeHandle;
   bool mDeleted;
   bool mCanReceiveAudio;
   bool mCanReceiveNotes;
   bool mCanReceivePulses;

   ofMutex mSliderMutex;
   
   PatchCableSource* mMainPatchCableSource;
   std::vector<PatchCableSource*> mPatchCableSources;
};

#endif
