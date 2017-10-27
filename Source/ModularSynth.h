#ifndef _MODULAR_SYNTH
#define _MODULAR_SYNTH

#include "../JuceLibraryCode/JuceHeader.h"

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

class IAudioSource;
class MidiInstrument;
class InputChannel;
class OutputChannel;
class IAudioReceiver;
class INoteReceiver;
class MidiDevice;
class Sample;
class PatchCable;
class MidiController;
class NVGcontext;
class QuickSpawnMenu;

#define MAX_OUTPUT_CHANNELS 8
#define MAX_INPUT_CHANNELS 8

enum LogEventType
{
   kLogEventType_Normal,
   kLogEventType_Error
};

class ConsoleListener : public IDrawableModule, public ITextEntryListener
{
public:
   void GetModuleDimensions(int& width, int& height) override { width = 1; height = 1; }
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
   
   void Setup(GlobalManagers* globalManagers, juce::Component* mainComponent);
   void LoadResources(void* nanoVG, void* fontBoundsNanoVG);
   void Poll();
   void Draw(void* vg);
   
   void Exit();
   
   void KeyPressed(int key, bool isRepeat);
   void KeyReleased(int key);
   void MouseMoved(int x, int y );
   void MouseDragged(int x, int y, int button);
   void MousePressed(int x, int y, int button);
   void MouseReleased(int x, int y, int button);
   void MouseScrolled(float x, float y);
   void FilesDropped(vector<string> files, int x, int y);
   
   void AudioOut(float** output, int bufferSize, int nChannels);
   void AudioIn(const float** input, int bufferSize, int nChannels);

   void OnConsoleInput();
   void ClearConsoleInput();
   
   bool IsReady();
   
   void AddMidiDevice(MidiDevice* device);
   void ArrangeAudioSourceDependencies();
   IDrawableModule* SpawnModuleOnTheFly(string moduleName, float x, float y);
   void SetMoveModule(IDrawableModule* module, float offsetX, float offsetY);
   
   bool SetInputChannel(int channel, InputChannel* input);
   bool SetOutputChannel(int channel, OutputChannel* input);
   
   IDrawableModule* FindModule(string name, bool fail = true);
   IAudioReceiver* FindAudioReceiver(string name, bool fail = true);
   INoteReceiver* FindNoteReceiver(string name, bool fail = true);
   IUIControl* FindUIControl(string path);
   MidiController* FindMidiController(string name, bool fail = true);
   void MoveToFront(IDrawableModule* module);
   IDrawableModule* GetModuleAt(int x, int y);
   bool InMidiMapMode();
   void GetAllModules(vector<IDrawableModule*>& out) { mModuleContainer.GetAllModules(out); }
   
   void PushModalFocusItem(IDrawableModule* item);
   void PopModalFocusItem();
   IDrawableModule* GetTopModalFocusItem() const;
   
   void LogEvent(string event, LogEventType type);
   
   void LoadLayoutFromFile(string jsonFile, bool makeDefaultLayout = true);
   void LoadLayoutFromString(string jsonString);
   void LoadLayout(ofxJSONElement json);
   string GetLoadedLayout() const { return mLoadedLayoutPath; }
   
   void AddLissajousDrawer(IDrawableModule* module) { mLissajousDrawers.push_back(module); }
   bool IsLissajousDrawer(IDrawableModule* module) { return VectorContains(module, mLissajousDrawers); }
   
   void GrabSample(ChannelBuffer* data, bool window = false, int numBars = -1);
   Sample* GetHeldSample() const { return mHeldSample; }
   void ClearHeldSample();
   
   float GetRawMouseX() { return mMousePos.x; }
   float GetRawMouseY() { return mMousePos.y; }
   float GetMouseX(float rawX = FLT_MAX) { return (rawX == FLT_MAX ? mMousePos.x : rawX) / gDrawScale - mDrawOffset.x; }
   float GetMouseY(float rawY = FLT_MAX) { return ((rawY == FLT_MAX ? mMousePos.y : rawY) - 4) / gDrawScale - mDrawOffset.y; }
   ofVec2f GetDrawOffset() { return mDrawOffset; }
   void SetDrawOffset(ofVec2f offset) { mDrawOffset = offset; }
   const ofRectangle& GetDrawRect() const { return mDrawRect; }
   
   void SetResizeModule(IDrawableModule* module) { mResizeModule = module; }
   
   void SetGroupSelectContext(ModuleContainer* context) { mGroupSelectContext = context; }
   
   bool HasNotMovedMouseSinceClick() { return mClickStartX < INT_MAX; }
   IDrawableModule* GetMoveModule() { return mMoveModule; }
   ModuleFactory* GetModuleFactory() { return &mModuleFactory; }
   GlobalManagers* GetGlobalManagers() { return mGlobalManagers; }
   juce::Component* GetMainComponent() { return mMainComponent; }
   IDrawableModule* GetLastClickedModule() const;
   EffectFactory* GetEffectFactory() { return &mEffectFactory; }
   const vector<IDrawableModule*>& GetGroupSelectedModules() const { return mGroupSelectedModules; }
   bool ShouldAccentuateActiveModules() const;
   
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
   
   void SaveLayout(string jsonFile = "", bool makeDefaultLayout = true);
   ofxJSONElement GetLayout();
   void SaveLayoutAsPopup();
   void SaveOutput();
   void SaveState(string file);
   void LoadState(string file);
   void SaveStatePopup();
   void LoadStatePopup();
   
private:
   void ResetLayout();
   void ReconnectMidiDevices();
   void DrawConsole();
   void ZoomView(float zoomAmount);
   void CheckClick(IDrawableModule* clickedModule, int x, int y, bool rightButton);
   void UpdateUserPrefsLayout();
   void LoadStatePopupImp();
   IDrawableModule* DuplicateModule(IDrawableModule* module);
   void DeleteAllModules();
   
   ofSoundStream mSoundStream;
   int mIOBufferSize;
   
   vector<MidiInstrument*> mInstruments;
   vector<IAudioSource*> mSources;
   InputChannel* mInput[MAX_INPUT_CHANNELS];
   OutputChannel* mOutput[MAX_OUTPUT_CHANNELS];
   vector<IDrawableModule*> mLissajousDrawers;
   vector<IDrawableModule*> mDeletedModules;
   
   vector<IDrawableModule*> mModalFocusItemStack;
   
   IDrawableModule* mMoveModule;
   int mMoveModuleOffsetX;
   int mMoveModuleOffsetY;
   
   ofVec2f mLastMoveMouseScreenPos;
   ofVec2f mLastMouseDragPos;

   char mConsoleText[MAX_TEXTENTRY_LENGTH];
   TextEntry* mConsoleEntry;
   ConsoleListener* mConsoleListener;
   
   std::vector<MidiDevice*> mMidiDevices;

   LocationZoomer mZoomer;
   QuickSpawnMenu* mQuickSpawn;

   RollingBuffer mOutputBuffer;
   long long mRecordingLength;
   
   std::vector< std::pair<string,double> > mEvents;
   std::vector<string> mErrors;
   
   ofVec2f mDrawOffset;
   
   NamedMutex mAudioThreadMutex;
   
   bool mAudioPaused;
   
   ModuleFactory mModuleFactory;
   EffectFactory mEffectFactory;
   
   int mClickStartX; //to detect click and release in place
   int mClickStartY;
   
   string mLoadedLayoutPath;
   
   Sample* mHeldSample;
   
   float* mSaveOutputBuffer[2];
   
   IDrawableModule* mLastClickedModule;
   
   ofxJSONElement mUserPrefs;
   bool mInitialized;
   
   ofRectangle mDrawRect;
   
   vector<IDrawableModule*> mGroupSelectedModules;
   ModuleContainer* mGroupSelectContext;
   bool mHasDuplicatedDuringDrag;
   
   IDrawableModule* mResizeModule;
   
   bool mShowLoadStatePopup;
   
   vector<PatchCable*> mPatchCables;
   
   ofMutex mKeyInputMutex;
   vector< pair<int,bool> > mQueuedKeyInput;
   
   ofVec2f mMousePos;
   
   GlobalManagers* mGlobalManagers;
   juce::Component* mMainComponent;
   
   CriticalSection mRenderLock;
   float mFrameRate;
   
   ModuleContainer mModuleContainer;
};

extern ModularSynth* TheSynth;

#endif
