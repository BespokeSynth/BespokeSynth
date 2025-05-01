#pragma once

#undef LoadString //undo some junk from a windows define

#include <limits>
#include "SynthGlobals.h"
#include "IDrawableModule.h"
#include "TextEntry.h"
#include "RollingBuffer.h"
#include "NamedMutex.h"
#include "ofxJSONElement.h"
#include "ModuleFactory.h"
#include "LocationZoomer.h"
#include "EffectFactory.h"
#include "ModuleContainer.h"
#include "Minimap.h"
#include <thread>

#ifdef BESPOKE_LINUX
#include <climits>
#endif

namespace juce
{
   class AudioDeviceManager;
   class AudioFormatManager;
   class Component;
   class OpenGLContext;
   class String;
   class MouseInputSource;
   class AudioPluginFormatManager;
   class KnownPluginList;
}

class IAudioSource;
class IAudioReceiver;
class INoteReceiver;
class MidiDevice;
class Sample;
class PatchCable;
class MidiController;
class NVGcontext;
class QuickSpawnMenu;
class ADSRDisplay;
class UserPrefsEditor;
class Minimap;
class ScriptWarningPopup;
class NoteOutputQueue;

enum LogEventType
{
   kLogEventType_Verbose,
   kLogEventType_Warning,
   kLogEventType_Error
};

class ConsoleListener : public IDrawableModule, public ITextEntryListener
{
public:
   void GetModuleDimensions(float& width, float& height) override
   {
      width = 1;
      height = 1;
   }
   void TextEntryActivated(TextEntry* entry) override;
   void TextEntryComplete(TextEntry* entry) override;

   bool IsEnabled() const override { return false; }

private:
   void DrawModule() override {}
};

class ModularSynth
{
public:
   ModularSynth();
   virtual ~ModularSynth();

   void Setup(juce::AudioDeviceManager* globalAudioDeviceManager, juce::AudioFormatManager* globalAudioFormatManager, juce::Component* mainComponent, juce::OpenGLContext* openGLContext);
   void LoadResources(void* nanoVG, void* fontBoundsNanoVG);
   void InitIOBuffers(int inputChannelCount, int outputChannelCount);
   void Poll();
   void Draw(void* vg);
   void PostRender();

   void Exit();

   void Focus();

   void KeyPressed(int key, bool isRepeat);
   void KeyReleased(int key);
   void MouseMoved(int x, int y);
   void MouseDragged(int x, int y, int button, const juce::MouseInputSource& source);
   void MousePressed(int x, int y, int button, const juce::MouseInputSource& source);
   void MouseReleased(int x, int y, int button, const juce::MouseInputSource& source);
   void MouseScrolled(float xScroll, float yScroll, bool isSmoothScroll, bool isInvertedScroll, bool canZoomCanvas);
   void MouseMagnify(int x, int y, float scaleFactor, const juce::MouseInputSource& source);
   void FilesDropped(std::vector<std::string> files, int x, int y);

   void AddExtraPoller(IPollable* poller);
   void RemoveExtraPoller(IPollable* poller);

   void AudioOut(float* const* output, int bufferSize, int nChannels);
   void AudioIn(const float* const* input, int bufferSize, int nChannels);

   void OnConsoleInput(std::string command = "");
   void ClearConsoleInput();

   bool IsReady();
   bool IsAudioPaused() const { return mAudioPaused; }
   void SetAudioPaused(bool paused) { mAudioPaused = paused; }

   void AddMidiDevice(MidiDevice* device);
   void ArrangeAudioSourceDependencies();
   IDrawableModule* SpawnModuleOnTheFly(ModuleFactory::Spawnable spawnable, float x, float y, bool addToContainer = true, std::string name = "");

   void SetMoveModule(IDrawableModule* module, float offsetX, float offsetY, bool canStickToCursor);

   int GetNumInputChannels() const { return (int)mInputBuffers.size(); }
   int GetNumOutputChannels() const { return (int)mOutputBuffers.size(); }
   float* GetInputBuffer(int channel);
   float* GetOutputBuffer(int channel);

   IDrawableModule* FindModule(std::string name, bool fail = false);
   IAudioReceiver* FindAudioReceiver(std::string name, bool fail = false);
   INoteReceiver* FindNoteReceiver(std::string name, bool fail = false);
   IUIControl* FindUIControl(std::string path);
   MidiController* FindMidiController(std::string name, bool fail = false);
   void MoveToFront(IDrawableModule* module);
   bool InMidiMapMode();
   void GetAllModules(std::vector<IDrawableModule*>& out) { mModuleContainer.GetAllModules(out); }

   void PushModalFocusItem(IDrawableModule* item);
   void PopModalFocusItem();
   IDrawableModule* GetTopModalFocusItem() const;
   std::vector<IDrawableModule*> GetModalFocusItemStack() const { return mModalFocusItemStack; }
   bool IsModalFocusItem(IDrawableModule* item) const;

   void LogEvent(std::string event, LogEventType type);
   void SetNextDrawTooltip(std::string tooltip) { mNextDrawTooltip = tooltip; }

   bool LoadLayoutFromFile(std::string jsonFile, bool makeDefaultLayout = true);
   bool LoadLayoutFromString(std::string jsonString);
   void LoadLayout(ofxJSONElement json);
   std::string GetLoadedLayout() const { return mLoadedLayoutPath; }
   void ReloadInitialLayout() { mWantReloadInitialLayout = true; }
   bool HasFatalError() { return mFatalError != ""; }

   void AddLissajousDrawer(IDrawableModule* module) { mLissajousDrawers.push_back(module); }
   bool IsLissajousDrawer(IDrawableModule* module) { return VectorContains(module, mLissajousDrawers); }

   void GrabSample(ChannelBuffer* data, std::string name, bool window = false, int numBars = -1);
   void GrabSample(std::string filePath);
   Sample* GetHeldSample() const { return mHeldSample; }
   void ClearHeldSample();

   float GetRawMouseX() { return mMousePos.x; }
   float GetRawMouseY() { return mMousePos.y; }
   float GetMouseX(ModuleContainer* context, float rawX = FLT_MAX);
   float GetMouseY(ModuleContainer* context, float rawY = FLT_MAX);
   void SetMousePosition(ModuleContainer* context, float x, float y);
   bool IsMouseButtonHeld(int button) const;
   ofVec2f& GetDrawOffset() { return mModuleContainer.GetDrawOffsetRef(); }
   void SetDrawOffset(ofVec2f offset) { mModuleContainer.SetDrawOffset(offset); }
   const ofRectangle& GetDrawRect() const { return mDrawRect; }
   void SetPixelRatio(double ratio) { mPixelRatio = ratio; }
   double GetPixelRatio() const { return mPixelRatio; }
   long GetFrameCount() { return mFrameCount; }
   void SetUIScale(float scale) { mUILayerModuleContainer.SetDrawScale(scale); }
   float GetUIScale() { return mUILayerModuleContainer.GetDrawScale(); }
   ModuleContainer* GetRootContainer() { return &mModuleContainer; }
   ModuleContainer* GetUIContainer() { return &mUILayerModuleContainer; }
   bool ShouldShowGridSnap() const;
   bool MouseMovedSignificantlySincePressed() const { return mMouseMovedSignificantlySincePressed; }

   void ZoomView(float zoomAmount, bool fromMouse);
   void SetZoomLevel(float zoomLevel);
   void PanView(float x, float y);
   void PanTo(float x, float y);
   void SetRawSpaceMouseTwist(float twist, bool isUsing)
   {
      mSpaceMouseInfo.mTwist = twist;
      mSpaceMouseInfo.mUsingTwist = isUsing;
   }
   void SetRawSpaceMouseZoom(float zoom, bool isUsing)
   {
      mSpaceMouseInfo.mZoom = zoom;
      mSpaceMouseInfo.mUsingZoom = isUsing;
   }
   void SetRawSpaceMousePan(float x, float y, bool isUsing)
   {
      mSpaceMouseInfo.mPan.set(x, y);
      mSpaceMouseInfo.mUsingPan = isUsing;
   }

   void SetResizeModule(IDrawableModule* module) { mResizeModule = module; }

   void SetGroupSelectContext(ModuleContainer* context) { mGroupSelectContext = context; }
   bool IsGroupSelecting() const { return mGroupSelectContext != nullptr; }

   IDrawableModule* GetMoveModule() { return mMoveModule; }
   ModuleFactory* GetModuleFactory() { return &mModuleFactory; }
   juce::AudioDeviceManager& GetAudioDeviceManager() { return *mGlobalAudioDeviceManager; }
   juce::AudioFormatManager& GetAudioFormatManager() { return *mGlobalAudioFormatManager; }
   juce::AudioPluginFormatManager& GetAudioPluginFormatManager() { return *mAudioPluginFormatManager.get(); }
   juce::KnownPluginList& GetKnownPluginList() { return *mKnownPluginList.get(); }
   juce::Component* GetMainComponent() { return mMainComponent; }
   juce::OpenGLContext* GetOpenGLContext() { return mOpenGLContext; }
   IDrawableModule* GetLastClickedModule() const;
   EffectFactory* GetEffectFactory() { return &mEffectFactory; }
   const std::vector<IDrawableModule*>& GetGroupSelectedModules() const { return mGroupSelectedModules; }
   bool ShouldAccentuateActiveModules() const;
   bool ShouldDimModule(IDrawableModule* module);
   LocationZoomer* GetLocationZoomer() { return &mZoomer; }
   IDrawableModule* GetModuleAtCursor(int offsetX = 0, int offsetY = 0);

   void RegisterPatchCable(PatchCable* cable);
   void UnregisterPatchCable(PatchCable* cable);

   template <class T>
   std::vector<std::string> GetModuleNames() { return mModuleContainer.GetModuleNames<T>(); }

   void LockRender(bool lock)
   {
      if (lock)
      {
         mRenderLock.lock();
      }
      else
      {
         mRenderLock.unlock();
      }
   }
   void UpdateFrameRate(float fps) { mFrameRate = fps; }
   float GetFrameRate() const { return mFrameRate; }
   std::recursive_mutex& GetRenderLock() { return mRenderLock; }
   NamedMutex* GetAudioMutex() { return &mAudioThreadMutex; }
   static std::thread::id GetAudioThreadID() { return sAudioThreadId; }
   NoteOutputQueue* GetNoteOutputQueue() { return mNoteOutputQueue; }

   IDrawableModule* CreateModule(const ofxJSONElement& moduleInfo);
   void SetUpModule(IDrawableModule* module, const ofxJSONElement& moduleInfo);
   void OnModuleAdded(IDrawableModule* module);
   void OnModuleDeleted(IDrawableModule* module);
   void AddDynamicModule(IDrawableModule* module);

   void ScheduleEnvelopeEditorSpawn(ADSRDisplay* adsrDisplay);

   bool IsLoadingState() const { return mIsLoadingState; }
   bool IsLoadingModule() const { return mIsLoadingModule; }
   bool IsDuplicatingModule() const { return mIsDuplicatingModule; }
   void SetIsLoadingState(bool loading) { mIsLoadingState = loading; }

   static std::string GetUserPrefsPath();
   static void CrashHandler(void*);
   static void DumpStats(bool isCrash, void* crashContext);

   void SaveLayout(std::string jsonFile = "", bool makeDefaultLayout = true);
   ofxJSONElement GetLayout();
   void SaveLayoutAsPopup();
   void SaveOutput();
   void SaveState(std::string file, bool autosave);
   void LoadState(std::string file);
   void SetStartupSaveStateFile(std::string bskPath);
   void SaveCurrentState();
   void SaveStatePopup();
   void LoadStatePopup();
   void ToggleQuickSpawn();
   QuickSpawnMenu* GetQuickSpawn() { return mQuickSpawn; }
   std::string GetLastSavePath() { return mCurrentSaveStatePath; }

   UserPrefsEditor* GetUserPrefsEditor() { return mUserPrefsEditor; }
   juce::Component* GetFileChooserParent() const;

   const juce::String& GetTextFromClipboard() const;
   void CopyTextToClipboard(const juce::String& text);

   void SetFatalError(std::string error);

   static bool sShouldAutosave;
   static float sBackgroundLissajousR;
   static float sBackgroundLissajousG;
   static float sBackgroundLissajousB;
   static float sBackgroundR;
   static float sBackgroundG;
   static float sBackgroundB;
   static float sCableAlpha;

   static int sLoadingFileSaveStateRev;
   static int sLastLoadedFileSaveStateRev;
   static constexpr int kSaveStateRev = 426;

private:
   void ResetLayout();
   void ReconnectMidiDevices();
   void DrawConsole();
   void CheckClick(IDrawableModule* clickedModule, float x, float y, bool rightButton);
   void UpdateUserPrefsLayout();
   void LoadStatePopupImp();
   IDrawableModule* DuplicateModule(IDrawableModule* module);
   void DeleteAllModules();
   void TriggerClapboard();
   void DoAutosave();
   void FindCircularDependencies();
   bool FindCircularDependencySearch(std::list<IAudioSource*> chain, IAudioSource* searchFrom);
   void ClearCircularDependencyMarkers();
   bool IsCurrentSaveStateATemplate() const;

   void ReadClipboardTextFromSystem();

   int mIOBufferSize{ 0 };

   std::vector<IAudioSource*> mSources;
   std::vector<IDrawableModule*> mLissajousDrawers;
   std::vector<IDrawableModule*> mDeletedModules;
   bool mHasCircularDependency{ false };

   std::vector<IDrawableModule*> mModalFocusItemStack;

   IDrawableModule* mMoveModule{ nullptr };
   int mMoveModuleOffsetX{ 0 };
   int mMoveModuleOffsetY{ 0 };
   bool mMoveModuleCanStickToCursor{ false }; //if the most current mMoveModule can stick to the cursor if you release the mouse button before moving it

   ofVec2f mLastMoveMouseScreenPos;
   ofVec2f mLastMouseDragPos;
   bool mIsMousePanning{ false };
   std::array<bool, 5> mIsMouseButtonHeld{ false };
   struct SpaceMouseInfo
   {
      float mTwist{ 0 };
      float mZoom{ 0 };
      ofVec2f mPan{ 0, 0 };
      bool mUsingTwist{ false };
      bool mUsingZoom{ false };
      bool mUsingPan{ false };
   };
   SpaceMouseInfo mSpaceMouseInfo;

   char mConsoleText[MAX_TEXTENTRY_LENGTH]{ 0 };
   TextEntry* mConsoleEntry{ nullptr };
   ConsoleListener* mConsoleListener{ nullptr };

   std::vector<MidiDevice*> mMidiDevices;

   LocationZoomer mZoomer;
   QuickSpawnMenu* mQuickSpawn{ nullptr };
   std::unique_ptr<Minimap> mMinimap{ nullptr };
   UserPrefsEditor* mUserPrefsEditor{ nullptr };

   RollingBuffer* mGlobalRecordBuffer{ nullptr };
   int mRecordingLength{ 0 };

   struct LogEventItem
   {
      LogEventItem(double _time, std::string _text, LogEventType _type)
      : time(_time)
      , text(_text)
      , type(_type)
      {}
      double time{ 0 };
      std::string text;
      LogEventType type{ LogEventType::kLogEventType_Verbose };
   };
   std::list<LogEventItem> mEvents;
   std::list<std::string> mErrors;

   NamedMutex mAudioThreadMutex;
   static std::thread::id sAudioThreadId;
   NoteOutputQueue* mNoteOutputQueue{ nullptr };

   bool mAudioPaused{ false };
   bool mIsLoadingState{ false };
   bool mArrangeDependenciesWhenLoadCompletes{ false };

   ModuleFactory mModuleFactory;
   EffectFactory mEffectFactory;

   int mClickStartX{ std::numeric_limits<int>::max() }; //to detect click and release in place
   int mClickStartY{ std::numeric_limits<int>::max() };
   bool mMouseMovedSignificantlySincePressed{ true };
   bool mLastClickWasEmptySpace{ false };
   bool mIsShiftPressed{ false };
   double mLastShiftPressTime{ -9999 };
   ofVec2f mLastShiftPressMousePos{};

   std::string mLoadedLayoutPath;
   bool mWantReloadInitialLayout{ false };
   std::string mCurrentSaveStatePath;
   std::string mStartupSaveStateFile;

   Sample* mHeldSample{ nullptr };

   IDrawableModule* mLastClickedModule{ nullptr };
   bool mInitialized{ false };

   ofRectangle mDrawRect;

   std::vector<IDrawableModule*> mGroupSelectedModules;
   ModuleContainer* mGroupSelectContext{ nullptr };
   bool mHasDuplicatedDuringDrag{ false };
   bool mHasAutopatchedToTargetDuringDrag{ false };

   IDrawableModule* mResizeModule{ nullptr };

   bool mShowLoadStatePopup{ false };

   std::vector<PatchCable*> mPatchCables;

   ofMutex mKeyInputMutex;
   std::vector<std::pair<int, bool> > mQueuedKeyInput;

   ofVec2f mMousePos;
   std::string mNextDrawTooltip;
   bool mHideTooltipsUntilMouseMove{ false };

   juce::AudioDeviceManager* mGlobalAudioDeviceManager{ nullptr };
   juce::AudioFormatManager* mGlobalAudioFormatManager{ nullptr };
   juce::Component* mMainComponent{ nullptr };
   juce::OpenGLContext* mOpenGLContext{ nullptr };

   std::recursive_mutex mRenderLock;
   float mFrameRate{ 0 };
   long mFrameCount{ 0 };

   ModuleContainer mModuleContainer;
   ModuleContainer mUILayerModuleContainer;

   ADSRDisplay* mScheduledEnvelopeEditorSpawnDisplay{ nullptr };

   bool mIsLoadingModule{ false };
   bool mIsDuplicatingModule{ false };

   std::list<IPollable*> mExtraPollers;

   std::string mFatalError;

   double mLastClapboardTime{ -9999 };

   double mPixelRatio{ 1 };

   std::vector<float*> mInputBuffers;
   std::vector<float*> mOutputBuffers;

   std::unique_ptr<juce::AudioPluginFormatManager> mAudioPluginFormatManager;
   std::unique_ptr<juce::KnownPluginList> mKnownPluginList;
};

extern ModularSynth* TheSynth;
