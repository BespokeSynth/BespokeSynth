#include "ModularSynth.h"
#include "IAudioSource.h"
#include "IAudioEffect.h"
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
#include "PerformanceTimer.h"
#include "FileStream.h"
#include "PatchCable.h"
#include "ADSRDisplay.h"
#include <JuceHeader.h>
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

#if BESPOKE_WINDOWS
#include <Windows.h>
#include <DbgHelp.h>
#include <Winbase.h>
#endif

ModularSynth* TheSynth = nullptr;

//static
bool ModularSynth::sShouldAutosave = false;
float ModularSynth::sBackgroundLissajousR = 0.408f;
float ModularSynth::sBackgroundLissajousG = 0.245f;
float ModularSynth::sBackgroundLissajousB = 0.418f;

#if BESPOKE_WINDOWS
LONG WINAPI TopLevelExceptionHandler(PEXCEPTION_POINTERS pExceptionInfo);
#endif

void AtExit()
{
   TheSynth->Exit();
}

ModularSynth::ModularSynth()
: mMoveModule(nullptr)
, mIsMousePanning(false)
, mGlobalRecordBuffer(nullptr)
, mAudioPaused(false)
, mIsLoadingState(false)
, mClickStartX(INT_MAX)
, mClickStartY(INT_MAX)
, mWantReloadInitialLayout(false)
, mHeldSample(nullptr)
, mConsoleListener(nullptr)
, mUserPrefsEditor(nullptr)
, mLastClickedModule(nullptr)
, mInitialized(false)
, mRecordingLength(0)
, mGroupSelectContext(nullptr)
, mResizeModule(nullptr)
, mShowLoadStatePopup(false)
, mLastSaveTime(-9999)
, mHasDuplicatedDuringDrag(false)
, mHasAutopatchedToTargetDuringDrag(false)
, mFrameRate(0)
, mQuickSpawn(nullptr)
, mScheduledEnvelopeEditorSpawnDisplay(nullptr)
, mFrameCount(0)
, mIsLoadingModule(false)
, mLastClapboardTime(-9999)
, mScrollMultiplierHorizontal(1)
, mScrollMultiplierVertical(1)
, mPixelRatio(1)
{
   mConsoleText[0] = 0;
   assert(TheSynth == nullptr);
   TheSynth = this;

#if BESPOKE_WINDOWS
   SetUnhandledExceptionFilter(TopLevelExceptionHandler);
#endif
}

ModularSynth::~ModularSynth()
{
   DeleteAllModules();
   
   delete mGlobalRecordBuffer;
   delete[] mSaveOutputBuffer[0];
   delete[] mSaveOutputBuffer[1];

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
   string filename;
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
            NULL)) {

            DWORD64 displacement = 0;

            if (SymFromAddr(process, (DWORD64)stack_frame.AddrPC.Offset, &displacement, symbol))
            {
               IMAGEHLP_MODULE64 moduleInfo;
               juce::zerostruct(moduleInfo);
               moduleInfo.SizeOfStruct = sizeof(moduleInfo);

               if (::SymGetModuleInfo64(process, symbol->ModBase, &moduleInfo))
                  log.appendText(moduleInfo.ModuleName + String(": "));

               log.appendText(symbol->Name + String(" + 0x") + String::toHexString((juce::int64)displacement) + "\n");
            }

         }
         log.appendText("\n\n\n");
      }
#endif

      log.appendText("backtrace:\n");
      log.appendText(juce::SystemStats::getStackBacktrace());
      log.appendText("\n\n\n");
   }

   log.appendText("OS: " + juce::SystemStats::getOperatingSystemName()+"\n");
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
   log.appendText("build: bespoke " + JUCEApplication::getInstance()->getApplicationVersion() + " (" + string(__DATE__) + " " + string(__TIME__) + ")\n");
}

bool ModularSynth::IsReady()
{
   return gTime > 100;
}

void ModularSynth::Setup(GlobalManagers* globalManagers, juce::Component* mainComponent, juce::OpenGLContext* openGLContext)
{
   mGlobalManagers = globalManagers;
   mMainComponent = mainComponent;
   mOpenGLContext = openGLContext;
   int recordBufferLengthMinutes = 30;
   
   bool loaded = mUserPrefs.open(GetUserPrefsPath(false));
   if (loaded)
   {
      sShouldAutosave = mUserPrefs["autosave"].isNull() ? false : (mUserPrefs["autosave"].asInt() > 0);

      if (!mUserPrefs["scroll_multiplier_horizontal"].isNull())
         mScrollMultiplierHorizontal = mUserPrefs["scroll_multiplier_horizontal"].asDouble();
      if (!mUserPrefs["scroll_multiplier_vertical"].isNull())
         mScrollMultiplierVertical = mUserPrefs["scroll_multiplier_vertical"].asDouble();

      if (!mUserPrefs["record_buffer_length_minutes"].isNull())
         recordBufferLengthMinutes = mUserPrefs["record_buffer_length_minutes"].asDouble();
   }
   /*else
   {
      mFatalError = "couldn't find or load data/"+GetUserPrefsPath(true);
#if BESPOKE_MAC
      if (!juce::File(GetUserPrefsPath(false)).existsAsFile())
         mFatalError += "\nplease install to /Applications/BespokeSynth or launch via run_bespoke.command";
#endif
      LogEvent("couldn't find or load userprefs.json", kLogEventType_Error);
   }*/

   mIOBufferSize = gBufferSize;
   
   mGlobalRecordBuffer = new RollingBuffer(recordBufferLengthMinutes * 60 * gSampleRate);
   mGlobalRecordBuffer->SetNumChannels(2);
   mSaveOutputBuffer[0] = new float[mGlobalRecordBuffer->Size()];
   mSaveOutputBuffer[1] = new float[mGlobalRecordBuffer->Size()];
   
   juce::File(ofToDataPath("savestate")).createDirectory();
   juce::File(ofToDataPath("savestate/autosave")).createDirectory();
   juce::File(ofToDataPath("recordings")).createDirectory();
   juce::File(ofToDataPath("samples")).createDirectory();
   juce::File(ofToDataPath("scripts")).createDirectory();
   juce::File(ofToDataPath("internal")).createDirectory();
   
   SynthInit();

   new Transport();
   new Scale();
   TheScale->CreateUIControls();
   TheTransport->CreateUIControls();

   TheScale->Init();
   TheTransport->Init();
   
   DrumPlayer::SetUpHitDirectories();
   
   ResetLayout();
   
   mConsoleListener = new ConsoleListener();
   mConsoleEntry = new TextEntry(mConsoleListener,"console",0,20,50,mConsoleText);
   mConsoleEntry->SetRequireEnter(true);
}

void ModularSynth::LoadResources(void* nanoVG, void* fontBoundsNanoVG)
{
   gNanoVG = (NVGcontext*)nanoVG;
   gFontBoundsNanoVG = (NVGcontext*)fontBoundsNanoVG;
   LoadGlobalResources();

   if (!gFont.IsLoaded())
      mFatalError = "couldn't load font from " + gFont.GetFontPath();
}

void ModularSynth::InitIOBuffers(int inputChannelCount, int outputChannelCount)
{
   for (int i = 0; i < inputChannelCount; ++i)
      mInputBuffers.push_back(new float[gBufferSize]);
   for (int i = 0; i < outputChannelCount; ++i)
      mOutputBuffers.push_back(new float[gBufferSize]);
}


string ModularSynth::GetUserPrefsPath(bool relative)
{
   string filename = "userprefs.json";
   if (JUCEApplication::getCommandLineParameterArray().size() > 0)
   {
      juce::String specified = JUCEApplication::getCommandLineParameterArray()[0];
      if (specified.endsWith(".json"))
      {
         filename = specified.toStdString();
         if (!juce::File(ofToDataPath(filename)).existsAsFile())
             TheSynth->SetFatalError("couldn't find command-line-specified userprefs file at " + ofToDataPath(filename));
      }
   }
   
   if (relative)
      return filename;
   return ofToDataPath(filename);
}

static int sFrameCount = 0;
void ModularSynth::Poll()
{
   if (mFatalError == "")
   {
      string defaultLayout = "layouts/blank.json";
      if (!mUserPrefs["layout"].isNull())
         defaultLayout = mUserPrefs["layout"].asString();
      
      if (!mInitialized && sFrameCount > 3) //let some frames render before blocking for a load
      {
         if(!mInitialSaveStatePath.empty()) {
             LoadState(mInitialSaveStatePath);
         }else {
            LoadLayoutFromFile(ofToDataPath(defaultLayout));
         }
         mInitialized = true;
      }

      if (mWantReloadInitialLayout)
      {
         LoadLayoutFromFile(ofToDataPath(defaultLayout));
         mWantReloadInitialLayout = false;
      }
   }
   
   mZoomer.Update();
   
   if (!mIsLoadingState)
   {
      for (auto p : mExtraPollers)
         p->Poll();
      mModuleContainer.Poll();
      mUILayerModuleContainer.Poll();
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
   
   ++sFrameCount;
}

void ModularSynth::DeleteAllModules()
{
   mModuleContainer.Clear();
   
   for (int i=0; i<mDeletedModules.size(); ++i)
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
   gDrawScale = ofClamp(gDrawScale,minZoom,maxZoom);
   zoomAmount = (gDrawScale - oldDrawScale) / oldDrawScale; //find actual adjusted amount
   ofVec2f zoomCenter;
   if (fromMouse)
      zoomCenter = ofVec2f(GetMouseX(&mModuleContainer), GetMouseY(&mModuleContainer)) + GetDrawOffset();
   else
      zoomCenter = ofVec2f(ofGetWidth() / gDrawScale * .5f, ofGetHeight() / gDrawScale * .5f);
   GetDrawOffset() -= zoomCenter * zoomAmount;
   mZoomer.CancelMovement();
}

void ModularSynth::PanView(float x, float y)
{
   GetDrawOffset() += ofVec2f(x, y) / gDrawScale;
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
      if (gFont.IsLoaded())
         DrawTextNormal(mFatalError,100,100, 20);
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
   
   DrawLissajous(mGlobalRecordBuffer, 0, 0, ofGetWidth(), ofGetHeight(), sBackgroundLissajousR, sBackgroundLissajousG, sBackgroundLissajousB);
   
   if (gTime == 1 && mFatalError == "")
   {
      string loading("Bespoke is initializing audio...");
      DrawTextNormal(loading,ofGetWidth()/2-GetStringWidth(loading,30)/2,ofGetHeight()/2-6, 30);
      return;
   }
   
   if (!mInitialized && mFatalError == "")
   {
      string loading("Bespoke is loading...");
      DrawTextNormal(loading,ofGetWidth()/2-GetStringWidth(loading,30)/2,ofGetHeight()/2-6, 30);
      return;
   }
   
   ofPushMatrix();
   
   ofScale(gDrawScale,gDrawScale,gDrawScale);
   
   ofPushMatrix();
   
   ofTranslate(GetDrawOffset().x, GetDrawOffset().y);

	ofNoFill();

   TheSaveDataPanel->SetShowing(TheSaveDataPanel->GetModule());
   TheSaveDataPanel->UpdatePosition();
   
   mModuleContainer.Draw();
   mModuleContainer.DrawPatchCables(false);
   mModuleContainer.DrawUnclipped();
   
   for (int i=0; i<mLissajousDrawers.size(); ++i)
   {
      float moduleX, moduleY;
      mLissajousDrawers[i]->GetPosition(moduleX, moduleY);
      IAudioSource* source = dynamic_cast<IAudioSource*>(mLissajousDrawers[i]);
      DrawLissajous(source->GetVizBuffer(), moduleX, moduleY-240, 240, 240);
   }
   
   if (mGroupSelectContext != nullptr)
   {
      ofPushStyle();
      ofSetColor(255,255,255);
      ofRect(mClickStartX, mClickStartY, GetMouseX(&mModuleContainer)-mClickStartX, GetMouseY(&mModuleContainer)-mClickStartY);
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
      ofSetColor(255,255,255);
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
      ofSetColor(255,255,255,(1 - (gTime - mLastClapboardTime) / 100) * 255);
      ofFill();
      ofRect(0, 0, ofGetWidth(), ofGetHeight());
   }
   
   ofPopMatrix();

   ofPushMatrix();
   {
      float uiScale = mUILayerModuleContainer.GetDrawScale();
      if (uiScale < .01f)
      {
         //safety check in case anything ever make the UI inaccessible
         LogEvent("correcting UI scale", kLogEventType_Error);
         mUILayerModuleContainer.SetDrawScale(1);
         uiScale = mUILayerModuleContainer.GetDrawScale();
      }
      ofScale(uiScale, uiScale, uiScale);
      ofTranslate(mUILayerModuleContainer.GetDrawOffset().x, mUILayerModuleContainer.GetDrawOffset().y);
      
      mUILayerModuleContainer.Draw();
      mUILayerModuleContainer.DrawUnclipped();
      
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
   
   string tooltip = "";
   ModuleContainer* tooltipContainer = nullptr;
   if (HelpDisplay::sShowTooltips && !IUIControl::WasLastHoverSetViaTab())
   {
      HelpDisplay* helpDisplay = TheTitleBar->GetHelpDisplay();

      if (gHoveredUIControl && string(gHoveredUIControl->Name()) != "enabled")
      {
         tooltip = helpDisplay->GetUIControlTooltip(gHoveredUIControl);
         tooltipContainer = gHoveredUIControl->GetModuleParent()->GetOwningContainer();
      }
      else if (gHoveredModule)
      {
         if (gHoveredModule == mQuickSpawn)
         {
            string name = mQuickSpawn->GetHoveredModuleTypeName();
            ofStringReplace(name, " " + string(ModuleFactory::kEffectChainSuffix), "");   //strip this suffix if it's there
            tooltip = helpDisplay->GetModuleTooltipFromName(name);
            tooltipContainer = mQuickSpawn->GetOwningContainer();
         }
         else if (gHoveredModule == GetTopModalFocusItem() && dynamic_cast<DropdownListModal*>(gHoveredModule))
         {
            DropdownListModal* list = dynamic_cast<DropdownListModal*>(gHoveredModule);
            if (list->GetOwner()->GetModuleParent() == TheTitleBar)
            {
               string moduleTypeName = dynamic_cast<DropdownListModal*>(gHoveredModule)->GetHoveredLabel();
               ofStringReplace(moduleTypeName, " (exp.)", "");
               tooltip = helpDisplay->GetModuleTooltipFromName(moduleTypeName);
               tooltipContainer = &mUILayerModuleContainer;
            }
            else if (dynamic_cast<EffectChain*>(list->GetOwner()->GetParent()) != nullptr)
            {
               string effectName = dynamic_cast<DropdownListModal*>(gHoveredModule)->GetHoveredLabel();
               tooltip = helpDisplay->GetModuleTooltipFromName(effectName);
               tooltipContainer = list->GetModuleParent()->GetOwningContainer();
            }
         }
         else if (GetMouseY(&mModuleContainer) < gHoveredModule->GetPosition().y)  //this means we're hovering over the module's title bar
         {
            tooltip = helpDisplay->GetModuleTooltip(gHoveredModule);
            tooltipContainer = gHoveredModule->GetOwningContainer();
         }
      }
   }
   
   if (mNextDrawTooltip != "")
   {
      tooltip = mNextDrawTooltip;
      tooltipContainer = &mModuleContainer;
   }
   mNextDrawTooltip = "";
   
   if (tooltip != "" && tooltipContainer != nullptr)
   {
      ofPushMatrix();
      float scale = tooltipContainer->GetDrawScale();
      ofVec2f offset = tooltipContainer->GetDrawOffset();
      ofScale(scale, scale, scale);
      ofTranslate(offset.x, offset.y);
      
      float x = GetMouseX(tooltipContainer) + 25;
      float y = GetMouseY(tooltipContainer) + 30;
      float maxWidth = 300;

      float fontSize = 15;
      nvgFontFaceId(gNanoVG, gFont.GetFontHandle());
      nvgFontSize(gNanoVG, fontSize);
      float bounds[4];
      nvgTextBoxBounds(gNanoVG, x, y, maxWidth, tooltip.c_str(), nullptr, bounds);
      float padding = 3;
      ofRectangle rect(bounds[0]-padding, bounds[1] - padding, bounds[2] - bounds[0] + padding*2, bounds[3] - bounds[1] + padding*2);
      
      float minX = 5 - offset.x;
      float maxX = ofGetWidth() / scale  - rect.width - 5 - offset.x;
      float minY = 5 - offset.y;
      float maxY = ofGetHeight() / scale - rect.height - 5 - offset.y;
      
      float onscreenRectX = ofClamp(rect.x, minX, maxX);
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
      gFont.DrawStringWrap(tooltip, fontSize, x + (onscreenRectX - rect.x), y + (onscreenRectY - rect.y), maxWidth);
      
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
   if (!mErrors.empty())
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
   }
   
   float consoleY = TheTitleBar->GetRect().height + 15;
   
   if (IKeyboardFocusListener::GetActiveKeyboardFocus() == mConsoleEntry)
   {
      mConsoleEntry->SetPosition(0, consoleY-15);
      mConsoleEntry->Draw();
      consoleY += 17;
   }
   else
   {
      if (gHoveredUIControl != nullptr)
      {
         ofPushStyle();
         ofSetColor(0, 255, 255);
         DrawTextNormal(gHoveredUIControl->Path(), 0, consoleY-4);
         ofPopStyle();
      }
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
            ofSetColor(200, 200, 200);
         DrawTextNormal(it->text, 10, consoleY);
         vector<string> lines = ofSplitString(it->text, "\n");
         ofPopStyle();
         consoleY += 15 * lines.size();
      }

      if (!mErrors.empty())
      {
         consoleY = 0;
         ofPushStyle();
         ofSetColor(255,0,0);
         for (auto it = mErrors.begin(); it != mErrors.end(); ++it)
         {
            DrawTextNormal(*it, 600, consoleY);
            vector<string> lines = ofSplitString(*it, "\n");
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
   mSoundStream.stop();
   mModuleContainer.Exit();
   DeleteAllModules();
   ofExit();
}

IDrawableModule* ModularSynth::GetLastClickedModule() const
{
   return mLastClickedModule;
}

void ModularSynth::KeyPressed(int key, bool isRepeat)
{
   if (gHoveredUIControl &&
       IKeyboardFocusListener::GetActiveKeyboardFocus() == nullptr &&
       !isRepeat)
   {
      if (key == OF_KEY_DOWN || key == OF_KEY_UP)
      {
         float inc;
         if ((key == OF_KEY_DOWN && gHoveredUIControl->InvertScrollDirection() == false) ||
             (key == OF_KEY_UP   && gHoveredUIControl->InvertScrollDirection() == true))
            inc = -1;
         else
            inc = 1;
         if (GetKeyModifiers() & kModifier_Shift)
            inc *= .01f;
         gHoveredUIControl->Increment(inc);
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
      else if (key != ' ' && key != OF_KEY_TAB && key != '`' && key < CHAR_MAX && juce::CharacterFunctions::isPrintable((char)key))
      {
         gHoveredUIControl->AttemptTextInput();
      }
   }
   
   if (IKeyboardFocusListener::GetActiveKeyboardFocus())  //active text entry captures all input
   {
      IKeyboardFocusListener::GetActiveKeyboardFocus()->OnKeyPressed(key, isRepeat);
      return;
   }
   
   key = KeyToLower(key);  //now convert to lowercase because everything else just cares about keys as buttons (unmodified by shift)
   
   if (key == OF_KEY_BACKSPACE && !isRepeat)
   {
      for (auto module : mGroupSelectedModules)
         module->GetOwningContainer()->DeleteModule(module);
      mGroupSelectedModules.clear();
   }
   
   if (key == KeyPress::F2Key && !isRepeat)
   {
      ADSRDisplay::ToggleDisplayMode();
   }

   if (key == '`' && !isRepeat)
   {
      if (GetKeyModifiers() == kModifier_Shift)
      {
         TriggerClapboard();
      }
      else
      {
         bzero(mConsoleText, MAX_TEXTENTRY_LENGTH);
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
         IUIControl::SetNewManualHover(-1);
      else
         IUIControl::SetNewManualHover(1);
   }

   mZoomer.OnKeyPressed(key);
   
   if (key < CHAR_MAX && CharacterFunctions::isDigit((char)key) && (GetKeyModifiers() == kModifier_Alt))
   {
      int num = key - '0';
      assert(num >= 0 && num <= 9);
      gHotBindUIControl[num] = gHoveredUIControl;
   }
   
   mModuleContainer.KeyPressed(key, isRepeat);
   mUILayerModuleContainer.KeyPressed(key, isRepeat);

   if (key == '/' && !isRepeat)
      ofToggleFullscreen();
   
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
   
   mModuleContainer.KeyReleased(key);
   mUILayerModuleContainer.KeyReleased(key);
}

float ModularSynth::GetMouseX(ModuleContainer* context, float rawX /*= FLT_MAX*/)
{
   return (rawX == FLT_MAX ? mMousePos.x : rawX) / context->GetDrawScale() - context->GetDrawOffset().x;
}

float ModularSynth::GetMouseY(ModuleContainer* context, float rawY /*= FLT_MAX*/)
{
#if BESPOKE_MAC
   const float kYOffset = -4;
#else
   const float kYOffset = 0;
#endif
   return ((rawY == FLT_MAX ? mMousePos.y : rawY) + kYOffset) / context->GetDrawScale() - context->GetDrawOffset().y;
}

bool ModularSynth::IsMouseButtonHeld(int button)
{
   if (button >= 0 && button < (int)mIsMouseButtonHeld.size())
      return mIsMouseButtonHeld[button];

   return false;
}

void ModularSynth::MouseMoved(int intX, int intY )
{
   bool changed = (mMousePos.x != intX || mMousePos.y != intY);

   mMousePos.x = intX;
   mMousePos.y = intY;
   
   if (IsKeyHeld(' ') || mIsMousePanning)
   {
      GetDrawOffset() += (ofVec2f(intX,intY) - mLastMoveMouseScreenPos) / gDrawScale;
      mZoomer.CancelMovement();
   }
   
   mLastMoveMouseScreenPos = ofVec2f(intX,intY);

   if (changed)
   {
      for (auto* modal : mModalFocusItemStack)
      {
         float x = GetMouseX(modal->GetOwningContainer());
         float y = GetMouseY(modal->GetOwningContainer());
         modal->NotifyMouseMoved(x, y);
      }
   }

   if (mMoveModule)
   {
      float x = GetMouseX(&mModuleContainer);
      float y = GetMouseY(&mModuleContainer);
      
      float oldX, oldY;
      mMoveModule->GetPosition(oldX, oldY);
      mMoveModule->Move(x + mMoveModuleOffsetX - oldX, y + mMoveModuleOffsetY - oldY);
      
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
               for (auto* patchCableSource : mMoveModule->GetPatchCableSources())
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
      float x = GetMouseX(&mModuleContainer);
      float y = GetMouseY(&mModuleContainer);
      mModuleContainer.MouseMoved(x, y);
      
      x = GetMouseX(&mUILayerModuleContainer);
      y = GetMouseY(&mUILayerModuleContainer);
      mUILayerModuleContainer.MouseMoved(x, y);
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
         if (x < uiX - 10 || y < uiY - 10 || x > uiX + w + 10 || y > uiY + h + 10)
            gHoveredUIControl = nullptr;
      }
   }
   
   gHoveredModule = GetModuleAtCursor();
}

void ModularSynth::MouseDragged(int intX, int intY, int button)
{
   mMousePos.x = intX;
   mMousePos.y = intY;
   
   float x = GetMouseX(&mModuleContainer);
   float y = GetMouseY(&mModuleContainer);
   
   ofVec2f drag = ofVec2f(x,y) - mLastMouseDragPos;
   mLastMouseDragPos = ofVec2f(x,y);

   if (GetMoveModule() && (abs(mClickStartX-x) >= 1 || abs(mClickStartY-y) >= 1))
   {
      mClickStartX = INT_MAX;  //moved enough from click spot to reset
      mClickStartY = INT_MAX;
   }
   
   for (auto* modal : mModalFocusItemStack)
   {
      float x = GetMouseX(modal->GetOwningContainer());
      float y = GetMouseY(modal->GetOwningContainer());
      modal->NotifyMouseMoved(x, y);
   }
   
   if (GetKeyModifiers() == kModifier_Alt && !mHasDuplicatedDuringDrag)
   {
      vector<IDrawableModule*> newGroupSelectedModules;
      map<IDrawableModule*, IDrawableModule*> oldToNewModuleMap;
      for (auto module : mGroupSelectedModules)
      {
         if (!module->IsSingleton())
         {
            IDrawableModule* newModule = DuplicateModule(module);
            newGroupSelectedModules.push_back(newModule);
            oldToNewModuleMap[module] = newModule;
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
   
   for (auto module : mGroupSelectedModules)
      module->Move(drag.x, drag.y);

   if (mMoveModule)
   {
      mMoveModule->Move(drag.x, drag.y);
      return;
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

   mModuleContainer.MouseMoved(x, y);
   
   x = GetMouseX(&mUILayerModuleContainer);
   y = GetMouseY(&mUILayerModuleContainer);
   mUILayerModuleContainer.MouseMoved(x, y);
}

void ModularSynth::MousePressed(int intX, int intY, int button)
{
   mZoomer.ExitVanityPanningMode();
   
   if (PatchCable::sActivePatchCable != nullptr)
      return;
   
   mMousePos.x = intX;
   mMousePos.y = intY;

   if (button >= 0 && button < (int)mIsMouseButtonHeld.size())
      mIsMouseButtonHeld[button] = true;
   
   float x = GetMouseX(&mModuleContainer);
   float y = GetMouseY(&mModuleContainer);
   
   mLastMouseDragPos = ofVec2f(x,y);
   mGroupSelectContext = nullptr;
   
   bool rightButton = button == 2;

   IKeyboardFocusListener::ClearActiveKeyboardFocus(K(notifyListeners));

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
            float uiX,uiY;
            slider->GetPosition(uiX, uiY);
            float w, h;
            slider->GetDimensions(w, h);
            
            if (x < uiX || y < uiY || x > uiX + w || y > uiY + h)
               PopModalFocusItem();
         }
         else  //otherwise, always dismiss if you click outside it
         {
            PopModalFocusItem();
         }
      }
      else
      {
         return;
      }
   }

   if (InMidiMapMode())
   {
      if (gBindToUIControl == gHoveredUIControl)   //if it's the same, clear it
         gBindToUIControl = nullptr;
      else
         gBindToUIControl = gHoveredUIControl;
      return;
   }
   
   IDrawableModule* clicked = GetModuleAtCursor();
   
   for (auto cable : mPatchCables)
   {
      if (clicked &&
          (clicked == GetTopModalFocusItem() ||
           clicked->AlwaysOnTop() ||
           mModuleContainer.IsHigherThan(clicked, cable->GetOwningModule())))
         break;
      if (cable->TestClick(x,y,rightButton))
         return;
   }
   
   mClickStartX = x;
   mClickStartY = y;
   if (clicked == nullptr)
   {
      if (rightButton)
         mIsMousePanning = true;
      else
         SetGroupSelectContext(&mModuleContainer);
   }
   if (clicked != nullptr && clicked != TheTitleBar)
      mLastClickedModule = clicked;
   else
      mLastClickedModule = nullptr;
   mHasDuplicatedDuringDrag = false;
   
   if (mGroupSelectedModules.empty() == false)
   {
      if (!VectorContains(clicked, mGroupSelectedModules))
         mGroupSelectedModules.clear();
      return;
   }

   if (clicked)
   {
      x = GetMouseX(clicked->GetModuleParent()->GetOwningContainer());
      y = GetMouseY(clicked->GetModuleParent()->GetOwningContainer());
      CheckClick(clicked, x, y, rightButton);
   }
   else if (TheSaveDataPanel != nullptr)
      TheSaveDataPanel->SetModule(nullptr);
}

void ModularSynth::MouseScrolled(float x, float y, bool canZoomCanvas)
{
   x *= mScrollMultiplierHorizontal;
   y *= mScrollMultiplierVertical;

   if (IsKeyHeld(' ') || (GetModuleAtCursor() == nullptr && gHoveredUIControl == nullptr))
   {
      if (canZoomCanvas)
         ZoomView(y/50, true);
   }
   else if (gHoveredUIControl)
   {
#if JUCE_WINDOWS
      y += x / 4; //taking advantage of logitech horizontal scroll wheel
#endif

      float val = gHoveredUIControl->GetMidiValue();
      float movementScale = 3;
      FloatSlider* floatSlider = dynamic_cast<FloatSlider*>(gHoveredUIControl);
      IntSlider* intSlider = dynamic_cast<IntSlider*>(gHoveredUIControl);
      ClickButton* clickButton = dynamic_cast<ClickButton*>(gHoveredUIControl);
      if (floatSlider || intSlider)
      {
         float w,h;
         gHoveredUIControl->GetDimensions(w, h);
         movementScale = 200.0f / w;
            
         if (GetKeyModifiers() & kModifier_Shift)
            movementScale *= .01f;
      }

      if (clickButton)
         return;
         
      float change = y/100 * movementScale;
         
      if (floatSlider && floatSlider->GetModulator() && floatSlider->GetModulator()->Active() && floatSlider->GetModulator()->CanAdjustRange())
      {
         IModulator* modulator = floatSlider->GetModulator();
         float min = floatSlider->GetMin();
         float max = floatSlider->GetMax();
         float modMin = ofMap(modulator->GetMin(),min,max,0,1);
         float modMax = ofMap(modulator->GetMax(),min,max,0,1);
            
         modulator->GetMin() = ofMap(modMin - change,0,1,min,max,K(clamp));
         modulator->GetMax() = ofMap(modMax + change,0,1,min,max,K(clamp));
            
         return;
      }
         
      if (gHoveredUIControl->InvertScrollDirection())
         val -= change;
      else
         val += change;
      val = ofClamp(val, 0, 1);
      gHoveredUIControl->SetFromMidiCC(val);

      gHoveredUIControl->NotifyMouseScrolled(GetMouseX(&mModuleContainer), GetMouseY(&mModuleContainer), x, y);
   }
   else
   {
      IDrawableModule* module = GetModuleAtCursor();
      if (module)
         module->NotifyMouseScrolled(GetMouseX(&mModuleContainer), GetMouseY(&mModuleContainer), x, y);
   }
}

void ModularSynth::MouseMagnify(int intX, int intY, float scaleFactor)
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

IDrawableModule* ModularSynth::GetModuleAtCursor()
{
   float x = GetMouseX(&mUILayerModuleContainer);
   float y = GetMouseY(&mUILayerModuleContainer);
   IDrawableModule* uiLayerModule = mUILayerModuleContainer.GetModuleAt(x, y);
   if (uiLayerModule)
      return uiLayerModule;
   
   x = GetMouseX(&mModuleContainer);
   y = GetMouseY(&mModuleContainer);
   return mModuleContainer.GetModuleAt(x, y);
}

void ModularSynth::CheckClick(IDrawableModule* clickedModule, int x, int y, bool rightButton)
{
   if (clickedModule != TheTitleBar)
      MoveToFront(clickedModule);
   
   //check to see if we clicked in the move area
   float moduleX, moduleY;
   clickedModule->GetPosition(moduleX, moduleY);
   int modulePosX = x - moduleX;
   int modulePosY = y - moduleY;
   
   if (modulePosY < 0 && clickedModule != TheTitleBar && (!clickedModule->HasEnableCheckbox() || modulePosX > 20))
   {
      mMoveModule = clickedModule;
      mMoveModuleOffsetX = moduleX - x;
      mMoveModuleOffsetY = moduleY - y;
   }
   
   float parentX = 0;
   float parentY = 0;
   if (clickedModule->GetParent())
      clickedModule->GetParent()->GetPosition(parentX, parentY);
   
   //do the regular click
   clickedModule->TestClick(x - parentX,y - parentY,rightButton);
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
   
   list<PatchCable*> cablesToRemove;
   for (auto* cable : mPatchCables)
   {
      if (cable->GetOwningModule() == module)
         cablesToRemove.push_back(cable);
   }
   for (auto* cable : cablesToRemove)
      RemoveFromVector(cable, mPatchCables);
   
   RemoveFromVector(dynamic_cast<IAudioSource*>(module),mSources);
   RemoveFromVector(module,mLissajousDrawers);
   TheTransport->RemoveAudioPoller(dynamic_cast<IAudioPoller*>(module));
   //delete module; TODO(Ryan) deleting is hard... need to clear out everything with a reference to this, or switch to smart pointers
   
   if (module == TheChaosEngine)
      TheChaosEngine = nullptr;
   if (module == TheLFOController)
      TheLFOController = nullptr;
   
   mAudioThreadMutex.Unlock();
}

void ModularSynth::MouseReleased(int intX, int intY, int button)
{
   mMousePos.x = intX;
   mMousePos.y = intY;

   if (button >= 0 && button < (int)mIsMouseButtonHeld.size())
      mIsMouseButtonHeld[button] = false;
   
   float x = GetMouseX(&mModuleContainer);
   float y = GetMouseY(&mModuleContainer);
   
   if (GetTopModalFocusItem())
   {
      GetTopModalFocusItem()->MouseReleased();
   }
   
   if (mResizeModule)
      mResizeModule = nullptr;

   mModuleContainer.MouseReleased();
   mUILayerModuleContainer.MouseReleased();

   if (mMoveModule)
   {
      float moduleX, moduleY;
      mMoveModule->GetPosition(moduleX, moduleY);
      mMoveModule = nullptr;
   }
   
   if (mHeldSample)
   {
      IDrawableModule* module = GetModuleAtCursor();
      if (module)
      {
         float moduleX, moduleY;
         module->GetPosition(moduleX, moduleY);
         module->SampleDropped(x-moduleX, y-moduleY, GetHeldSample());
      }
      ClearHeldSample();
   }
   
   if (mGroupSelectContext != nullptr)
   {
      ofRectangle rect = ofRectangle(ofPoint(MIN(mClickStartX, x), MIN(mClickStartY, y)), ofPoint(MAX(mClickStartX, x), MAX(mClickStartY, y)));
      if (rect.width > 10 || rect.height > 10)
      {
         mGroupSelectContext->GetModulesWithinRect(rect, mGroupSelectedModules);
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
      mGroupSelectContext = nullptr;
   }
   
   mClickStartX = INT_MAX;
   mClickStartY = INT_MAX;

   mIsMousePanning = false;
}

void ModularSynth::AudioOut(float** output, int bufferSize, int nChannels)
{
   PROFILER(audioOut_total);
   
   static bool sFirst = true;
   if (sFirst)
   {
      FloatVectorOperations::disableDenormalisedNumberSupport();
      sFirst = false;
   }
   
   if (mAudioPaused)
   {
      for (int ch=0; ch<nChannels; ++ch)
      {
         for (int i=0; i<bufferSize; ++i)
            output[ch][i] = 0;
      }
      return;
   }
   
   ScopedMutex mutex(&mAudioThreadMutex, "audioOut()");
   
   /////////// AUDIO PROCESSING STARTS HERE /////////////
   assert(bufferSize == mIOBufferSize);
   assert(nChannels == (int)mOutputBuffers.size());
   assert(mIOBufferSize == gBufferSize);  //need to be the same for now
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
      for (int i=0; i<mSources.size(); ++i)
         mSources[i]->Process(gTime);

      //put it into speakers
      for (int i = 0; i < nChannels; ++i)
         BufferCopy(output[i], mOutputBuffers[i], gBufferSize);
   }
   
   if (gTime - mLastClapboardTime < 100)
   {
      for (int ch=0; ch<nChannels; ++ch)
      {
         for (int i=0; i<bufferSize; ++i)
         {
            float sample = sin(GetPhaseInc(440) * i) * (1 - ((gTime - mLastClapboardTime) / 100));
            output[ch][i] = sample;
         }
      }
   }
   /////////// AUDIO PROCESSING ENDS HERE /////////////
   if (nChannels >= 1)
      mGlobalRecordBuffer->WriteChunk(output[0], bufferSize, 0);
   if (nChannels >= 2)
      mGlobalRecordBuffer->WriteChunk(output[1], bufferSize, 1);
   mRecordingLength += bufferSize;
   mRecordingLength = MIN(mRecordingLength, mGlobalRecordBuffer->Size());
   
   Profiler::PrintCounters();
}

void ModularSynth::AudioIn(const float** input, int bufferSize, int nChannels)
{
   if (mAudioPaused)
      return;
   
   ScopedMutex mutex(&mAudioThreadMutex, "audioIn()");

   assert(bufferSize == mIOBufferSize);
   assert(nChannels == (int)mInputBuffers.size());
   
   for (int i=0; i<nChannels; ++i)
   {
      BufferCopy(mInputBuffers[i], input[i], bufferSize);
   }
}

float* ModularSynth::GetInputBuffer(int channel)
{
   return mInputBuffers[channel];
}

float* ModularSynth::GetOutputBuffer(int channel)
{
   return mOutputBuffers[channel];
}

void ModularSynth::TriggerClapboard()
{
   mLastClapboardTime = gTime; //for synchronizing internally recorded audio and externally recorded video
}

bool endsWith(std::string_view str, std::string_view suffix)
{
    return str.size() >= suffix.size() && 0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
}

void ModularSynth::FilesDropped(vector<string> files, int intX, int intY)
{
   if (files.size() > 0)
   {
      float x = GetMouseX(&mModuleContainer, intX);
      float y = GetMouseY(&mModuleContainer, intY);
      IDrawableModule* target = GetModuleAtCursor();

      if (files.size() == 1 && endsWith(files[0], ".bsk"))
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
   SourceDepInfo(IAudioSource* me) : mMe(me) {}
   IAudioSource* mMe;
   vector<IAudioSource*> mDeps;
};

void ModularSynth::ArrangeAudioSourceDependencies()
{
   //ofLog() << "Calculating audio source dependencies:";
   
   vector<SourceDepInfo> deps;
   for (int i=0; i<mSources.size(); ++i)
      deps.push_back(SourceDepInfo(mSources[i]));
      
   for (int i=0; i<mSources.size(); ++i)
   {
      for (int j=0; j<mSources.size(); ++j)
      {
         for (int k=0; k<mSources[i]->GetNumTargets(); ++k)
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
   
   mSources.clear();
   int loopCount = 0;
   while (deps.size() > 0 && loopCount < 1000) //stupid circular dependency detection, make better
   {
      for (int i=0; i<deps.size(); ++i)
      {
         bool hasDeps = false;
         for (int j=0; j<deps[i].mDeps.size(); ++j)
         {
            bool found = false;
            for (int k=0; k<mSources.size(); ++k)
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
            i-=1;
         }
      }
      ++loopCount;
   }
   
   if (loopCount == 1000)  //circular dependency, don't lose the rest of the sources
   {
      ofLog() << "circular dependency detected";
      for (int i=0; i<deps.size(); ++i)
         mSources.push_back(deps[i].mMe);
   }
   
   /*ofLog() << "new ordering:";
   for (int i=0; i<mSources.size(); ++i)
      ofLog() << dynamic_cast<IDrawableModule*>(mSources[i])->Name();*/
}

void ModularSynth::ResetLayout()
{
   mMainComponent->getTopLevelComponent()->setName("bespoke synth");
   mCurrentSaveStatePath = "";

   mModuleContainer.Clear();
   mUILayerModuleContainer.Clear();
   
   for (int i=0; i<mDeletedModules.size(); ++i)
      delete mDeletedModules[i];

   mDeletedModules.clear();
   mSources.clear();
   mLissajousDrawers.clear();
   mMoveModule = nullptr;
   LFOPool::Shutdown();
   IKeyboardFocusListener::ClearActiveKeyboardFocus(!K(notifyListeners));
   ScriptModule::sBackgroundTextString = "";
   
   mErrors.clear();
   
   vector<PatchCable*> cablesToDelete = mPatchCables;
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
   
   TitleBar* titleBar = new TitleBar();
   titleBar->SetPosition(0,0);
   titleBar->SetName("titlebar");
   titleBar->SetTypeName("titlebar");
   titleBar->CreateUIControls();
   titleBar->SetModuleFactory(&mModuleFactory);
   titleBar->Init();
   mUILayerModuleContainer.AddModule(titleBar);
   
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

   mUserPrefsEditor = new UserPrefsEditor();
   mUserPrefsEditor->SetName("userprefseditor");
   mUserPrefsEditor->CreateUIControls();
   mUserPrefsEditor->Init();
   mUserPrefsEditor->SetPosition(100, 250);
   mUserPrefsEditor->SetShowing(false);
   mModuleContainer.AddModule(mUserPrefsEditor);
   if (mFatalError != "")
   {
      mUserPrefsEditor->Show();
      TheTitleBar->SetShowing(false);
   }
   
   GetDrawOffset().set(0,0);
   
   float uiScale = 1;
   if (!mUserPrefs["ui_scale"].isNull())
      uiScale = mUserPrefs["ui_scale"].asDouble();
   SetUIScale(uiScale);
}

bool ModularSynth::LoadLayoutFromFile(string jsonFile, bool makeDefaultLayout /*= true*/)
{
   ofLog() << "Loading layout: " << jsonFile;
   mLoadedLayoutPath = String(jsonFile).replace(ofToDataPath("").c_str(), "").toStdString();
   
   ofxJSONElement root;
   bool loaded = root.open(jsonFile);
   
   if (!loaded)
   {
      LogEvent("Couldn't load, error parsing "+jsonFile, kLogEventType_Error);
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
      if (output1 != nullptr && output1->GetPosition().y > ofGetHeight() - 40)
      {
         float offset = ofGetHeight() - output1->GetPosition().y - 40;
         if (gain != nullptr)
            gain->SetPosition(gain->GetPosition().x, gain->GetPosition().y + offset);
         if (splitter != nullptr)
            splitter->SetPosition(splitter->GetPosition().x, splitter->GetPosition().y + offset);
         if (output1 != nullptr)
            output1->SetPosition(output1->GetPosition().x, output1->GetPosition().y + offset);
         if (output2 != nullptr)
            output2->SetPosition(output2->GetPosition().x, output2->GetPosition().y + offset);
      }

      if (output2 != nullptr && output2->GetPosition().x > ofGetWidth() - 100)
      {
         float offset = ofGetWidth() - output2->GetPosition().x - 100;
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

bool ModularSynth::LoadLayoutFromString(string jsonString)
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
   ScopedLock renderLock(mRenderLock);
   
   ResetLayout();
   
   mModuleContainer.LoadModules(json["modules"]);
   
   //timer.PrintCosts();
   
   mZoomer.LoadFromSaveData(json["zoomlocations"]);
   ArrangeAudioSourceDependencies();
}

void ModularSynth::UpdateUserPrefsLayout()
{
   //mUserPrefs["layout"] = mLoadedLayoutPath;
   //mUserPrefs.save(GetUserPrefsPath(), true);
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

   if (moduleInfo["comment_out"].asBool()) //hack since json doesn't allow comments
      return nullptr;

   string type = moduleInfo["type"].asString();
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
         LogEvent("Couldn't create unknown module type \""+type+"\"", kLogEventType_Error);
         return nullptr;
      }
      
      if (module->IsSingleton() == false)
         module->CreateUIControls();
      module->LoadBasics(moduleInfo, type);
      assert(strlen(module->Name()) > 0);
   }
   catch (UnknownModuleException& e)
   {
      LogEvent("Couldn't find referenced module \""+e.mSearchName+"\" when loading \""+moduleInfo["name"].asString()+"\"", kLogEventType_Error);
   }
   
   return module;
}

void ModularSynth::SetUpModule(IDrawableModule* module, const ofxJSONElement& moduleInfo)
{
   assert(module != nullptr);

   try
   {
      mIsLoadingModule = true;
      module->LoadLayout(moduleInfo);
      mIsLoadingModule = false;
   }
   catch (UnknownModuleException& e)
   {
      LogEvent("Couldn't find referenced module \""+e.mSearchName+"\" when setting up \""+moduleInfo["name"].asString()+"\"", kLogEventType_Error);
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

IDrawableModule* ModularSynth::FindModule(string name, bool fail)
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
   return mModuleContainer.FindModule(IClickable::sLoadContext+name, fail);
}

MidiController* ModularSynth::FindMidiController(string name, bool fail)
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
      LogEvent("Couldn't find referenced midi controller \""+name+"\"", kLogEventType_Error);
   }
   catch (WrongModuleTypeException& e)
   {
      LogEvent("\""+name+"\" is not a midicontroller", kLogEventType_Error);
   }
   
   return m;
}

IUIControl* ModularSynth::FindUIControl(string path)
{
   if (path == "")
      return nullptr;
   if (path[0] == '$')
   {
      if (Prefab::sLoadingPrefab)
         return nullptr;
      return mModuleContainer.FindUIControl(path.substr(1, path.length() - 1));
   }
   return mModuleContainer.FindUIControl(IClickable::sLoadContext+path);
}

void ModularSynth::GrabSample(ChannelBuffer* data, bool window, int numBars)
{
   delete mHeldSample;
   mHeldSample = new Sample();
   mHeldSample->Create(data);
   mHeldSample->SetNumBars(numBars);
   
   //window sample to avoid clicks
   if (window)
   {
      int length = data->BufferSize();
      const int fadeSamples = 15;
      if (length > fadeSamples * 2) //only window if there's enough space
      {
         for (int i=0; i<fadeSamples; ++i)
         {
            for (int ch=0; ch<mHeldSample->NumChannels(); ++ch)
            {
               float fade = float(i)/fadeSamples;
               mHeldSample->Data()->GetChannel(ch)[i] *= fade;
               mHeldSample->Data()->GetChannel(ch)[length-1-i] *= fade;
            }
         }
      }
   }
}

void ModularSynth::GrabSample(string filePath)
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

void ModularSynth::LogEvent(string event, LogEventType type)
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
   {
      FileStreamOut out(ofToDataPath("tmp").c_str());
      module->SaveState(out);
   }
   
   ofxJSONElement layoutData;
   module->SaveLayout(layoutData);
   vector<IDrawableModule*> allModules;
   mModuleContainer.GetAllModules(allModules);
   string newName = GetUniqueName(layoutData["name"].asString(), allModules);
   layoutData["name"] = newName;
   
   IDrawableModule* newModule = CreateModule(layoutData);
   mModuleContainer.AddModule(newModule);
   SetUpModule(newModule, layoutData);
   newModule->Init();
   
   assert(newModule);
   
   newModule->SetName(module->Name()); //temporarily rename to the same as what we duplicated, so we can load state properly
   
   {
      FileStreamIn in(ofToDataPath("tmp").c_str());
      mIsLoadingModule = true;
      newModule->LoadState(in);
      mIsLoadingModule = false;
   }
   
   newModule->SetName(newName.c_str());
   
   return newModule;
}

ofxJSONElement ModularSynth::GetLayout()
{
   ofxJSONElement root;
   
   root["modules"] = mModuleContainer.WriteModules();
   root["zoomlocations"] = mZoomer.GetSaveData();
   
   return root;
}

void ModularSynth::SaveLayout(string jsonFile, bool makeDefaultLayout /*= true*/)
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
   FileChooser chooser("Save current layout as...", File(ofToDataPath("layouts/newlayout.json")), "*.json", true, false, mMainComponent->getTopLevelComponent());
   if (chooser.browseForFileToSave(true))
      SaveLayout(chooser.getResult().getRelativePathFrom(File(ofToDataPath(""))).toStdString());
}

void ModularSynth::SaveCurrentState()
{
   if (mCurrentSaveStatePath.empty())
   {
      SaveStatePopup();
      return;
   }

   SaveState(mCurrentSaveStatePath, false);
}

void ModularSynth::SaveStatePopup()
{
   FileChooser chooser("Save current state as...", File(ofToDataPath(ofGetTimestampString("savestate/%Y-%m-%d_%H-%M.bsk"))), "*.bsk", true, false, mMainComponent->getTopLevelComponent());
   if (chooser.browseForFileToSave(true))
      SaveState(chooser.getResult().getFullPathName().toStdString(), false);
}

void ModularSynth::LoadStatePopup()
{
   mShowLoadStatePopup = true;
}

void ModularSynth::LoadStatePopupImp()
{
   FileChooser chooser("Load state", File(ofToDataPath("savestate")), "*.bsk", true, false, mMainComponent->getTopLevelComponent());
   if (chooser.browseForFileToOpen())
      LoadState(chooser.getResult().getFullPathName().toStdString());
}

void ModularSynth::SaveState(string file, bool autosave)
{
   if (!autosave)
   {
      mCurrentSaveStatePath = file;
      mLastSaveTime = gTime;
      string filename = File(mCurrentSaveStatePath).getFileName().toStdString();
      mMainComponent->getTopLevelComponent()->setName("bespoke synth - "+filename);
   }

   mAudioThreadMutex.Lock("SaveState()");
   
   FileStreamOut out(file.c_str());
   
   out << GetLayout().getRawString(true);
   mModuleContainer.SaveState(out);
   
   mAudioThreadMutex.Unlock();
}

void ModularSynth::SetInitialState(string file){
    mInitialSaveStatePath = std::move(file);
}

void ModularSynth::LoadState(string file)
{
   ofLog() << "LoadState() " << file;

   if (!juce::File(file).existsAsFile())
   {
      LogEvent("couldn't find file " + file, kLogEventType_Error);
      return;
   }

   if (mInitialized)
      TitleBar::sShowInitialHelpOverlay = false;  //don't show initial help popup
   
   mAudioThreadMutex.Lock("LoadState()");
   LockRender(true);
   mAudioPaused = true;
   mIsLoadingState = true;
   LockRender(false);
   mAudioThreadMutex.Unlock();
   
   FileStreamIn in(ofToDataPath(file).c_str());
   
   string jsonString;
   in >> jsonString;
   bool layoutLoaded = LoadLayoutFromString(jsonString);
   
   if (layoutLoaded)
   {
      mIsLoadingModule = true;
      mModuleContainer.LoadState(in);
      mIsLoadingModule = false;
      
      TheTransport->Reset();
   }
   
   mCurrentSaveStatePath = file;
   string filename = File(mCurrentSaveStatePath).getFileName().toStdString();
   mMainComponent->getTopLevelComponent()->setName("bespoke synth - " + filename);

   mAudioThreadMutex.Lock("LoadState()");
   LockRender(true);
   mAudioPaused = false;
   mIsLoadingState = false;
   LockRender(false);
   mAudioThreadMutex.Unlock();
}

IAudioReceiver* ModularSynth::FindAudioReceiver(string name, bool fail)
{
   IAudioReceiver* a = nullptr;
   
   if (name == "")
      return nullptr;
   
   try
   {
      a = dynamic_cast<IAudioReceiver*>(FindModule(name,fail));
      if (a == nullptr)
         throw WrongModuleTypeException();
   }
   catch (UnknownModuleException& e)
   {
      LogEvent("Couldn't find referenced audio receiver \""+name+"\"", kLogEventType_Error);
   }
   catch (WrongModuleTypeException& e)
   {
      LogEvent("\""+name+"\" is not an audio receiver", kLogEventType_Error);
   }
   
   return a;
}

INoteReceiver* ModularSynth::FindNoteReceiver(string name, bool fail)
{
   INoteReceiver* n = nullptr;
   
   if (name == "")
      return nullptr;
   
   try
   {
      n = dynamic_cast<INoteReceiver*>(FindModule(name,fail));
      if (n == nullptr)
         throw WrongModuleTypeException();
   }
   catch (UnknownModuleException& e)
   {
      LogEvent("Couldn't find referenced note receiver \""+name+"\"", kLogEventType_Error);
   }
   catch (WrongModuleTypeException& e)
   {
      LogEvent("\""+name+"\" is not a note receiver", kLogEventType_Error);
   }
      
   return n;
}

void ModularSynth::OnConsoleInput()
{
   vector<string> tokens = ofSplitString(mConsoleText," ",true,true);
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
         ScopedLock renderLock(mRenderLock);
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
         const vector<IDrawableModule*> modules = mModuleContainer.GetModules();
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
            SaveState(ofToDataPath("savestate/"+tokens[1]), false);
      }
      else if (tokens[0] == "loadstate")
      {
         if (tokens.size() >= 2)
            LoadState(ofToDataPath("savestate/"+tokens[1]));
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
         ofVec2f grabOffset(-40,20);
         IDrawableModule* module = SpawnModuleOnTheFly(mConsoleText, GetMouseX(&mModuleContainer) + grabOffset.x, GetMouseY(&mModuleContainer) + grabOffset.y);
         TheSynth->SetMoveModule(module, grabOffset.x, grabOffset.y);
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

IDrawableModule* ModularSynth::SpawnModuleOnTheFly(string moduleName, float x, float y, bool addToContainer)
{
   if (mInitialized)
      TitleBar::sShowInitialHelpOverlay = false;  //don't show initial help popup

   vector<string> tokens = ofSplitString(moduleName," ");
   if (tokens.size() == 0)
      return nullptr;

   if (sShouldAutosave)
      DoAutosave();

   string moduleType = tokens[0];
   
   moduleType = ModuleFactory::FixUpTypeName(moduleType);

   string vstToSetUp = "";
   if (tokens.size() > 1 && tokens[tokens.size() - 1] == ModuleFactory::kVSTSuffix)
   {
      moduleType = "vstplugin";
      for (size_t i = 0; i < tokens.size() - 1; ++i)
      {
         vstToSetUp += tokens[i];
         if (i != tokens.size() - 2)
            vstToSetUp += " ";
      }
   }

   string prefabToSetUp = "";
   if (tokens.size() > 1 && tokens[tokens.size() - 1] == ModuleFactory::kPrefabSuffix)
   {
      moduleType = "prefab";
      for (size_t i = 0; i < tokens.size() - 1; ++i)
      {
         prefabToSetUp += tokens[i];
         if (i != tokens.size() - 2)
            prefabToSetUp += " ";
      }
   }

   string midiControllerToSetUp = "";
   if (tokens.size() > 1 && tokens[tokens.size() - 1] == ModuleFactory::kMidiControllerSuffix)
   {
      moduleType = "midicontroller";
      for (size_t i = 0; i < tokens.size() - 1; ++i)
      {
         midiControllerToSetUp += tokens[i];
         if (i != tokens.size() - 2)
            midiControllerToSetUp += " ";
      }
   }

   string effectToSetUp = "";
   if (tokens.size() > 1 && tokens[tokens.size() - 1] == ModuleFactory::kEffectChainSuffix)
   {
      moduleType = "effectchain";
      for (size_t i = 0; i < tokens.size() - 1; ++i)
      {
         effectToSetUp += tokens[i];
         if (i != tokens.size() - 2)
            effectToSetUp += " ";
      }
   }

   ofxJSONElement dummy;
   dummy["type"] = moduleType;
   vector<IDrawableModule*> allModules;
   mModuleContainer.GetAllModules(allModules);
   dummy["name"] = GetUniqueName(moduleType, allModules);
   dummy["onthefly"] = true;

   if (moduleType == "effectchain")
   {
      for (int i=1; i<tokens.size(); ++i)
      {
         if (VectorContains(tokens[i],GetEffectFactory()->GetSpawnableEffects()))
         {
            ofxJSONElement effect;
            effect["type"] = tokens[i];
            dummy["effects"].append(effect);
         }
      }
   }

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

         if (vstToSetUp != "")
         {
            VSTPlugin* plugin = dynamic_cast<VSTPlugin*>(module);
            if (plugin != nullptr)
               plugin->SetVST(vstToSetUp);
         }
      }
   }
   catch (LoadingJSONException& e)
   {
      LogEvent("Error spawning \""+moduleName+"\" on the fly", kLogEventType_Warning);
   }
   catch (UnknownModuleException& e)
   {
      LogEvent("Error spawning \""+moduleName+"\" on the fly, couldn't find \""+e.mSearchName+"\"", kLogEventType_Warning);
   }

   if (prefabToSetUp != "")
   {
      Prefab* prefab = dynamic_cast<Prefab*>(module);
      if (prefab != nullptr)
         prefab->LoadPrefab("prefabs" + GetPathSeparator() + prefabToSetUp);
   }

   if (midiControllerToSetUp != "")
   {
      MidiController* controller = dynamic_cast<MidiController*>(module);
      if (controller != nullptr)
      {
         controller->GetSaveData().SetString("devicein", midiControllerToSetUp);
         controller->SetUpFromSaveData();
      }
   }

   if (effectToSetUp != "")
   {
      EffectChain* effectChain = dynamic_cast<EffectChain*>(module);
      if (effectChain != nullptr)
         effectChain->AddEffect(effectToSetUp, K(onTheFly));
   }

   return module;
}

void ModularSynth::SetMoveModule(IDrawableModule* module, float offsetX, float offsetY)
{
   mMoveModule = module;
   mMoveModuleOffsetX = offsetX;
   mMoveModuleOffsetY = offsetY;
}

void ModularSynth::AddMidiDevice(MidiDevice* device)
{
   if (!VectorContains(device, mMidiDevices))
      mMidiDevices.push_back(device);
}

void ModularSynth::ReconnectMidiDevices()
{
   for (int i=0; i<mMidiDevices.size(); ++i)
      mMidiDevices[i]->Reconnect();
}

void ModularSynth::SaveOutput()
{
   ScopedMutex mutex(&mAudioThreadMutex, "SaveOutput()");

   string recordingsPath = "recordings/";
   if (!mUserPrefs["recordings_path"].isNull())
      recordingsPath = mUserPrefs["recordings_path"].asString();
   
   string filename = ofGetTimestampString(recordingsPath + "recording_%Y-%m-%d_%H-%M.wav");
   //string filenamePos = ofGetTimestampString("recordings/pos_%Y-%m-%d_%H-%M.wav");

   assert(mRecordingLength <= mGlobalRecordBuffer->Size());
   
   for (int i=0; i<mRecordingLength; ++i)
   {
      mSaveOutputBuffer[0][i] = mGlobalRecordBuffer->GetSample((int)mRecordingLength-i-1, 0);
      mSaveOutputBuffer[1][i] = mGlobalRecordBuffer->GetSample((int)mRecordingLength-i-1, 1);
   }

   Sample::WriteDataToFile(filename.c_str(), mSaveOutputBuffer, (int)mRecordingLength, 2);
   
   //mOutputBufferMeasurePos.ReadChunk(mSaveOutputBuffer, mRecordingLength);
   //Sample::WriteDataToFile(filenamePos.c_str(), mSaveOutputBuffer, mRecordingLength, 1);
   
   mGlobalRecordBuffer->ClearBuffer();
   mRecordingLength = 0;
}

void ModularSynth::SetFatalError(string error)
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
