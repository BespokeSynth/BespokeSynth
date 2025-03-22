#include "ModularSynth.h"
#include "IAudioSource.h"
#include "OpenFrameworksPort.h"
#include "SynthGlobals.h"
#include "Scale.h"
#include "Transport.h"
#include "TextEntry.h"
#include "InputChannel.h"
#include "OutputChannel.h"
#include "TitleBar.h"
#include "LFOController.h"
#include "MidiController.h"
#include "ChaosEngine.h"
#include "ModuleSaveDataPanel.h"
#include "Profiler.h"
#include "Sample.h"
#include "FloatSliderLFOControl.h"
//#include <CoreServices/CoreServices.h>
#include "fenv.h"
#include <stdlib.h>
#include "GridController.h"
#include "FileStream.h"
#include "PatchCable.h"
#include "ADSRDisplay.h"
#include "QuickSpawnMenu.h"
#include "AudioToCV.h"
#include "ScriptModule.h"
#include "DrumPlayer.h"
#include "VSTPlugin.h"
#include "Prefab.h"
#include "HelpDisplay.h"
#include "nanovg/nanovg.h"
#include "UserPrefsEditor.h"
#include "Canvas.h"
#include "EffectChain.h"
#include "ClickButton.h"
#include "UserPrefs.h"
#include "NoteOutputQueue.h"

#include "juce_audio_processors/juce_audio_processors.h"
#include "juce_audio_formats/juce_audio_formats.h"

#if BESPOKE_WINDOWS
#include <windows.h>
#include <dbghelp.h>
#include <winbase.h>
#endif

ModularSynth* TheSynth = nullptr;
namespace
{
   juce::String TheClipboard;
}

//static
bool ModularSynth::sShouldAutosave = false;
float ModularSynth::sBackgroundLissajousR = 0.408f;
float ModularSynth::sBackgroundLissajousG = 0.245f;
float ModularSynth::sBackgroundLissajousB = 0.418f;
float ModularSynth::sBackgroundR = 0.09f;
float ModularSynth::sBackgroundG = 0.09f;
float ModularSynth::sBackgroundB = 0.09f;
float ModularSynth::sCableAlpha = 1.0f;
int ModularSynth::sLoadingFileSaveStateRev = ModularSynth::kSaveStateRev;
int ModularSynth::sLastLoadedFileSaveStateRev = ModularSynth::kSaveStateRev;
std::thread::id ModularSynth::sAudioThreadId;

#if BESPOKE_WINDOWS
LONG WINAPI TopLevelExceptionHandler(PEXCEPTION_POINTERS pExceptionInfo);
#endif

using namespace juce;

void AtExit()
{
   TheSynth->Exit();
}

ModularSynth::ModularSynth()
{
   assert(TheSynth == nullptr);
   TheSynth = this;

#if BESPOKE_WINDOWS
   SetUnhandledExceptionFilter(TopLevelExceptionHandler);
#endif

   mAudioPluginFormatManager = std::make_unique<juce::AudioPluginFormatManager>();
   mKnownPluginList = std::make_unique<juce::KnownPluginList>();

   mAudioPluginFormatManager->addDefaultFormats();
}

ModularSynth::~ModularSynth()
{
   DeleteAllModules();

   delete mGlobalRecordBuffer;
   mAudioPluginFormatManager.reset();
   mKnownPluginList.reset();

   SetMemoryTrackingEnabled(false); //avoid crashes when the tracking lists themselves are deleted

   assert(TheSynth == this);
   TheSynth = nullptr;

   ScriptModule::UninitializePython();
}

void ModularSynth::CrashHandler(void*)
{
   DumpStats(true, nullptr);
}

#if BESPOKE_WINDOWS
LONG WINAPI TopLevelExceptionHandler(PEXCEPTION_POINTERS pExceptionInfo)
{
   ModularSynth::DumpStats(true, pExceptionInfo);

   return EXCEPTION_CONTINUE_SEARCH;
}
#endif

void ModularSynth::DumpStats(bool isCrash, void* crashContext)
{
   std::string filename;
   if (isCrash)
      filename = ofToDataPath(ofGetTimestampString("crash_%Y-%m-%d_%H-%M.txt"));
   else
      filename = ofToDataPath(ofGetTimestampString("stats_%Y-%m-%d_%H-%M.txt"));
   juce::File log(filename);

   if (isCrash)
   {
#if BESPOKE_WINDOWS
      if (crashContext != nullptr)
      {
         log.appendText("stack frame:\n");
         PEXCEPTION_POINTERS pExceptionInfo = (PEXCEPTION_POINTERS)crashContext;

         HANDLE process = GetCurrentProcess();
         SymInitialize(process, NULL, TRUE);

         // StackWalk64() may modify context record passed to it, so we will
         // use a copy.
         CONTEXT context_record = *pExceptionInfo->ContextRecord;
         // Initialize stack walking.
         STACKFRAME64 stack_frame;
         memset(&stack_frame, 0, sizeof(stack_frame));
#if defined(_WIN64)
         int machine_type = IMAGE_FILE_MACHINE_AMD64;
         stack_frame.AddrPC.Offset = context_record.Rip;
         stack_frame.AddrFrame.Offset = context_record.Rbp;
         stack_frame.AddrStack.Offset = context_record.Rsp;
#else
         int machine_type = IMAGE_FILE_MACHINE_I386;
         stack_frame.AddrPC.Offset = context_record.Eip;
         stack_frame.AddrFrame.Offset = context_record.Ebp;
         stack_frame.AddrStack.Offset = context_record.Esp;
#endif
         stack_frame.AddrPC.Mode = AddrModeFlat;
         stack_frame.AddrFrame.Mode = AddrModeFlat;
         stack_frame.AddrStack.Mode = AddrModeFlat;

         juce::HeapBlock<SYMBOL_INFO> symbol;
         symbol.calloc(sizeof(SYMBOL_INFO) + 256, 1);
         symbol->MaxNameLen = 255;
         symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

         while (StackWalk64(machine_type,
                            GetCurrentProcess(),
                            GetCurrentThread(),
                            &stack_frame,
                            &context_record,
                            NULL,
                            &SymFunctionTableAccess64,
                            &SymGetModuleBase64,
                            NULL))
         {

            DWORD64 displacement = 0;

            if (SymFromAddr(process, (DWORD64)stack_frame.AddrPC.Offset, &displacement, symbol))
            {
               IMAGEHLP_MODULE64 moduleInfo;
               juce::zerostruct(moduleInfo);
               moduleInfo.SizeOfStruct = sizeof(moduleInfo);

               if (::SymGetModuleInfo64(process, symbol->ModBase, &moduleInfo))
                  log.appendText(String(moduleInfo.ModuleName) + String(": "));

               log.appendText(String(symbol->Name) + String(" + 0x") + String::toHexString((juce::int64)displacement) + "\n");
            }
         }
         log.appendText("\n\n\n");
      }
#endif

      log.appendText("backtrace:\n");
      log.appendText(juce::SystemStats::getStackBacktrace());
      log.appendText("\n\n\n");
   }

   log.appendText("OS: " + juce::SystemStats::getOperatingSystemName() + "\n");
   log.appendText("CPU vendor: " + juce::SystemStats::getCpuVendor() + "\n");
   log.appendText("CPU model: " + juce::SystemStats::getCpuModel() + "\n");
   log.appendText("CPU speed: " + ofToString(juce::SystemStats::getCpuSpeedInMegahertz()) + " MHz\n");
   log.appendText("num cores: " + ofToString(juce::SystemStats::getNumCpus()) + "\n");
   log.appendText("num CPUs: " + ofToString(juce::SystemStats::getNumPhysicalCpus()) + "\n");
   log.appendText("RAM: " + ofToString(juce::SystemStats::getMemorySizeInMegabytes()) + " MB\n");
   log.appendText("computer: " + juce::SystemStats::getComputerName() + "\n");
   log.appendText("language: " + juce::SystemStats::getUserLanguage() + "\n");
   log.appendText("region: " + juce::SystemStats::getUserRegion() + "\n");
   log.appendText("display language: " + juce::SystemStats::getDisplayLanguage() + "\n");
   log.appendText("description: " + juce::SystemStats::getDeviceDescription() + "\n");
   log.appendText("manufacturer: " + juce::SystemStats::getDeviceManufacturer() + "\n");
   log.appendText("build: bespoke " + GetBuildInfoString() + ")\n");
}

bool ModularSynth::IsReady()
{
   return gTime > 100;
}

void ModularSynth::Setup(juce::AudioDeviceManager* globalAudioDeviceManager, juce::AudioFormatManager* globalAudioFormatManager, juce::Component* mainComponent, juce::OpenGLContext* openGLContext)
{
   mGlobalAudioDeviceManager = globalAudioDeviceManager;
   mGlobalAudioFormatManager = globalAudioFormatManager;
   mMainComponent = mainComponent;
   mOpenGLContext = openGLContext;

   sShouldAutosave = UserPrefs.autosave.Get();

   mIOBufferSize = gBufferSize;

   mGlobalRecordBuffer = new RollingBuffer(UserPrefs.record_buffer_length_minutes.Get() * 60 * gSampleRate);
   mGlobalRecordBuffer->SetNumChannels(2);

   juce::File(ofToDataPath("savestate")).createDirectory();
   juce::File(ofToDataPath("savestate/autosave")).createDirectory();
   juce::File(ofToDataPath("recordings")).createDirectory();
   juce::File(ofToSamplePath("")).createDirectory();
   juce::File(ofToDataPath("scripts")).createDirectory();
   juce::File(ofToDataPath("internal")).createDirectory();
   juce::File(ofToDataPath("vst")).createDirectory();

   SynthInit();

   new Transport();
   new Scale();
   TheScale->CreateUIControls();
   TheTransport->CreateUIControls();

   TheScale->Init();
   TheTransport->Init();

   DrumPlayer::SetUpHitDirectories();

   sBackgroundLissajousR = UserPrefs.lissajous_r.Get();
   sBackgroundLissajousG = UserPrefs.lissajous_g.Get();
   sBackgroundLissajousB = UserPrefs.lissajous_b.Get();
   sBackgroundR = UserPrefs.background_r.Get();
   sBackgroundG = UserPrefs.background_g.Get();
   sBackgroundB = UserPrefs.background_b.Get();
   sCableAlpha = UserPrefs.cable_alpha.Get();

   Time time = Time::getCurrentTime();
   if (fabsf(sBackgroundR - UserPrefs.background_r.GetDefault()) < .001f && fabsf(sBackgroundG - UserPrefs.background_g.GetDefault()) < .001f && fabsf(sBackgroundB - UserPrefs.background_b.GetDefault()) < .001f && time.getMonth() + 1 == 10 && time.getDayOfMonth() == 31)
   {
      sBackgroundLissajousR = 0.722f;
      sBackgroundLissajousG = 0.328f;
      sBackgroundLissajousB = 0.0f;
   }

   ResetLayout();

   mConsoleListener = new ConsoleListener();
   mConsoleEntry = new TextEntry(mConsoleListener, "console", 0, 20, 50, mConsoleText);
   mConsoleEntry->SetRequireEnter(true);
}

void ModularSynth::LoadResources(void* nanoVG, void* fontBoundsNanoVG)
{
   gNanoVG = (NVGcontext*)nanoVG;
   gFontBoundsNanoVG = (NVGcontext*)fontBoundsNanoVG;
   LoadGlobalResources();

   if (!gFont.IsLoaded())
      mFatalError = "couldn't load font from " + gFont.GetFontPath() + "\nmaybe bespoke can't find your resources directory?";
}

void ModularSynth::InitIOBuffers(int inputChannelCount, int outputChannelCount)
{
   for (int i = 0; i < inputChannelCount; ++i)
      mInputBuffers.push_back(new float[gBufferSize]);
   for (int i = 0; i < outputChannelCount; ++i)
      mOutputBuffers.push_back(new float[gBufferSize]);
}


std::string ModularSynth::GetUserPrefsPath()
{
   std::string filename = "userprefs.json";
   for (int i = 0; i < JUCEApplication::getCommandLineParameterArray().size(); ++i)
   {
      juce::String specified = JUCEApplication::getCommandLineParameterArray()[i];
      if (specified.endsWith(".json"))
      {
         filename = specified.toStdString();
         if (!juce::File(ofToDataPath(filename)).existsAsFile())
            TheSynth->SetFatalError("couldn't find command-line-specified userprefs file at " + ofToDataPath(filename));
         break;
      }
   }

   return ofToDataPath(filename);
}

static int sFrameCount = 0;
void ModularSynth::Poll()
{
   if (mFatalError == "")
   {
      if (!mInitialized && sFrameCount > 3) //let some frames render before blocking for a load
      {
         mUserPrefsEditor->CreatePrefsFileIfNonexistent();

         if (!mStartupSaveStateFile.empty())
            LoadState(mStartupSaveStateFile);
         else
            LoadLayoutFromFile(ofToDataPath(UserPrefs.layout.Get()));
         mInitialized = true;
      }

      if (mWantReloadInitialLayout)
      {
         LoadLayoutFromFile(ofToDataPath(UserPrefs.layout.Get()));
         mWantReloadInitialLayout = false;
      }
   }

   mZoomer.Update();

   if (!mIsLoadingState)
   {
      for (auto p : mExtraPollers)
         p->Poll();
      mUILayerModuleContainer.Poll();
      mModuleContainer.Poll();
   }

   if (mShowLoadStatePopup)
   {
      mShowLoadStatePopup = false;
      LoadStatePopupImp();
   }

   if (mScheduledEnvelopeEditorSpawnDisplay != nullptr)
   {
      mScheduledEnvelopeEditorSpawnDisplay->SpawnEnvelopeEditor();
      mScheduledEnvelopeEditorSpawnDisplay = nullptr;
   }

   {
      static MouseCursor sCurrentCursor = MouseCursor::NormalCursor;
      MouseCursor desiredCursor;

      if (mIsLoadingState)
      {
         desiredCursor = MouseCursor::WaitCursor;
      }
      else if (gHoveredUIControl != nullptr && gHoveredUIControl->IsMouseDown())
      {
         if (GetKeyModifiers() == kModifier_Shift)
            desiredCursor = MouseCursor::CrosshairCursor;
         else if (dynamic_cast<FloatSlider*>(gHoveredUIControl) != nullptr && dynamic_cast<FloatSlider*>(gHoveredUIControl)->GetModulator() != nullptr && dynamic_cast<FloatSlider*>(gHoveredUIControl)->GetModulator()->Active())
            desiredCursor = MouseCursor::UpDownLeftRightResizeCursor;
         else
            desiredCursor = MouseCursor::LeftRightResizeCursor;
      }
      else if (gHoveredUIControl != nullptr && gHoveredUIControl->IsTextEntry())
      {
         desiredCursor = MouseCursor::IBeamCursor;
      }
      else if (gHoveredUIControl != nullptr && dynamic_cast<Canvas*>(gHoveredUIControl) != nullptr)
      {
         desiredCursor = dynamic_cast<Canvas*>(gHoveredUIControl)->GetMouseCursorType();
      }
      else if (mIsMousePanning)
      {
         desiredCursor = MouseCursor::DraggingHandCursor;
      }
      else if (GetKeyModifiers() == kModifier_Shift)
      {
         desiredCursor = MouseCursor::PointingHandCursor;
      }
      else
      {
         desiredCursor = MouseCursor::NormalCursor;
      }

      if (desiredCursor != sCurrentCursor)
      {
         sCurrentCursor = desiredCursor;
         mMainComponent->setMouseCursor(desiredCursor);
      }
   }

   bool shiftPressed = (GetKeyModifiers() == kModifier_Shift);
   if (shiftPressed && !mIsShiftPressed && IKeyboardFocusListener::GetActiveKeyboardFocus() == nullptr)
   {
      double timeBetweenPresses = gTime - mLastShiftPressTime;
      float mouseMoveBetweenPressesSq = (mMousePos - mLastShiftPressMousePos).distanceSquared();
      if (timeBetweenPresses < 400 && mouseMoveBetweenPressesSq < 3 * 3)
      {
         ToggleQuickSpawn();
         mLastShiftPressTime = -9999; //clear timer
      }
      else
      {
         mLastShiftPressTime = gTime;
         mLastShiftPressMousePos = mMousePos;
      }
   }
   mIsShiftPressed = shiftPressed;

   if (mArrangeDependenciesWhenLoadCompletes && !mIsLoadingState)
   {
      ArrangeAudioSourceDependencies();
      mArrangeDependenciesWhenLoadCompletes = false;
   }

   if (gHoveredUIControl != nullptr && gHoveredUIControl->IsShowing() == false)
      gHoveredUIControl = nullptr;

   ++sFrameCount;
}

void ModularSynth::DeleteAllModules()
{
   mModuleContainer.Clear();

   for (int i = 0; i < mDeletedModules.size(); ++i)
      delete mDeletedModules[i];
   mDeletedModules.clear();

   delete TheScale;
   TheScale = nullptr;
   delete TheTransport;
   TheTransport = nullptr;
   delete mConsoleListener;
   mConsoleListener = nullptr;
}

bool SortPointsByY(ofVec2f a, ofVec2f b)
{
   return a.y < b.y;
}

void ModularSynth::ZoomView(float zoomAmount, bool fromMouse)
{
   float oldDrawScale = gDrawScale;
   gDrawScale *= 1 + zoomAmount;
   float minZoom = .1f;
   float maxZoom = 8;
   gDrawScale = ofClamp(gDrawScale, minZoom, maxZoom);
   zoomAmount = (gDrawScale - oldDrawScale) / oldDrawScale; //find actual adjusted amount
   ofVec2f zoomCenter;
   if (fromMouse)
      zoomCenter = ofVec2f(GetMouseX(&mModuleContainer), GetMouseY(&mModuleContainer)) + GetDrawOffset();
   else
      zoomCenter = ofVec2f(ofGetWidth() / gDrawScale * .5f, ofGetHeight() / gDrawScale * .5f);
   GetDrawOffset() -= zoomCenter * zoomAmount;
   mZoomer.CancelMovement();
   mHideTooltipsUntilMouseMove = true;
}

void ModularSynth::SetZoomLevel(float zoomLevel)
{
   float oldDrawScale = gDrawScale;
   gDrawScale = zoomLevel;
   float zoomAmount = (gDrawScale - oldDrawScale) / oldDrawScale;
   ofVec2f zoomCenter = ofVec2f(ofGetWidth() / gDrawScale * .5f, ofGetHeight() / gDrawScale * .5f);
   GetDrawOffset() -= zoomCenter * zoomAmount;
   mZoomer.CancelMovement();
   mHideTooltipsUntilMouseMove = true;
}

void ModularSynth::PanView(float x, float y)
{
   GetDrawOffset() += ofVec2f(x, y) / gDrawScale;
   mHideTooltipsUntilMouseMove = true;
}

void ModularSynth::PanTo(float x, float y)
{
   SetDrawOffset(ofVec2f(ofGetWidth() / gDrawScale / 2 - x, ofGetHeight() / gDrawScale / 2 - y));
   mHideTooltipsUntilMouseMove = true;
}

void ModularSynth::Draw(void* vg)
{
   gNanoVG = (NVGcontext*)vg;

   ofNoFill();

   //DrawTextNormal("fps: "+ofToString(ofGetFrameRate(),4)+" "+ofToString(ofGetWidth()*ofGetHeight()), 100, 100,50);
   //return;

   mModuleContainer.SetDrawScale(gDrawScale);
   mDrawRect.set(-GetDrawOffset().x, -GetDrawOffset().y, ofGetWidth() / gDrawScale, ofGetHeight() / gDrawScale);

   if (mFatalError != "")
   {
      ofSetColor(255, 255, 255, 255);
      DrawFallbackText(("bespoke " + GetBuildInfoString()).c_str(), 100, 50);

      if (gFont.IsLoaded())
         DrawTextNormal(mFatalError, 100, 100, 18);
      else
         DrawFallbackText(mFatalError.c_str(), 100, 100);
   }

   if (ScriptModule::sBackgroundTextString != "")
   {
      ofPushStyle();
      ofSetColor(ScriptModule::sBackgroundTextColor);
      DrawTextBold(ScriptModule::sBackgroundTextString, ScriptModule::sBackgroundTextPos.x, ScriptModule::sBackgroundTextPos.y + ScriptModule::sBackgroundTextSize, ScriptModule::sBackgroundTextSize);
      ofPopStyle();
   }

   if (UserPrefs.draw_background_lissajous.Get())
      DrawLissajous(mGlobalRecordBuffer, 0, 0, ofGetWidth(), ofGetHeight(), sBackgroundLissajousR, sBackgroundLissajousG, sBackgroundLissajousB, UserPrefs.background_lissajous_autocorrelate.Get());

   if (gTime == 1 && mFatalError == "")
   {
      std::string loading("Bespoke is initializing audio...");
      DrawTextNormal(loading, ofGetWidth() / 2 - GetStringWidth(loading, 28) / 2, ofGetHeight() / 2 - 6, 28);
      return;
   }

   if (!mInitialized && mFatalError == "")
   {
      std::string loading("Bespoke is loading...");
      DrawTextNormal(loading, ofGetWidth() / 2 - GetStringWidth(loading, 28) / 2, ofGetHeight() / 2 - 6, 28);
      return;
   }

   ofPushMatrix();

   ofScale(gDrawScale, gDrawScale, gDrawScale);

   ofPushMatrix();

   ofTranslate(GetDrawOffset().x, GetDrawOffset().y);

   if (ShouldShowGridSnap())
   {
      ofPushStyle();
      ofSetLineWidth(.5f);
      ofSetColor(255, 255, 255, 40);
      float gridSnapSize = UserPrefs.grid_snap_size.Get();
      int gridLinesVertical = (int)ceil((ofGetWidth() / gDrawScale) / gridSnapSize);
      for (int i = 0; i < gridLinesVertical; ++i)
      {
         float x = i * gridSnapSize - floor(GetDrawOffset().x / gridSnapSize) * gridSnapSize;
         ofLine(x, -GetDrawOffset().y, x, -GetDrawOffset().y + ofGetHeight() / gDrawScale);
      }
      int gridLinesHorizontal = (int)ceil((ofGetHeight() / gDrawScale) / gridSnapSize);
      for (int i = 0; i < gridLinesHorizontal; ++i)
      {
         float y = i * gridSnapSize - floor(GetDrawOffset().y / gridSnapSize) * gridSnapSize;
         ofLine(-GetDrawOffset().x, y, -GetDrawOffset().x + ofGetWidth() / gDrawScale, y);
      }
      ofPopStyle();
   }

   ofNoFill();

   TheSaveDataPanel->SetShowing(TheSaveDataPanel->GetModule());
   TheSaveDataPanel->UpdatePosition();

   mModuleContainer.DrawContents();

   IClickable* dropTarget = nullptr;
   if (PatchCable::sActivePatchCable != nullptr)
      dropTarget = PatchCable::sActivePatchCable->GetDropTarget();
   if (dropTarget)
   {
      ofPushStyle();

      ofSetColor(255, 255, 255, 100);
      ofSetLineWidth(.5f);
      ofFill();
      ofRectangle rect = dropTarget->GetRect();

      IDrawableModule* dropTargetModule = dynamic_cast<IDrawableModule*>(dropTarget);
      if (dropTargetModule && dropTargetModule->HasTitleBar())
      {
         rect.y -= IDrawableModule::TitleBarHeight();
         rect.height += IDrawableModule::TitleBarHeight();
      }

      ofRect(rect);

      ofPopStyle();
   }

   for (int i = 0; i < mLissajousDrawers.size(); ++i)
   {
      float moduleX, moduleY;
      mLissajousDrawers[i]->GetPosition(moduleX, moduleY);
      IAudioSource* source = dynamic_cast<IAudioSource*>(mLissajousDrawers[i]);
      DrawLissajous(source->GetVizBuffer(), moduleX, moduleY - 240, 240, 240);
   }

   if (mGroupSelectContext != nullptr)
   {
      ofPushStyle();
      ofSetColor(255, 255, 255);
      ofRect(mClickStartX, mClickStartY, GetMouseX(&mModuleContainer) - mClickStartX, GetMouseY(&mModuleContainer) - mClickStartY);
      ofPopStyle();
   }

   const bool kDrawCursorDot = false;
   if (kDrawCursorDot)
   {
      ofPushStyle();
      ofSetColor(255, 255, 255);
      ofCircle(GetMouseX(&mModuleContainer), GetMouseY(&mModuleContainer), 1);
      ofPopStyle();
   }

   /*TODO_PORT(Ryan)
   const int starvationDisplayTicks = 500;
   if (mSoundStream.getTickCount() - mSoundStream.GetLastStarvationTick() < starvationDisplayTicks)
   {
      ofPushStyle();
      ofSetColor(255,255,255, (1.0f-(float(mSoundStream.getTickCount() - mSoundStream.GetLastStarvationTick())/starvationDisplayTicks))*255);
      DrawTextNormal("X", 5, 15);
      ofPopStyle();
   }*/

   if (mHeldSample)
   {
      ofPushMatrix();
      ofPushStyle();
      ofTranslate(GetMouseX(&mModuleContainer), GetMouseY(&mModuleContainer));
      ofClipWindow(0, 0, 120, 70, true);
      DrawAudioBuffer(120, 70, mHeldSample->Data(), 0, mHeldSample->LengthInSamples(), -1);
      ofSetColor(255, 255, 255);
      DrawTextNormal(mHeldSample->Name(), 0, 13);
      ofPopStyle();
      ofPopMatrix();
   }

   /*ofPushStyle();
   ofNoFill();
   ofSetLineWidth(3);
   ofSetColor(0,255,0,100);
   ofSetCircleResolution(100);
   ofCircle(GetMouseX(&mModuleContainer), GetMouseY(&mModuleContainer), 30 + (TheTransport->GetMeasurePos() * 20));
   ofPopStyle();*/

   ofPopMatrix();

   if (gTime - mLastClapboardTime < 100)
   {
      ofSetColor(255, 255, 255, (1 - (gTime - mLastClapboardTime) / 100) * 255);
      ofFill();
      ofRect(0, 0, ofGetWidth(), ofGetHeight());
   }

   ofPopMatrix();

   ofPushMatrix();
   {
      float uiScale = mUILayerModuleContainer.GetDrawScale();
      if (uiScale < .01f)
      {
         //safety check in case anything ever makes the UI inaccessible
         LogEvent("correcting UI scale", kLogEventType_Error);
         mUILayerModuleContainer.SetDrawScale(1);
         uiScale = mUILayerModuleContainer.GetDrawScale();
      }
      ofScale(uiScale, uiScale, uiScale);
      ofTranslate(mUILayerModuleContainer.GetDrawOffset().x, mUILayerModuleContainer.GetDrawOffset().y);

      mUILayerModuleContainer.DrawContents();

      Profiler::Draw();
      DrawConsole();
   }
   ofPopMatrix();

   for (auto* modal : mModalFocusItemStack)
   {
      ofPushMatrix();
      float scale = modal->GetOwningContainer()->GetDrawScale();
      ofVec2f offset = modal->GetOwningContainer()->GetDrawOffset();
      ofScale(scale, scale, scale);
      ofTranslate(offset.x, offset.y);
      modal->Draw();
      ofPopMatrix();
   }

   std::string tooltip = "";
   ModuleContainer* tooltipContainer = nullptr;
   ofVec2f tooltipPos(FLT_MAX, FLT_MAX);
   if (mNextDrawTooltip != "")
   {
      tooltip = mNextDrawTooltip;
      tooltipContainer = &mModuleContainer;
   }
   else if (HelpDisplay::sShowTooltips &&
            !mHideTooltipsUntilMouseMove &&
            !IUIControl::WasLastHoverSetManually() &&
            mGroupSelectContext == nullptr &&
            PatchCable::sActivePatchCable == nullptr &&
            mGroupSelectedModules.empty() &&
            mHeldSample == nullptr)
   {
      HelpDisplay* helpDisplay = TheTitleBar->GetHelpDisplay();

      bool hasValidHoveredControl = gHoveredUIControl && gHoveredUIControl->GetModuleParent() && !gHoveredUIControl->GetModuleParent()->IsDeleted() && std::string(gHoveredUIControl->Name()) != "enabled";

      if (gHoveredModule && (!hasValidHoveredControl || (gHoveredUIControl != nullptr && gHoveredUIControl->GetModuleParent() != gHoveredModule)))
      {
         if (gHoveredModule == mQuickSpawn)
         {
            std::string name = mQuickSpawn->GetHoveredModuleTypeName();
            ofStringReplace(name, " " + std::string(ModuleFactory::kEffectChainSuffix), ""); //strip this suffix if it's there
            tooltip = helpDisplay->GetModuleTooltipFromName(name);
            tooltipContainer = mQuickSpawn->GetOwningContainer();
         }
         else if (gHoveredModule == GetTopModalFocusItem() && dynamic_cast<DropdownListModal*>(gHoveredModule))
         {
            DropdownListModal* list = dynamic_cast<DropdownListModal*>(gHoveredModule);
            if (list->GetOwner()->GetModuleParent() == TheTitleBar)
            {
               std::string moduleTypeName = dynamic_cast<DropdownListModal*>(gHoveredModule)->GetHoveredLabel();
               ofStringReplace(moduleTypeName, " (exp.)", "");
               tooltip = helpDisplay->GetModuleTooltipFromName(moduleTypeName);
               tooltipContainer = &mUILayerModuleContainer;
            }
            else if (dynamic_cast<EffectChain*>(list->GetOwner()->GetParent()) != nullptr)
            {
               std::string effectName = dynamic_cast<DropdownListModal*>(gHoveredModule)->GetHoveredLabel();
               tooltip = helpDisplay->GetModuleTooltipFromName(effectName);
               tooltipContainer = list->GetModuleParent()->GetOwningContainer();
            }
         }
         else if (GetMouseY(&mModuleContainer) < gHoveredModule->GetPosition().y && gHoveredModule->HasTitleBar()) //this means we're hovering over the module's title bar
         {
            tooltip = helpDisplay->GetModuleTooltip(gHoveredModule);
            tooltipContainer = gHoveredModule->GetOwningContainer();
         }

         if (tooltipContainer != nullptr)
         {
            tooltipPos.x = gHoveredModule->GetRect().getMaxX() + 10;
            tooltipPos.y = GetMouseY(tooltipContainer) + 7;
         }
      }
      else if (hasValidHoveredControl && !gHoveredUIControl->IsMouseDown())
      {
         tooltip = helpDisplay->GetUIControlTooltip(gHoveredUIControl);
         tooltipContainer = gHoveredUIControl->GetModuleParent()->GetOwningContainer();
         tooltipPos.x = gHoveredUIControl->GetRect().getMaxX() + 10;
         tooltipPos.y = GetMouseY(tooltipContainer) + 18;
      }
   }

   mNextDrawTooltip = "";

   if (tooltip != "" && tooltipContainer != nullptr)
   {
      if (tooltipPos.x == FLT_MAX)
      {
         tooltipPos.x = GetMouseX(tooltipContainer) + 25;
         tooltipPos.y = GetMouseY(tooltipContainer) + 30;
      }

      ofPushMatrix();
      float scale = tooltipContainer->GetDrawScale();
      ofVec2f offset = tooltipContainer->GetDrawOffset();
      ofScale(scale, scale, scale);
      ofTranslate(offset.x, offset.y);

      float maxWidth = 300;

      float fontSize = 13;
      nvgFontFaceId(gNanoVG, gFont.GetFontHandle());
      nvgFontSize(gNanoVG, fontSize);
      float bounds[4];
      nvgTextBoxBounds(gNanoVG, tooltipPos.x, tooltipPos.y, maxWidth, tooltip.c_str(), nullptr, bounds);
      float padding = 3;
      ofRectangle rect(bounds[0] - padding, bounds[1] - padding, bounds[2] - bounds[0] + padding * 2, bounds[3] - bounds[1] + padding * 2);

      float minX = 5 - offset.x;
      float maxX = ofGetWidth() / scale - rect.width - 5 - offset.x;
      float minY = 5 - offset.y;
      float maxY = ofGetHeight() / scale - rect.height - 5 - offset.y;

      float onscreenRectX = ofClamp(rect.x, minX, maxX);
      if (onscreenRectX < rect.x)
      {
         tooltipPos.y += 20;
         rect.y += 20;
      }
      float onscreenRectY = ofClamp(rect.y, minY, maxY);

      float tooltipBackgroundAlpha = 180;

      ofFill();
      ofSetColor(50, 50, 50, tooltipBackgroundAlpha);
      ofRect(onscreenRectX, onscreenRectY, rect.width, rect.height);

      ofNoFill();
      ofSetColor(255, 255, 255, tooltipBackgroundAlpha);
      ofRect(onscreenRectX, onscreenRectY, rect.width, rect.height);

      ofSetColor(255, 255, 255);
      //DrawTextNormal(tooltip, x + 5, y + 12);
      gFont.DrawStringWrap(tooltip, fontSize, tooltipPos.x + (onscreenRectX - rect.x), tooltipPos.y + (onscreenRectY - rect.y), maxWidth);

      ofPopMatrix();
   }

   ofPushStyle();
   ofNoFill();
   float centerX = ofGetWidth() * .5f;
   float centerY = ofGetHeight() * .5f;
   if (mSpaceMouseInfo.mTwist != 0)
   {
      if (mSpaceMouseInfo.mUsingTwist)
         ofSetColor(0, 255, 255, 100);
      else
         ofSetColor(0, 100, 255, 100);
      ofSetLineWidth(1.5f);
      ofCircle(centerX + sin(mSpaceMouseInfo.mTwist * M_PI) * 20, centerY + cos(mSpaceMouseInfo.mTwist * M_PI) * 20, 3);
      ofCircle(centerX + sin(mSpaceMouseInfo.mTwist * M_PI + M_PI) * 20, centerY + cos(mSpaceMouseInfo.mTwist * M_PI + M_PI) * 20, 3);
   }
   ofSetLineWidth(3);
   if (mSpaceMouseInfo.mZoom != 0)
   {
      if (mSpaceMouseInfo.mUsingZoom)
         ofSetColor(0, 255, 255, 100);
      else
         ofSetColor(0, 100, 255, 100);
      ofCircle(centerX, centerY, 15 - (mSpaceMouseInfo.mZoom * 15));
   }
   if (mSpaceMouseInfo.mPan.x != 0 || mSpaceMouseInfo.mPan.y != 0)
   {
      if (mSpaceMouseInfo.mUsingPan)
         ofSetColor(0, 255, 255, 100);
      else
         ofSetColor(0, 100, 255, 100);
      ofLine(centerX, centerY, centerX + mSpaceMouseInfo.mPan.x * 40, centerY + mSpaceMouseInfo.mPan.y * 40);
   }
   ofPopStyle();

   ++mFrameCount;
}

void ModularSynth::PostRender()
{
   mModuleContainer.PostRender();
   mUILayerModuleContainer.PostRender();
}

void ModularSynth::DrawConsole()
{
   /*if (!mErrors.empty())
   {
      ofPushStyle();
      ofFill();
      ofSetColor(255,0,0,128);
      ofBeginShape();
      ofVertex(0,0);
      ofVertex(20,0);
      ofVertex(0,20);
      ofEndShape();
      ofPopStyle();
   }*/

   float consoleY = TheTitleBar->GetRect().height + 15;

   if (IKeyboardFocusListener::GetActiveKeyboardFocus() == mConsoleEntry)
   {
      mConsoleEntry->SetPosition(0, consoleY - 15);
      mConsoleEntry->Draw();
      consoleY += 17;
   }
   else
   {
      if (gHoveredUIControl != nullptr)
      {
         ofPushStyle();
         ofSetColor(0, 255, 255);
         DrawTextNormal(gHoveredUIControl->Path(), 0, consoleY - 4);
         ofPopStyle();
      }
   }

   if (mHasCircularDependency)
   {
      ofPushStyle();
      float pulse = ofMap(sin(gTime / 500 * PI * 2), -1, 1, .5f, 1);
      ofSetColor(255 * pulse, 255 * pulse, 0);
      DrawTextNormal("circular dependency detected", 0, consoleY + 20);
      ofPopStyle();
   }

   if (IKeyboardFocusListener::GetActiveKeyboardFocus() == mConsoleEntry)
   {
      int outputLines = (int)mEvents.size();
      if (IKeyboardFocusListener::GetActiveKeyboardFocus() == mConsoleEntry)
         outputLines += mErrors.size();
      if (outputLines > 0)
      {
         ofPushStyle();
         ofSetColor(0, 0, 0, 150);
         ofFill();
         float titleBarW, titleBarH;
         TheTitleBar->GetDimensions(titleBarW, titleBarH);
         ofRect(0, consoleY - 16, titleBarW, outputLines * 15 + 3);
         ofPopStyle();
      }

      for (auto it = mEvents.begin(); it != mEvents.end(); ++it)
      {
         ofPushStyle();
         if (it->type == kLogEventType_Error)
            ofSetColor(255, 0, 0);
         else if (it->type == kLogEventType_Warning)
            ofSetColor(255, 255, 0);
         else
            ofSetColor(255, 255, 255);
         gFontFixedWidth.DrawString(it->text, 13, 10, consoleY);
         std::vector<std::string> lines = ofSplitString(it->text, "\n");
         ofPopStyle();
         consoleY += 15 * lines.size();
      }

      if (!mErrors.empty())
      {
         consoleY = 0;
         ofPushStyle();
         ofSetColor(255, 0, 0);
         for (auto it = mErrors.begin(); it != mErrors.end(); ++it)
         {
            gFontFixedWidth.DrawString(*it, 13, 600, consoleY);
            std::vector<std::string> lines = ofSplitString(*it, "\n");
            consoleY += 15 * lines.size();
         }
         ofPopStyle();
      }
   }
}

void ModularSynth::Exit()
{
   mAudioThreadMutex.Lock("exiting");
   mAudioPaused = true;
   mAudioThreadMutex.Unlock();
   mModuleContainer.Exit();
   DeleteAllModules();
   ofExit();
}

void ModularSynth::Focus()
{
   ReadClipboardTextFromSystem();
}

IDrawableModule* ModularSynth::GetLastClickedModule() const
{
   return mLastClickedModule;
}

void ModularSynth::KeyPressed(int key, bool isRepeat)
{
   mLastShiftPressTime = -9999; //reset timer for detecing double-shift press, so it doens't happen while typing

   if (!isRepeat)
      mHideTooltipsUntilMouseMove = true;

   if (gHoveredUIControl &&
       IKeyboardFocusListener::GetActiveKeyboardFocus() == nullptr &&
       !isRepeat)
   {
      if (key == OF_KEY_DOWN || key == OF_KEY_UP || key == OF_KEY_LEFT || key == OF_KEY_RIGHT)
      {
         if (GetKeyModifiers() != kModifier_Command)
         {
            float inc;
            if (key == OF_KEY_LEFT)
               inc = -1;
            else if (key == OF_KEY_RIGHT)
               inc = 1;
            else if ((key == OF_KEY_DOWN && gHoveredUIControl->InvertScrollDirection() == false) ||
                     (key == OF_KEY_UP && gHoveredUIControl->InvertScrollDirection() == true))
               inc = -1;
            else
               inc = 1;
            if (GetKeyModifiers() & kModifier_Shift)
               inc *= .01f;
            gHoveredUIControl->Increment(inc);
         }
      }
      else if (key == '[')
      {
         gHoveredUIControl->Halve();
      }
      else if (key == ']')
      {
         gHoveredUIControl->Double();
      }
      else if (key == '\\')
      {
         gHoveredUIControl->ResetToOriginal();
      }
      else if ((toupper(key) == 'C' || toupper(key) == 'X') && GetKeyModifiers() == kModifier_Command)
      {
         TheSynth->CopyTextToClipboard(ofToString(gHoveredUIControl->GetValue()));
      }
      else if (key != ' ' && key != OF_KEY_TAB && key != '`' && key < CHAR_MAX && juce::CharacterFunctions::isPrintable((char)key) && (GetKeyModifiers() & kModifier_Alt) == false)
      {
         gHoveredUIControl->AttemptTextInput();
      }
   }

   if (key == OF_KEY_ESC && PatchCable::sActivePatchCable != nullptr)
   {
      PatchCable::sActivePatchCable->Release();
      return;
   }

   if (IKeyboardFocusListener::GetActiveKeyboardFocus() != nullptr &&
       IKeyboardFocusListener::GetActiveKeyboardFocus()->ShouldConsumeKey(key)) //active text entry captures all input
   {
      IKeyboardFocusListener::GetActiveKeyboardFocus()->OnKeyPressed(key, isRepeat);
      return;
   }

   if (gHoveredModule != nullptr)
   {
      IKeyboardFocusListener* focus = dynamic_cast<IKeyboardFocusListener*>(gHoveredModule);
      if (focus && focus->ShouldConsumeKey(key))
      {
         focus->OnKeyPressed(key, isRepeat);
         return;
      }
   }

   key = KeyToLower(key); //now convert to lowercase because everything else just cares about keys as buttons (unmodified by shift)

   if ((key == juce::KeyPress::backspaceKey || key == juce::KeyPress::deleteKey) && !isRepeat)
   {
      for (auto module : mGroupSelectedModules)
         module->GetOwningContainer()->DeleteModule(module);
      mGroupSelectedModules.clear();
   }

   if (key == KeyPress::F2Key && !isRepeat)
   {
      ADSRDisplay::ToggleDisplayMode();
   }

   if (key == KeyPress::F3Key && !isRepeat)
   {
      if (gHoveredModule && mGroupSelectedModules.empty())
         gHoveredModule->TogglePinned();
   }

   if (key == '`' && !isRepeat)
   {
      if (GetKeyModifiers() == kModifier_Shift)
      {
         TriggerClapboard();
      }
      else
      {
         std::memset(mConsoleText, 0, MAX_TEXTENTRY_LENGTH);
         mConsoleEntry->MakeActiveTextEntry(true);
      }
   }

   if (key == KeyPress::F1Key && !isRepeat)
   {
      HelpDisplay::sShowTooltips = !HelpDisplay::sShowTooltips;
   }

   if (key == OF_KEY_TAB)
   {
      if (GetKeyModifiers() == kModifier_Shift)
         IUIControl::SetNewManualHoverViaTab(-1);
      else
         IUIControl::SetNewManualHoverViaTab(1);
   }

   if (key == OF_KEY_LEFT || key == OF_KEY_RIGHT || key == OF_KEY_UP || key == OF_KEY_DOWN)
   {
      if (GetKeyModifiers() == kModifier_Command)
      {
         ofVec2f dir;
         if (key == OF_KEY_LEFT)
            dir = ofVec2f(-1, 0);
         if (key == OF_KEY_RIGHT)
            dir = ofVec2f(1, 0);
         if (key == OF_KEY_UP)
            dir = ofVec2f(0, -1);
         if (key == OF_KEY_DOWN)
            dir = ofVec2f(0, 1);
         IUIControl::SetNewManualHoverViaArrow(dir);
      }
   }

   if (key == OF_KEY_RETURN)
   {
      if (mMoveModule)
         mMoveModule = nullptr; //drop module

      if (IUIControl::WasLastHoverSetManually())
      {
         TextEntry* textEntry = dynamic_cast<TextEntry*>(gHoveredUIControl);
         if (textEntry != nullptr)
            textEntry->MakeActiveTextEntry(true);
      }
   }

   mZoomer.OnKeyPressed(key);

   if (CharacterFunctions::isDigit((char)key) && (GetKeyModifiers() & kModifier_Alt))
   {
      int num = key - '0';
      assert(num >= 0 && num <= 9);
      gHotBindUIControl[num] = gHoveredUIControl;
   }

   mUILayerModuleContainer.KeyPressed(key, isRepeat);
   mModuleContainer.KeyPressed(key, isRepeat);

   //if (key == '/' && !isRepeat)
   //   ofToggleFullscreen();

   if (key == 'p' && GetKeyModifiers() == kModifier_Shift && !isRepeat)
      mAudioPaused = !mAudioPaused;

   if (key == 's' && GetKeyModifiers() == kModifier_Command && !isRepeat)
      SaveCurrentState();

   if (key == 's' && GetKeyModifiers() == (kModifier_Command | kModifier_Shift) && !isRepeat)
      SaveStatePopup();

   if (key == 'l' && GetKeyModifiers() == kModifier_Command && !isRepeat)
      LoadStatePopup();

   //if (key == 'c' && !isRepeat)
   //   mousePressed(GetMouseX(&mModuleContainer), GetMouseY(&mModuleContainer), 0);

   //if (key == '=' && !isRepeat)
   //   ZoomView(.1f);
   //if (key == '-' && !isRepeat)
   //   ZoomView(-.1f);
}

void ModularSynth::KeyReleased(int key)
{
   key = KeyToLower(key);

   //if (key == 'c')
   //   mouseReleased(GetMouseX(&mModuleContainer), GetMouseY(&mModuleContainer), 0);

   mUILayerModuleContainer.KeyReleased(key);
   mModuleContainer.KeyReleased(key);
}

float ModularSynth::GetMouseX(ModuleContainer* context, float rawX /*= FLT_MAX*/)
{
   return ((rawX == FLT_MAX ? mMousePos.x : rawX) + UserPrefs.mouse_offset_x.Get()) / context->GetDrawScale() - context->GetDrawOffset().x;
}

float ModularSynth::GetMouseY(ModuleContainer* context, float rawY /*= FLT_MAX*/)
{
   return ((rawY == FLT_MAX ? mMousePos.y : rawY) + UserPrefs.mouse_offset_y.Get()) / context->GetDrawScale() - context->GetDrawOffset().y;
}

void ModularSynth::SetMousePosition(ModuleContainer* context, float x, float y)
{
   x = (x + context->GetDrawOffset().x) * context->GetDrawScale() - UserPrefs.mouse_offset_x.Get() + mMainComponent->getScreenX();
   y = (y + context->GetDrawOffset().y) * context->GetDrawScale() - UserPrefs.mouse_offset_y.Get() + mMainComponent->getScreenY();
   Desktop::setMousePosition(juce::Point<int>(x, y));
}

bool ModularSynth::IsMouseButtonHeld(int button) const
{
   if (button >= 0 && button < (int)mIsMouseButtonHeld.size())
      return mIsMouseButtonHeld[button];

   return false;
}

bool ModularSynth::ShouldShowGridSnap() const
{
   return (mMoveModule || (!mGroupSelectedModules.empty() && IsMouseButtonHeld(1))) && (GetKeyModifiers() & kModifier_Command);
}

void ModularSynth::MouseMoved(int intX, int intY)
{
   bool changed = (mMousePos.x != intX || mMousePos.y != intY);

   mMousePos.x = intX;
   mMousePos.y = intY;

   if (IsKeyHeld(' ') || mIsMousePanning)
   {
      GetDrawOffset() += (ofVec2f(intX, intY) - mLastMoveMouseScreenPos) / gDrawScale;
      mZoomer.CancelMovement();

      if (UserPrefs.wrap_mouse_on_pan.Get() &&
          (intX <= 0 || intY <= 0 || intX >= ofGetWidth() || intY >= ofGetHeight()))
      {
         int wrappedX = (intX + (int)ofGetWidth()) % (int)ofGetWidth();
         int wrappedY = (intY + (int)ofGetHeight()) % (int)ofGetHeight();

         if (intX == 0 && wrappedX == 0)
            wrappedX = ofGetWidth() - 1;
         if (intY == 0 && wrappedY == 0)
            wrappedY = ofGetHeight() - 1;

         Desktop::setMousePosition(juce::Point<int>(wrappedX + mMainComponent->getScreenX(),
                                                    wrappedY + mMainComponent->getScreenY()));

         intX = wrappedX;
         intY = wrappedY;
      }
   }

   mLastMoveMouseScreenPos = ofVec2f(intX, intY);

   if (changed)
   {
      for (auto* modal : mModalFocusItemStack)
      {
         float x = GetMouseX(modal->GetOwningContainer());
         float y = GetMouseY(modal->GetOwningContainer());
         modal->NotifyMouseMoved(x, y);
      }

      mHideTooltipsUntilMouseMove = false;
   }

   if (mMoveModule)
   {
      float x = GetMouseX(&mModuleContainer);
      float y = GetMouseY(&mModuleContainer);

      float oldX, oldY;
      mMoveModule->GetPosition(oldX, oldY);
      float newX = x + mMoveModuleOffsetX;
      float newY = y + mMoveModuleOffsetY;

      if (ShouldShowGridSnap())
      {
         newX = round(newX / UserPrefs.grid_snap_size.Get()) * UserPrefs.grid_snap_size.Get();
         newY = round((newY - mMoveModule->TitleBarHeight()) / UserPrefs.grid_snap_size.Get()) * UserPrefs.grid_snap_size.Get() + mMoveModule->TitleBarHeight();
         if (GetKeyModifiers() & kModifier_Shift) // Snap to center of the module
         {
            newX -= std::fmod(mMoveModule->GetRect().width / 2, UserPrefs.grid_snap_size.Get());
            newY -= std::fmod(mMoveModule->GetRect().height / 2, UserPrefs.grid_snap_size.Get());
         }
      }

      mMoveModule->Move(newX - oldX, newY - oldY);

      if (GetKeyModifiers() == kModifier_Shift)
      {
         for (auto* module : mModuleContainer.GetModules())
         {
            if (module == mMoveModule)
               continue;
            for (auto* patchCableSource : module->GetPatchCableSources())
            {
               if (patchCableSource->TestHover(x, y))
               {
                  patchCableSource->FindValidTargets();
                  if (!patchCableSource->IsValidTarget(mMoveModule))
                     continue;

                  if (patchCableSource->GetPatchCables().size() == 0)
                  {
                     PatchCableSource::sAllowInsert = false;
                     patchCableSource->SetTarget(mMoveModule);
                     PatchCableSource::sAllowInsert = true;
                     break;
                  }
                  else if (patchCableSource->GetPatchCables().size() == 1 && patchCableSource->GetTarget() != mMoveModule &&
                           mMoveModule->GetPatchCableSource()) //insert
                  {
                     mMoveModule->GetPatchCableSource()->FindValidTargets();
                     if (mMoveModule->GetPatchCableSource()->IsValidTarget(patchCableSource->GetTarget()))
                     {
                        PatchCableSource::sAllowInsert = false;
                        mMoveModule->SetTarget(patchCableSource->GetTarget());
                        patchCableSource->SetTarget(mMoveModule);
                        PatchCableSource::sAllowInsert = true;
                        break;
                     }
                  }
               }
            }

            if (!mHasAutopatchedToTargetDuringDrag)
            {
               const auto patchCableSources = mMoveModule->GetPatchCableSources();
               for (auto* patchCableSource : patchCableSources)
               {
                  if (patchCableSource && patchCableSource->GetTarget() == nullptr && module->HasTitleBar())
                  {
                     ofRectangle titleBarRect(module->GetPosition().x, module->GetPosition().y - IDrawableModule::TitleBarHeight(), module->IClickable::GetDimensions().x, IDrawableModule::TitleBarHeight());
                     if (titleBarRect.contains(patchCableSource->GetPosition().x, patchCableSource->GetPosition().y))
                     {
                        patchCableSource->FindValidTargets();
                        if (patchCableSource->IsValidTarget(module))
                        {
                           PatchCableSource::sAllowInsert = false;
                           patchCableSource->SetTarget(module);
                           mHasAutopatchedToTargetDuringDrag = true;
                           PatchCableSource::sAllowInsert = true;
                           break;
                        }
                     }
                  }
               }
            }
         }
      }
      else
      {
         mHasAutopatchedToTargetDuringDrag = false;
      }

      return;
   }

   if (changed)
   {
      float x = GetMouseX(&mUILayerModuleContainer);
      float y = GetMouseY(&mUILayerModuleContainer);
      mUILayerModuleContainer.MouseMoved(x, y);

      x = GetMouseX(&mModuleContainer);
      y = GetMouseY(&mModuleContainer);
      mModuleContainer.MouseMoved(x, y);
   }

   if (gHoveredUIControl && changed)
   {
      if (!gHoveredUIControl->IsMouseDown())
      {
         float x = GetMouseX(gHoveredUIControl->GetModuleParent()->GetOwningContainer());
         float y = GetMouseY(gHoveredUIControl->GetModuleParent()->GetOwningContainer());

         float uiX, uiY;
         gHoveredUIControl->GetPosition(uiX, uiY);
         float w, h;
         gHoveredUIControl->GetDimensions(w, h);
         const float kHoverBreakDistance = 5;
         if (x < uiX - kHoverBreakDistance || y < uiY - kHoverBreakDistance || x > uiX + w + kHoverBreakDistance || y > uiY + h + kHoverBreakDistance || //moved far enough away from ui control
             (y < gHoveredUIControl->GetModuleParent()->GetPosition().y && uiY > gHoveredUIControl->GetModuleParent()->GetPosition().y)) //hovering over title bar (and it's not the enable/disable checkbox)
            gHoveredUIControl = nullptr;
      }
   }

   gHoveredModule = GetModuleAtCursor();
}

void ModularSynth::MouseDragged(int intX, int intY, int button, const juce::MouseInputSource& source)
{
   float x = GetMouseX(&mModuleContainer);
   float y = GetMouseY(&mModuleContainer);

   mLastMouseDragPos = ofVec2f(x, y);

   if (button == 3)
      return;

   if ((GetKeyModifiers() & kModifier_Alt) && !mHasDuplicatedDuringDrag)
   {
      std::vector<IDrawableModule*> newGroupSelectedModules;
      std::map<IDrawableModule*, IDrawableModule*> oldToNewModuleMap;
      for (auto module : mGroupSelectedModules)
      {
         if (!module->IsSingleton())
         {
            IDrawableModule* newModule = DuplicateModule(module);
            newGroupSelectedModules.push_back(newModule);
            oldToNewModuleMap[module] = newModule;

            if (module == mLastClickedModule)
               mLastClickedModule = newModule;
         }
      }
      for (auto module : newGroupSelectedModules)
      {
         for (auto* cableSource : module->GetPatchCableSources())
         {
            for (auto* cable : cableSource->GetPatchCables())
            {
               if (VectorContains(dynamic_cast<IDrawableModule*>(cable->GetTarget()), mGroupSelectedModules))
               {
                  cableSource->SetPatchCableTarget(cable, oldToNewModuleMap[dynamic_cast<IDrawableModule*>(cable->GetTarget())], false);
               }
            }
         }
      }
      mGroupSelectedModules = newGroupSelectedModules;

      if (mMoveModule && !mMoveModule->IsSingleton())
         mMoveModule = DuplicateModule(mMoveModule);

      mHasDuplicatedDuringDrag = true;
   }

   if (mGroupSelectContext != nullptr)
   {
      const float gx = GetMouseX(mGroupSelectContext);
      const float gy = GetMouseY(mGroupSelectContext);
      ofRectangle rect = ofRectangle(ofPoint(MIN(mClickStartX, gx), MIN(mClickStartY, gy)), ofPoint(MAX(mClickStartX, gx), MAX(mClickStartY, gy)));
      if (rect.width > 10 || rect.height > 10)
      {
         mGroupSelectContext->GetModulesWithinRect(rect, mGroupSelectedModules, true);
         if (mGroupSelectedModules.size() > 0)
         {
            for (int i = (int)mGroupSelectedModules.size() - 1; i >= 0; --i) //do this backwards to preserve existing order
               MoveToFront(mGroupSelectedModules[i]);
         }
      }
      else
      {
         mGroupSelectedModules.clear();
      }
   }
   else if (mLastClickedModule)
   {
      float oldX, oldY;
      mLastClickedModule->GetPosition(oldX, oldY);
      float newX = x + mMoveModuleOffsetX;
      float newY = y + mMoveModuleOffsetY;

      if (ShouldShowGridSnap())
      {
         newX = round(newX / UserPrefs.grid_snap_size.Get()) * UserPrefs.grid_snap_size.Get();
         newY = round((newY - mLastClickedModule->TitleBarHeight()) / UserPrefs.grid_snap_size.Get()) * UserPrefs.grid_snap_size.Get() + mLastClickedModule->TitleBarHeight();
         if (GetKeyModifiers() & kModifier_Shift) // Snap to center of the module
         {
            newX -= std::fmod(mLastClickedModule->GetRect().width / 2, UserPrefs.grid_snap_size.Get());
            newY -= std::fmod(mLastClickedModule->GetRect().height / 2, UserPrefs.grid_snap_size.Get());
         }
      }

      float adjustedDragX = newX - oldX;
      float adjustedDragY = newY - oldY;

      for (auto module : mGroupSelectedModules)
         module->Move(adjustedDragX, adjustedDragY);
   }

   if (mMoveModule)
   {
      float oldX, oldY;
      mMoveModule->GetPosition(oldX, oldY);
      float newX = x + mMoveModuleOffsetX;
      float newY = y + mMoveModuleOffsetY;

      if (ShouldShowGridSnap())
      {
         newX = round(newX / UserPrefs.grid_snap_size.Get()) * UserPrefs.grid_snap_size.Get();
         newY = round((newY - mMoveModule->TitleBarHeight()) / UserPrefs.grid_snap_size.Get()) * UserPrefs.grid_snap_size.Get() + mMoveModule->TitleBarHeight();
         if (GetKeyModifiers() & kModifier_Shift) // Snap to center of the module
         {
            newX -= std::fmod(mMoveModule->GetRect().width / 2, UserPrefs.grid_snap_size.Get());
            newY -= std::fmod(mMoveModule->GetRect().height / 2, UserPrefs.grid_snap_size.Get());
         }
      }

      mMoveModule->Move(newX - oldX, newY - oldY);
   }

   if (mResizeModule)
   {
      float moduleX, moduleY;
      mResizeModule->GetPosition(moduleX, moduleY);
      float newWidth = x - moduleX;
      float newHeight = y - moduleY;
      ofVec2f minimumDimensions = mResizeModule->GetMinimumDimensions();
      newWidth = MAX(newWidth, minimumDimensions.x);
      newHeight = MAX(newHeight, minimumDimensions.y);
      mResizeModule->Resize(newWidth, newHeight);
   }
}

void ModularSynth::MousePressed(int intX, int intY, int button, const juce::MouseInputSource& source)
{
   bool rightButton = button == 2;

   mZoomer.ExitVanityPanningMode();

   mMousePos.x = intX;
   mMousePos.y = intY;
   mLastClickWasEmptySpace = false;

   if (button >= 0 && button < (int)mIsMouseButtonHeld.size())
      mIsMouseButtonHeld[button] = true;

   float x = GetMouseX(&mModuleContainer);
   float y = GetMouseY(&mModuleContainer);

   mLastMouseDragPos = ofVec2f(x, y);
   mGroupSelectContext = nullptr;

   IKeyboardFocusListener::sKeyboardFocusBeforeClick = IKeyboardFocusListener::GetActiveKeyboardFocus();
   IKeyboardFocusListener::ClearActiveKeyboardFocus(K(notifyListeners));

   if (button == 3)
   {
      mClickStartX = x;
      mClickStartY = y;
      mIsMousePanning = true;
      return;
   }

   if (PatchCable::sActivePatchCable != nullptr)
   {
      if (rightButton)
         PatchCable::sActivePatchCable->Destroy(true);
      return;
   }

   if (mMoveModule)
   {
      mMoveModule = nullptr;
      return;
   }

   if (InMidiMapMode())
   {
      if (gBindToUIControl == gHoveredUIControl) //if it's the same, clear it
         gBindToUIControl = nullptr;
      else
         gBindToUIControl = gHoveredUIControl;
      return;
   }

   IDrawableModule* hoveredUIControlModuleParent = nullptr;
   if (gHoveredUIControl != nullptr)
      hoveredUIControlModuleParent = gHoveredUIControl->GetModuleParent();

   if (hoveredUIControlModuleParent != nullptr && !hoveredUIControlModuleParent->IsDeleted() && !hoveredUIControlModuleParent->IsHoveringOverResizeHandle() &&
       !IUIControl::WasLastHoverSetManually() &&
       mGroupSelectedModules.empty() &&
       mQuickSpawn->IsShowing() == false &&
       (GetTopModalFocusItem() == nullptr || hoveredUIControlModuleParent == GetTopModalFocusItem()))
   {
      //if we have a hovered UI control, clamp clicks within its rect and direct them straight to it
      ofVec2f controlClickPos(GetMouseX(hoveredUIControlModuleParent->GetOwningContainer()), GetMouseY(hoveredUIControlModuleParent->GetOwningContainer()));
      controlClickPos -= gHoveredUIControl->GetParent()->GetPosition();

      ofRectangle controlRect = gHoveredUIControl->GetRect(K(local));
      controlClickPos.x = std::clamp(controlClickPos.x, controlRect.getMinX(), controlRect.getMaxX());
      controlClickPos.y = std::clamp(controlClickPos.y, controlRect.getMinY(), controlRect.getMaxY());

      if (hoveredUIControlModuleParent != TheTitleBar)
         mLastClickedModule = hoveredUIControlModuleParent;

      gHoveredUIControl->TestClick(controlClickPos.x, controlClickPos.y, rightButton);
   }
   else
   {
      if (GetTopModalFocusItem())
      {
         float modalX = GetMouseX(GetTopModalFocusItem()->GetOwningContainer());
         float modalY = GetMouseY(GetTopModalFocusItem()->GetOwningContainer());
         bool clicked = GetTopModalFocusItem()->TestClick(modalX, modalY, rightButton);
         if (!clicked)
         {
            FloatSliderLFOControl* lfo = dynamic_cast<FloatSliderLFOControl*>(GetTopModalFocusItem());
            if (lfo) //if it's an LFO, don't dismiss it if you're adjusting the slider
            {
               FloatSlider* slider = lfo->GetOwner();
               float uiX, uiY;
               slider->GetPosition(uiX, uiY);
               float w, h;
               slider->GetDimensions(w, h);

               if (x < uiX || y < uiY || x > uiX + w || y > uiY + h)
                  PopModalFocusItem();
            }
            else //otherwise, always dismiss if you click outside it
            {
               PopModalFocusItem();
            }
         }
         else
         {
            return;
         }
      }

      IDrawableModule* clickedModule = GetModuleAtCursor();

      for (auto cable : mPatchCables)
      {
         bool checkCable = true;
         if (clickedModule != nullptr) //if we clicked on a module
         {
            IDrawableModule* targetedModule = (cable->GetTarget() != nullptr) ? cable->GetTarget()->GetModuleParent() : nullptr;
            if (targetedModule == nullptr || (clickedModule != targetedModule && clickedModule != targetedModule->GetParent())) //and it's not the module the cable is connected to, or its parent
            {
               if (clickedModule == GetTopModalFocusItem() || clickedModule->AlwaysOnTop() || //and the module is sorted above the source of the cable
                   mModuleContainer.IsHigherThan(clickedModule, cable->GetOwningModule()))
               {
                  checkCable = false; //don't test this cable
               }
            }
         }

         bool cableClicked = false;
         if (checkCable)
            cableClicked = cable->TestClick(x, y, rightButton);
         if (cableClicked)
            return;
      }

      mClickStartX = x;
      mClickStartY = y;
      if (clickedModule == nullptr)
      {
         if (rightButton)
         {
            mIsMousePanning = true;
         }
         else
         {
            bool beginGroupSelect = true;

            //only start lassoing if we have a bit of space from a module, to avoid a common issue I'm seeing where people lasso accidentally
            if (GetModuleAtCursor(7, 7) || GetModuleAtCursor(-7, 7) || GetModuleAtCursor(7, -7) || GetModuleAtCursor(-7, -7))
               beginGroupSelect = false;

            if (beginGroupSelect)
            {
               SetGroupSelectContext(&mModuleContainer);
               gHoveredUIControl = nullptr;
            }
         }
      }
      if (clickedModule != nullptr && clickedModule != TheTitleBar)
      {
         mLastClickedModule = clickedModule;
         mMoveModuleOffsetX = clickedModule->GetPosition().x - x;
         mMoveModuleOffsetY = clickedModule->GetPosition().y - y;
      }
      else
      {
         mLastClickedModule = nullptr;
      }
      mHasDuplicatedDuringDrag = false;

      if (mGroupSelectedModules.empty() == false)
      {
         if (!VectorContains(clickedModule, mGroupSelectedModules))
            mGroupSelectedModules.clear();
         return;
      }

      if (clickedModule)
      {
         x = GetMouseX(clickedModule->GetModuleParent()->GetOwningContainer());
         y = GetMouseY(clickedModule->GetModuleParent()->GetOwningContainer());
         CheckClick(clickedModule, x, y, rightButton);
      }
      else if (TheSaveDataPanel != nullptr)
      {
         TheSaveDataPanel->SetModule(nullptr);
      }

      if (clickedModule == nullptr)
         mLastClickWasEmptySpace = true;

      if (mQuickSpawn != nullptr && mQuickSpawn->IsShowing() && clickedModule != mQuickSpawn)
         mQuickSpawn->Hide();
   }
}

void ModularSynth::ToggleQuickSpawn()
{
   if (mQuickSpawn != nullptr && mQuickSpawn->IsShowing())
   {
      mQuickSpawn->Hide();
   }
   else
   {
      mQuickSpawn->ShowSpawnCategoriesPopup();
   }
}

void ModularSynth::MouseScrolled(float xScroll, float yScroll, bool isSmoothScroll, bool isInvertedScroll, bool canZoomCanvas)
{
   xScroll *= UserPrefs.scroll_multiplier_horizontal.Get();
   yScroll *= UserPrefs.scroll_multiplier_vertical.Get();

   if (IsKeyHeld(' ') || (GetModuleAtCursor() == nullptr && gHoveredUIControl == nullptr) || (dynamic_cast<Prefab*>(GetModuleAtCursor()) != nullptr && gHoveredUIControl == nullptr))
   {
      if (canZoomCanvas)
         ZoomView(yScroll / 50, true);
   }
   else if (gHoveredUIControl)
   {
#if JUCE_WINDOWS
      yScroll += xScroll / 4; //taking advantage of logitech horizontal scroll wheel
#endif

      TextEntry* textEntry = dynamic_cast<TextEntry*>(gHoveredUIControl);
      if (textEntry)
      {
         if (isSmoothScroll) //slow this down into steps if you're using a smooth trackpad
         {
            if (fabs(yScroll) < .1f) //need more than a miniscule change
               return;
            static float sLastSmoothScrollTimeMs = -999;
            if (sLastSmoothScrollTimeMs + 100 > gTime)
               return;
            sLastSmoothScrollTimeMs = gTime;
         }
         float val = textEntry->GetValue();
         float change = yScroll > 0 ? 1 : -1;
         if (GetKeyModifiers() & kModifier_Shift)
            change *= .01f;
         float min, max;
         textEntry->GetRange(min, max);
         textEntry->SetValue(std::clamp(val + change, min, max), NextBufferTime(false));
         return;
      }

      float val = gHoveredUIControl->GetMidiValue();
      float movementScale = 3;
      FloatSlider* floatSlider = dynamic_cast<FloatSlider*>(gHoveredUIControl);
      IntSlider* intSlider = dynamic_cast<IntSlider*>(gHoveredUIControl);
      ClickButton* clickButton = dynamic_cast<ClickButton*>(gHoveredUIControl);
      if (floatSlider || intSlider)
      {
         float w, h;
         gHoveredUIControl->GetDimensions(w, h);
         movementScale = 200.0f / w;
      }

      if (GetKeyModifiers() & kModifier_Shift)
         movementScale *= .01f;

      if (clickButton)
         return;

      float change = yScroll / 100 * movementScale;

      if (floatSlider && floatSlider->GetModulator() && floatSlider->GetModulator()->Active() && floatSlider->GetModulator()->CanAdjustRange())
      {
         IModulator* modulator = floatSlider->GetModulator();
         float min = floatSlider->GetMin();
         float max = floatSlider->GetMax();
         float modMin = ofMap(modulator->GetMin(), min, max, 0, 1);
         float modMax = ofMap(modulator->GetMax(), min, max, 0, 1);

         modulator->GetMin() = ofMap(modMin - change, 0, 1, min, max, K(clamp));
         modulator->GetMax() = ofMap(modMax + change, 0, 1, min, max, K(clamp));

         return;
      }

      if (gHoveredUIControl->InvertScrollDirection())
         val -= change;
      else
         val += change;
      val = ofClamp(val, 0, 1);
      gHoveredUIControl->SetFromMidiCC(val, NextBufferTime(false), false);

      gHoveredUIControl->NotifyMouseScrolled(GetMouseX(&mModuleContainer), GetMouseY(&mModuleContainer), xScroll, yScroll, isSmoothScroll, isInvertedScroll);
   }
   else
   {
      IDrawableModule* module = GetModuleAtCursor();
      if (module)
         module->NotifyMouseScrolled(GetMouseX(module->GetOwningContainer()), GetMouseY(module->GetOwningContainer()), xScroll, yScroll, isSmoothScroll, isInvertedScroll);
   }
}

void ModularSynth::MouseMagnify(int intX, int intY, float scaleFactor, const juce::MouseInputSource& source)
{
   mMousePos.x = intX;
   mMousePos.y = intY;
   ZoomView(scaleFactor - 1, true);
}

bool ModularSynth::InMidiMapMode()
{
   return IsKeyHeld('m', kModifier_Shift);
}

bool ModularSynth::ShouldAccentuateActiveModules() const
{
   return IsKeyHeld('s', kModifier_Shift);
}

bool ModularSynth::ShouldDimModule(IDrawableModule* module)
{
   if (TheSynth->GetGroupSelectedModules().empty() == false)
   {
      if (!VectorContains(module->GetModuleParent(), TheSynth->GetGroupSelectedModules()))
         return true;
   }

   if (PatchCable::sActivePatchCable &&
       (PatchCable::sActivePatchCable->GetConnectionType() != kConnectionType_Modulator && PatchCable::sActivePatchCable->GetConnectionType() != kConnectionType_UIControl && PatchCable::sActivePatchCable->GetConnectionType() != kConnectionType_ValueSetter) &&
       !PatchCable::sActivePatchCable->IsValidTarget(module))
   {
      return true;
   }

   if (TheSynth->GetHeldSample() != nullptr && !module->CanDropSample())
      return true;

   if (ScriptModule::sHasLoadedUntrustedScript &&
       dynamic_cast<ScriptModule*>(module) == nullptr &&
       dynamic_cast<ScriptWarningPopup*>(module) == nullptr)
      return true;

   return false;
}

void ModularSynth::RegisterPatchCable(PatchCable* cable)
{
   mPatchCables.push_back(cable);
}

void ModularSynth::UnregisterPatchCable(PatchCable* cable)
{
   RemoveFromVector(cable, mPatchCables);
}

void ModularSynth::PushModalFocusItem(IDrawableModule* item)
{
   mModalFocusItemStack.push_back(item);
}

void ModularSynth::PopModalFocusItem()
{
   if (mModalFocusItemStack.empty() == false)
      mModalFocusItemStack.pop_back();
}

IDrawableModule* ModularSynth::GetTopModalFocusItem() const
{
   if (mModalFocusItemStack.empty())
      return nullptr;
   return mModalFocusItemStack.back();
}

bool ModularSynth::IsModalFocusItem(IDrawableModule* item) const
{
   return std::find(mModalFocusItemStack.begin(), mModalFocusItemStack.end(), item) == mModalFocusItemStack.end();
}

IDrawableModule* ModularSynth::GetModuleAtCursor(int offsetX /*=0*/, int offsetY /*=0*/)
{
   float x = GetMouseX(&mUILayerModuleContainer) + offsetX;
   float y = GetMouseY(&mUILayerModuleContainer) + offsetY;
   IDrawableModule* uiLayerModule = mUILayerModuleContainer.GetModuleAt(x, y);
   if (uiLayerModule)
      return uiLayerModule;

   x = GetMouseX(&mModuleContainer) + offsetX;
   y = GetMouseY(&mModuleContainer) + offsetY;
   return mModuleContainer.GetModuleAt(x, y);
}

void ModularSynth::CheckClick(IDrawableModule* clickedModule, float x, float y, bool rightButton)
{
   if (clickedModule != TheTitleBar)
      MoveToFront(clickedModule);

   //check to see if we clicked in the move area
   ofRectangle moduleRect = clickedModule->GetRect();
   float modulePosX = x - moduleRect.x;
   float modulePosY = y - moduleRect.y;

   if (modulePosY < 0 && clickedModule != TheTitleBar && (!clickedModule->HasEnabledCheckbox() || modulePosX > 20) && modulePosX < moduleRect.width - 15)
      SetMoveModule(clickedModule, moduleRect.x - x, moduleRect.y - y, false);

   float parentX = 0;
   float parentY = 0;
   if (clickedModule->GetParent())
      clickedModule->GetParent()->GetPosition(parentX, parentY);

   //do the regular click
   clickedModule->TestClick(x - parentX, y - parentY, rightButton);
}

void ModularSynth::MoveToFront(IDrawableModule* module)
{
   if (module->GetOwningContainer())
      module->GetOwningContainer()->MoveToFront(module);
}

void ModularSynth::OnModuleDeleted(IDrawableModule* module)
{
   if (!module->CanBeDeleted() || module->IsDeleted())
      return;

   mDeletedModules.push_back(module);

   mAudioThreadMutex.Lock("delete");

   std::list<PatchCable*> cablesToRemove;
   for (auto* cable : mPatchCables)
   {
      if (cable->GetOwningModule() == module)
         cablesToRemove.push_back(cable);
   }
   for (auto* cable : cablesToRemove)
      RemoveFromVector(cable, mPatchCables);

   RemoveFromVector(dynamic_cast<IAudioSource*>(module), mSources);
   RemoveFromVector(module, mLissajousDrawers);
   TheTransport->RemoveAudioPoller(dynamic_cast<IAudioPoller*>(module));
   //delete module; TODO(Ryan) deleting is hard... need to clear out everything with a reference to this, or switch to smart pointers

   if (module == TheChaosEngine)
      TheChaosEngine = nullptr;
   if (module == TheLFOController)
      TheLFOController = nullptr;

   mAudioThreadMutex.Unlock();
}

void ModularSynth::MouseReleased(int intX, int intY, int button, const juce::MouseInputSource& source)
{
   mMousePos.x = intX;
   mMousePos.y = intY;
   mMouseMovedSignificantlySincePressed = source.hasMovedSignificantlySincePressed();

   if (button >= 0 && button < (int)mIsMouseButtonHeld.size())
      mIsMouseButtonHeld[button] = false;

   mIsMousePanning = false;

   bool rightButton = button == 2;

   if (button == 3)
      return;

   float x = GetMouseX(&mModuleContainer);
   float y = GetMouseY(&mModuleContainer);

   if (GetTopModalFocusItem())
   {
      GetTopModalFocusItem()->MouseReleased();
   }

   if (mResizeModule)
      mResizeModule = nullptr;

   if (mMoveModule)
   {
      Prefab::sJustReleasedModule = mMoveModule;
      if (!mMouseMovedSignificantlySincePressed)
      {
         if (mMoveModule->WasMinimizeAreaClicked())
         {
            mMoveModule->ToggleMinimized();
            mMoveModule = nullptr;
         }

         if (!mMoveModuleCanStickToCursor)
            mMoveModule = nullptr;
      }
      else
      {
         mMoveModule = nullptr;
      }
   }

   mUILayerModuleContainer.MouseReleased();
   mModuleContainer.MouseReleased();

   if (mHeldSample)
   {
      IDrawableModule* module = GetModuleAtCursor();
      if (module)
      {
         float moduleX, moduleY;
         module->GetPosition(moduleX, moduleY);
         module->SampleDropped(x - moduleX, y - moduleY, GetHeldSample());
      }
      ClearHeldSample();
   }

   if (!mGroupSelectedModules.empty() && !mMouseMovedSignificantlySincePressed)
      mGroupSelectedModules.clear();

   if (rightButton && mLastClickWasEmptySpace && !mMouseMovedSignificantlySincePressed && mQuickSpawn != nullptr)
      mQuickSpawn->ShowSpawnCategoriesPopup();

   mClickStartX = INT_MAX;
   mClickStartY = INT_MAX;
   mGroupSelectContext = nullptr;
   Prefab::sJustReleasedModule = nullptr;
}

void ModularSynth::AudioOut(float* const* output, int bufferSize, int nChannels)
{
   PROFILER(audioOut_total);

   sAudioThreadId = std::this_thread::get_id();

   static bool sFirst = true;
   if (sFirst)
   {
      FloatVectorOperations::disableDenormalisedNumberSupport();
      sFirst = false;
   }

   if (mAudioPaused)
   {
      for (int ch = 0; ch < nChannels; ++ch)
      {
         for (int i = 0; i < bufferSize; ++i)
            output[ch][i] = 0;
      }
      return;
   }

   ScopedMutex mutex(&mAudioThreadMutex, "audioOut()");

   /////////// AUDIO PROCESSING STARTS HERE /////////////
   mNoteOutputQueue->Process();

   int oversampling = UserPrefs.oversampling.Get();

   assert(bufferSize * oversampling == mIOBufferSize);
   assert(nChannels == (int)mOutputBuffers.size());
   assert(mIOBufferSize == gBufferSize); //need to be the same for now
   //if we want these different, need to fix outBuffer here, and also fix audioIn()
   //by now, many assumptions will have to be fixed to support mIOBufferSize and gBufferSize diverging
   for (int ioOffset = 0; ioOffset < mIOBufferSize; ioOffset += gBufferSize)
   {
      for (size_t i = 0; i < mOutputBuffers.size(); ++i)
         Clear(mOutputBuffers[i], mIOBufferSize);

      double elapsed = gInvSampleRateMs * mIOBufferSize;
      gTime += elapsed;
      TheTransport->Advance(elapsed);

      //process all audio
      for (int i = 0; i < mSources.size(); ++i)
         mSources[i]->Process(gTime);

      if (gTime - mLastClapboardTime < 100)
      {
         for (int ch = 0; ch < nChannels; ++ch)
         {
            for (int i = 0; i < bufferSize; ++i)
            {
               float sample = sin(GetPhaseInc(440) * i) * (1 - ((gTime - mLastClapboardTime) / 100));
               output[ch][i] = sample;
            }
         }
      }

      //put it into speakers
      for (int ch = 0; ch < nChannels; ++ch)
      {
         if (oversampling == 1)
         {
            BufferCopy(output[ch], mOutputBuffers[ch], gBufferSize);
            if (ch < 2)
               mGlobalRecordBuffer->WriteChunk(output[ch], bufferSize, ch);
         }
         else
         {
            for (int sampleIndex = 0; sampleIndex < gBufferSize / oversampling; ++sampleIndex)
            {
               output[ch][sampleIndex] = 0;
               for (int subsampleIndex = 0; subsampleIndex < oversampling; ++subsampleIndex)
               {
                  float sample = mOutputBuffers[ch][sampleIndex * oversampling + subsampleIndex];
                  output[ch][sampleIndex] += sample / oversampling;
                  if (ch < 2)
                     mGlobalRecordBuffer->Write(sample, ch);
               }
            }
         }
      }
   }

   /////////// AUDIO PROCESSING ENDS HERE /////////////
   mRecordingLength += bufferSize * oversampling;
   mRecordingLength = MIN(mRecordingLength, mGlobalRecordBuffer->Size());

   Profiler::PrintCounters();
}

void ModularSynth::AudioIn(const float* const* input, int bufferSize, int nChannels)
{
   if (mAudioPaused)
      return;

   ScopedMutex mutex(&mAudioThreadMutex, "audioIn()");

   int oversampling = UserPrefs.oversampling.Get();

   assert(bufferSize * oversampling == mIOBufferSize);
   assert(nChannels == (int)mInputBuffers.size());

   for (int i = 0; i < nChannels; ++i)
   {
      if (oversampling == 1)
      {
         BufferCopy(mInputBuffers[i], input[i], bufferSize);
      }
      else
      {
         for (int sampleIndex = 0; sampleIndex < bufferSize * oversampling; ++sampleIndex)
         {
            mInputBuffers[i][sampleIndex] = input[i][sampleIndex / oversampling];
         }
      }
   }
}

float* ModularSynth::GetInputBuffer(int channel)
{
   assert(channel >= 0 && channel < mInputBuffers.size());
   return mInputBuffers[channel];
}

float* ModularSynth::GetOutputBuffer(int channel)
{
   assert(channel >= 0 && channel < mOutputBuffers.size());
   return mOutputBuffers[channel];
}

void ModularSynth::TriggerClapboard()
{
   mLastClapboardTime = gTime; //for synchronizing internally recorded audio and externally recorded video
}

void ModularSynth::FilesDropped(std::vector<std::string> files, int intX, int intY)
{
   if (files.size() > 0)
   {
      float x = GetMouseX(&mModuleContainer, intX);
      float y = GetMouseY(&mModuleContainer, intY);
      IDrawableModule* target = GetModuleAtCursor();

      if (files.size() == 1 && juce::String(files[0]).endsWith(".bsk"))
      {
         LoadState(files[0]);
         return;
      }

      if (target != nullptr)
      {
         float moduleX, moduleY;
         target->GetPosition(moduleX, moduleY);
         x -= moduleX;
         y -= moduleY;
         target->FilesDropped(files, x, y);
      }
   }
}

struct SourceDepInfo
{
   SourceDepInfo(IAudioSource* me)
   : mMe(me)
   {}
   IAudioSource* mMe;
   std::vector<IAudioSource*> mDeps;
};

void ModularSynth::ArrangeAudioSourceDependencies()
{
   if (mIsLoadingState)
   {
      mArrangeDependenciesWhenLoadCompletes = true;
      return;
   }

   //ofLog() << "Calculating audio source dependencies:";

   std::vector<SourceDepInfo> deps;
   for (int i = 0; i < mSources.size(); ++i)
      deps.push_back(SourceDepInfo(mSources[i]));

   for (int i = 0; i < mSources.size(); ++i)
   {
      for (int j = 0; j < mSources.size(); ++j)
      {
         for (int k = 0; k < mSources[i]->GetNumTargets(); ++k)
         {
            if (mSources[i]->GetTarget(k) != nullptr &&
                mSources[i]->GetTarget(k) == dynamic_cast<IAudioReceiver*>(mSources[j]))
            {
               deps[j].mDeps.push_back(mSources[i]);
            }
         }
      }
   }

   /*for (int i=0; i<deps.size(); ++i)
   {
      string depStr;
      for (int j=0;j<deps[i].mDeps.size();++j)
      {
         depStr += dynamic_cast<IDrawableModule*>(deps[i].mDeps[j])->Name();
         if (j<deps[i].mDeps.size()-1)
            depStr += ", ";
      }
      ofLog() << dynamic_cast<IDrawableModule*>(deps[i].mMe)->Name() << "depends on:" << depStr;
   }*/

   //TODO(Ryan) detect circular dependencies
   const int kMaxLoopCount = 1000; //how many times we loop over the graph before deciding that it must contain a circular dependency
   mSources.clear();
   int loopCount = 0;
   while (deps.size() > 0 && loopCount < kMaxLoopCount) //stupid circular dependency detection, make better
   {
      for (int i = 0; i < deps.size(); ++i)
      {
         bool hasDeps = false;
         for (int j = 0; j < deps[i].mDeps.size(); ++j)
         {
            bool found = false;
            for (int k = 0; k < mSources.size(); ++k)
            {
               if (deps[i].mDeps[j] == mSources[k])
                  found = true;
            }
            if (!found) //has a dep that hasn't been added yet
               hasDeps = true;
         }
         if (!hasDeps)
         {
            mSources.push_back(deps[i].mMe);
            deps.erase(deps.begin() + i);
            i -= 1;
         }
      }
      ++loopCount;
   }

   if (loopCount == kMaxLoopCount) //circular dependency, don't lose the rest of the sources
   {
      mHasCircularDependency = true;
      ofLog() << "circular dependency detected";
      for (int i = 0; i < deps.size(); ++i)
         mSources.push_back(deps[i].mMe);
      FindCircularDependencies();
   }
   else
   {
      if (mHasCircularDependency) //we used to have a circular dependency, now we don't
         ClearCircularDependencyMarkers();
      mHasCircularDependency = false;
   }

   /*ofLog() << "new ordering:";
   for (int i=0; i<mSources.size(); ++i)
      ofLog() << dynamic_cast<IDrawableModule*>(mSources[i])->Name();*/
}

void ModularSynth::FindCircularDependencies()
{
   ClearCircularDependencyMarkers();
   for (int i = 0; i < mSources.size(); ++i)
   {
      std::list<IAudioSource*> chain;
      if (FindCircularDependencySearch(chain, mSources[i]))
         break;
   }
}

bool ModularSynth::FindCircularDependencySearch(std::list<IAudioSource*> chain, IAudioSource* searchFrom)
{
   /*std::string debugString = "FindCircularDependencySearch(): ";
   for (auto& element : chain)
      debugString += dynamic_cast<IDrawableModule*>(element)->GetDisplayName() + "->";
   debugString += dynamic_cast<IDrawableModule*>(searchFrom)->GetDisplayName();
   ofLog() << debugString;*/

   chain.push_back(searchFrom);
   IAudioSource* end = chain.back();
   for (int i = 0; i < end->GetNumTargets(); ++i)
   {
      IAudioReceiver* receiver = end->GetTarget(i);
      IAudioSource* targetAsSource = dynamic_cast<IAudioSource*>(receiver);
      if (targetAsSource != nullptr)
      {
         if (ListContains(targetAsSource, chain)) //found a circular dependency
         {
            std::string debugString = "FindCircularDependencySearch(): found! ";
            for (auto& element : chain)
               debugString += dynamic_cast<IDrawableModule*>(element)->GetDisplayName() + "->";
            debugString += dynamic_cast<IDrawableModule*>(targetAsSource)->GetDisplayName();
            ofLog() << debugString;

            chain.push_back(targetAsSource);
            std::vector<IAudioSource*> chainVec;
            for (auto& element : chain)
               chainVec.push_back(element);
            for (int j = 0; j < (int)chainVec.size() - 1; ++j)
            {
               IDrawableModule* module = dynamic_cast<IDrawableModule*>(chainVec[j]);
               for (int k = 0; k < (int)module->GetPatchCableSources().size(); ++k)
               {
                  if (dynamic_cast<IAudioSource*>(module->GetPatchCableSource(k)->GetTarget()) == chainVec[j + 1])
                     module->GetPatchCableSource(k)->SetIsPartOfCircularDependency(true);
               }
            }

            return true;
         }
         else
         {
            if (FindCircularDependencySearch(chain, targetAsSource))
               return true;
         }
      }
   }

   return false;
}

void ModularSynth::ClearCircularDependencyMarkers()
{
   for (int i = 0; i < mSources.size(); ++i)
   {
      IDrawableModule* module = dynamic_cast<IDrawableModule*>(mSources[i]);
      for (int j = 0; j < (int)module->GetPatchCableSources().size(); ++j)
         module->GetPatchCableSource(j)->SetIsPartOfCircularDependency(false);
   }
}

void ModularSynth::ResetLayout()
{
   mMainComponent->getTopLevelComponent()->setName("bespoke synth");
   mCurrentSaveStatePath = "";

   mModuleContainer.Clear();
   mUILayerModuleContainer.Clear();

   for (int i = 0; i < mDeletedModules.size(); ++i)
      delete mDeletedModules[i];

   mDeletedModules.clear();
   mSources.clear();
   mLissajousDrawers.clear();
   mMoveModule = nullptr;
   TheTransport->ClearListenersAndPollers();
   TheScale->ClearListeners();
   LFOPool::Shutdown();
   IKeyboardFocusListener::ClearActiveKeyboardFocus(!K(notifyListeners));
   ScriptModule::sBackgroundTextString = "";
   ScriptModule::sHasLoadedUntrustedScript = false; //reset
   ScriptModule::sScriptsRequestingInitExecution.clear();

   mErrors.clear();

   std::vector<PatchCable*> cablesToDelete = mPatchCables;
   for (auto cable : cablesToDelete)
      delete cable;
   assert(mPatchCables.size() == 0); //everything should have been cleared out by that

   gBindToUIControl = nullptr;
   mModalFocusItemStack.clear();
   gHoveredUIControl = nullptr;
   mLastClickedModule = nullptr;

   LFOPool::Init();
   mZoomer.Init();

   delete TheTitleBar;
   delete TheSaveDataPanel;
   delete mQuickSpawn;
   delete mUserPrefsEditor;
   delete mNoteOutputQueue;

   TitleBar* titleBar = new TitleBar();
   titleBar->SetPosition(0, 0);
   titleBar->SetName("titlebar");
   titleBar->SetTypeName("titlebar", kModuleCategory_Other);
   titleBar->CreateUIControls();
   titleBar->SetModuleFactory(&mModuleFactory);
   titleBar->Init();
   mUILayerModuleContainer.AddModule(titleBar);

   if (UserPrefs.show_minimap.Get())
   {
      mMinimap = std::make_unique<Minimap>();
      mMinimap->SetName("minimap");
      mMinimap->SetTypeName("minimap", kModuleCategory_Other);
      mMinimap->SetShouldDrawOutline(false);
      mMinimap->CreateUIControls();
      mMinimap->SetShowing(true);
      mMinimap->Init();
      mUILayerModuleContainer.AddModule(mMinimap.get());

      TitleBar::sShowInitialHelpOverlay = false; //don't show initial help popup, it collides with minimap, and a user who has customized the settings likely doesn't need it
   }

   ModuleSaveDataPanel* saveDataPanel = new ModuleSaveDataPanel();
   saveDataPanel->SetPosition(-200, 50);
   saveDataPanel->SetName("savepanel");
   saveDataPanel->CreateUIControls();
   saveDataPanel->Init();
   mModuleContainer.AddModule(saveDataPanel);

   mQuickSpawn = new QuickSpawnMenu();
   mQuickSpawn->SetName("quickspawn");
   mQuickSpawn->CreateUIControls();
   mQuickSpawn->Init();
   mUILayerModuleContainer.AddModule(mQuickSpawn);
   mModuleContainer.AddModule(mQuickSpawn->GetMainContainerFollower());

   mUserPrefsEditor = new UserPrefsEditor();
   mUserPrefsEditor->SetName("userprefseditor");
   mUserPrefsEditor->SetTypeName("userprefseditor", kModuleCategory_Other);
   mUserPrefsEditor->CreateUIControls();
   mUserPrefsEditor->Init();
   mUserPrefsEditor->SetShowing(false);
   mModuleContainer.AddModule(mUserPrefsEditor);
   if (mFatalError != "")
   {
      mUserPrefsEditor->Show();
      TheTitleBar->SetShowing(false);
   }

   GetDrawOffset().set(0, 0);

   SetUIScale(UserPrefs.ui_scale.Get());

   mNoteOutputQueue = new NoteOutputQueue();
}

bool ModularSynth::LoadLayoutFromFile(std::string jsonFile, bool makeDefaultLayout /*= true*/)
{
   ofLog() << "Loading layout: " << jsonFile;
   mLoadedLayoutPath = String(jsonFile).replace(ofToDataPath("").c_str(), "").toStdString();

   ofxJSONElement root;
   bool loaded = root.open(jsonFile);

   if (!loaded)
   {
      LogEvent("Couldn't load, error parsing " + jsonFile, kLogEventType_Error);
      LogEvent("Try loading it up in a json validator", kLogEventType_Error);
      return false;
   }

   LoadLayout(root);

   if (juce::String(jsonFile).endsWith("blank.json"))
   {
      IDrawableModule* gain = FindModule("gain");
      IDrawableModule* splitter = FindModule("splitter");
      IDrawableModule* output1 = FindModule("output 1");
      IDrawableModule* output2 = FindModule("output 2");
      if (output1 != nullptr && output1->GetPosition().y > ofGetHeight() / gDrawScale - 40)
      {
         float offset = ofGetHeight() / gDrawScale - output1->GetPosition().y - 40;
         if (gain != nullptr)
            gain->SetPosition(gain->GetPosition().x, gain->GetPosition().y + offset);
         if (splitter != nullptr)
            splitter->SetPosition(splitter->GetPosition().x, splitter->GetPosition().y + offset);
         if (output1 != nullptr)
            output1->SetPosition(output1->GetPosition().x, output1->GetPosition().y + offset);
         if (output2 != nullptr)
            output2->SetPosition(output2->GetPosition().x, output2->GetPosition().y + offset);
      }

      if (output2 != nullptr && output2->GetPosition().x > ofGetWidth() / gDrawScale - 100)
      {
         float offset = ofGetWidth() / gDrawScale - output2->GetPosition().x - 100;
         if (gain != nullptr)
            gain->SetPosition(gain->GetPosition().x + offset, gain->GetPosition().y);
         if (splitter != nullptr)
            splitter->SetPosition(splitter->GetPosition().x + offset, splitter->GetPosition().y);
         if (output1 != nullptr)
            output1->SetPosition(output1->GetPosition().x + offset, output1->GetPosition().y);
         if (output2 != nullptr)
            output2->SetPosition(output2->GetPosition().x + offset, output2->GetPosition().y);
      }
   }

   if (makeDefaultLayout)
      UpdateUserPrefsLayout();

   return true;
}

bool ModularSynth::LoadLayoutFromString(std::string jsonString)
{
   ofxJSONElement root;
   bool loaded = root.parse(jsonString);

   if (!loaded)
   {
      LogEvent("Couldn't load, error parsing json string", kLogEventType_Error);
      ofLog() << jsonString;
      return false;
   }

   LoadLayout(root);
   return true;
}

void ModularSynth::LoadLayout(ofxJSONElement json)
{
   ScriptModule::UninitializePython();
   Transport::sDoEventLookahead = false;

   //ofLoadURLAsync("http://bespoke.com/telemetry/"+jsonFile);

   ScopedMutex mutex(&mAudioThreadMutex, "LoadLayout()");
   std::lock_guard<std::recursive_mutex> renderLock(mRenderLock);

   ResetLayout();

   mModuleContainer.LoadModules(json["modules"]);
   mUILayerModuleContainer.LoadModules(json["ui_modules"]);

   //timer.PrintCosts();

   mZoomer.LoadFromSaveData(json["zoomlocations"]);
   ArrangeAudioSourceDependencies();
}

void ModularSynth::UpdateUserPrefsLayout()
{
   //mUserPrefsFile["layout"] = mLoadedLayoutPath;
   //mUserPrefsFile.save(GetUserPrefsPath(), true);
}

void ModularSynth::AddExtraPoller(IPollable* poller)
{
   if (!ListContains(poller, mExtraPollers))
      mExtraPollers.push_front(poller);
}

void ModularSynth::RemoveExtraPoller(IPollable* poller)
{
   if (ListContains(poller, mExtraPollers))
      mExtraPollers.remove(poller);
}

IDrawableModule* ModularSynth::CreateModule(const ofxJSONElement& moduleInfo)
{
   IDrawableModule* module = nullptr;

   try
   {
      if (moduleInfo["comment_out"].asBool()) //hack since json doesn't allow comments
         return nullptr;

      std::string type = moduleInfo["type"].asString();
      type = ModuleFactory::FixUpTypeName(type);

      try
      {
         if (type == "transport")
            module = TheTransport;
         else if (type == "scale")
            module = TheScale;
         else
            module = mModuleFactory.MakeModule(type);

         if (module == nullptr)
         {
            LogEvent("Couldn't create unknown module type \"" + type + "\"", kLogEventType_Error);
            return nullptr;
         }

         if (module->IsSingleton() == false)
            module->CreateUIControls();
         module->LoadBasics(moduleInfo, type);
         assert(strlen(module->Name()) > 0);
      }
      catch (UnknownModuleException& e)
      {
         LogEvent("Couldn't find referenced module \"" + e.mSearchName + "\" when loading \"" + moduleInfo["name"].asString() + "\"", kLogEventType_Error);
      }
   }
   catch (Json::LogicError& e)
   {
      TheSynth->LogEvent(__PRETTY_FUNCTION__ + std::string(" json error: ") + e.what(), kLogEventType_Error);
   }

   return module;
}

void ModularSynth::SetUpModule(IDrawableModule* module, const ofxJSONElement& moduleInfo)
{
   assert(module != nullptr);

   try
   {
      mIsLoadingModule = true;
      module->LoadLayoutBase(moduleInfo);
      mIsLoadingModule = false;
   }
   catch (UnknownModuleException& e)
   {
      LogEvent("Couldn't find referenced module \"" + e.mSearchName + "\" when setting up \"" + moduleInfo["name"].asString() + "\"", kLogEventType_Error);
   }
}

void ModularSynth::OnModuleAdded(IDrawableModule* module)
{
   IAudioSource* source = dynamic_cast<IAudioSource*>(module);
   if (source)
      mSources.push_back(source);
}

void ModularSynth::AddDynamicModule(IDrawableModule* module)
{
   mModuleContainer.AddModule(module);
}

void ModularSynth::ScheduleEnvelopeEditorSpawn(ADSRDisplay* adsrDisplay)
{
   mScheduledEnvelopeEditorSpawnDisplay = adsrDisplay;
}

IDrawableModule* ModularSynth::FindModule(std::string name, bool fail)
{
   if (name[0] == '$')
   {
      if (Prefab::sLoadingPrefab)
      {
         if (fail)
            throw UnknownModuleException(name);
         return nullptr;
      }
      return mModuleContainer.FindModule(name.substr(1, name.length() - 1), fail);
   }
   return mModuleContainer.FindModule(IClickable::sPathLoadContext + name, fail);
}

MidiController* ModularSynth::FindMidiController(std::string name, bool fail)
{
   if (name == "")
      return nullptr;

   MidiController* m = nullptr;

   try
   {
      m = dynamic_cast<MidiController*>(FindModule(name, fail));
      if (m == nullptr && fail)
         throw WrongModuleTypeException();
   }
   catch (UnknownModuleException& e)
   {
      LogEvent("Couldn't find referenced midi controller \"" + name + "\"", kLogEventType_Error);
   }
   catch (WrongModuleTypeException& e)
   {
      LogEvent("\"" + name + "\" is not a midicontroller", kLogEventType_Error);
   }

   return m;
}

IUIControl* ModularSynth::FindUIControl(std::string path)
{
   if (path == "")
      return nullptr;
   if (path[0] == '$')
   {
      if (Prefab::sLoadingPrefab)
         return nullptr;
      return mModuleContainer.FindUIControl(path.substr(1, path.length() - 1));
   }
   return mModuleContainer.FindUIControl(IClickable::sPathLoadContext + path);
}

void ModularSynth::GrabSample(ChannelBuffer* data, std::string name, bool window, int numBars)
{
   delete mHeldSample;
   mHeldSample = new Sample();
   mHeldSample->Create(data);
   mHeldSample->SetName(name);
   mHeldSample->SetNumBars(numBars);

   //window sample to avoid clicks
   if (window)
   {
      int length = data->BufferSize();
      const int fadeSamples = 15;
      if (length > fadeSamples * 2) //only window if there's enough space
      {
         for (int i = 0; i < fadeSamples; ++i)
         {
            for (int ch = 0; ch < mHeldSample->NumChannels(); ++ch)
            {
               float fade = float(i) / fadeSamples;
               mHeldSample->Data()->GetChannel(ch)[i] *= fade;
               mHeldSample->Data()->GetChannel(ch)[length - 1 - i] *= fade;
            }
         }
      }
   }
}

void ModularSynth::GrabSample(std::string filePath)
{
   delete mHeldSample;
   mHeldSample = new Sample();
   mHeldSample->Read(filePath.c_str());
}

void ModularSynth::ClearHeldSample()
{
   delete mHeldSample;
   mHeldSample = nullptr;
}

void ModularSynth::LogEvent(std::string event, LogEventType type)
{
   if (type == kLogEventType_Warning)
   {
      !ofLog() << "warning: " << event;
   }
   else if (type == kLogEventType_Error)
   {
      !ofLog() << "error: " << event;
      mErrors.push_back(event);
   }

   mEvents.push_back(LogEventItem(gTime, event, type));
   if (mEvents.size() > 30)
      mEvents.pop_front();
}

IDrawableModule* ModularSynth::DuplicateModule(IDrawableModule* module)
{
   juce::MemoryBlock block;
   {
      FileStreamOut out(block);
      module->SaveState(out);
   }

   ofxJSONElement layoutData;
   module->SaveLayoutBase(layoutData);
   std::vector<IDrawableModule*> modules = mModuleContainer.GetModules();
   std::string newName = GetUniqueName(layoutData["name"].asString(), modules);
   layoutData["name"] = newName;

   IDrawableModule* newModule = CreateModule(layoutData);
   mModuleContainer.AddModule(newModule);
   SetUpModule(newModule, layoutData);
   newModule->Init();

   assert(newModule);

   newModule->SetName(module->Name()); //temporarily rename to the same as what we duplicated, so we can load state properly

   {
      FileStreamIn in(block);
      mIsLoadingModule = true;
      mIsDuplicatingModule = true;
      newModule->LoadState(in, newModule->LoadModuleSaveStateRev(in));
      mIsDuplicatingModule = false;
      mIsLoadingModule = false;
   }

   newModule->SetName(newName.c_str());

   return newModule;
}

ofxJSONElement ModularSynth::GetLayout()
{
   ofxJSONElement root;

   root["modules"] = mModuleContainer.WriteModules();
   root["ui_modules"] = mUILayerModuleContainer.WriteModules();
   root["zoomlocations"] = mZoomer.GetSaveData();

   return root;
}

void ModularSynth::SaveLayout(std::string jsonFile, bool makeDefaultLayout /*= true*/)
{
   if (jsonFile.empty())
      jsonFile = ofToDataPath(mLoadedLayoutPath);

   ofxJSONElement root = GetLayout();
   root.save(jsonFile, true);

   mLoadedLayoutPath = String(jsonFile).replace(ofToDataPath("").c_str(), "").toStdString();

   TheTitleBar->ListLayouts();
   if (makeDefaultLayout)
      UpdateUserPrefsLayout();
}

void ModularSynth::SaveLayoutAsPopup()
{
   FileChooser chooser("Save current layout as...", File(ofToDataPath("layouts/newlayout.json")), "*.json", true, false, GetFileChooserParent());
   if (chooser.browseForFileToSave(true))
      SaveLayout(chooser.getResult().getFullPathName().toStdString());
}

void ModularSynth::SaveCurrentState()
{
   if (mCurrentSaveStatePath.empty() || IsCurrentSaveStateATemplate())
   {
      SaveStatePopup();
      return;
   }

   SaveState(mCurrentSaveStatePath, false);
}

juce::Component* ModularSynth::GetFileChooserParent() const
{
#if BESPOKE_LINUX
   return nullptr;
#else
   return mMainComponent->getTopLevelComponent();
#endif
}

void ModularSynth::SaveStatePopup()
{
   File targetFile;
   String savestateDirPath = ofToDataPath("savestate/");
   String templateName = "";
   String date = ofGetTimestampString("%Y-%m-%d_%H-%M");
   if (IsCurrentSaveStateATemplate())
      templateName = File(mCurrentSaveStatePath).getFileNameWithoutExtension().toStdString() + "_";

   targetFile = File(savestateDirPath + templateName + date + ".bsk");

   FileChooser chooser("Save current state as...", targetFile, "*.bsk", true, false, GetFileChooserParent());
   if (chooser.browseForFileToSave(true))
      SaveState(chooser.getResult().getFullPathName().toStdString(), false);
}

void ModularSynth::LoadStatePopup()
{
   mShowLoadStatePopup = true;
}

void ModularSynth::LoadStatePopupImp()
{
   FileChooser chooser("Load state", File(ofToDataPath("savestate")), "*.bsk;*.bskt", true, false, GetFileChooserParent());
   if (chooser.browseForFileToOpen())
      LoadState(chooser.getResult().getFullPathName().toStdString());
}

void ModularSynth::SaveState(std::string file, bool autosave)
{
   if (!autosave)
   {
      mCurrentSaveStatePath = file;
      std::string filename = File(mCurrentSaveStatePath).getFileName().toStdString();
      mMainComponent->getTopLevelComponent()->setName("bespoke synth - " + filename);
      TheTitleBar->DisplayTemporaryMessage("saved " + filename);
   }

   mAudioThreadMutex.Lock("SaveState()");

   //write to a temp file first, so we don't corrupt data if we crash mid-save
   std::string tmpFilePath = ofToDataPath("tmp");

   {
      FileStreamOut out(tmpFilePath);

      mZoomer.WriteCurrentLocation(-1);
      out << GetLayout().getRawString(true);
      mModuleContainer.SaveState(out);
      mUILayerModuleContainer.SaveState(out);
   }

   juce::File writtenFile(tmpFilePath);
   juce::File targetFile(file);
   writtenFile.copyFileTo(targetFile);

   mAudioThreadMutex.Unlock();
}

void ModularSynth::SetStartupSaveStateFile(std::string bskPath)
{
   mStartupSaveStateFile = std::move(bskPath);
}

void ModularSynth::LoadState(std::string file)
{
   ofLog() << "LoadState() " << file;

   if (!juce::File(file).existsAsFile())
   {
      LogEvent("couldn't find file " + file, kLogEventType_Error);
      return;
   }

   if (mInitialized)
      TitleBar::sShowInitialHelpOverlay = false; //don't show initial help popup

   FileStreamIn in(ofToDataPath(file));

   if (in.Eof())
   {
      LogEvent("File is empty: " + file, kLogEventType_Error);
      return;
   }

   mAudioThreadMutex.Lock("LoadState()");
   LockRender(true);
   mAudioPaused = true;
   mIsLoadingState = true;
   LockRender(false);
   mAudioThreadMutex.Unlock();

   //TODO(Ryan) here's a little hack to allow older BSK files that were saved in 32-bit to load.
   //I guess this could bite me if someone ever has a very massive json. the number corresponds to a long-standing sanity check in FileStreamIn::operator>>(std::string &var), so this shouldn't break any current behavior.
   //this should definitely be removed if anything about the structure of the BSK format changes.
   uint64_t firstLength[1];
   in.Peek(firstLength, sizeof(uint64_t));
   if (firstLength[0] >= FileStreamIn::sMaxStringLength)
      FileStreamIn::s32BitMode = true;

   std::string jsonString;
   in >> jsonString;
   bool layoutLoaded = LoadLayoutFromString(jsonString);

   if (layoutLoaded)
   {
      mIsLoadingModule = true;
      mModuleContainer.LoadState(in);
      if (ModularSynth::sLastLoadedFileSaveStateRev >= 424)
         mUILayerModuleContainer.LoadState(in);
      mIsLoadingModule = false;

      TheTransport->Reset();
   }

   FileStreamIn::s32BitMode = false;

   mCurrentSaveStatePath = file;
   File savePath(mCurrentSaveStatePath);
   std::string filename = savePath.getFileName().toStdString();
   mMainComponent->getTopLevelComponent()->setName("bespoke synth - " + filename);

   mAudioThreadMutex.Lock("LoadState()");
   LockRender(true);
   mAudioPaused = false;
   mIsLoadingState = false;
   LockRender(false);
   mAudioThreadMutex.Unlock();
}

bool ModularSynth::IsCurrentSaveStateATemplate() const
{
   if (mCurrentSaveStatePath == "")
      return false;
   File savePath(mCurrentSaveStatePath);
   return savePath.getFileExtension().toStdString() == ".bskt";
}

IAudioReceiver* ModularSynth::FindAudioReceiver(std::string name, bool fail)
{
   IAudioReceiver* a = nullptr;

   if (name == "")
      return nullptr;

   try
   {
      a = dynamic_cast<IAudioReceiver*>(FindModule(name, fail));
      if (a == nullptr)
         throw WrongModuleTypeException();
   }
   catch (UnknownModuleException& e)
   {
      LogEvent("Couldn't find referenced audio receiver \"" + name + "\"", kLogEventType_Error);
   }
   catch (WrongModuleTypeException& e)
   {
      LogEvent("\"" + name + "\" is not an audio receiver", kLogEventType_Error);
   }

   return a;
}

INoteReceiver* ModularSynth::FindNoteReceiver(std::string name, bool fail)
{
   INoteReceiver* n = nullptr;

   if (name == "")
      return nullptr;

   try
   {
      n = dynamic_cast<INoteReceiver*>(FindModule(name, fail));
      if (n == nullptr)
         throw WrongModuleTypeException();
   }
   catch (UnknownModuleException& e)
   {
      LogEvent("Couldn't find referenced note receiver \"" + name + "\"", kLogEventType_Error);
   }
   catch (WrongModuleTypeException& e)
   {
      LogEvent("\"" + name + "\" is not a note receiver", kLogEventType_Error);
   }

   return n;
}

void ModularSynth::OnConsoleInput(std::string command /* = "" */)
{
   if (command.empty())
      command = mConsoleText;
   std::vector<std::string> tokens = ofSplitString(command, " ", true, true);

   if (tokens.size() > 0)
   {
      if (tokens[0] == "")
      {
      }
      else if (tokens[0] == "clearerrors")
      {
         mErrors.clear();
      }
      else if (tokens[0] == "clearall")
      {
         mAudioThreadMutex.Lock("clearall");
         std::lock_guard<std::recursive_mutex> renderLock(mRenderLock);
         ResetLayout();
         mAudioThreadMutex.Unlock();
      }
      else if (tokens[0] == "load")
      {
         LoadLayout(ofToDataPath(tokens[1]));
      }
      else if (tokens[0] == "save")
      {
         if (tokens.size() > 1)
            SaveLayout(ofToDataPath(tokens[1]));
         else
            SaveLayout();
      }
      else if (tokens[0] == "write")
      {
         SaveOutput();
      }
      else if (tokens[0] == "reconnect")
      {
         ReconnectMidiDevices();
      }
      else if (tokens[0] == "profiler")
      {
         Profiler::ToggleProfiler();
      }
      else if (tokens[0] == "clear")
      {
         mErrors.clear();
         mEvents.clear();
      }
      else if (tokens[0] == "minimizeall")
      {
         const std::vector<IDrawableModule*> modules = mModuleContainer.GetModules();
         for (auto iter = modules.begin(); iter != modules.end(); ++iter)
         {
            (*iter)->SetMinimized(true);
         }
      }
      else if (tokens[0] == "resettime")
      {
         gTime = 0;
      }
      else if (tokens[0] == "hightime")
      {
         gTime += 1000000;
      }
      else if (tokens[0] == "tempo")
      {
         if (tokens.size() >= 2)
         {
            float tempo = atof(tokens[1].c_str());
            if (tempo > 0)
               TheTransport->SetTempo(tempo);
         }
      }
      else if (tokens[0] == "home")
      {
         mZoomer.GoHome();
      }
      else if (tokens[0] == "saveas")
      {
         SaveLayoutAsPopup();
      }
      else if (tokens[0] == "dev")
      {
         gShowDevModules = true;
         TheTitleBar->SetModuleFactory(&mModuleFactory);
      }
      else if (tokens[0] == "dumpmem")
      {
         DumpUnfreedMemory();
      }
      else if (tokens[0] == "savestate")
      {
         if (tokens.size() >= 2)
            SaveState(ofToDataPath("savestate/" + tokens[1]), false);
      }
      else if (tokens[0] == "loadstate")
      {
         if (tokens.size() >= 2)
            LoadState(ofToDataPath("savestate/" + tokens[1]));
      }
      else if (tokens[0] == "s")
      {
         SaveState(ofToDataPath("savestate/quicksave.bsk"), false);
      }
      else if (tokens[0] == "l")
      {
         LoadState(ofToDataPath("savestate/quicksave.bsk"));
      }
      else if (tokens[0] == "getwindowinfo")
      {
         ofLog() << "pos:(" << mMainComponent->getTopLevelComponent()->getPosition().x << ", " << mMainComponent->getTopLevelComponent()->getPosition().y << ") size:(" << ofGetWidth() << ", " << ofGetHeight() << ")";
      }
      else if (tokens[0] == "getmouse")
      {
         ofLog() << "mouse pos raw:(" << mMousePos.x << ", " << mMousePos.y << ") "
                 << "   mouse pos canvas:(" << GetMouseX(&mModuleContainer) << ", " << GetMouseY(&mModuleContainer) << ")";
      }
      else if (tokens[0] == "screenshotmodule")
      {
         TheTitleBar->GetHelpDisplay()->ScreenshotModule(mModuleContainer.GetModules()[0]);
      }
      else if (tokens[0] == "forcecrash")
      {
         Sample* nullPointer = nullptr;
         ofLog() << ofToString(nullPointer->Data()->GetChannel(0)[0]);
      }
      else if (tokens[0] == "dumpstats")
      {
         DumpStats(false, nullptr);
      }
      else
      {
         ofLog() << "Creating: " << mConsoleText;
         ofVec2f grabOffset(-40, 10);
         ModuleFactory::Spawnable spawnable;
         spawnable.mLabel = mConsoleText;
         IDrawableModule* module = SpawnModuleOnTheFly(spawnable, GetMouseX(&mModuleContainer) + grabOffset.x, GetMouseY(&mModuleContainer) + grabOffset.y);
         TheSynth->SetMoveModule(module, grabOffset.x, grabOffset.y, true);
      }
   }
}

void ModularSynth::ClearConsoleInput()
{
   mConsoleText[0] = 0;
   mConsoleEntry->UpdateDisplayString();
}

namespace
{
   class FileTimeComparator
   {
   public:
      int compareElements(juce::File first, juce::File second)
      {
         return (first.getCreationTime() < second.getCreationTime()) ? 1 : ((first.getCreationTime() == second.getCreationTime()) ? 0 : -1);
      }
   };
}

void ModularSynth::DoAutosave()
{
   const int kMaxAutosaveSlots = 10;

   juce::File parentDirectory(ofToDataPath("savestate/autosave"));
   Array<juce::File> autosaveFiles;
   parentDirectory.findChildFiles(autosaveFiles, juce::File::findFiles, false, "*.bsk");
   if (autosaveFiles.size() >= kMaxAutosaveSlots)
   {
      FileTimeComparator cmp;
      autosaveFiles.sort(cmp, false);
      for (int i = kMaxAutosaveSlots; i < autosaveFiles.size(); ++i) //delete oldest files beyond slot limit
         autosaveFiles[i].deleteFile();
   }

   SaveState(ofToDataPath(ofGetTimestampString("savestate/autosave/autosave_%Y-%m-%d_%H-%M-%S.bsk")), true);
}

IDrawableModule* ModularSynth::SpawnModuleOnTheFly(ModuleFactory::Spawnable spawnable, float x, float y, bool addToContainer, std::string name)
{
   if (mInitialized)
      TitleBar::sShowInitialHelpOverlay = false; //don't show initial help popup

   if (sShouldAutosave)
      DoAutosave();

   std::string moduleType = spawnable.mLabel;

   moduleType = ModuleFactory::FixUpTypeName(moduleType);

   if (spawnable.mSpawnMethod == ModuleFactory::SpawnMethod::EffectChain)
      moduleType = "effectchain";

   if (spawnable.mSpawnMethod == ModuleFactory::SpawnMethod::Prefab)
      moduleType = "prefab";

   if (spawnable.mSpawnMethod == ModuleFactory::SpawnMethod::Plugin)
      moduleType = "vstplugin";

   if (spawnable.mSpawnMethod == ModuleFactory::SpawnMethod::MidiController)
      moduleType = "midicontroller";

   if (spawnable.mSpawnMethod == ModuleFactory::SpawnMethod::Preset)
      moduleType = spawnable.mPresetModuleType;

   if (name == "")
      name = moduleType;

   ofxJSONElement dummy;
   dummy["type"] = moduleType;
   std::vector<IDrawableModule*> modules = mModuleContainer.GetModules();
   dummy["name"] = GetUniqueName(name, modules);
   dummy["onthefly"] = true;
   dummy["position"][0u] = x;
   dummy["position"][1u] = y;

   IDrawableModule* module = nullptr;

   try
   {
      ScopedMutex mutex(&mAudioThreadMutex, "CreateModule");
      module = CreateModule(dummy);
      if (module != nullptr)
      {
         if (addToContainer)
            mModuleContainer.AddModule(module);
         SetUpModule(module, dummy);
         module->Init();
      }
   }
   catch (LoadingJSONException& e)
   {
      LogEvent("Error spawning \"" + spawnable.mLabel + "\" on the fly", kLogEventType_Warning);
   }
   catch (UnknownModuleException& e)
   {
      LogEvent("Error spawning \"" + spawnable.mLabel + "\" on the fly, couldn't find \"" + e.mSearchName + "\"", kLogEventType_Warning);
   }

   if (spawnable.mSpawnMethod == ModuleFactory::SpawnMethod::EffectChain)
   {
      EffectChain* effectChain = dynamic_cast<EffectChain*>(module);
      if (effectChain != nullptr)
         effectChain->AddEffect(spawnable.mLabel, spawnable.mLabel, K(onTheFly));
   }

   if (spawnable.mSpawnMethod == ModuleFactory::SpawnMethod::Prefab)
   {
      Prefab* prefab = dynamic_cast<Prefab*>(module);
      if (prefab != nullptr)
         prefab->LoadPrefab("prefabs" + GetPathSeparator() + spawnable.mLabel);
   }

   if (spawnable.mSpawnMethod == ModuleFactory::SpawnMethod::Plugin)
   {
      VSTPlugin* plugin = dynamic_cast<VSTPlugin*>(module);
      if (plugin != nullptr)
         plugin->SetVST(spawnable.mPluginDesc);
   }

   if (spawnable.mSpawnMethod == ModuleFactory::SpawnMethod::MidiController)
   {
      MidiController* controller = dynamic_cast<MidiController*>(module);
      if (controller != nullptr)
      {
         controller->GetSaveData().SetString("devicein", spawnable.mLabel);
         controller->SetUpFromSaveDataBase();
      }
   }

   if (spawnable.mSpawnMethod == ModuleFactory::SpawnMethod::Preset)
   {
      std::string presetFilePath = ofToDataPath("presets/" + spawnable.mPresetModuleType + "/" + spawnable.mLabel);
      ModuleSaveDataPanel::LoadPreset(module, presetFilePath);
      module->SetName(GetUniqueName(juce::String(spawnable.mLabel).replace(".preset", "").toStdString(), modules).c_str());
   }

   return module;
}

void ModularSynth::SetMoveModule(IDrawableModule* module, float offsetX, float offsetY, bool canStickToCursor)
{
   mMoveModule = module;
   mMoveModuleOffsetX = offsetX;
   mMoveModuleOffsetY = offsetY;
   mMoveModuleCanStickToCursor = canStickToCursor;
}

void ModularSynth::AddMidiDevice(MidiDevice* device)
{
   if (!VectorContains(device, mMidiDevices))
      mMidiDevices.push_back(device);
}

void ModularSynth::ReconnectMidiDevices()
{
   for (int i = 0; i < mMidiDevices.size(); ++i)
      mMidiDevices[i]->Reconnect();
}

void ModularSynth::SaveOutput()
{
   ScopedMutex mutex(&mAudioThreadMutex, "SaveOutput()");

   std::string save_prefix = "recording_";
   if (!mCurrentSaveStatePath.empty())
   {
      // This assumes that mCurrentSaveStatePath always has a valid filename at the end
      std::string filename = File(mCurrentSaveStatePath).getFileNameWithoutExtension().toStdString();
      save_prefix = filename + "_";
   }

   std::string filename = ofGetTimestampString(UserPrefs.recordings_path.Get() + save_prefix + "%Y-%m-%d_%H-%M.wav");
   //string filenamePos = ofGetTimestampString("recordings/pos_%Y-%m-%d_%H-%M.wav");

   assert(mRecordingLength <= mGlobalRecordBuffer->Size());

   int channels = 2;
   auto wavFormat = std::make_unique<juce::WavAudioFormat>();
   juce::File outputFile(ofToDataPath(filename));
   outputFile.create();
   auto outputTo = outputFile.createOutputStream();
   assert(outputTo != nullptr);
   bool b1{ false };
   auto writer = std::unique_ptr<juce::AudioFormatWriter>(wavFormat->createWriterFor(outputTo.release(), gSampleRate, channels, 16, b1, 0));

   int samplesRemaining = mRecordingLength;
   const int chunkSize = 256;
   float leftChannel[chunkSize];
   float rightChannel[chunkSize];
   float* chunk[2]{ leftChannel, rightChannel };
   while (samplesRemaining > 0)
   {
      int numSamples = MIN(chunkSize, samplesRemaining);
      for (int i = 0; i < numSamples; ++i)
      {
         chunk[0][i] = mGlobalRecordBuffer->GetSample(samplesRemaining - 1, 0);
         chunk[1][i] = mGlobalRecordBuffer->GetSample(samplesRemaining - 1, 1);
         --samplesRemaining;
      }
      writer->writeFromFloatArrays(chunk, channels, numSamples);
   }

   mGlobalRecordBuffer->ClearBuffer();
   mRecordingLength = 0;

   TheTitleBar->DisplayTemporaryMessage("wrote " + filename);
}

const String& ModularSynth::GetTextFromClipboard() const
{
   return TheClipboard;
}

void ModularSynth::CopyTextToClipboard(const String& text)
{
   TheClipboard = text;
   SystemClipboard::copyTextToClipboard(text);
}

void ModularSynth::ReadClipboardTextFromSystem()
{
   TheClipboard = SystemClipboard::getTextFromClipboard();
}

void ModularSynth::SetFatalError(std::string error)
{
   if (mFatalError == "")
   {
      mFatalError = error;
      if (mUserPrefsEditor != nullptr)
         mUserPrefsEditor->Show();
      if (TheTitleBar != nullptr)
         TheTitleBar->SetShowing(false);
   }
}

void ConsoleListener::TextEntryActivated(TextEntry* entry)
{
   TheSynth->ClearConsoleInput();
}

void ConsoleListener::TextEntryComplete(TextEntry* entry)
{
   TheSynth->OnConsoleInput();
}
