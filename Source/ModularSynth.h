#ifndef _MODULAR_SYNTH
#define _MODULAR_SYNTH

#include <JuceHeader.h>

#undef LoadString //undo some junk from a windows define

#include "SynthGlobals.h"
#include "IAudioReceiver.h"
#include "IDrawableModule.h"
#include "TextEntry.h"
#include "RollingBuffer.h"
#include "NamedMutex.h"
#include "ofxJSONElement.h"
#include "ModuleFactory.h"
#include "LocationZoomer.h"
#include "EffectFactory.h"
#include "ModuleContainer.h"
#ifdef BESPOKE_LINUX
#include <climits>
#endif

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

enum LogEventType
{
   kLogEventType_Verbose,
   kLogEventType_Warning,
   kLogEventType_Error
};

class ConsoleListener : public IDrawableModule, public ITextEntryListener
{
public:
   void GetModuleDimensions(float& width, float& height) override { width = 1; height = 1; }
   void TextEntryActivated(TextEntry* entry) override;
   void TextEntryComplete(TextEntry* entry) override;
private:
   void DrawModule() override {}
   bool Enabled() const override { return false; }
};

class ModularSynth
{
public:
   ModularSynth();
   virtual ~ModularSynth();
   
   void Setup(GlobalManagers* globalManagers, juce::Component* mainComponent, juce::OpenGLContext* openGLContext);
   void LoadResources(void* nanoVG, void* fontBoundsNanoVG);
   void InitIOBuffers(int inputChannelCount, int outputChannelCount);
   void Poll();
   void Draw(void* vg);
   void PostRender();
   
   void Exit();
   
   void KeyPressed(int key, bool isRepeat);
   void KeyReleased(int key);
   void MouseMoved(int x, int y );
   void MouseDragged(int x, int y, int button);
   void MousePressed(int x, int y, int button);
   void MouseReleased(int x, int y, int button);
   void MouseScrolled(float x, float y, bool canZoomCanvas);
   void MouseMagnify(int x, int y, float scaleFactor);
   void FilesDropped(vector<string> files, int x, int y);
   
   void AddExtraPoller(IPollable* poller);
   void RemoveExtraPoller(IPollable* poller);
   
   void AudioOut(float** output, int bufferSize, int nChannels);
   void AudioIn(const float** input, int bufferSize, int nChannels);

   void OnConsoleInput();
   void ClearConsoleInput();
   
   bool IsReady();
   bool IsAudioPaused() const { return mAudioPaused; }
   void ToggleAudioPaused() { mAudioPaused = !mAudioPaused; }
   
   void AddMidiDevice(MidiDevice* device);
   void ArrangeAudioSourceDependencies();
   IDrawableModule* SpawnModuleOnTheFly(string moduleName, float x, float y, bool addToContainer = true);
   void SetMoveModule(IDrawableModule* module, float offsetX, float offsetY);
   
   int GetNumInputChannels() const { return (int)mInputBuffers.size(); }
   int GetNumOutputChannels() const { return (int)mOutputBuffers.size(); }
   float* GetInputBuffer(int channel);
   float* GetOutputBuffer(int channel);
   
   IDrawableModule* FindModule(string name, bool fail = false);
   IAudioReceiver* FindAudioReceiver(string name, bool fail = false);
   INoteReceiver* FindNoteReceiver(string name, bool fail = false);
   IUIControl* FindUIControl(string path);
   MidiController* FindMidiController(string name, bool fail = false);
   void MoveToFront(IDrawableModule* module);
   bool InMidiMapMode();
   void GetAllModules(vector<IDrawableModule*>& out) { mModuleContainer.GetAllModules(out); }
   
   void PushModalFocusItem(IDrawableModule* item);
   void PopModalFocusItem();
   IDrawableModule* GetTopModalFocusItem() const;
   vector<IDrawableModule*> GetModalFocusItemStack() const { return mModalFocusItemStack; }
   bool IsModalFocusItem(IDrawableModule* item) const;
   
   void LogEvent(string event, LogEventType type);
   void SetNextDrawTooltip(string tooltip) { mNextDrawTooltip = tooltip; }
   
   bool LoadLayoutFromFile(string jsonFile, bool makeDefaultLayout = true);
   bool LoadLayoutFromString(string jsonString);
   void LoadLayout(ofxJSONElement json);
   string GetLoadedLayout() const { return mLoadedLayoutPath; }
   void ReloadInitialLayout() { mWantReloadInitialLayout = true; }
   
   void AddLissajousDrawer(IDrawableModule* module) { mLissajousDrawers.push_back(module); }
   bool IsLissajousDrawer(IDrawableModule* module) { return VectorContains(module, mLissajousDrawers); }
   
   void GrabSample(ChannelBuffer* data, bool window = false, int numBars = -1);
   void GrabSample(string filePath);
   Sample* GetHeldSample() const { return mHeldSample; }
   void ClearHeldSample();
   
   float GetRawMouseX() { return mMousePos.x; }
   float GetRawMouseY() { return mMousePos.y; }
   float GetMouseX(ModuleContainer* context, float rawX = FLT_MAX);
   float GetMouseY(ModuleContainer* context, float rawY = FLT_MAX);
   bool IsMouseButtonHeld(int button);
   ofVec2f& GetDrawOffset() { return mModuleContainer.GetDrawOffsetRef(); }
   void SetDrawOffset(ofVec2f offset) { mModuleContainer.SetDrawOffset(offset); }
   const ofRectangle& GetDrawRect() const { return mDrawRect; }
   void SetPixelRatio(double ratio) { mPixelRatio = ratio; }
   double GetPixelRatio() const { return mPixelRatio; }
   long GetFrameCount() { return mFrameCount; }
   void SetUIScale(float scale) { mUILayerModuleContainer.SetDrawScale(scale); }
   ModuleContainer* GetRootContainer() { return &mModuleContainer; }

   void ZoomView(float zoomAmount, bool fromMouse);
   void PanView(float x, float y);
   void SetRawSpaceMouseTwist(float twist, bool isUsing) { mSpaceMouseInfo.mTwist = twist; mSpaceMouseInfo.mUsingTwist = isUsing; }
   void SetRawSpaceMouseZoom(float zoom, bool isUsing) { mSpaceMouseInfo.mZoom = zoom; mSpaceMouseInfo.mUsingZoom = isUsing; }
   void SetRawSpaceMousePan(float x, float y, bool isUsing) { mSpaceMouseInfo.mPan.set(x, y); mSpaceMouseInfo.mUsingPan = isUsing; }
   
   void SetResizeModule(IDrawableModule* module) { mResizeModule = module; }
   
   void SetGroupSelectContext(ModuleContainer* context) { mGroupSelectContext = context; }
   
   bool HasNotMovedMouseSinceClick() { return mClickStartX < INT_MAX; }
   IDrawableModule* GetMoveModule() { return mMoveModule; }
   ModuleFactory* GetModuleFactory() { return &mModuleFactory; }
   GlobalManagers* GetGlobalManagers() { return mGlobalManagers; }
   juce::Component* GetMainComponent() { return mMainComponent; }
   juce::OpenGLContext* GetOpenGLContext() { return mOpenGLContext; }
   IDrawableModule* GetLastClickedModule() const;
   EffectFactory* GetEffectFactory() { return &mEffectFactory; }
   const vector<IDrawableModule*>& GetGroupSelectedModules() const { return mGroupSelectedModules; }
   bool ShouldAccentuateActiveModules() const;
   LocationZoomer* GetLocationZoomer() { return &mZoomer; }
   
   void RegisterPatchCable(PatchCable* cable);
   void UnregisterPatchCable(PatchCable* cable);
   
   template<class T> vector<string> GetModuleNames() { return mModuleContainer.GetModuleNames<T>(); }
   
   void LockRender(bool lock) { if (lock) { mRenderLock.enter(); } else { mRenderLock.exit(); } }
   void UpdateFrameRate(float fps) { mFrameRate = fps; }
   float GetFrameRate() const { return mFrameRate; }
   CriticalSection* GetRenderLock() { return &mRenderLock; }
   NamedMutex* GetAudioMutex() { return &mAudioThreadMutex; }
   
   IDrawableModule* CreateModule(const ofxJSONElement& moduleInfo);
   void SetUpModule(IDrawableModule* module, const ofxJSONElement& moduleInfo);
   void OnModuleAdded(IDrawableModule* module);
   void OnModuleDeleted(IDrawableModule* module);
   void AddDynamicModule(IDrawableModule* module);
   
   void ScheduleEnvelopeEditorSpawn(ADSRDisplay* adsrDisplay);
   
   bool IsLoadingState() const { return mIsLoadingState; }
   bool IsLoadingModule() const { return mIsLoadingModule; }
   void SetIsLoadingState(bool loading) { mIsLoadingState = loading; }
   
   static string GetUserPrefsPath(bool relative);
   static void CrashHandler(void*);
   static void DumpStats(bool isCrash, void* crashContext);
   
   void SaveLayout(string jsonFile = "", bool makeDefaultLayout = true);
   ofxJSONElement GetLayout();
   void SaveLayoutAsPopup();
   void SaveOutput();
   void SaveState(string file, bool autosave);
   void LoadState(string file);
   // Set a state file to be loaded when the synth is initialized.
   void SetInitialState(string file);
   void SaveCurrentState();
   void SaveStatePopup();
   void LoadStatePopup();
   double GetLastSaveTime() { return mLastSaveTime; }
   string GetLastSavePath() { return mCurrentSaveStatePath; }

   ofxJSONElement GetUserPrefs() { return mUserPrefs; }
   UserPrefsEditor* GetUserPrefsEditor() { return mUserPrefsEditor; }
   
   void SetFatalError(string error);

   static bool sShouldAutosave;
   static float sBackgroundLissajousR;
   static float sBackgroundLissajousG;
   static float sBackgroundLissajousB;
   
private:
   void ResetLayout();
   void ReconnectMidiDevices();
   void DrawConsole();
   void CheckClick(IDrawableModule* clickedModule, int x, int y, bool rightButton);
   void UpdateUserPrefsLayout();
   void LoadStatePopupImp();
   IDrawableModule* DuplicateModule(IDrawableModule* module);
   void DeleteAllModules();
   void TriggerClapboard();
   void DoAutosave();
   IDrawableModule* GetModuleAtCursor();
   
   ofSoundStream mSoundStream;
   int mIOBufferSize;
   
   vector<IAudioSource*> mSources;
   vector<IDrawableModule*> mLissajousDrawers;
   vector<IDrawableModule*> mDeletedModules;
   
   vector<IDrawableModule*> mModalFocusItemStack;
   
   IDrawableModule* mMoveModule;
   int mMoveModuleOffsetX;
   int mMoveModuleOffsetY;
   
   ofVec2f mLastMoveMouseScreenPos;
   ofVec2f mLastMouseDragPos;
   bool mIsMousePanning;
   array<bool, 5> mIsMouseButtonHeld{ false };
   struct SpaceMouseInfo
   {
      SpaceMouseInfo() : mTwist(0), mZoom(0), mPan(0, 0), mUsingTwist(false), mUsingZoom(false), mUsingPan(false) {}
      float mTwist;
      float mZoom;
      ofVec2f mPan;
      bool mUsingTwist;
      bool mUsingZoom;
      bool mUsingPan;
   };
   SpaceMouseInfo mSpaceMouseInfo;

   char mConsoleText[MAX_TEXTENTRY_LENGTH];
   TextEntry* mConsoleEntry;
   ConsoleListener* mConsoleListener;
   
   std::vector<MidiDevice*> mMidiDevices;

   LocationZoomer mZoomer;
   QuickSpawnMenu* mQuickSpawn;
   UserPrefsEditor* mUserPrefsEditor;

   RollingBuffer* mGlobalRecordBuffer;
   long long mRecordingLength;
   
   struct LogEventItem
   {
      LogEventItem(double _time, string _text, LogEventType _type) : time(_time), text(_text), type(_type) {}
      double time;
      string text;
      LogEventType type;
   };
   std::list<LogEventItem> mEvents;
   std::list<string> mErrors;
   
   NamedMutex mAudioThreadMutex;
   
   bool mAudioPaused;
   bool mIsLoadingState;
   
   ModuleFactory mModuleFactory;
   EffectFactory mEffectFactory;
   
   int mClickStartX; //to detect click and release in place
   int mClickStartY;
   
   string mLoadedLayoutPath;
   bool mWantReloadInitialLayout;
   string mCurrentSaveStatePath;
   string mInitialSaveStatePath;
   double mLastSaveTime;
   
   Sample* mHeldSample;
   
   float* mSaveOutputBuffer[2];
   
   IDrawableModule* mLastClickedModule;
   
   ofxJSONElement mUserPrefs;
   bool mInitialized;
   
   ofRectangle mDrawRect;
   
   vector<IDrawableModule*> mGroupSelectedModules;
   ModuleContainer* mGroupSelectContext;
   bool mHasDuplicatedDuringDrag;
   bool mHasAutopatchedToTargetDuringDrag;
   
   IDrawableModule* mResizeModule;
   
   bool mShowLoadStatePopup;
   
   vector<PatchCable*> mPatchCables;
   
   ofMutex mKeyInputMutex;
   vector< pair<int,bool> > mQueuedKeyInput;
   
   ofVec2f mMousePos;
   string mNextDrawTooltip;
   
   GlobalManagers* mGlobalManagers;
   juce::Component* mMainComponent;
   juce::OpenGLContext* mOpenGLContext;
   
   CriticalSection mRenderLock;
   float mFrameRate;
   long mFrameCount;
   
   ModuleContainer mModuleContainer;
   ModuleContainer mUILayerModuleContainer;
   
   ADSRDisplay* mScheduledEnvelopeEditorSpawnDisplay;
   
   bool mIsLoadingModule;
   
   list<IPollable*> mExtraPollers;
   
   string mFatalError;
   
   double mLastClapboardTime;

   float mScrollMultiplierHorizontal;
   float mScrollMultiplierVertical;

   double mPixelRatio;

   vector<float*> mInputBuffers;
   vector<float*> mOutputBuffers;
};

extern ModularSynth* TheSynth;

#endif
