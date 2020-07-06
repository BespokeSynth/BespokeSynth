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
#include "Checkbox.h"
#include "FileStream.h"
#include "IPatchable.h"

class IUIControl;
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
   virtual void PostRender() {}
   void DrawFrame(float width, float height, bool drawModule, float& titleBarHeight, float& highlight);
   void DrawPatchCables();
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
   vector<IUIControl*> GetUIControls() const;
   void AddChild(IDrawableModule* child);
   void RemoveChild(IDrawableModule* child);
   IDrawableModule* FindChild(const char* name) const;
   void GetDimensions(float& width, float& height) override;
   virtual void GetModuleDimensions(float& width, float& height) { width = 10; height = 10; }
   virtual void Init();
   virtual void Exit();
   bool Minimized() const { return mMinimizeAnimation > 0; }
   virtual void MouseReleased() override;
   virtual void FilesDropped(vector<string> files, int x, int y) {}
   virtual string GetTitleLabel() { return "&&&fixme&&&"; }
   virtual bool HasTitleBar() const { return true; }
   static float TitleBarHeight() { return mTitleBarHeight; }
   static ofColor GetColor(ModuleType type);
   virtual void SetEnabled(bool enabled) {}
   virtual bool CanMinimize() { return true; }
   virtual void SampleDropped(int x, int y, Sample* sample) {}
   void BasePoll();  //calls poll, using this to guarantee base poll is always called
   bool IsWithinRect(const ofRectangle& rect);
   bool IsVisible();
   vector<IDrawableModule*> GetChildren() const { return mChildren; }
   virtual bool IsResizable() const { return false; }
   virtual void Resize(float width, float height) { assert(false); }
   void SetType(string type) { mTypeName = type; }
   void SetTarget(IClickable* target);
   void SetUpPatchCables(string targets);
   void AddPatchCableSource(PatchCableSource* source);
   void RemovePatchCableSource(PatchCableSource* source);
   bool TestClick(int x, int y, bool right, bool testOnly = false) override;
   string GetTypeName() const { return mTypeName; }
   ModuleType GetModuleType() const { return mModuleType; }
   virtual bool IsSingleton() const { return false; }
   void ComputeSliders(int samplesIn);
   void SetOwningContainer(ModuleContainer* container) { mOwningContainer = container; }
   ModuleContainer* GetOwningContainer() const { return mOwningContainer; }
   virtual ModuleContainer* GetContainer() { return nullptr; }
   void SetShouldDrawOutline(bool should) { mShouldDrawOutline = should; }
   ofVec2f GetMinimumDimensions();
   
   virtual void CheckboxUpdated(Checkbox* checkbox) {}
   
   virtual void LoadBasics(const ofxJSONElement& moduleInfo, string typeName);
   virtual void CreateUIControls();
   virtual void LoadLayout(const ofxJSONElement& moduleInfo) {}
   virtual void SaveLayout(ofxJSONElement& moduleInfo);
   virtual void SetUpFromSaveData() {}
   virtual bool IsSaveable() { return true; }
   ModuleSaveData& GetSaveData() { return mModuleSaveData; }
   virtual void SaveState(FileStreamOut& out);
   virtual void LoadState(FileStreamIn& in);
   virtual void PostLoadState() {}
   virtual vector<IUIControl*> ControlsToNotSetDuringLoadState() const;
   virtual vector<IUIControl*> ControlsToIgnoreInSaveState() const;
   virtual bool CanSaveState() const { return true; }
   virtual bool HasDebugDraw() const { return false; }
   
   //IPatchable
   PatchCableSource* GetPatchCableSource(int index=0) override { if (index == 0) return mMainPatchCableSource; else return mPatchCableSources[index]; }
   vector<PatchCableSource*> GetPatchCableSources() { return mPatchCableSources; }
   
   static void FindClosestSides(float xThis,float yThis,float wThis,float hThis,float xThat,float yThat,float wThat,float hThat, float& startX,float& startY,float& endX,float& endY);
   
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

   vector<IUIControl*> mUIControls;
   vector<IDrawableModule*> mChildren;
   vector<FloatSlider*> mFloatSliders;
   static const int mTitleBarHeight = 12;
   string mTypeName;
   static const int sResizeCornerSize = 8;
   ModuleContainer* mOwningContainer;

   bool mMinimized;
   bool mMinimizeAreaClicked;
   float mMinimizeAnimation;
   bool mUIControlsCreated;
   bool mInitialized;
   string mLastTitleLabel;
   float mTitleLabelWidth;
   bool mShouldDrawOutline;

   ofMutex mSliderMutex;
   
   PatchCableSource* mMainPatchCableSource;
   vector<PatchCableSource*> mPatchCableSources;
};

#endif
